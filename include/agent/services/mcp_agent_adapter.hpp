/**
 * @file mcp_agent_adapter.hpp
 * @brief MCP protocol adapter for agents
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_AGENT_SERVICES_MCP_AGENT_ADAPTER_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_AGENT_SERVICES_MCP_AGENT_ADAPTER_HPP_INCLUDED

#include "../../export.hpp"

#ifdef MCP_PROTOCOL_ENABLED

#include "../core/agent_core.hpp"
#include <memory>
#include <string>
#include <functional>
#include <chrono>
#include <future>
#include <map>

// Forward declarations for MCP types to avoid including headers in this header
namespace mcp {
    class Server;
    class Client;
    namespace types {
        struct ServerCapabilities;
        struct ClientCapabilities;
        struct Tool;
        struct Resource;
        struct Prompt;
        struct CallToolResult;
        struct ReadResourceResult;
        struct GetPromptResult;
        struct ListToolsResult;
        struct ListResourcesResult;
        struct ListPromptsResult;
        struct InitializeResult;
    }
    namespace transport {
        class Transport;
    }
}

namespace nlohmann {
    class json;
}

namespace kolosal::services {

/**
 * @brief Adapter class to expose Kolosal agents as MCP servers and clients
 * 
 * This class provides:
 * - MCP server functionality to expose agent capabilities as tools/resources/prompts
 * - MCP client functionality to interact with other MCP servers
 * - Bidirectional communication between agents through MCP protocol
 * - Tool registration and invocation
 * - Resource exposure and retrieval
 * - Prompt templates and execution
 */
class KOLOSAL_SERVER_API MCPAgentAdapter {
public:
    /**
     * @brief Configuration for MCP adapter
     */
    struct MCPConfig {
        std::string server_name = "kolosal-agent";
        std::string server_version = "2.0.0";
        std::string server_instructions = "Kolosal AI Agent with MCP Protocol Support";
        bool enable_tool_streaming = true;
        bool enable_resource_templates = true;
        bool enable_prompt_templates = true;
        std::chrono::milliseconds default_timeout{30000};
    };

    /**
     * @brief Constructor
     * @param agent The agent to adapt for MCP protocol
     * @param config MCP configuration
     */
    explicit MCPAgentAdapter(std::shared_ptr<kolosal::agents::AgentCore> agent, 
                            const MCPConfig& config = MCPConfig{});
    
    ~MCPAgentAdapter();

    // Server functionality - expose agent as MCP server
    
    /**
     * @brief Start MCP server with given transport
     * @param transport Transport to use for communication
     * @return true if server started successfully
     */
    bool startServer(std::shared_ptr<mcp::transport::Transport> transport);
    
    /**
     * @brief Stop MCP server
     */
    void stopServer();
    
    /**
     * @brief Check if server is running
     * @return true if server is running
     */
    bool isServerRunning() const;

    // Client functionality - use agent as MCP client
    
    /**
     * @brief Initialize as MCP client
     * @param transport Transport to use for communication
     * @param timeout Timeout for initialization
     * @return Future with initialization result
     */
    std::future<mcp::types::InitializeResult> initializeClient(
        std::shared_ptr<mcp::transport::Transport> transport,
        std::chrono::milliseconds timeout = std::chrono::seconds(30));
    
    /**
     * @brief Disconnect client
     */
    void disconnectClient();
    
    /**
     * @brief Check if client is connected
     * @return true if client is connected
     */
    bool isClientConnected() const;

    // Client operations
    
    /**
     * @brief List available tools from connected MCP server
     * @param timeout Timeout for request
     * @return Future with list of tools
     */
    std::future<mcp::types::ListToolsResult> listRemoteTools(
        std::chrono::milliseconds timeout = std::chrono::seconds(30));
    
    /**
     * @brief Call a tool on the connected MCP server
     * @param name Tool name
     * @param params Tool parameters as JSON
     * @param timeout Timeout for request
     * @return Future with tool result
     */
    std::future<mcp::types::CallToolResult> callRemoteTool(
        const std::string& name, 
        const nlohmann::json& params,
        std::chrono::milliseconds timeout = std::chrono::seconds(30));
    
    /**
     * @brief List available resources from connected MCP server
     * @param timeout Timeout for request
     * @return Future with list of resources
     */
    std::future<mcp::types::ListResourcesResult> listRemoteResources(
        std::chrono::milliseconds timeout = std::chrono::seconds(30));
    
    /**
     * @brief Read a resource from the connected MCP server
     * @param uri Resource URI
     * @param timeout Timeout for request
     * @return Future with resource content
     */
    std::future<mcp::types::ReadResourceResult> readRemoteResource(
        const std::string& uri,
        std::chrono::milliseconds timeout = std::chrono::seconds(30));
    
    /**
     * @brief List available prompts from connected MCP server
     * @param timeout Timeout for request
     * @return Future with list of prompts
     */
    std::future<mcp::types::ListPromptsResult> listRemotePrompts(
        std::chrono::milliseconds timeout = std::chrono::seconds(30));
    
    /**
     * @brief Get a prompt from the connected MCP server
     * @param name Prompt name
     * @param args Prompt arguments as JSON
     * @param timeout Timeout for request
     * @return Future with prompt content
     */
    std::future<mcp::types::GetPromptResult> getRemotePrompt(
        const std::string& name,
        const nlohmann::json& args,
        std::chrono::milliseconds timeout = std::chrono::seconds(30));

    // Agent function registration as MCP tools
    
    /**
     * @brief Register agent function as MCP tool
     * @param function_name Name of the agent function
     * @param tool_name MCP tool name (if different from function name)
     * @param description Tool description
     * @param input_schema JSON schema for tool input (optional)
     * @return true if registered successfully
     */
    bool registerAgentFunctionAsTool(
        const std::string& function_name,
        const std::string& tool_name = "",
        const std::string& description = "",
        const std::optional<nlohmann::json>& input_schema = std::nullopt);
    
    /**
     * @brief Unregister MCP tool
     * @param tool_name Tool name to unregister
     * @return true if unregistered successfully
     */
    bool unregisterTool(const std::string& tool_name);

    // Agent memory/data as MCP resources
    
    /**
     * @brief Register agent memory as MCP resource
     * @param memory_type Type of memory to expose
     * @param resource_uri Resource URI
     * @param name Resource name
     * @param description Resource description
     * @return true if registered successfully
     */
    bool registerAgentMemoryAsResource(
        const std::string& memory_type,
        const std::string& resource_uri,
        const std::string& name,
        const std::string& description = "");
    
    /**
     * @brief Unregister MCP resource
     * @param resource_uri Resource URI to unregister
     * @return true if unregistered successfully
     */
    bool unregisterResource(const std::string& resource_uri);

    // Agent capabilities as MCP prompts
    
    /**
     * @brief Register agent capability as MCP prompt template
     * @param capability_name Name of the agent capability
     * @param prompt_name MCP prompt name
     * @param description Prompt description
     * @param arguments Prompt arguments definition
     * @return true if registered successfully
     */
    bool registerAgentCapabilityAsPrompt(
        const std::string& capability_name,
        const std::string& prompt_name,
        const std::string& description,
        const std::vector<std::pair<std::string, std::string>>& arguments = {});
    
    /**
     * @brief Unregister MCP prompt
     * @param prompt_name Prompt name to unregister
     * @return true if unregistered successfully
     */
    bool unregisterPrompt(const std::string& prompt_name);

    // Configuration and status
    
    /**
     * @brief Get current MCP configuration
     * @return Current configuration
     */
    const MCPConfig& getConfig() const { return config_; }
    
    /**
     * @brief Update MCP configuration
     * @param config New configuration
     */
    void updateConfig(const MCPConfig& config);
    
    /**
     * @brief Get server capabilities
     * @return Server capabilities
     */
    mcp::types::ServerCapabilities getServerCapabilities() const;
    
    /**
     * @brief Get client capabilities  
     * @return Client capabilities
     */
    mcp::types::ClientCapabilities getClientCapabilities() const;

    // Auto-registration helpers
    
    /**
     * @brief Auto-register all agent functions as MCP tools
     * @param include_builtin Whether to include built-in functions
     * @return Number of functions registered
     */
    size_t autoRegisterAgentFunctions(bool include_builtin = false);
    
    /**
     * @brief Auto-register agent memory types as MCP resources
     * @return Number of resources registered
     */
    size_t autoRegisterAgentMemory();
    
    /**
     * @brief Auto-register agent capabilities as MCP prompts
     * @return Number of prompts registered
     */
    size_t autoRegisterAgentCapabilities();

    // Event handling
    using EventCallback = std::function<void(const std::string&, const nlohmann::json&)>;
    
    /**
     * @brief Register event callback
     * @param event_type Event type to listen for
     * @param callback Callback function
     */
    void registerEventCallback(const std::string& event_type, EventCallback callback);
    
    /**
     * @brief Unregister event callback
     * @param event_type Event type to stop listening for
     */
    void unregisterEventCallback(const std::string& event_type);

private:
    std::shared_ptr<kolosal::agents::AgentCore> agent_;
    MCPConfig config_;
    
    // MCP server and client instances
    std::unique_ptr<mcp::Server> mcp_server_;
    std::unique_ptr<mcp::Client> mcp_client_;
    
    // Registration tracking
    std::map<std::string, std::string> tool_to_function_map_;
    std::map<std::string, std::string> resource_to_memory_map_;
    std::map<std::string, std::string> prompt_to_capability_map_;
    
    // Event callbacks
    std::map<std::string, EventCallback> event_callbacks_;
    
    // Internal state
    mutable std::mutex state_mutex_;
    bool server_running_ = false;
    bool client_connected_ = false;

    // Internal helper methods
    void setupServerCapabilities();
    void setupClientCapabilities();
    nlohmann::json convertAgentDataToJson(const kolosal::agents::AgentData& data);
    kolosal::agents::AgentData convertJsonToAgentData(const nlohmann::json& json);
    void handleServerError(const std::string& operation, const std::exception& e);
    void handleClientError(const std::string& operation, const std::exception& e);
    void notifyEvent(const std::string& event_type, const nlohmann::json& data);
    
    // Tool/Resource/Prompt handlers
    nlohmann::json handleToolCall(const std::string& tool_name, const nlohmann::json& params);
    mcp::types::ReadResourceResult handleResourceRead(const std::string& resource_uri);
    std::string handlePromptRequest(const std::string& prompt_name, const nlohmann::json& args);
};

} // namespace kolosal::services

#endif // MCP_PROTOCOL_ENABLED

#endif // KOLOSAL_AGENT_INCLUDE_AGENT_SERVICES_MCP_AGENT_ADAPTER_HPP_INCLUDED
