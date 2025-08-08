#include "../../include/integration/unified_server.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <json.hpp>

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
    
    // Initialize agent manager
    agent_manager_ = std::make_shared<YAMLConfigurableAgentManager>();
    
    // Initialize agent service
    agent_service_ = std::make_shared<AgentService>(agent_manager_);
    
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
    
    logEvent("INFO", "Starting unified Kolosal server...");
    
    try {
        // Start LLM server if configured
        if (config_.auto_start_server) {
            if (!startLLMServer()) {
                logEvent("ERROR", "Failed to start LLM server");
                return false;
            }
        } else {
            // Create client for external server
            std::string server_url = "http://" + config_.server_host + ":" + std::to_string(config_.server_port);
            llm_server_client_ = std::make_shared<KolosalServerClient>(server_url);
            
            // Test connection
            if (!llm_server_client_->waitForServerReady(10)) {
                logEvent("ERROR", "Cannot connect to external LLM server at " + server_url);
                return false;
            }
        }
        
        // Start agent system
        if (config_.auto_start_agents) {
            if (!startAgentSystem()) {
                logEvent("ERROR", "Failed to start agent system");
                if (config_.auto_start_server) {
                    stopLLMServer();
                }
                return false;
            }
        }
        
        // Start health monitoring
        if (config_.enable_health_monitoring) {
            health_monitoring_active_ = true;
            health_monitoring_thread_ = std::thread(&UnifiedKolosalServer::healthMonitoringLoop, this);
        }
        
        // Start agent service health monitoring
        if (agent_service_) {
            agent_service_->startHealthMonitoring(config_.health_check_interval);
        }
        
        running_ = true;
        logEvent("INFO", "Unified Kolosal server started successfully");
        return true;
        
    } catch (const std::exception& e) {
        logEvent("ERROR", "Exception during server startup: " + std::string(e.what()));
        return false;
    }
}

void UnifiedKolosalServer::stop() {
    if (!running_.load()) {
        return;
    }
    
    logEvent("INFO", "Stopping unified Kolosal server...");
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
    
    // Stop agent system
    stopAgentSystem();
    
    // Stop LLM server if we started it
    if (server_started_by_us_.load()) {
        stopLLMServer();
    }
    
    logEvent("INFO", "Unified Kolosal server stopped");
}

bool UnifiedKolosalServer::isRunning() const {
    return running_.load();
}

UnifiedKolosalServer::SystemStatus UnifiedKolosalServer::getSystemStatus() const {
    std::lock_guard<std::mutex> lock(status_mutex_);
    return current_status_;
}

std::string UnifiedKolosalServer::getSystemStatusJson() const {
    auto status = getSystemStatus();
    
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

bool UnifiedKolosalServer::performHealthCheck() {
    bool llm_healthy = performLLMServerHealthCheck();
    bool agent_healthy = performAgentSystemHealthCheck();
    
    std::lock_guard<std::mutex> lock(status_mutex_);
    current_status_.llm_server_healthy = llm_healthy;
    current_status_.llm_server_running = (llm_server_client_ != nullptr);
    current_status_.agent_system_running = (agent_manager_ && agent_manager_->is_running());
    current_status_.last_health_check = std::chrono::system_clock::now();
    
    if (agent_manager_) {
        auto agent_ids = agent_manager_->list_agents();
        current_status_.total_agents = static_cast<int>(agent_ids.size());
        
        int running_count = 0;
        for (const auto& agent_id : agent_ids) {
            auto agent = agent_manager_->get_agent(agent_id);
            if (agent && agent->is_running()) {
                running_count++;
            }
        }
        current_status_.running_agents = running_count;
    }
    
    return llm_healthy && agent_healthy;
}

bool UnifiedKolosalServer::startLLMServer() {
    logEvent("INFO", "Starting LLM server...");
    
    // Find server executable if not specified
    std::string server_path = config_.server_executable_path;
    if (server_path.empty()) {
        server_path = findServerExecutable();
        if (server_path.empty()) {
            logEvent("ERROR", "Kolosal-server executable not found");
            return false;
        }
    }
    
    // Start server process
    if (!startServerProcess(server_path)) {
        return false;
    }
    
    // Create client
    std::string server_url = "http://" + config_.server_host + ":" + std::to_string(config_.server_port);
    llm_server_client_ = std::make_shared<KolosalServerClient>(server_url);
    
    // Wait for server to be ready
    if (!llm_server_client_->waitForServerReady(config_.server_startup_timeout_seconds)) {
        logEvent("ERROR", "LLM server failed to become ready within timeout");
        stopServerProcess();
        return false;
    }
    
    server_started_by_us_ = true;
    logEvent("INFO", "LLM server started successfully");
    return true;
}

bool UnifiedKolosalServer::startAgentSystem() {
    logEvent("INFO", "Starting agent system...");
    
    if (!agent_manager_) {
        logEvent("ERROR", "Agent manager not initialized");
        return false;
    }
    
    // Load configuration
    if (!agent_manager_->load_configuration(config_.agent_config_file)) {
        logEvent("ERROR", "Failed to load agent configuration from: " + config_.agent_config_file);
        return false;
    }
    
    // Start the agent system
    try {
        agent_manager_->start();
        logEvent("INFO", "Agent system started successfully");
        return true;
    } catch (const std::exception& e) {
        logEvent("ERROR", "Exception starting agent system: " + std::string(e.what()));
        return false;
    }
}

void UnifiedKolosalServer::stopLLMServer() {
    if (llm_server_client_) {
        logEvent("INFO", "Shutting down LLM server...");
        llm_server_client_->shutdownServer();
    }
    
    if (server_started_by_us_.load()) {
        stopServerProcess();
        server_started_by_us_ = false;
    }
    
    llm_server_client_.reset();
}

void UnifiedKolosalServer::stopAgentSystem() {
    if (agent_manager_) {
        logEvent("INFO", "Stopping agent system...");
        agent_manager_->stop();
    }
}

void UnifiedKolosalServer::healthMonitoringLoop() {
    while (health_monitoring_active_.load() && running_.load()) {
        try {
            bool healthy = performHealthCheck();
            
            if (!healthy) {
                logEvent("WARN", "Health check failed");
                
                if (auto_recovery_enabled_.load()) {
                    if (recovery_attempts_.load() < MAX_RECOVERY_ATTEMPTS) {
                        logEvent("INFO", "Attempting auto-recovery...");
                        recovery_attempts_++;
                        
                        // Implement recovery logic here
                        // For now, just log the attempt
                    }
                }
            } else {
                recovery_attempts_ = 0; // Reset on successful health check
            }
            
            // Update metrics
            updateMetrics();
            
            // Notify callback if set
            if (health_callback_) {
                health_callback_(getSystemStatus());
            }
            
        } catch (const std::exception& e) {
            logEvent("ERROR", "Exception in health monitoring: " + std::string(e.what()));
        }
        
        std::this_thread::sleep_for(config_.health_check_interval);
    }
}

bool UnifiedKolosalServer::performLLMServerHealthCheck() {
    if (!llm_server_client_) {
        return false;
    }
    
    return llm_server_client_->isServerHealthy();
}

bool UnifiedKolosalServer::performAgentSystemHealthCheck() {
    if (!agent_manager_) {
        return false;
    }
    
    return agent_manager_->is_running();
}

void UnifiedKolosalServer::updateMetrics() {
    // This would update various metrics
    // Implementation depends on how metrics are collected
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    // Update response times
    if (agent_service_) {
        auto system_metrics = agent_service_->getSystemMetrics();
        std::lock_guard<std::mutex> status_lock(status_mutex_);
        current_status_.average_response_time_ms = system_metrics.average_response_time_ms;
    }
}

void UnifiedKolosalServer::logEvent(const std::string& level, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    char timestamp[64];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);
    
    std::cout << "[" << timestamp << "] [" << level << "] " << message << std::endl;
}

// Platform-specific server process management
std::string UnifiedKolosalServer::findServerExecutable() const {
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
            auto server_path = search_path / server_name;
            if (std::filesystem::exists(server_path)) {
                return server_path.string();
            }
        }
    }
    
    return "";
}

bool UnifiedKolosalServer::startServerProcess(const std::string& server_path) {
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
        logEvent("ERROR", "Failed to start server process. Error: " + std::to_string(GetLastError()));
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
        logEvent("ERROR", "Failed to fork server process");
        return false;
    }
#endif
}

void UnifiedKolosalServer::stopServerProcess() {
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

bool UnifiedKolosalServer::enableAutoRecovery(bool enable) {
    auto_recovery_enabled_ = enable;
    return true;
}

void UnifiedKolosalServer::setHealthCheckCallback(std::function<void(const SystemStatus&)> callback) {
    health_callback_ = callback;
}

UnifiedKolosalServer::ServerMetrics UnifiedKolosalServer::getMetrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return metrics_;
}

void UnifiedKolosalServer::resetMetrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_ = ServerMetrics{};
    metrics_.metrics_start_time = std::chrono::system_clock::now();
}

std::shared_ptr<KolosalServerClient> UnifiedKolosalServer::getLLMServerClient() const {
    return llm_server_client_;
}

std::shared_ptr<YAMLConfigurableAgentManager> UnifiedKolosalServer::getAgentManager() const {
    return agent_manager_;
}

std::shared_ptr<AgentService> UnifiedKolosalServer::getAgentService() const {
    return agent_service_;
}

// Factory methods
std::unique_ptr<UnifiedKolosalServer> UnifiedServerFactory::createDefaultServer() {
    return std::make_unique<UnifiedKolosalServer>(buildDefaultConfig());
}

std::unique_ptr<UnifiedKolosalServer> UnifiedServerFactory::createProductionServer(int port) {
    return std::make_unique<UnifiedKolosalServer>(buildProductionConfig(port));
}

std::unique_ptr<UnifiedKolosalServer> UnifiedServerFactory::createDevelopmentServer(int port) {
    return std::make_unique<UnifiedKolosalServer>(buildDevelopmentConfig(port));
}

UnifiedKolosalServer::ServerConfig UnifiedServerFactory::buildDefaultConfig() {
    UnifiedKolosalServer::ServerConfig config;
    config.server_port = 8080;
    config.auto_start_server = true;
    config.auto_start_agents = true;
    config.enable_health_monitoring = true;
    config.enable_metrics_collection = true;
    return config;
}

UnifiedKolosalServer::ServerConfig UnifiedServerFactory::buildProductionConfig(int port) {
    auto config = buildDefaultConfig();
    config.server_port = port;
    config.health_check_interval = std::chrono::seconds(60);
    config.server_startup_timeout_seconds = 120;
    return config;
}

UnifiedKolosalServer::ServerConfig UnifiedServerFactory::buildDevelopmentConfig(int port) {
    auto config = buildDefaultConfig();
    config.server_port = port;
    config.health_check_interval = std::chrono::seconds(10);
    config.server_startup_timeout_seconds = 30;
    return config;
}

} // namespace kolosal::integration
