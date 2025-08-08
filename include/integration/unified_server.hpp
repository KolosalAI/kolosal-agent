#pragma once

#include "../export.hpp"
#include "../kolosal_server_client.h"
#include "../services/agent_service.hpp"
#include "../agent/multi_agent_system.hpp"
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
class KOLOSAL_AGENT_API UnifiedKolosalServer {
public:
    struct ServerConfig {
        // LLM Server configuration
        std::string server_executable_path = "";
        std::string server_host = "127.0.0.1";
        int server_port = 8080;
        bool auto_start_server = true;
        int server_startup_timeout_seconds = 60;
        
        // Agent system configuration
        std::string agent_config_file = "config.yaml";
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
    explicit UnifiedKolosalServer(const ServerConfig& config = ServerConfig{});
    ~UnifiedKolosalServer();

    // Lifecycle management
    bool start();
    void stop();
    bool isRunning() const;
    
    // Status and monitoring
    SystemStatus getSystemStatus() const;
    std::string getSystemStatusJson() const;
    bool performHealthCheck();
    
    // Configuration
    bool reloadConfiguration(const std::string& config_file = "");
    ServerConfig getConfiguration() const;
    void updateConfiguration(const ServerConfig& config);

    // Component access
    std::shared_ptr<KolosalServerClient> getLLMServerClient() const;
    std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> getAgentManager() const;
    std::shared_ptr<kolosal::services::AgentService> getAgentService() const;
    
    // Advanced features
    bool enableAutoRecovery(bool enable = true);
    void setHealthCheckCallback(std::function<void(const SystemStatus&)> callback);
    
    // Metrics and analytics
    struct ServerMetrics {
        size_t total_llm_requests = 0;
        size_t successful_llm_requests = 0;
        size_t total_agent_function_calls = 0;
        size_t successful_agent_function_calls = 0;
        double average_llm_response_time_ms = 0.0;
        double average_agent_response_time_ms = 0.0;
        std::chrono::system_clock::time_point metrics_start_time;
    };
    
    ServerMetrics getMetrics() const;
    void resetMetrics();

private:
    ServerConfig config_;
    mutable std::atomic<bool> running_{false};
    mutable std::atomic<bool> health_monitoring_active_{false};
    
    // Component instances
    std::shared_ptr<KolosalServerClient> llm_server_client_;
    std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager_;
    std::shared_ptr<kolosal::services::AgentService> agent_service_;
    
    // Health monitoring
    std::thread health_monitoring_thread_;
    mutable std::mutex status_mutex_;
    SystemStatus current_status_;
    std::function<void(const SystemStatus&)> health_callback_;
    
    // Auto-recovery
    std::atomic<bool> auto_recovery_enabled_{false};
    std::atomic<int> recovery_attempts_{0};
    static constexpr int MAX_RECOVERY_ATTEMPTS = 3;
    
    // Metrics
    mutable std::mutex metrics_mutex_;
    ServerMetrics metrics_;

    // Internal methods
    bool startLLMServer();
    bool startAgentSystem();
    void stopLLMServer();
    void stopAgentSystem();
    
    void healthMonitoringLoop();
    bool performLLMServerHealthCheck();
    bool performAgentSystemHealthCheck();
    
    void handleHealthCheckFailure(const std::string& component, const std::string& error);
    bool attemptAutoRecovery(const std::string& component);
    
    void updateMetrics();
    void logEvent(const std::string& level, const std::string& message);
    
    // Server process management methods
    std::string findServerExecutable() const;
    bool startServerProcess(const std::string& server_path);
    void stopServerProcess();
    
    // Server process management (platform-specific)
#ifdef _WIN32
    HANDLE server_process_handle_ = nullptr;
    DWORD server_process_id_ = 0;
#else
    pid_t server_process_id_ = 0;
#endif
    std::atomic<bool> server_started_by_us_{false};
};

/**
 * @brief Factory for creating and configuring unified servers
 */
class KOLOSAL_AGENT_API UnifiedServerFactory {
public:
    static std::unique_ptr<UnifiedKolosalServer> createDefaultServer();
    static std::unique_ptr<UnifiedKolosalServer> createFromConfigFile(const std::string& config_file);
    static std::unique_ptr<UnifiedKolosalServer> createProductionServer(int port = 8080);
    static std::unique_ptr<UnifiedKolosalServer> createDevelopmentServer(int port = 8080);
    
    // Configuration builders
    static UnifiedKolosalServer::ServerConfig buildDefaultConfig();
    static UnifiedKolosalServer::ServerConfig buildProductionConfig(int port);
    static UnifiedKolosalServer::ServerConfig buildDevelopmentConfig(int port);
};

} // namespace kolosal::integration
