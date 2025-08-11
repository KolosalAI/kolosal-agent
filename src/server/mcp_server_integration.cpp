/**
 * @file mcp_server_integration.cpp
 * @brief Implementation of integration between Kolosal server and MCP protocol
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "server/mcp_server_integration.hpp"

#ifdef MCP_PROTOCOL_ENABLED

#include "logger/logger_system.hpp"

// MCP includes
#include "mcp/transport/stdio_transport.hpp"
#include "mcp/transport/http_sse_transport.hpp"
#include "mcp/utils/logging.hpp"

// JSON handling
#include <nlohmann/json.hpp>

#include <algorithm>
#include <random>
#include <sstream>

using json = nlohmann::json;

namespace kolosal::server {

MCPServerIntegration::MCPServerIntegration(
    std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager,
    const MCPIntegrationConfig& config)
    : agent_manager_(std::move(agent_manager)), config_(config) {
    
    if (!agent_manager_) {
        throw std::invalid_argument("Agent manager cannot be null");
    }
    
    // Initialize statistics
    stats_.last_updated = std::chrono::system_clock::now();
    
    // Set up default request router
    request_router_ = [this](const std::string& operation, const std::string& capability) -> std::string {
        return routeRequest(operation, capability);
    };
}

MCPServerIntegration::~MCPServerIntegration() {
    try {
        stop();
    } catch (const std::exception& e) {
        // Log error but don't throw from destructor
    }
}

// Lifecycle management implementation

bool MCPServerIntegration::initialize() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (initialized_.load()) {
        return true;
    }
    
    try {
        // Validate configuration
        if (!validateConfig(config_)) {
            return false;
        }
        
        // Setup default transports
        setupDefaultTransports();
        
        // Auto-expose agents if configured
        if (config_.auto_expose_all_agents) {
            exposeAllAgents();
        }
        
        initialized_ = true;
        return true;
        
    } catch (const std::exception& e) {
        notifyError("initialization", e.what());
        return false;
    }
}

bool MCPServerIntegration::start() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (running_.load()) {
        return true;
    }
    
    if (!initialized_.load()) {
        if (!initialize()) {
            return false;
        }
    }
    
    try {
        // Start all exposed agent servers
        for (auto& [agent_id, adapter] : agent_adapters_) {
            auto transport_it = transports_.find(agent_id);
            if (transport_it != transports_.end()) {
                adapter->startServer(transport_it->second);
            }
        }
        
        // Start background services
        startBackgroundServices();
        
        running_ = true;
        
        notifyConnection("mcp_integration", true);
        
        return true;
        
    } catch (const std::exception& e) {
        notifyError("start", e.what());
        return false;
    }
}

void MCPServerIntegration::stop() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!running_.load()) {
        return;
    }
    
    try {
        // Stop background services first
        stopBackgroundServices();
        
        // Stop all agent servers
        for (auto& [agent_id, adapter] : agent_adapters_) {
            adapter->stopServer();
        }
        
        // Disconnect all external clients
        for (auto& [server_id, adapter] : external_connections_) {
            adapter->disconnectClient();
        }
        
        running_ = false;
        
        notifyConnection("mcp_integration", false);
        
    } catch (const std::exception& e) {
        notifyError("stop", e.what());
    }
}

// Agent exposure and management implementation

bool MCPServerIntegration::exposeAgent(const std::string& agent_id, const std::string& custom_name) {
    if (!validateAgentId(agent_id)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    // Check if already exposed
    if (agent_adapters_.find(agent_id) != agent_adapters_.end()) {
        return true;
    }
    
    try {
        // Get the agent
        auto agent = agent_manager_->get__agent(agent_id);
        if (!agent) {
            notifyError("exposeAgent", "Agent not found: " + agent_id);
            return false;
        }
        
        // Create MCP adapter configuration
        kolosal::services::MCPAgentAdapter::MCPConfig adapter_config;
        adapter_config.server_name = custom_name.empty() ? 
            (config_.server_name + "-" + agent_id) : custom_name;
        adapter_config.server_version = config_.server_version;
        adapter_config.enable_tool_streaming = config_.enable_streaming;
        adapter_config.default_timeout = config_.client_timeout;
        
        // Create adapter
        auto adapter = std::make_shared<kolosal::services::MCPAgentAdapter>(agent, adapter_config);
        
        // Register event callbacks
        adapter->registerEventCallback("tool_call", [this, agent_id](const std::string& event, const json& data) {
            std::lock_guard<std::mutex> stats_lock(stats_mutex_);
            stats_.total_requests++;
            // Extract duration if available
            double duration = data.contains("duration_ms") ? data["duration_ms"].get<double>() : 0.0;
            notifyRequest(agent_id, "tool_call", duration);
        });
        
        // Auto-register agent functions, memory, and capabilities
        if (config_.auto_expose_all_agents) {
            adapter->autoRegisterAgentFunctions(false);
            adapter->autoRegisterAgentMemory();
            adapter->autoRegisterAgentCapabilities();
        }
        
        // Store the adapter
        agent_adapters_[agent_id] = adapter;
        
        // Update statistics
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.exposed_agents++;
        stats_.registered_tools += adapter->autoRegisterAgentFunctions(false);
        stats_.last_updated = std::chrono::system_clock::now();
        
        return true;
        
    } catch (const std::exception& e) {
        notifyError("exposeAgent", e.what());
        return false;
    }
}

bool MCPServerIntegration::removeAgentExposure(const std::string& agent_id) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    auto it = agent_adapters_.find(agent_id);
    if (it == agent_adapters_.end()) {
        return false;
    }
    
    try {
        // Stop the server
        it->second->stopServer();
        
        // Remove from map
        agent_adapters_.erase(it);
        
        // Remove associated transport
        transports_.erase(agent_id);
        
        // Update statistics
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        if (stats_.exposed_agents > 0) {
            stats_.exposed_agents--;
        }
        stats_.last_updated = std::chrono::system_clock::now();
        
        return true;
        
    } catch (const std::exception& e) {
        notifyError("removeAgentExposure", e.what());
        return false;
    }
}

size_t MCPServerIntegration::exposeAllAgents() {
    if (!agent_manager_) {
        return 0;
    }
    
    size_t exposed_count = 0;
    auto agent_ids = agent_manager_->list_agents();
    
    for (const auto& agent_id : agent_ids) {
        if (exposeAgent(agent_id)) {
            exposed_count++;
        }
    }
    
    return exposed_count;
}

std::vector<std::string> MCPServerIntegration::getExposedAgents() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    std::vector<std::string> result;
    result.reserve(agent_adapters_.size());
    
    for (const auto& [agent_id, adapter] : agent_adapters_) {
        result.push_back(agent_id);
    }
    
    return result;
}

// Transport management implementation

bool MCPServerIntegration::addTransport(const std::string& name, 
                                      std::shared_ptr<mcp::transport::Transport> transport) {
    if (!transport) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(state_mutex_);
    transports_[name] = transport;
    return true;
}

bool MCPServerIntegration::removeTransport(const std::string& name) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return transports_.erase(name) > 0;
}

std::vector<std::string> MCPServerIntegration::getTransportNames() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    std::vector<std::string> result;
    result.reserve(transports_.size());
    
    for (const auto& [name, transport] : transports_) {
        result.push_back(name);
    }
    
    return result;
}

// External MCP client connections implementation

bool MCPServerIntegration::connectToExternalServer(
    const std::string& server_id,
    std::shared_ptr<mcp::transport::Transport> transport,
    const std::vector<std::string>& target_agents) {
    
    if (!transport) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    try {
        // Create a client adapter
        kolosal::services::MCPAgentAdapter::MCPConfig client_config;
        client_config.server_name = config_.server_name + "-client-" + server_id;
        client_config.server_version = config_.server_version;
        client_config.default_timeout = config_.client_timeout;
        
        // Create dummy agent for client functionality (we could refactor this later)
        // For now, we'll use the first available agent or create a minimal one
        std::shared_ptr<kolosal::agents::AgentCore> client_agent;
        if (!target_agents.empty()) {
            client_agent = agent_manager_->get__agent(target_agents[0]);
        }
        
        if (!client_agent) {
            notifyError("connectToExternalServer", "No suitable agent found for client connection");
            return false;
        }
        
        auto client_adapter = std::make_shared<kolosal::services::MCPAgentAdapter>(client_agent, client_config);
        
        // Initialize the client connection
        auto init_future = client_adapter->initializeClient(transport, config_.client_timeout);
        
        // Wait for initialization (with timeout)
        auto status = init_future.wait_for(config_.client_timeout);
        if (status != std::future_status::ready) {
            notifyError("connectToExternalServer", "Connection timeout for server: " + server_id);
            return false;
        }
        
        // Check if initialization was successful
        try {
            auto init_result = init_future.get();
            // Connection successful
            external_connections_[server_id] = client_adapter;
            
            notifyConnection(server_id, true);
            return true;
            
        } catch (const std::exception& e) {
            notifyError("connectToExternalServer", "Failed to connect to " + server_id + ": " + e.what());
            return false;
        }
        
    } catch (const std::exception& e) {
        notifyError("connectToExternalServer", e.what());
        return false;
    }
}

bool MCPServerIntegration::disconnectFromExternalServer(const std::string& server_id) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    auto it = external_connections_.find(server_id);
    if (it == external_connections_.end()) {
        return false;
    }
    
    try {
        it->second->disconnectClient();
        external_connections_.erase(it);
        
        notifyConnection(server_id, false);
        return true;
        
    } catch (const std::exception& e) {
        notifyError("disconnectFromExternalServer", e.what());
        return false;
    }
}

std::vector<std::string> MCPServerIntegration::getConnectedServers() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    std::vector<std::string> result;
    result.reserve(external_connections_.size());
    
    for (const auto& [server_id, adapter] : external_connections_) {
        if (adapter->isClientConnected()) {
            result.push_back(server_id);
        }
    }
    
    return result;
}

// Monitoring and statistics implementation

MCPServerIntegration::MCPStats MCPServerIntegration::getStatistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void MCPServerIntegration::resetStatistics() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    stats_ = MCPStats{};
    stats_.last_updated = std::chrono::system_clock::now();
    stats_.exposed_agents = agent_adapters_.size();
    stats_.active_connections = external_connections_.size();
}

json MCPServerIntegration::getHealthStatus() const {
    json health;
    
    // Basic status
    health["running"] = running_.load();
    health["initialized"] = initialized_.load();
    health["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // Statistics
    auto stats = getStatistics();
    health["statistics"] = {
        {"total_requests", stats.total_requests},
        {"successful_requests", stats.successful_requests},
        {"failed_requests", stats.failed_requests},
        {"active_connections", stats.active_connections},
        {"exposed_agents", stats.exposed_agents},
        {"registered_tools", stats.registered_tools},
        {"average_response_time_ms", stats.average_response_time_ms}
    };
    
    // Agent status
    health["agents"] = json::array();
    std::lock_guard<std::mutex> lock(state_mutex_);
    for (const auto& [agent_id, adapter] : agent_adapters_) {
        json agent_status;
        agent_status["id"] = agent_id;
        agent_status["server_running"] = adapter->isServerRunning();
        health["agents"].push_back(agent_status);
    }
    
    // External connections
    health["external_connections"] = json::array();
    for (const auto& [server_id, adapter] : external_connections_) {
        json connection_status;
        connection_status["server_id"] = server_id;
        connection_status["connected"] = adapter->isClientConnected();
        health["external_connections"].push_back(connection_status);
    }
    
    return health;
}

// Configuration management

void MCPServerIntegration::updateConfig(const MCPIntegrationConfig& config) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    config_ = config;
}

bool MCPServerIntegration::validateConfig(const MCPIntegrationConfig& config) {
    // Validate port range
    if (config.server_port == 0 || config.server_port > 65535) {
        return false;
    }
    
    // Validate connection limits
    if (config.max_client_connections == 0 || config.max_concurrent_requests == 0) {
        return false;
    }
    
    // Validate timeout values
    if (config.client_timeout.count() <= 0 || config.keepalive_interval.count() <= 0) {
        return false;
    }
    
    return true;
}

// Event handling implementation

void MCPServerIntegration::setConnectionCallback(ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    connection_callback_ = callback;
}

void MCPServerIntegration::setRequestCallback(RequestCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    request_callback_ = callback;
}

void MCPServerIntegration::setErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    error_callback_ = callback;
}

// Batch operations implementation

std::future<std::vector<json>> MCPServerIntegration::executeBatchToolCalls(
    const std::vector<std::tuple<std::string, std::string, json>>& requests,
    std::chrono::milliseconds timeout) {
    
    return std::async(std::launch::async, [this, requests, timeout]() -> std::vector<json> {
        std::vector<json> results;
        results.reserve(requests.size());
        
        std::vector<std::future<json>> futures;
        futures.reserve(requests.size());
        
        // Launch all requests asynchronously
        for (const auto& [agent_id, tool_name, params] : requests) {
            futures.emplace_back(std::async(std::launch::async, [this, agent_id, tool_name, params, timeout]() -> json {
                try {
                    std::lock_guard<std::mutex> lock(state_mutex_);
                    auto it = agent_adapters_.find(agent_id);
                    if (it == agent_adapters_.end()) {
                        return json{{"error", "Agent not found: " + agent_id}};
                    }
                    
                    // This would require a synchronous tool call method
                    // For now, we'll simulate the result
                    json result;
                    result["agent_id"] = agent_id;
                    result["tool_name"] = tool_name;
                    result["success"] = true;
                    result["message"] = "Batch execution completed";
                    return result;
                    
                } catch (const std::exception& e) {
                    return json{{"error", e.what()}};
                }
            }));
        }
        
        // Collect results
        for (auto& future : futures) {
            try {
                auto status = future.wait_for(timeout);
                if (status == std::future_status::ready) {
                    results.push_back(future.get());
                } else {
                    results.push_back(json{{"error", "Request timeout"}});
                }
            } catch (const std::exception& e) {
                results.push_back(json{{"error", e.what()}});
            }
        }
        
        return results;
    });
}

size_t MCPServerIntegration::broadcastToAllAgents(const std::string& message_type, const json& payload) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    size_t delivered_count = 0;
    
    for (const auto& [agent_id, adapter] : agent_adapters_) {
        try {
            // This would require implementing a broadcast method in the adapter
            // For now, we'll just count it as delivered
            delivered_count++;
            
        } catch (const std::exception& e) {
            notifyError("broadcastToAllAgents", "Failed to deliver to " + agent_id + ": " + e.what());
        }
    }
    
    return delivered_count;
}

// Private helper methods implementation

void MCPServerIntegration::setupDefaultTransports() {
    if (config_.enable_stdio_transport) {
        auto stdio_transport = std::make_shared<mcp::transport::StdioTransport>();
        transports_["stdio"] = stdio_transport;
    }
    
    if (config_.enable_http_sse_transport) {
        // HTTP SSE transport would need configuration
        // This is a placeholder for actual HTTP SSE transport creation
        // auto sse_transport = std::make_shared<mcp::transport::HttpSseTransport>(config_.server_host, config_.server_port);
        // transports_["http_sse"] = sse_transport;
    }
}

void MCPServerIntegration::startBackgroundServices() {
    stop_background_services_ = false;
    
    // Start health monitor thread
    health_monitor_thread_ = std::thread([this]() {
        healthMonitorLoop();
    });
    
    // Start statistics updater thread
    stats_updater_thread_ = std::thread([this]() {
        statsUpdaterLoop();
    });
}

void MCPServerIntegration::stopBackgroundServices() {
    stop_background_services_ = true;
    
    if (health_monitor_thread_.joinable()) {
        health_monitor_thread_.join();
    }
    
    if (stats_updater_thread_.joinable()) {
        stats_updater_thread_.join();
    }
}

void MCPServerIntegration::healthMonitorLoop() {
    while (!stop_background_services_) {
        try {
            updateStatistics();
            cleanupExpiredConnections();
            
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
        } catch (const std::exception& e) {
            notifyError("healthMonitor", e.what());
        }
    }
}

void MCPServerIntegration::statsUpdaterLoop() {
    while (!stop_background_services_) {
        try {
            updateStatistics();
            
            std::this_thread::sleep_for(std::chrono::seconds(10));
            
        } catch (const std::exception& e) {
            notifyError("statsUpdater", e.what());
        }
    }
}

void MCPServerIntegration::updateStatistics() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    // Update basic counts
    stats_.exposed_agents = agent_adapters_.size();
    stats_.active_connections = 0;
    
    // Count active connections
    for (const auto& [server_id, adapter] : external_connections_) {
        if (adapter->isClientConnected()) {
            stats_.active_connections++;
        }
    }
    
    stats_.last_updated = std::chrono::system_clock::now();
}

std::string MCPServerIntegration::routeRequest(const std::string& operation, const std::string& capability) {
    if (capability_routing_enabled_.load()) {
        // Find agents with the required capability
        std::vector<std::string> capable_agents;
        
        std::lock_guard<std::mutex> lock(state_mutex_);
        for (const auto& [agent_id, adapter] : agent_adapters_) {
            // This would require checking agent capabilities
            // For now, we'll use a simple approach
            capable_agents.push_back(agent_id);
        }
        
        if (!capable_agents.empty()) {
            if (round_robin_enabled_.load()) {
                size_t index = round_robin_counter_.fetch_add(1) % capable_agents.size();
                return capable_agents[index];
            } else {
                // Random selection
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist(0, capable_agents.size() - 1);
                return capable_agents[dist(gen)];
            }
        }
    }
    
    // Fallback to first available agent
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!agent_adapters_.empty()) {
        return agent_adapters_.begin()->first;
    }
    
    return "";
}

void MCPServerIntegration::notifyConnection(const std::string& server_id, bool connected) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    if (connection_callback_) {
        try {
            connection_callback_(server_id, connected);
        } catch (const std::exception& e) {
            // Ignore callback errors
        }
    }
}

void MCPServerIntegration::notifyRequest(const std::string& agent_id, const std::string& operation, double duration_ms) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    if (request_callback_) {
        try {
            request_callback_(agent_id, operation, duration_ms);
        } catch (const std::exception& e) {
            // Ignore callback errors
        }
    }
    
    // Update statistics
    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    stats_.successful_requests++;
    
    // Update rolling average
    double total_time = stats_.average_response_time_ms * (stats_.successful_requests - 1) + duration_ms;
    stats_.average_response_time_ms = total_time / stats_.successful_requests;
}

void MCPServerIntegration::notifyError(const std::string& component, const std::string& error_message) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    if (error_callback_) {
        try {
            error_callback_(component, error_message);
        } catch (const std::exception& e) {
            // Ignore callback errors
        }
    }
    
    // Update error statistics
    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    stats_.failed_requests++;
}

bool MCPServerIntegration::validateAgentId(const std::string& agent_id) const {
    if (agent_id.empty()) {
        return false;
    }
    
    // Check if agent exists in the manager
    if (agent_manager_) {
        auto agent_ids = agent_manager_->list_agents();
        return std::find(agent_ids.begin(), agent_ids.end(), agent_id) != agent_ids.end();
    }
    
    return false;
}

void MCPServerIntegration::cleanupExpiredConnections() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    // Remove disconnected external connections
    auto it = external_connections_.begin();
    while (it != external_connections_.end()) {
        if (!it->second->isClientConnected()) {
            notifyConnection(it->first, false);
            it = external_connections_.erase(it);
        } else {
            ++it;
        }
    }
}

bool MCPServerIntegration::checkRateLimit(const std::string& client_id) {
    if (!config_.enable_rate_limiting) {
        return true;
    }
    
    std::lock_guard<std::mutex> lock(rate_limit_mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto& limit_data = rate_limits_[client_id];
    
    // Reset counter if a minute has passed
    if (now - limit_data.last_request > std::chrono::minutes(1)) {
        limit_data.request_count = 0;
        limit_data.last_request = now;
    }
    
    // Check if within limits
    if (limit_data.request_count >= config_.max_requests_per_minute) {
        return false;
    }
    
    limit_data.request_count++;
    return true;
}

// Cross-agent communication (placeholder implementations)

bool MCPServerIntegration::enableCrossAgentCommunication(const std::vector<std::string>& agent_ids) {
    // This would set up agent-to-agent MCP connections
    // Implementation depends on specific requirements
    return true;
}

bool MCPServerIntegration::disableCrossAgentCommunication(const std::vector<std::string>& agent_ids) {
    // This would tear down agent-to-agent MCP connections
    // Implementation depends on specific requirements
    return true;
}

// Request routing configuration

void MCPServerIntegration::setRequestRouter(std::function<std::string(const std::string&, const std::string&)> router) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    request_router_ = router;
}

void MCPServerIntegration::enableRoundRobinBalancing(bool enable) {
    round_robin_enabled_ = enable;
}

void MCPServerIntegration::enableCapabilityBasedRouting(bool enable) {
    capability_routing_enabled_ = enable;
}

} // namespace kolosal::server

#endif // MCP_PROTOCOL_ENABLED
