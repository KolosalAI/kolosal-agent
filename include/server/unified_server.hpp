/**
 * @file unified_server.hpp
 * @brief Unifi    // Server configuration (for LLM server)
    std::string server_executable_path = "";
    std::string server_host = "127.0.0.1";
    int server_port = 8080;
    bool auto_start_server = true;
    int server_startup_timeout_seconds = 60;
    // Agent API server configuration
    std::string agent_api_host = "127.0.0.1";
    int agent_api_port = 8081;  // Use different port for agent API
    // Agent system configuration
    std::string agent_config_file = "config.yaml";
    bool auto_start_agents = true;
    bool enable_agent_api = true;integrating LLM and agents
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_SYSTEM_INTEGRATION_UNIFIED_SERVER_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_SYSTEM_INTEGRATION_UNIFIED_SERVER_HPP_INCLUDED

#include "../export.hpp"
#include "server_client_interface.h"
#include "../agent/services/agent_service.hpp"
#include "../agent/core/multi_agent_system.hpp"
#include "../api/simple_http_server.hpp"
#include "../api/agent_management_route.hpp"

#ifdef MCP_PROTOCOL_ENABLED
#include "mcp_server_integration.hpp"
#endif

#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#endif

namespace kolosal::integration {

/**
 * @brief Unified server that integrates both Kolosal LLM server and Agent system
 * 
 * This class provides a unified interface for managing both the LLM inference server
 * and the multi-agent system, with automatic health monitoring and coordination
 * between the two systems.
 */
/**
 * @brief Represents k o l o s a l_ a g e n t_ a p i functionality
 */
class KOLOSAL_AGENT_API UnifiedKolosalServer {
public:
    struct ServerConfig {
    // LLM Server configuration  
    std::string server_executable_path = "";
    std::string server_host = "127.0.0.1";
    int server_port = 8080;
    bool auto_start_server = true;
    int server_startup_timeout_seconds = 60;
    // Agent API server configuration
    std::string agent_api_host = "127.0.0.1";
    int agent_api_port = 8081;  // Use different port for agent API
    // Agent system configuration
    std::string agent_config_file = "agent_config.yaml";
    bool auto_start_agents = true;
    bool enable_agent_api = true;
    // Integration configuration
    bool enable_health_monitoring = true;
    std::chrono::seconds health_check_interval = std::chrono::seconds(30);
    bool enable_metrics_collection = true;
    // API configuration
    bool enable_cors = true;
        std::vector<std::string> allowed_origins = {"*"};
    };

    struct SystemStatus {
    bool llm_server_running = false;
    bool llm_server_healthy = false;
    bool agent_system_running = false;
    int total_agents = 0;
    int running_agents = 0;
    std::string last_error = "";
        std::chrono::system_clock::time_point last_health_check;
    double average_response_time_ms = 0.0;
    };

    /**
     * @brief Constructor
     * @param config Server configuration
     */
    explicit UnifiedKolosalServer(const ServerConfig& config = ServerConfig {});
    ~UnifiedKolosalServer();

    // Lifecycle management
    bool start();
    void stop();
    bool is_Running() const;
    
    // Status and monitoring
    SystemStatus getSystem_Status() const;
    std::string getSystemStatus_Json() const;
    bool performHealth_Check();
    
    // Configuration
    bool reload_Configuration(const std::string& config_file = "");
    ServerConfig get_Configuration() const;
    void update_Configuration(const ServerConfig& config);

    // Component access
    std::shared_ptr<KolosalServerClient> getLLMServer_Client() const;
    std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> getAgent_Manager() const;
    std::shared_ptr<kolosal::services::AgentService> getAgent_Service() const;
    
    // Advanced features
    bool enableAuto_Recovery(const bool enable = true);
    void setHealthCheck_Callback(std::function<void(const SystemStatus&)> callback);
    
    // Metrics and analytics
    struct ServerMetrics {
        const size_t total_llm_requests = 0;
        const size_t successful_llm_requests = 0;
        const size_t total_agent_function_calls = 0;
        const size_t successful_agent_function_calls = 0;
        const double average_llm_response_time_ms = 0.0;
        const double average_agent_response_time_ms = 0.0;
        std::chrono::system_clock::time_point metrics_start_time;
    };
    
    ServerMetrics get_Metrics() const;
    void reset_Metrics();

private:
    ServerConfig config_;
    mutable std::atomic<bool> running_ {false};
    mutable std::atomic<bool> health_monitoring_active_ {false};
    
    // Component instances
    std::shared_ptr<KolosalServerClient> llm_server_client_;
    std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager_;
    std::shared_ptr<kolosal::services::AgentService> agent_service_;
    std::unique_ptr<kolosal::api::SimpleHttpServer> agent_http_server_;
    std::shared_ptr<kolosal::api::AgentManagementRoute> agent_management_route_;

    // Health monitoring
    std::thread health_monitoring_thread_;
    mutable std::mutex status_mutex_;
    SystemStatus current_status_;
    std::function<void(const SystemStatus&)> health_callback_;
    
    // Auto-recovery
    std::atomic<bool> auto_recovery_enabled_ {false};
    std::atomic<int> recovery_attempts_ {0};
    static constexpr const int MAX_RECOVERY_ATTEMPTS = 3;
    // Metrics
    mutable std::mutex metrics_mutex_;
    ServerMetrics metrics_;

    // Internal methods
    bool start_LLMServer();
    bool startAgent_System();
    bool start_AgentHttpServer();
    void stop_LLMServer();
    void stopAgent_System();
    void stop_AgentHttpServer();
    
    void healthMonitoring_Loop();
    bool performLLMServerHealth_Check();
    bool performAgentSystemHealth_Check();
    
    void handleHealthCheck_Failure(const std::string& component, const std::string& error);
    bool attemptAuto_Recovery(const std::string& component);
    
    void update_Metrics();
    void log_Event(const std::string& level, const std::string& message);
    
    // Server process management methods
    std::string findServer_Executable() const;
    bool startServer_Process(const std::string& server_path);
    void stopServer_Process();
    
    // Server process management (platform-specific)
#ifdef _WIN32
    HANDLE server_process_handle_ = nullptr;
    DWORD server_process_id_ = 0;
#else
    pid_t server_process_id_ = 0;
#endif
    std::atomic<bool> server_started_by_us_ {false};
};

/**
 * @brief Factory for creating and configuring unified servers
 */
class KOLOSAL_AGENT_API UnifiedServerFactory {
public:

    static std::unique_ptr<UnifiedKolosalServer> createFromConfig_File(const std::string& config_file);
    static std::unique_ptr<UnifiedKolosalServer> createProduction_Server(int port = 8080);
    static std::unique_ptr<UnifiedKolosalServer> createDevelopment_Server(const int port = 8080);
    // Configuration builders

    static UnifiedKolosalServer::ServerConfig buildProduction_Config(int port);
    static UnifiedKolosalServer::ServerConfig buildDevelopment_Config(int port);
};

} // namespace kolosal::integration

#endif // KOLOSAL_AGENT_INCLUDE_SYSTEM_INTEGRATION_UNIFIED_SERVER_HPP_INCLUDED
