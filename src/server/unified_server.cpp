/**
 * @file unified_server.cpp
 * @brief Unified server integrating LLM and agents
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "server/unified_server.hpp"
#include "api/simple_http_server.hpp"
#include "api/agent_management_route.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "../external/nlohmann/json.hpp"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#endif

using json = nlohmann::json;
using namespace kolosal::agents;
using namespace kolosal::services;

namespace kolosal::integration {

UnifiedKolosalServer::UnifiedKolosalServer(const ServerConfig& config)
    : config_(config) {
    // Initialize agent manager and service
    agent_manager_ = std::make_shared<YAMLConfigurableAgentManager>();
    agent_service_ = std::make_shared<AgentService>(agent_manager_);
    
    // Initialize HTTP server components if agent API is enabled
    if (config_.enable_agent_api) {
        kolosal::api::SimpleHttpServer::ServerConfig http_config;
        http_config.host = config_.agent_api_host;
        http_config.port = config_.agent_api_port;
        http_config.backlog = 10;
        agent_http_server_ = std::make_unique<kolosal::api::SimpleHttpServer>(http_config);
        
        // Create agent management route
        agent_management_route_ = std::make_shared<kolosal::api::AgentManagementRoute>(agent_manager_);
        agent_http_server_->add_Route(agent_management_route_);
    }
    
    // Initialize current status
    current_status_.last_health_check = std::chrono::system_clock::now();
    
    // Initialize metrics
    metrics_.metrics_start_time = std::chrono::system_clock::now();
}

UnifiedKolosalServer::~UnifiedKolosalServer() {
    stop();
}

bool UnifiedKolosalServer::start() {
    if (running_.load()) {
        return true;
    }
    
    log_Event("information", "Starting unified Kolosal server...");
    
    try {
        bool llm_started_or_connected = false;
        // Start or connect to LLM server according to configuration, but treat failure as non-fatal
        if (config_.auto_start_server) {
            if (!start_LLMServer()) {
                log_Event("ERROR", "Failed to start LLM server (continuing without LLM)");
            } else {
                llm_started_or_connected = true;
            }
        } else {
            // Create client for external server
            std::string server_url = "http://" + config_.server_host + ":" + std::to_string(config_.server_port);
            llm_server_client_ = std::make_shared<KolosalServerClient>(server_url);
            // Test connection
            if (!llm_server_client_->waitForServer_Ready(10)) {
                log_Event("ERROR", "Cannot connect to external LLM server at " + server_url + " (continuing without LLM)");
                llm_server_client_.reset();
            } else {
                llm_started_or_connected = true;
            }
        }
        
        // Start agent system
        bool agents_started = false;
        if (config_.auto_start_agents) {
            if (!startAgent_System()) {
                log_Event("ERROR", "Failed to start agent system");
            } else {
                agents_started = true;
            }
        }
        
        // Start agent HTTP server if enabled
        bool http_started = false;
        if (config_.enable_agent_api && agent_http_server_) {
            if (!start_AgentHttpServer()) {
                log_Event("ERROR", "Failed to start agent HTTP server (continuing without Agent API)");
            } else {
                http_started = true;
            }
        }
        
        // If none of the subsystems started, treat startup as failure
        if (!llm_started_or_connected && !agents_started && !http_started) {
            log_Event("ERROR", "No subsystems started successfully (LLM, agents, or Agent API). Aborting startup.");
            if (server_started_by_us_.load()) {
                stop_LLMServer();
            }
            return false;
        }

        // Start health monitoring
        if (config_.enable_health_monitoring) {
            health_monitoring_active_ = true;
            health_monitoring_thread_ = std::thread(&UnifiedKolosalServer::healthMonitoring_Loop, this);
        }
        
        // Start agent service health monitoring
        if (agent_service_) {
            agent_service_->startHealthMonitoring(config_.health_check_interval);
        }
        
        running_ = true;
        log_Event("information", "Unified Kolosal server started successfully");
        return true;
        
    } catch (const std::exception& e) {
        log_Event("ERROR", "Exception during server startup: " + std::string(e.what()));
        return false;
    }
}

void UnifiedKolosalServer::stop() {
    if (!running_.load()) {
        return;
    }
    
    log_Event("information", "Stopping unified Kolosal server...");
    running_ = false;
    
    // Stop health monitoring
    if (health_monitoring_active_.load()) {
        health_monitoring_active_ = false;
        if (health_monitoring_thread_.joinable()) {
            health_monitoring_thread_.join();
        }
    }
    
    // Stop agent service health monitoring
    if (agent_service_) {
    agent_service_->stopHealthMonitoring();
    }
    
    // Stop agent HTTP server
    if (config_.enable_agent_api && agent_http_server_) {
        stop_AgentHttpServer();
    }
    
    // Stop agent system
    stopAgent_System();
    
    // Stop LLM server if we started it
    if (server_started_by_us_.load()) {
        stop_LLMServer();
    }
    
    log_Event("information", "Unified Kolosal server stopped");
}

bool UnifiedKolosalServer::is_Running() const {
    return running_.load();
}

UnifiedKolosalServer::SystemStatus UnifiedKolosalServer::getSystem_Status() const {
    std::lock_guard<std::mutex> lock(status_mutex_);
    return current_status_;
}

std::string UnifiedKolosalServer::getSystemStatus_Json() const {
    const auto status = getSystem_Status();
    json status_json;
    status_json["llm_server_running"] = status.llm_server_running;
    status_json["llm_server_healthy"] = status.llm_server_healthy;
    status_json["agent_system_running"] = status.agent_system_running;
    status_json["total_agents"] = status.total_agents;
    status_json["running_agents"] = status.running_agents;
    status_json["last_error"] = status.last_error;
    status_json["average_response_time_ms"] = status.average_response_time_ms;
    status_json["last_health_check"] = std::chrono::duration_cast<std::chrono::seconds>(
        status.last_health_check.time_since_epoch()).count();
    status_json["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return status_json.dump(2);
}

bool UnifiedKolosalServer::performHealth_Check() {
    const bool llm_healthy = performLLMServerHealth_Check();
    const bool agent_healthy = performAgentSystemHealth_Check();
    std::lock_guard<std::mutex> lock(status_mutex_);
    current_status_.llm_server_healthy = llm_healthy;
    current_status_.llm_server_running = (llm_server_client_ != nullptr);
    current_status_.agent_system_running = (agent_manager_ && agent_manager_->is__running());
    current_status_.last_health_check = std::chrono::system_clock::now();
    
    if (agent_manager_) {
        const auto agent_ids = agent_manager_->list_agents();
        current_status_.total_agents = static_cast<int>(agent_ids.size());
        
    int running_count = 0;
        for (const auto& agent_id : agent_ids) {
            const auto agent = agent_manager_->get__agent(agent_id);
            if (agent && agent->is__running()) {
                running_count++;
            }
        }
        current_status_.running_agents = running_count;
    }
    
    return llm_healthy && agent_healthy;
}

bool UnifiedKolosalServer::start_LLMServer() {
    log_Event("information", "Starting LLM server...");
    
    // Find server executable if not specified
    std::string server_path = config_.server_executable_path;
    if (server_path.empty()) {
        server_path = findServer_Executable();
        if (server_path.empty()) {
            log_Event("ERROR", "Kolosal-server executable not found");
            return false;
        }
    }
    
    // Start server process
    if (!startServer_Process(server_path)) {
        return false;
    }
    
    // Create client
    std::string server_url = "http://" + config_.server_host + ":" + std::to_string(config_.server_port);
    llm_server_client_ = std::make_shared<KolosalServerClient>(server_url);
    
    // Wait for server to be ready
    if (!llm_server_client_->waitForServer_Ready(config_.server_startup_timeout_seconds)) {
        log_Event("ERROR", "LLM server failed to become ready within timeout");
        stopServer_Process();
        return false;
    }
    
    server_started_by_us_ = true;
    log_Event("information", "LLM server started successfully");
    return true;
}

bool UnifiedKolosalServer::startAgent_System() {
    log_Event("information", "Starting agent system...");
    
    if (!agent_manager_) {
        log_Event("ERROR", "Agent manager not initialized");
        return false;
    }
    
    // Load configuration
    if (!agent_manager_->load_configuration(config_.agent_config_file)) {
        log_Event("ERROR", "Failed to load agent configuration from: " + config_.agent_config_file);
        return false;
    }
    
    // Start the agent system
    try {
        agent_manager_->start();
        log_Event("information", "Agent system started successfully");
        return true;
    } catch (const std::exception& e) {
        log_Event("ERROR", "Exception starting agent system: " + std::string(e.what()));
        return false;
    }
}

void UnifiedKolosalServer::stop_LLMServer() {
    if (llm_server_client_) {
        log_Event("information", "Shutting down LLM server...");
        llm_server_client_->shutdown_Server();
    }
    
    if (server_started_by_us_.load()) {
        stopServer_Process();
        server_started_by_us_ = false;
    }
    
    llm_server_client_.reset();
}

void UnifiedKolosalServer::stopAgent_System() {
    if (agent_manager_) {
        log_Event("information", "Stopping agent system...");
        agent_manager_->stop();
    }
}

bool UnifiedKolosalServer::start_AgentHttpServer() {
    if (!agent_http_server_) {
        log_Event("ERROR", "Agent HTTP server not initialized");
        return false;
    }
    
    // Try starting on configured port, with fallback to next ports if unavailable
    const int max_attempts = 10;
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        const int port_to_try = config_.agent_api_port + attempt;
        log_Event("information", "Starting agent HTTP server on " + config_.agent_api_host + ":" + std::to_string(port_to_try));

        // Recreate server with the new port if not the first attempt
        if (attempt > 0) {
            kolosal::api::SimpleHttpServer::ServerConfig http_config;
            http_config.host = config_.agent_api_host;
            http_config.port = port_to_try;
            http_config.backlog = 10;
            http_config.enable_cors = true;
            agent_http_server_ = std::make_unique<kolosal::api::SimpleHttpServer>(http_config);
            // Reattach routes
            if (agent_management_route_) {
                agent_http_server_->add_Route(agent_management_route_);
            }
        }

        if (agent_http_server_->start()) {
            config_.agent_api_port = port_to_try;
            log_Event("information", "Agent HTTP server started successfully on port " + std::to_string(port_to_try));
            return true;
        }

        log_Event("WARN", "Port " + std::to_string(port_to_try) + " unavailable. Trying next port...");
    }

    log_Event("ERROR", "Failed to start agent HTTP server on any tried port");
    return false;
}

void UnifiedKolosalServer::stop_AgentHttpServer() {
    if (agent_http_server_) {
        log_Event("information", "Stopping agent HTTP server...");
        agent_http_server_->stop();
        log_Event("information", "Agent HTTP server stopped");
    }
}

void UnifiedKolosalServer::healthMonitoring_Loop() {
    while (health_monitoring_active_.load() && running_.load()) {
        try {
            const bool healthy = performHealth_Check();
            if (!healthy) {
                log_Event("WARN", "Health check failed");
                
                if (auto_recovery_enabled_.load()) {
                    if (recovery_attempts_.load() < MAX_RECOVERY_ATTEMPTS) {
                        log_Event("information", "Attempting auto-recovery...");
                        recovery_attempts_++;
                        
                        // Implement recovery logic here
                        // For now, just log the attempt
                    }
                }
            } else {
                recovery_attempts_ = 0; // Reset on successful health check
            }
            
            // Update metrics
            update_Metrics();
            
            // Notify callback if set
            if (health_callback_) {
                health_callback_(getSystem_Status());
            }
            
        } catch (const std::exception& e) {
            log_Event("ERROR", "Exception in health monitoring: " + std::string(e.what()));
        }
        
        std::this_thread::sleep_for(config_.health_check_interval);
    }
}

bool UnifiedKolosalServer::performLLMServerHealth_Check() {
    if (!llm_server_client_) {
        return false;
    }
    
    return llm_server_client_->isServer_Healthy();
}

bool UnifiedKolosalServer::performAgentSystemHealth_Check() {
    if (!agent_manager_) {
        return false;
    }
    
    return agent_manager_->is__running();
}

void UnifiedKolosalServer::update_Metrics() {
    // This would update various metrics
    // Implementation depends on how metrics are collected
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    // Update response times
    if (agent_service_) {
    const auto system_metrics = agent_service_->getSystemMetrics();
        std::lock_guard<std::mutex> status_lock(status_mutex_);
        current_status_.average_response_time_ms = system_metrics.average_response_time_ms;
    }
}

void UnifiedKolosalServer::handleHealthCheck_Failure(const std::string& component, const std::string& error) {
    log_Event("ERROR", "Health check failure for " + component + ": " + error);
    
    std::lock_guard<std::mutex> lock(status_mutex_);
    current_status_.last_error = component + " failure: " + error;
    
    if (component == "llm_server") {
        current_status_.llm_server_healthy = false;
    } else if (component == "agent_system") {
        current_status_.agent_system_running = false;
    }
}

bool UnifiedKolosalServer::attemptAuto_Recovery(const std::string& component) {
    log_Event("information", "Attempting auto-recovery for component: " + component);
    
    try {
        if (component == "llm_server") {
            if (config_.auto_start_server && server_started_by_us_.load()) {
                stop_LLMServer();
                std::this_thread::sleep_for(std::chrono::seconds(2));
                return start_LLMServer();
            } else {
                // Try to reconnect to external server
                if (llm_server_client_) {
                    return llm_server_client_->waitForServer_Ready(10);
                }
            }
        } else if (component == "agent_system") {
            if (agent_manager_) {
                agent_manager_->stop();
                std::this_thread::sleep_for(std::chrono::seconds(1));
                return startAgent_System();
            }
        }
    } catch (const std::exception& e) {
        log_Event("ERROR", "Exception during auto-recovery for " + component + ": " + std::string(e.what()));
    }
    
    return false;
}

void UnifiedKolosalServer::log_Event(const std::string& level, const std::string& message) {
    const auto now = std::chrono::system_clock::now();
    const auto time_t = std::chrono::system_clock::to_time_t(now);
    const auto tm = *std::localtime(&time_t);
    char timestamp[64];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);
    
    std::cout << "[" << timestamp << "] [" << level << "] " << message << std::endl;
}

// Platform-specific server process management
std::string UnifiedKolosalServer::findServer_Executable() const {
    std::filesystem::path current_path = std::filesystem::current_path();
    std::vector<std::string> server_names = {
#ifdef _WIN32
        "kolosal-server.exe",
        "kolosal_server.exe"
#else
        "kolosal-server",
        "kolosal_server"
#endif
    };
    
    std::vector<std::filesystem::path> search_paths = {
        current_path,
        current_path / "kolosal-server",
        current_path / ".." / "kolosal-server",
        current_path / "bin",
        current_path / ".." / "bin",
        current_path / "build" / "bin",
        current_path / "build" / "kolosal-server"
    };
    
    for (const auto& search_path : search_paths) {
        for (const auto& server_name : server_names) {
            const auto server_path = search_path / server_name;
            if (std::filesystem::exists(server_path)) {
                return server_path.string();
            }
        }
    }
    
    return "";
}

bool UnifiedKolosalServer::startServer_Process(const std::string& server_path) {
    std::string command = "\"" + server_path + "\" --port " + std::to_string(config_.server_port) + 
                         " --host " + config_.server_host;
#ifdef _WIN32
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    if (CreateProcessA(
        nullptr,
        const_cast<char*>(command.c_str()),
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    )) {
        server_process_handle_ = pi.hProcess;
        server_process_id_ = pi.dwProcessId;
        CloseHandle(pi.hThread);
        return true;
    } else {
        log_Event("ERROR", "Failed to start server process. Error: " + std::to_string(GetLastError()));
        return false;
    }
#else
    server_process_id_ = fork();
    if (server_process_id_ == 0) {
        // Child process
        execlp(server_path.c_str(), server_path.c_str(), 
               "--port", std::to_string(config_.server_port).c_str(),
               "--host", config_.server_host.c_str(),
               nullptr);
        exit(1);
    } else if (server_process_id_ > 0) {
        return true;
    } else {
        log_Event("ERROR", "Failed to fork server process");
        return false;
    }
#endif
}

void UnifiedKolosalServer::stopServer_Process() {
#ifdef _WIN32
    if (server_process_handle_ && server_process_id_) {
    TerminateProcess(server_process_handle_, 0);
    WaitForSingleObject(server_process_handle_, 5000);
    CloseHandle(server_process_handle_);
        server_process_handle_ = nullptr;
        server_process_id_ = 0;
    }
#else
    if (server_process_id_ > 0) {
        kill(server_process_id_, SIGTERM);
        int status;
        waitpid(server_process_id_, &status, 0);
        server_process_id_ = 0;
    }
#endif
}

bool UnifiedKolosalServer::enableAuto_Recovery(bool enable) {
    auto_recovery_enabled_ = enable;
    return true;
}

void UnifiedKolosalServer::setHealthCheck_Callback(std::function<void(const SystemStatus&)> callback) {
    health_callback_ = callback;
}

UnifiedKolosalServer::ServerMetrics UnifiedKolosalServer::get_Metrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return metrics_;
}

void UnifiedKolosalServer::reset_Metrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    // Reset non-const fields only (const members remain at their initialized values)
    metrics_.metrics_start_time = std::chrono::system_clock::now();
}

UnifiedKolosalServer::ServerConfig UnifiedKolosalServer::get_Configuration() const {
    return config_;
}

bool UnifiedKolosalServer::reload_Configuration(const std::string& config_file) {
    if (!config_file.empty()) {
        config_.agent_config_file = config_file;
    }
    
    // Reload agent configuration if agent manager exists
    if (agent_manager_) {
        return agent_manager_->load_configuration(config_.agent_config_file);
    }
    
    return true;
}

void UnifiedKolosalServer::update_Configuration(const ServerConfig& config) {
    config_ = config;
}

std::shared_ptr<KolosalServerClient> UnifiedKolosalServer::getLLMServer_Client() const {
    return llm_server_client_;
}

std::shared_ptr<YAMLConfigurableAgentManager> UnifiedKolosalServer::getAgent_Manager() const {
    return agent_manager_;
}

std::shared_ptr<AgentService> UnifiedKolosalServer::getAgent_Service() const {
    return agent_service_;
}

// Factory methods
std::unique_ptr<UnifiedKolosalServer> UnifiedServerFactory::createDefault_Server() {
    return std::make_unique<UnifiedKolosalServer>(buildDefault_Config());
}

std::unique_ptr<UnifiedKolosalServer> UnifiedServerFactory::createFromConfig_File(const std::string& config_file) {
    auto config = buildDefault_Config();
    config.agent_config_file = config_file;
    return std::make_unique<UnifiedKolosalServer>(config);
}

std::unique_ptr<UnifiedKolosalServer> UnifiedServerFactory::createProduction_Server(int port) {
    return std::make_unique<UnifiedKolosalServer>(buildProduction_Config(port));
}

std::unique_ptr<UnifiedKolosalServer> UnifiedServerFactory::createDevelopment_Server(int port) {
    return std::make_unique<UnifiedKolosalServer>(buildDevelopment_Config(port));
}

UnifiedKolosalServer::ServerConfig UnifiedServerFactory::buildDefault_Config() {
    UnifiedKolosalServer::ServerConfig config;
    config.server_port = 8080;        // LLM server port
    config.agent_api_port = 8081;     // Agent API port
    config.auto_start_server = true;
    config.auto_start_agents = true;
    config.enable_agent_api = true;
    config.enable_health_monitoring = true;
    config.enable_metrics_collection = true;
    return config;
}

UnifiedKolosalServer::ServerConfig UnifiedServerFactory::buildProduction_Config(int port) {
    auto config = buildDefault_Config();
    config.server_port = port;
    config.agent_api_port = port + 1;  // Use next port for agent API
    config.health_check_interval = std::chrono::seconds(60);
    config.server_startup_timeout_seconds = 120;
    return config;
}

UnifiedKolosalServer::ServerConfig UnifiedServerFactory::buildDevelopment_Config(int port) {
    auto config = buildDefault_Config();
    config.server_port = port;
    config.agent_api_port = port + 1;  // Use next port for agent API
    config.health_check_interval = std::chrono::seconds(10);
    config.server_startup_timeout_seconds = 30;
    return config;
}

} // namespace kolosal::integration
