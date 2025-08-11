/**
 * @file mcp_agent_adapter.cpp
 * @brief Implementation of MCP protocol adapter for agents
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "agent/services/mcp_agent_adapter.hpp"

#ifdef MCP_PROTOCOL_ENABLED

#include "logger/logger_system.hpp"

// MCP includes
#include "mcp/server.hpp"
#include "mcp/client.hpp"
#include "mcp/types.hpp"
#include "mcp/transport/transport.hpp"
#include "mcp/utils/error.hpp"
#include "mcp/utils/logging.hpp"

// JSON handling
#include <nlohmann/json.hpp>

#include <stdexcept>
#include <sstream>

using json = nlohmann::json;

namespace kolosal::services {

MCPAgentAdapter::MCPAgentAdapter(std::shared_ptr<kolosal::agents::AgentCore> agent, 
                                const MCPConfig& config)
    : agent_(std::move(agent)), config_(config) {
    
    if (!agent_) {
        throw std::invalid_argument("Agent cannot be null");
    }
    
    // Initialize MCP logging level based on our logger
    if (auto logger = agent_->get__logger()) {
        // Set MCP logging to match our logging level
        mcp::utils::setLogLevel(mcp::utils::LogLevel::Info);
    }
    
    setupServerCapabilities();
    setupClientCapabilities();
}

MCPAgentAdapter::~MCPAgentAdapter() {
    try {
        stopServer();
        disconnectClient();
    } catch (const std::exception& e) {
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::ERROR, "Error in MCPAgentAdapter destructor: " + std::string(e.what()));
        }
    }
}

// Server functionality implementation

bool MCPAgentAdapter::startServer(std::shared_ptr<mcp::transport::Transport> transport) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (server_running_) {
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::WARNING, "MCP server already running for agent: " + agent_->get__agent_name());
        }
        return true;
    }
    
    try {
        // Create MCP server
        mcp_server_ = std::make_unique<mcp::Server>(
            config_.server_name + "-" + agent_->get__agent_name(),
            config_.server_version,
            config_.server_instructions
        );
        
        // Set server capabilities
        auto capabilities = getServerCapabilities();
        mcp_server_->setCapabilities(capabilities);
        
        // Start the server with the transport
        mcp_server_->run(transport);
        
        server_running_ = true;
        
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::INFO, "MCP server started for agent: " + agent_->get__agent_name());
        }
        
        notifyEvent("server_started", json{{"agent_id", agent_->get__agent_id()}});
        
        return true;
        
    } catch (const std::exception& e) {
        handleServerError("startServer", e);
        return false;
    }
}

void MCPAgentAdapter::stopServer() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!server_running_) {
        return;
    }
    
    try {
        if (mcp_server_) {
            mcp_server_->stop();
            mcp_server_.reset();
        }
        
        server_running_ = false;
        
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::INFO, "MCP server stopped for agent: " + agent_->get__agent_name());
        }
        
        notifyEvent("server_stopped", json{{"agent_id", agent_->get__agent_id()}});
        
    } catch (const std::exception& e) {
        handleServerError("stopServer", e);
    }
}

bool MCPAgentAdapter::isServerRunning() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return server_running_;
}

// Client functionality implementation

std::future<mcp::types::InitializeResult> MCPAgentAdapter::initializeClient(
    std::shared_ptr<mcp::transport::Transport> transport,
    std::chrono::milliseconds timeout) {
    
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    try {
        // Create MCP client
        mcp_client_ = std::make_unique<mcp::Client>(
            config_.server_name + "-client-" + agent_->get__agent_name(),
            config_.server_version
        );
        
        // Set client capabilities
        auto capabilities = getClientCapabilities();
        mcp_client_->setCapabilities(capabilities);
        
        // Initialize connection
        auto future = mcp_client_->initialize(transport, timeout);
        
        // Set connected flag when future completes successfully
        auto shared_future = std::shared_future<mcp::types::InitializeResult>(std::move(future));
        
        // Start a task to update connection status when initialization completes
        std::thread([this, shared_future]() {
            try {
                auto result = shared_future.get();
                std::lock_guard<std::mutex> lock(state_mutex_);
                client_connected_ = true;
                
                if (auto logger = agent_->get__logger()) {
                    logger->log(kolosal::LogLevel::INFO, "MCP client connected for agent: " + agent_->get__agent_name());
                }
                
                notifyEvent("client_connected", json{{"agent_id", agent_->get__agent_id()}});
                
            } catch (const std::exception& e) {
                handleClientError("initializeClient", e);
                std::lock_guard<std::mutex> lock(state_mutex_);
                client_connected_ = false;
            }
        }).detach();
        
        return shared_future;
        
    } catch (const std::exception& e) {
        handleClientError("initializeClient", e);
        
        // Return a future with an exception
        std::promise<mcp::types::InitializeResult> promise;
        promise.set_exception(std::current_exception());
        return promise.get_future();
    }
}

void MCPAgentAdapter::disconnectClient() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!client_connected_) {
        return;
    }
    
    try {
        if (mcp_client_) {
            mcp_client_->disconnect();
            mcp_client_.reset();
        }
        
        client_connected_ = false;
        
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::INFO, "MCP client disconnected for agent: " + agent_->get__agent_name());
        }
        
        notifyEvent("client_disconnected", json{{"agent_id", agent_->get__agent_id()}});
        
    } catch (const std::exception& e) {
        handleClientError("disconnectClient", e);
    }
}

bool MCPAgentAdapter::isClientConnected() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return client_connected_ && mcp_client_ && mcp_client_->isConnected();
}

// Client operations implementation

std::future<mcp::types::ListToolsResult> MCPAgentAdapter::listRemoteTools(
    std::chrono::milliseconds timeout) {
    
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!isClientConnected()) {
        std::promise<mcp::types::ListToolsResult> promise;
        promise.set_exception(std::make_exception_ptr(
            std::runtime_error("MCP client not connected")));
        return promise.get_future();
    }
    
    return mcp_client_->listTools(timeout);
}

std::future<mcp::types::CallToolResult> MCPAgentAdapter::callRemoteTool(
    const std::string& name, 
    const json& params,
    std::chrono::milliseconds timeout) {
    
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!isClientConnected()) {
        std::promise<mcp::types::CallToolResult> promise;
        promise.set_exception(std::make_exception_ptr(
            std::runtime_error("MCP client not connected")));
        return promise.get_future();
    }
    
    return mcp_client_->callTool(name, params, timeout);
}

std::future<mcp::types::ListResourcesResult> MCPAgentAdapter::listRemoteResources(
    std::chrono::milliseconds timeout) {
    
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!isClientConnected()) {
        std::promise<mcp::types::ListResourcesResult> promise;
        promise.set_exception(std::make_exception_ptr(
            std::runtime_error("MCP client not connected")));
        return promise.get_future();
    }
    
    return mcp_client_->listResources(timeout);
}

std::future<mcp::types::ReadResourceResult> MCPAgentAdapter::readRemoteResource(
    const std::string& uri,
    std::chrono::milliseconds timeout) {
    
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!isClientConnected()) {
        std::promise<mcp::types::ReadResourceResult> promise;
        promise.set_exception(std::make_exception_ptr(
            std::runtime_error("MCP client not connected")));
        return promise.get_future();
    }
    
    return mcp_client_->readResource(uri, timeout);
}

std::future<mcp::types::ListPromptsResult> MCPAgentAdapter::listRemotePrompts(
    std::chrono::milliseconds timeout) {
    
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!isClientConnected()) {
        std::promise<mcp::types::ListPromptsResult> promise;
        promise.set_exception(std::make_exception_ptr(
            std::runtime_error("MCP client not connected")));
        return promise.get_future();
    }
    
    return mcp_client_->listPrompts(timeout);
}

std::future<mcp::types::GetPromptResult> MCPAgentAdapter::getRemotePrompt(
    const std::string& name,
    const json& args,
    std::chrono::milliseconds timeout) {
    
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!isClientConnected()) {
        std::promise<mcp::types::GetPromptResult> promise;
        promise.set_exception(std::make_exception_ptr(
            std::runtime_error("MCP client not connected")));
        return promise.get_future();
    }
    
    return mcp_client_->getPrompt(name, args, timeout);
}

// Registration implementation

bool MCPAgentAdapter::registerAgentFunctionAsTool(
    const std::string& function_name,
    const std::string& tool_name,
    const std::string& description,
    const std::optional<json>& input_schema) {
    
    if (!mcp_server_) {
        return false;
    }
    
    try {
        std::string actual_tool_name = tool_name.empty() ? function_name : tool_name;
        
        // Register with MCP server
        mcp_server_->registerTool(
            actual_tool_name,
            description.empty() ? ("Agent function: " + function_name) : description,
            input_schema,
            [this, function_name](const json& params) -> json {
                return handleToolCall(function_name, params);
            }
        );
        
        // Track the mapping
        tool_to_function_map_[actual_tool_name] = function_name;
        
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::INFO, 
                       "Registered agent function '" + function_name + 
                       "' as MCP tool '" + actual_tool_name + "'");
        }
        
        notifyEvent("tool_registered", json{
            {"tool_name", actual_tool_name},
            {"function_name", function_name},
            {"agent_id", agent_->get__agent_id()}
        });
        
        return true;
        
    } catch (const std::exception& e) {
        handleServerError("registerAgentFunctionAsTool", e);
        return false;
    }
}

bool MCPAgentAdapter::unregisterTool(const std::string& tool_name) {
    // MCP library doesn't provide unregister functionality in the current API
    // We track it for future cleanup
    tool_to_function_map_.erase(tool_name);
    
    if (auto logger = agent_->get__logger()) {
        logger->log(kolosal::LogLevel::INFO, "Unregistered MCP tool: " + tool_name);
    }
    
    notifyEvent("tool_unregistered", json{
        {"tool_name", tool_name},
        {"agent_id", agent_->get__agent_id()}
    });
    
    return true;
}

bool MCPAgentAdapter::registerAgentMemoryAsResource(
    const std::string& memory_type,
    const std::string& resource_uri,
    const std::string& name,
    const std::string& description) {
    
    if (!mcp_server_) {
        return false;
    }
    
    try {
        mcp_server_->registerResource(
            resource_uri,
            name,
            description.empty() ? ("Agent memory: " + memory_type) : description,
            [this, memory_type](const std::string& uri) -> mcp::types::ReadResourceResult {
                return handleResourceRead(uri);
            }
        );
        
        // Track the mapping
        resource_to_memory_map_[resource_uri] = memory_type;
        
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::INFO, 
                       "Registered agent memory '" + memory_type + 
                       "' as MCP resource '" + resource_uri + "'");
        }
        
        notifyEvent("resource_registered", json{
            {"resource_uri", resource_uri},
            {"memory_type", memory_type},
            {"agent_id", agent_->get__agent_id()}
        });
        
        return true;
        
    } catch (const std::exception& e) {
        handleServerError("registerAgentMemoryAsResource", e);
        return false;
    }
}

bool MCPAgentAdapter::unregisterResource(const std::string& resource_uri) {
    // Track removal for future cleanup
    resource_to_memory_map_.erase(resource_uri);
    
    if (auto logger = agent_->get__logger()) {
        logger->log(kolosal::LogLevel::INFO, "Unregistered MCP resource: " + resource_uri);
    }
    
    notifyEvent("resource_unregistered", json{
        {"resource_uri", resource_uri},
        {"agent_id", agent_->get__agent_id()}
    });
    
    return true;
}

bool MCPAgentAdapter::registerAgentCapabilityAsPrompt(
    const std::string& capability_name,
    const std::string& prompt_name,
    const std::string& description,
    const std::vector<std::pair<std::string, std::string>>& arguments) {
    
    if (!mcp_server_) {
        return false;
    }
    
    try {
        // Convert arguments to MCP format
        std::vector<mcp::types::PromptArgument> mcp_args;
        for (const auto& [name, desc] : arguments) {
            mcp::types::PromptArgument arg;
            arg.name = name;
            arg.description = desc;
            arg.required = true; // Default to required
            mcp_args.push_back(arg);
        }
        
        mcp_server_->registerPrompt(
            prompt_name,
            description.empty() ? ("Agent capability: " + capability_name) : description,
            mcp_args,
            [this, capability_name](const json& args) -> std::string {
                return handlePromptRequest(capability_name, args);
            }
        );
        
        // Track the mapping
        prompt_to_capability_map_[prompt_name] = capability_name;
        
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::INFO, 
                       "Registered agent capability '" + capability_name + 
                       "' as MCP prompt '" + prompt_name + "'");
        }
        
        notifyEvent("prompt_registered", json{
            {"prompt_name", prompt_name},
            {"capability_name", capability_name},
            {"agent_id", agent_->get__agent_id()}
        });
        
        return true;
        
    } catch (const std::exception& e) {
        handleServerError("registerAgentCapabilityAsPrompt", e);
        return false;
    }
}

bool MCPAgentAdapter::unregisterPrompt(const std::string& prompt_name) {
    // Track removal for future cleanup
    prompt_to_capability_map_.erase(prompt_name);
    
    if (auto logger = agent_->get__logger()) {
        logger->log(kolosal::LogLevel::INFO, "Unregistered MCP prompt: " + prompt_name);
    }
    
    notifyEvent("prompt_unregistered", json{
        {"prompt_name", prompt_name},
        {"agent_id", agent_->get__agent_id()}
    });
    
    return true;
}

// Configuration and capabilities

void MCPAgentAdapter::updateConfig(const MCPConfig& config) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    config_ = config;
    
    if (auto logger = agent_->get__logger()) {
        logger->log(kolosal::LogLevel::INFO, "Updated MCP configuration for agent: " + agent_->get__agent_name());
    }
}

mcp::types::ServerCapabilities MCPAgentAdapter::getServerCapabilities() const {
    mcp::types::ServerCapabilities capabilities;
    
    // Tool capabilities
    mcp::types::ToolCapabilities tool_capabilities;
    tool_capabilities.supports_streaming = config_.enable_tool_streaming;
    capabilities.tools = tool_capabilities;
    
    // Resource capabilities
    mcp::types::ResourceCapabilities resource_capabilities;
    resource_capabilities.supports_templates = config_.enable_resource_templates;
    capabilities.resources = resource_capabilities;
    
    // Prompt capabilities
    mcp::types::PromptCapabilities prompt_capabilities;
    prompt_capabilities.supports_templates = config_.enable_prompt_templates;
    capabilities.prompts = prompt_capabilities;
    
    return capabilities;
}

mcp::types::ClientCapabilities MCPAgentAdapter::getClientCapabilities() const {
    mcp::types::ClientCapabilities capabilities;
    
    // Tool capabilities
    mcp::types::ToolCapabilities tool_capabilities;
    tool_capabilities.supports_streaming = config_.enable_tool_streaming;
    capabilities.tools = tool_capabilities;
    
    return capabilities;
}

// Auto-registration helpers

size_t MCPAgentAdapter::autoRegisterAgentFunctions(bool include_builtin) {
    size_t registered_count = 0;
    
    try {
        // Get available tools from the agent's tool registry
        if (auto tool_registry = agent_->get__tool_registry()) {
            auto tools = agent_->discover_tools();
            
            for (const auto& tool_name : tools) {
                try {
                    auto schema = agent_->get__tool_schema(tool_name);
                    
                    // Skip builtin functions if not requested
                    if (!include_builtin && tool_name.find("builtin_") == 0) {
                        continue;
                    }
                    
                    json input_schema;
                    input_schema["type"] = "object";
                    input_schema["properties"] = json::object();
                    
                    if (registerAgentFunctionAsTool(tool_name, "", "", input_schema)) {
                        registered_count++;
                    }
                    
                } catch (const std::exception& e) {
                    if (auto logger = agent_->get__logger()) {
                        logger->log(kolosal::LogLevel::WARNING, 
                                   "Failed to auto-register tool '" + tool_name + "': " + e.what());
                    }
                }
            }
        }
        
    } catch (const std::exception& e) {
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::ERROR, "Error in autoRegisterAgentFunctions: " + std::string(e.what()));
        }
    }
    
    if (auto logger = agent_->get__logger()) {
        logger->log(kolosal::LogLevel::INFO, 
                   "Auto-registered " + std::to_string(registered_count) + 
                   " agent functions as MCP tools");
    }
    
    return registered_count;
}

size_t MCPAgentAdapter::autoRegisterAgentMemory() {
    size_t registered_count = 0;
    
    try {
        // Register different memory types as resources
        std::vector<std::string> memory_types = {
            "general", "episodic", "semantic", "working", "long_term"
        };
        
        for (const auto& memory_type : memory_types) {
            std::string resource_uri = "kolosal://agent/" + agent_->get__agent_id() + "/memory/" + memory_type;
            std::string resource_name = agent_->get__agent_name() + " " + memory_type + " memory";
            std::string description = "Access to " + memory_type + " memory of agent " + agent_->get__agent_name();
            
            if (registerAgentMemoryAsResource(memory_type, resource_uri, resource_name, description)) {
                registered_count++;
            }
        }
        
    } catch (const std::exception& e) {
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::ERROR, "Error in autoRegisterAgentMemory: " + std::string(e.what()));
        }
    }
    
    if (auto logger = agent_->get__logger()) {
        logger->log(kolosal::LogLevel::INFO, 
                   "Auto-registered " + std::to_string(registered_count) + 
                   " agent memory types as MCP resources");
    }
    
    return registered_count;
}

size_t MCPAgentAdapter::autoRegisterAgentCapabilities() {
    size_t registered_count = 0;
    
    try {
        // Register agent capabilities as prompts
        auto capabilities = agent_->get__capabilities();
        
        for (const auto& capability : capabilities) {
            std::string prompt_name = "agent_" + capability;
            std::string description = "Execute " + capability + " capability of agent " + agent_->get__agent_name();
            
            std::vector<std::pair<std::string, std::string>> arguments = {
                {"context", "Context or input for the capability"},
                {"parameters", "Additional parameters for the capability"}
            };
            
            if (registerAgentCapabilityAsPrompt(capability, prompt_name, description, arguments)) {
                registered_count++;
            }
        }
        
    } catch (const std::exception& e) {
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::ERROR, "Error in autoRegisterAgentCapabilities: " + std::string(e.what()));
        }
    }
    
    if (auto logger = agent_->get__logger()) {
        logger->log(kolosal::LogLevel::INFO, 
                   "Auto-registered " + std::to_string(registered_count) + 
                   " agent capabilities as MCP prompts");
    }
    
    return registered_count;
}

// Event handling

void MCPAgentAdapter::registerEventCallback(const std::string& event_type, EventCallback callback) {
    event_callbacks_[event_type] = callback;
}

void MCPAgentAdapter::unregisterEventCallback(const std::string& event_type) {
    event_callbacks_.erase(event_type);
}

// Private helper methods

void MCPAgentAdapter::setupServerCapabilities() {
    // Server capabilities are set up when the server is created
}

void MCPAgentAdapter::setupClientCapabilities() {
    // Client capabilities are set up when the client is created
}

json MCPAgentAdapter::convertAgentDataToJson(const kolosal::agents::AgentData& data) {
    json result;
    
    // Convert AgentData to JSON
    // This is a simplified conversion - you may need to expand this based on AgentData structure
    try {
        // Assuming AgentData has a way to serialize to JSON or string
        // You'll need to implement this based on the actual AgentData structure
        result = json::object();
        // Add conversion logic here
        
    } catch (const std::exception& e) {
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::WARNING, "Failed to convert AgentData to JSON: " + std::string(e.what()));
        }
        result = json::object();
    }
    
    return result;
}

kolosal::agents::AgentData MCPAgentAdapter::convertJsonToAgentData(const json& json_data) {
    kolosal::agents::AgentData result;
    
    // Convert JSON to AgentData
    // This is a simplified conversion - you may need to expand this based on AgentData structure
    try {
        // Add conversion logic here based on the actual AgentData structure
        
    } catch (const std::exception& e) {
        if (auto logger = agent_->get__logger()) {
            logger->log(kolosal::LogLevel::WARNING, "Failed to convert JSON to AgentData: " + std::string(e.what()));
        }
    }
    
    return result;
}

void MCPAgentAdapter::handleServerError(const std::string& operation, const std::exception& e) {
    if (auto logger = agent_->get__logger()) {
        logger->log(kolosal::LogLevel::ERROR, 
                   "MCP Server error in " + operation + ": " + e.what());
    }
    
    notifyEvent("server_error", json{
        {"operation", operation},
        {"error", e.what()},
        {"agent_id", agent_->get__agent_id()}
    });
}

void MCPAgentAdapter::handleClientError(const std::string& operation, const std::exception& e) {
    if (auto logger = agent_->get__logger()) {
        logger->log(kolosal::LogLevel::ERROR, 
                   "MCP Client error in " + operation + ": " + e.what());
    }
    
    notifyEvent("client_error", json{
        {"operation", operation},
        {"error", e.what()},
        {"agent_id", agent_->get__agent_id()}
    });
}

void MCPAgentAdapter::notifyEvent(const std::string& event_type, const json& data) {
    auto it = event_callbacks_.find(event_type);
    if (it != event_callbacks_.end() && it->second) {
        try {
            it->second(event_type, data);
        } catch (const std::exception& e) {
            if (auto logger = agent_->get__logger()) {
                logger->log(kolosal::LogLevel::WARNING, 
                           "Error in event callback for " + event_type + ": " + e.what());
            }
        }
    }
}

// Tool/Resource/Prompt handlers

json MCPAgentAdapter::handleToolCall(const std::string& tool_name, const json& params) {
    try {
        // Find the mapped function name
        auto it = tool_to_function_map_.find(tool_name);
        if (it == tool_to_function_map_.end()) {
            throw std::runtime_error("Tool not found: " + tool_name);
        }
        
        const std::string& function_name = it->second;
        
        // Convert JSON parameters to AgentData
        kolosal::agents::AgentData agent_params = convertJsonToAgentData(params);
        
        // Execute the agent function
        auto result = agent_->execute_function(function_name, agent_params);
        
        // Convert result back to JSON
        json json_result;
        json_result["success"] = result.success;
        json_result["message"] = result.message;
        json_result["execution_time_ms"] = result.execution_time_ms;
        
        if (result.result_data.has_value()) {
            // Convert result data to JSON if present
            json_result["data"] = convertAgentDataToJson(std::any_cast<kolosal::agents::AgentData>(result.result_data));
        }
        
        return json_result;
        
    } catch (const std::exception& e) {
        json error_result;
        error_result["success"] = false;
        error_result["error"] = e.what();
        return error_result;
    }
}

mcp::types::ReadResourceResult MCPAgentAdapter::handleResourceRead(const std::string& resource_uri) {
    mcp::types::ReadResourceResult result;
    
    try {
        // Find the mapped memory type
        auto it = resource_to_memory_map_.find(resource_uri);
        if (it == resource_to_memory_map_.end()) {
            throw std::runtime_error("Resource not found: " + resource_uri);
        }
        
        const std::string& memory_type = it->second;
        
        // Query agent memory
        auto memories = agent_->recall_memories("", 10); // Get recent memories
        
        // Convert memories to resource content
        json content = json::array();
        for (const auto& memory : memories) {
            json memory_entry;
            memory_entry["content"] = memory.content;
            memory_entry["type"] = memory.type;
            memory_entry["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                memory.timestamp.time_since_epoch()).count();
            content.push_back(memory_entry);
        }
        
        // Create resource content
        mcp::types::ResourceContent resource_content;
        resource_content.uri = resource_uri;
        resource_content.mimeType = "application/json";
        resource_content.text = content.dump(2);
        
        result.contents.push_back(resource_content);
        
    } catch (const std::exception& e) {
        // Create error content
        mcp::types::ResourceContent error_content;
        error_content.uri = resource_uri;
        error_content.mimeType = "text/plain";
        error_content.text = "Error reading resource: " + std::string(e.what());
        result.contents.push_back(error_content);
    }
    
    return result;
}

std::string MCPAgentAdapter::handlePromptRequest(const std::string& prompt_name, const json& args) {
    try {
        // Find the mapped capability name
        auto it = prompt_to_capability_map_.find(prompt_name);
        if (it == prompt_to_capability_map_.end()) {
            throw std::runtime_error("Prompt not found: " + prompt_name);
        }
        
        const std::string& capability_name = it->second;
        
        // Extract context and parameters from arguments
        std::string context = args.contains("context") ? args["context"].get<std::string>() : "";
        std::string parameters = args.contains("parameters") ? args["parameters"].dump() : "{}";
        
        // Use agent's reasoning capability
        std::string response = agent_->reason_about(
            "Execute capability: " + capability_name + " with context: " + context,
            parameters
        );
        
        return response;
        
    } catch (const std::exception& e) {
        return "Error executing prompt: " + std::string(e.what());
    }
}

} // namespace kolosal::services

#endif // MCP_PROTOCOL_ENABLED
