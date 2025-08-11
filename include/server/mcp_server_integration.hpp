/**
 * @file mcp_server_integration.hpp
 * @brief Integration between Kolosal server and MCP protocol
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_SERVER_MCP_SERVER_INTEGRATION_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_SERVER_MCP_SERVER_INTEGRATION_HPP_INCLUDED

#include "../export.hpp"

#ifdef MCP_PROTOCOL_ENABLED

#include "agent/services/mcp_agent_adapter.hpp"
#include "agent/core/multi_agent_system.hpp"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <chrono>

// Forward declarations
namespace mcp {
    namespace transport {
        class Transport;
    }
}

namespace kolosal::server {

/**
 * @brief Integration layer between Kolosal multi-agent system and MCP protocol
 * 
 * This class provides:
 * - MCP server endpoints for external access to agents
 * - MCP client connections to external MCP services
 * - Agent-to-agent communication via MCP
 * - Centralized MCP transport management
 * - Load balancing and routing for MCP requests
 */
class KOLOSAL_SERVER_API MCPServerIntegration {
public:
    /**
     * @brief Configuration for MCP server integration
     */
    struct MCPIntegrationConfig {
        // Server configuration
        std::string server_host = "localhost";
        uint16_t server_port = 8080;
        std::string server_name = "kolosal-mcp-server";
        std::string server_version = "2.0.0";
        
        // Client configuration
        size_t max_client_connections = 100;
        std::chrono::seconds client_timeout{30};
        std::chrono::seconds keepalive_interval{60};
        
        // Transport configuration
        bool enable_stdio_transport = true;
        bool enable_http_sse_transport = true;
        bool enable_websocket_transport = false; // Future extension
        
        // Security and performance
        bool enable_authentication = false;
        bool enable_rate_limiting = true;
        size_t max_requests_per_minute = 1000;
        bool enable_request_logging = true;
        
        // Agent integration
        bool auto_expose_all_agents = true;
        bool enable_agent_discovery = true;
        bool enable_cross_agent_communication = true;
        
        // Advanced features
        bool enable_streaming = true;
        bool enable_batch_operations = true;
        size_t max_concurrent_requests = 50;
    };

    /**
     * @brief Statistics for MCP server integration
     */
    struct MCPStats {
        size_t total_requests = 0;
        size_t successful_requests = 0;
        size_t failed_requests = 0;
        size_t active_connections = 0;
        size_t total_connections = 0;
        size_t exposed_agents = 0;
        size_t registered_tools = 0;
        size_t registered_resources = 0;
        size_t registered_prompts = 0;
        double average_response_time_ms = 0.0;
        std::chrono::system_clock::time_point last_updated;
    };

    /**
     * @brief Constructor
     * @param agent_manager Shared pointer to the multi-agent system
     * @param config MCP integration configuration
     */
    explicit MCPServerIntegration(
        std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager,
        const MCPIntegrationConfig& config = MCPIntegrationConfig{});
    
    ~MCPServerIntegration();

    // Lifecycle management
    
    /**
     * @brief Initialize the MCP server integration
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Start all MCP servers and services
     * @return true if started successfully
     */
    bool start();
    
    /**
     * @brief Stop all MCP servers and services
     */
    void stop();
    
    /**
     * @brief Check if the integration is running
     * @return true if running
     */
    bool isRunning() const { return running_.load(); }

    // Agent exposure and management
    
    /**
     * @brief Expose agent as MCP server
     * @param agent_id Agent ID to expose
     * @param custom_name Custom MCP server name (optional)
     * @return true if exposed successfully
     */
    bool exposeAgent(const std::string& agent_id, const std::string& custom_name = "");
    
    /**
     * @brief Remove agent exposure
     * @param agent_id Agent ID to remove from exposure
     * @return true if removed successfully
     */
    bool removeAgentExposure(const std::string& agent_id);
    
    /**
     * @brief Expose all available agents
     * @return Number of agents exposed
     */
    size_t exposeAllAgents();
    
    /**
     * @brief Get list of exposed agent IDs
     * @return Vector of exposed agent IDs
     */
    std::vector<std::string> getExposedAgents() const;

    // MCP transport management
    
    /**
     * @brief Add custom MCP transport
     * @param name Transport name/identifier
     * @param transport Transport instance
     * @return true if added successfully
     */
    bool addTransport(const std::string& name, std::shared_ptr<mcp::transport::Transport> transport);
    
    /**
     * @brief Remove MCP transport
     * @param name Transport name to remove
     * @return true if removed successfully
     */
    bool removeTransport(const std::string& name);
    
    /**
     * @brief Get available transport names
     * @return Vector of transport names
     */
    std::vector<std::string> getTransportNames() const;

    // External MCP client connections
    
    /**
     * @brief Connect to external MCP server
     * @param server_id Identifier for this connection
     * @param transport Transport to use for connection
     * @param target_agents Agent IDs that can use this connection (empty = all)
     * @return true if connection established
     */
    bool connectToExternalServer(
        const std::string& server_id,
        std::shared_ptr<mcp::transport::Transport> transport,
        const std::vector<std::string>& target_agents = {});
    
    /**
     * @brief Disconnect from external MCP server
     * @param server_id Server connection ID
     * @return true if disconnected successfully
     */
    bool disconnectFromExternalServer(const std::string& server_id);
    
    /**
     * @brief Get list of connected external servers
     * @return Vector of server connection IDs
     */
    std::vector<std::string> getConnectedServers() const;

    // Cross-agent communication via MCP
    
    /**
     * @brief Enable agent-to-agent communication via MCP
     * @param agent_ids Agent IDs to enable communication for (empty = all)
     * @return true if enabled successfully
     */
    bool enableCrossAgentCommunication(const std::vector<std::string>& agent_ids = {});
    
    /**
     * @brief Disable agent-to-agent communication
     * @param agent_ids Agent IDs to disable communication for (empty = all)  
     * @return true if disabled successfully
     */
    bool disableCrossAgentCommunication(const std::vector<std::string>& agent_ids = {});

    // Request routing and load balancing
    
    /**
     * @brief Set custom request router
     * @param router Function to route requests to specific agents
     */
    void setRequestRouter(std::function<std::string(const std::string&, const std::string&)> router);
    
    /**
     * @brief Enable round-robin load balancing
     * @param enable Whether to enable round-robin
     */
    void enableRoundRobinBalancing(bool enable = true);
    
    /**
     * @brief Enable capability-based routing
     * @param enable Whether to enable capability-based routing
     */
    void enableCapabilityBasedRouting(bool enable = true);

    // Monitoring and statistics
    
    /**
     * @brief Get current statistics
     * @return Current MCP integration statistics
     */
    MCPStats getStatistics() const;
    
    /**
     * @brief Reset statistics counters
     */
    void resetStatistics();
    
    /**
     * @brief Get detailed health status
     * @return JSON object with detailed health information
     */
    nlohmann::json getHealthStatus() const;

    // Event handling and callbacks
    using ConnectionCallback = std::function<void(const std::string&, bool)>; // server_id, connected
    using RequestCallback = std::function<void(const std::string&, const std::string&, double)>; // agent_id, operation, duration_ms
    using ErrorCallback = std::function<void(const std::string&, const std::string&)>; // component, error_message
    
    /**
     * @brief Register connection event callback
     * @param callback Callback for connection events
     */
    void setConnectionCallback(ConnectionCallback callback);
    
    /**
     * @brief Register request event callback
     * @param callback Callback for request events
     */
    void setRequestCallback(RequestCallback callback);
    
    /**
     * @brief Register error event callback
     * @param callback Callback for error events
     */
    void setErrorCallback(ErrorCallback callback);

    // Configuration management
    
    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    const MCPIntegrationConfig& getConfig() const { return config_; }
    
    /**
     * @brief Update configuration (requires restart)
     * @param config New configuration
     */
    void updateConfig(const MCPIntegrationConfig& config);
    
    /**
     * @brief Validate configuration
     * @param config Configuration to validate
     * @return true if configuration is valid
     */
    static bool validateConfig(const MCPIntegrationConfig& config);

    // Batch operations
    
    /**
     * @brief Execute batch tool calls across multiple agents
     * @param requests Vector of tool call requests
     * @param timeout Timeout for batch operation
     * @return Future with batch results
     */
    std::future<std::vector<nlohmann::json>> executeBatchToolCalls(
        const std::vector<std::tuple<std::string, std::string, nlohmann::json>>& requests, // agent_id, tool_name, params
        std::chrono::milliseconds timeout = std::chrono::seconds(60));
    
    /**
     * @brief Broadcast message to all exposed agents
     * @param message_type Message type
     * @param payload Message payload
     * @return Number of agents that received the message
     */
    size_t broadcastToAllAgents(const std::string& message_type, const nlohmann::json& payload);

private:
    std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager_;
    MCPIntegrationConfig config_;
    
    // MCP adapters for each exposed agent
    std::map<std::string, std::shared_ptr<kolosal::services::MCPAgentAdapter>> agent_adapters_;
    
    // External server connections
    std::map<std::string, std::shared_ptr<kolosal::services::MCPAgentAdapter>> external_connections_;
    
    // Transport management
    std::map<std::string, std::shared_ptr<mcp::transport::Transport>> transports_;
    
    // Statistics and monitoring
    mutable std::mutex stats_mutex_;
    MCPStats stats_;
    
    // State management
    std::atomic<bool> running_{false};
    std::atomic<bool> initialized_{false};
    mutable std::mutex state_mutex_;
    
    // Request routing
    std::function<std::string(const std::string&, const std::string&)> request_router_;
    std::atomic<bool> round_robin_enabled_{false};
    std::atomic<bool> capability_routing_enabled_{true};
    std::atomic<size_t> round_robin_counter_{0};
    
    // Event callbacks
    ConnectionCallback connection_callback_;
    RequestCallback request_callback_;
    ErrorCallback error_callback_;
    mutable std::mutex callback_mutex_;
    
    // Background services
    std::thread health_monitor_thread_;
    std::thread stats_updater_thread_;
    std::atomic<bool> stop_background_services_{false};
    
    // Internal methods
    void setupDefaultTransports();
    void startBackgroundServices();
    void stopBackgroundServices();
    void healthMonitorLoop();
    void statsUpdaterLoop();
    void updateStatistics();
    std::string routeRequest(const std::string& operation, const std::string& capability);
    void notifyConnection(const std::string& server_id, bool connected);
    void notifyRequest(const std::string& agent_id, const std::string& operation, double duration_ms);
    void notifyError(const std::string& component, const std::string& error_message);
    bool validateAgentId(const std::string& agent_id) const;
    void cleanupExpiredConnections();
    
    // Rate limiting
    struct RateLimitData {
        std::chrono::steady_clock::time_point last_request;
        size_t request_count;
    };
    mutable std::mutex rate_limit_mutex_;
    std::map<std::string, RateLimitData> rate_limits_;
    bool checkRateLimit(const std::string& client_id);
};

} // namespace kolosal::server

#endif // MCP_PROTOCOL_ENABLED

#endif // KOLOSAL_AGENT_INCLUDE_SERVER_MCP_SERVER_INTEGRATION_HPP_INCLUDED
