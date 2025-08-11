/**
 * @file agent_service.cpp
 * @brief Implementation of high-level agent service operations
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 */

#include "agent/services/agent_service.hpp"
#include "agent/core/multi_agent_system.hpp"

#ifdef MCP_PROTOCOL_ENABLED
#include "mcp/transport/stdio_transport.hpp"
#include "mcp/utils/logging.hpp"
#endif

#include <algorithm>
#include <random>
#include <chrono>
#include <stdexcept>

namespace kolosal::services {

// Constructor
AgentService::AgentService(std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager)
    : agent_manager_(agent_manager) {
    if (!agent_manager_) {
        throw std::invalid_argument("AgentManager cannot be null");
    }
}

// Destructor
AgentService::~AgentService() {
    stopHealthMonitoring();
}

// Get all agent information
std::vector<AgentService::AgentInfo> AgentService::getAllAgentInfo() {
    std::vector<AgentInfo> agent_infos;
    
    if (!agent_manager_) {
        return agent_infos;
    }
    
    try {
        auto agent_ids = agent_manager_->list_agents();
        
        for (const auto& agent_id : agent_ids) {
            try {
                auto agent = agent_manager_->get__agent(agent_id);
                if (agent) {
                    agent_infos.push_back(createAgentInfo(agent_id, agent));
                }
            } catch (const std::exception&) {
                // Skip invalid agents
                continue;
            }
        }
    } catch (const std::exception&) {
        // Return empty vector on error
    }
    
    return agent_infos;
}

// Get system metrics
AgentService::SystemMetrics AgentService::getSystemMetrics() {
    SystemMetrics metrics = {};
    
    if (!agent_manager_) {
        return metrics;
    }
    
    try {
        auto agent_ids = agent_manager_->list_agents();
        metrics.total_agents = agent_ids.size();
        
        size_t running_count = 0;
        for (const auto& agent_id : agent_ids) {
            try {
                auto agent = agent_manager_->get__agent(agent_id);
                if (agent && agent->is__running()) {
                    running_count++;
                }
            } catch (const std::exception&) {
                continue;
            }
        }
        
        metrics.running_agents = running_count;
        
        // Calculate performance metrics from execution history
        std::lock_guard<std::mutex> lock(performance_mutex_);
        size_t total_executions = 0;
        double total_time = 0.0;
        
        for (const auto& [agent_id, executions] : execution_history_) {
            for (const auto& execution : executions) {
                total_executions++;
                total_time += execution.execution_time_ms;
                if (execution.success) {
                    metrics.total_functions_executed++;
                }
            }
        }
        
        if (total_executions > 0) {
            metrics.average_response_time_ms = total_time / total_executions;
        }
        
        metrics.last_updated = std::chrono::system_clock::now();
    } catch (const std::exception&) {
        // Return default metrics on error
    }
    
    return metrics;
}

// Start health monitoring
void AgentService::startHealthMonitoring(std::chrono::seconds interval) {
    if (health_monitoring_active_.load()) {
        return; // Already running
    }
    
    health_monitoring_active_.store(true);
    health_monitoring_thread_ = std::thread([this, interval]() {
        healthMonitoringLoop(interval);
    });
}

// Stop health monitoring
void AgentService::stopHealthMonitoring() {
    if (health_monitoring_active_.load()) {
        health_monitoring_active_.store(false);
        if (health_monitoring_thread_.joinable()) {
            health_monitoring_thread_.join();
        }
    }
}

// Async agent creation
std::future<std::string> AgentService::createAgentAsync(const kolosal::agents::AgentConfig& config) {
    return std::async(std::launch::async, [this, config]() -> std::string {
        if (!agent_manager_) {
            throw std::runtime_error("Agent manager not available");
        }
        
        try {
            auto agent_id = agent_manager_->create__agent_from_config(config);
            notifyEvent("agent_created", agent_id, config);
            return agent_id;
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to create agent: " + std::string(e.what()));
        }
    });
}

// Async agent start
std::future<bool> AgentService::startAgentAsync(const std::string& agent_id) {
    return std::async(std::launch::async, [this, agent_id]() -> bool {
        if (!agent_manager_) {
            return false;
        }
        
        try {
            bool result = agent_manager_->start_agent(agent_id);
            if (result) {
                notifyEvent("agent_started", agent_id, std::string("success"));
            }
            return result;
        } catch (const std::exception&) {
            return false;
        }
    });
}

// Async agent stop
std::future<bool> AgentService::stopAgentAsync(const std::string& agent_id) {
    return std::async(std::launch::async, [this, agent_id]() -> bool {
        if (!agent_manager_) {
            return false;
        }
        
        try {
            bool result = agent_manager_->stop_agent(agent_id);
            if (result) {
                notifyEvent("agent_stopped", agent_id, std::string("success"));
            }
            return result;
        } catch (const std::exception&) {
            return false;
        }
    });
}

// Async agent deletion
std::future<bool> AgentService::deleteAgentAsync(const std::string& agent_id) {
    return std::async(std::launch::async, [this, agent_id]() -> bool {
        if (!agent_manager_) {
            return false;
        }
        
        try {
            bool result = agent_manager_->delete_agent(agent_id);
            if (result) {
                // Clean up execution history
                std::lock_guard<std::mutex> lock(performance_mutex_);
                execution_history_.erase(agent_id);
                notifyEvent("agent_deleted", agent_id, std::string("success"));
            }
            return result;
        } catch (const std::exception&) {
            return false;
        }
    });
}

// Async agent restart
std::future<bool> AgentService::restartAgentAsync(const std::string& agent_id) {
    return std::async(std::launch::async, [this, agent_id]() -> bool {
        if (!agent_manager_) {
            return false;
        }
        
        try {
            // Stop then start
            bool stop_result = agent_manager_->stop_agent(agent_id);
            if (!stop_result) {
                return false;
            }
            
            // Small delay to ensure clean restart
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            bool start_result = agent_manager_->start_agent(agent_id);
            if (start_result) {
                notifyEvent("agent_restarted", agent_id, std::string("success"));
            }
            return start_result;
        } catch (const std::exception&) {
            return false;
        }
    });
}

// Get single agent info
std::optional<AgentService::AgentInfo> AgentService::getAgentInfo(const std::string& agent_id) {
    if (!agent_manager_) {
        return std::nullopt;
    }
    
    try {
        auto agent = agent_manager_->get__agent(agent_id);
        if (!agent) {
            return std::nullopt;
        }
        
        return createAgentInfo(agent_id, agent);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

// Check if agent is healthy
bool AgentService::isAgentHealthy(const std::string& agent_id) {
    if (!agent_manager_) {
        return false;
    }
    
    try {
        auto agent = agent_manager_->get__agent(agent_id);
        return agent && agent->is__running();
    } catch (const std::exception&) {
        return false;
    }
}

// Get unhealthy agents
std::vector<std::string> AgentService::getUnhealthyAgents() {
    std::vector<std::string> unhealthy_agents;
    
    if (!agent_manager_) {
        return unhealthy_agents;
    }
    
    try {
        auto agent_ids = agent_manager_->list_agents();
        
        for (const auto& agent_id : agent_ids) {
            if (!isAgentHealthy(agent_id)) {
                unhealthy_agents.push_back(agent_id);
            }
        }
    } catch (const std::exception&) {
        // Return empty vector on error
    }
    
    return unhealthy_agents;
}

// Execute function asynchronously
std::future<AgentService::ExecutionResult> AgentService::executeFunctionAsync(
    const std::string& agent_id,
    const std::string& function_name,
    const kolosal::agents::AgentData& parameters,
    const int priority) {
    
    return std::async(std::launch::async, [this, agent_id, function_name, parameters, priority]() -> ExecutionResult {
        ExecutionResult result;
        result.timestamp = std::chrono::system_clock::now();
        result.execution_id = generateExecutionId();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        if (!agent_manager_) {
            result.success = false;
            result.message = "Agent manager not available";
            return result;
        }
        
        try {
            auto agent = agent_manager_->get__agent(agent_id);
            if (!agent) {
                result.success = false;
                result.message = "Agent not found: " + agent_id;
                return result;
            }
            
            // Execute the function
            auto function_result = agent->execute_function(function_name, parameters);
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            result.execution_time_ms = duration.count() / 1000.0;
            
            result.success = true;
            result.message = "Function executed successfully";
            result.result_data = function_result;
            
            // Record execution for performance tracking
            recordExecution(agent_id, result);
            
        } catch (const std::exception& e) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            result.execution_time_ms = duration.count() / 1000.0;
            
            result.success = false;
            result.message = "Function execution failed: " + std::string(e.what());
            
            // Still record execution for performance tracking
            recordExecution(agent_id, result);
        }
        
        return result;
    });
}

// Create multiple agents async
std::future<std::vector<std::string>> AgentService::createMultipleAgentsAsync(
    const std::vector<kolosal::agents::AgentConfig>& configs) {
    
    return std::async(std::launch::async, [this, configs]() -> std::vector<std::string> {
        std::vector<std::string> agent_ids;
        
        for (const auto& config : configs) {
            try {
                auto future = createAgentAsync(config);
                auto agent_id = future.get();
                agent_ids.push_back(agent_id);
            } catch (const std::exception&) {
                // Continue with other agents even if one fails
                continue;
            }
        }
        
        return agent_ids;
    });
}

// Start multiple agents async
std::future<std::vector<bool>> AgentService::startMultipleAgentsAsync(
    const std::vector<std::string>& agent_ids) {
    
    return std::async(std::launch::async, [this, agent_ids]() -> std::vector<bool> {
        std::vector<std::future<bool>> futures;
        
        for (const auto& agent_id : agent_ids) {
            futures.push_back(startAgentAsync(agent_id));
        }
        
        std::vector<bool> results;
        for (auto& future : futures) {
            try {
                results.push_back(future.get());
            } catch (const std::exception&) {
                results.push_back(false);
            }
        }
        
        return results;
    });
}

// Stop multiple agents async
std::future<std::vector<bool>> AgentService::stopMultipleAgentsAsync(
    const std::vector<std::string>& agent_ids) {
    
    return std::async(std::launch::async, [this, agent_ids]() -> std::vector<bool> {
        std::vector<std::future<bool>> futures;
        
        for (const auto& agent_id : agent_ids) {
            futures.push_back(stopAgentAsync(agent_id));
        }
        
        std::vector<bool> results;
        for (auto& future : futures) {
            try {
                results.push_back(future.get());
            } catch (const std::exception&) {
                results.push_back(false);
            }
        }
        
        return results;
    });
}

// Execute function on multiple agents
std::future<std::vector<AgentService::ExecutionResult>> 
AgentService::executeFunctionOnMultipleAgentsAsync(
    const std::vector<std::string>& agent_ids,
    const std::string& function_name,
    const kolosal::agents::AgentData& parameters) {
    
    return std::async(std::launch::async, [this, agent_ids, function_name, parameters]() -> std::vector<ExecutionResult> {
        std::vector<std::future<ExecutionResult>> futures;
        
        for (const auto& agent_id : agent_ids) {
            futures.push_back(executeFunctionAsync(agent_id, function_name, parameters));
        }
        
        std::vector<ExecutionResult> results;
        for (auto& future : futures) {
            try {
                results.push_back(future.get());
            } catch (const std::exception& e) {
                ExecutionResult error_result;
                error_result.success = false;
                error_result.message = "Execution failed: " + std::string(e.what());
                error_result.timestamp = std::chrono::system_clock::now();
                results.push_back(error_result);
            }
        }
        
        return results;
    });
}

// Template management
bool AgentService::saveAgentTemplate(const std::string& template_name, const kolosal::agents::AgentConfig& config) {
    std::lock_guard<std::mutex> lock(templates_mutex_);
    agent_templates_[template_name] = config;
    return true;
}

std::optional<kolosal::agents::AgentConfig> AgentService::getAgentTemplate(const std::string& template_name) {
    std::lock_guard<std::mutex> lock(templates_mutex_);
    auto it = agent_templates_.find(template_name);
    if (it != agent_templates_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<std::string> AgentService::getAvailableTemplates() {
    std::lock_guard<std::mutex> lock(templates_mutex_);
    std::vector<std::string> template_names;
    for (const auto& [name, config] : agent_templates_) {
        template_names.push_back(name);
    }
    return template_names;
}

bool AgentService::deleteAgentTemplate(const std::string& template_name) {
    std::lock_guard<std::mutex> lock(templates_mutex_);
    return agent_templates_.erase(template_name) > 0;
}

// Notification system
void AgentService::registerNotificationCallback(const std::string& event_type, NotificationCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    notification_callbacks_[event_type] = callback;
}

void AgentService::unregisterNotificationCallback(const std::string& event_type) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    notification_callbacks_.erase(event_type);
}

// Check if health monitoring is active
bool AgentService::isHealthMonitoringActive() const {
    return health_monitoring_active_.load();
}

// Generate performance reports
std::vector<AgentService::PerformanceReport> AgentService::generatePerformanceReport(
    const std::vector<std::string>& agent_ids,
    std::chrono::hours time_window) {
    
    std::vector<PerformanceReport> reports;
    std::lock_guard<std::mutex> lock(performance_mutex_);
    
    auto cutoff_time = std::chrono::system_clock::now() - time_window;
    
    auto ids_to_process = agent_ids;
    if (ids_to_process.empty()) {
        // Get all agent IDs if none specified
        for (const auto& [agent_id, executions] : execution_history_) {
            ids_to_process.push_back(agent_id);
        }
    }
    
    for (const auto& agent_id : ids_to_process) {
        auto it = execution_history_.find(agent_id);
        if (it == execution_history_.end()) {
            continue;
        }
        
        PerformanceReport report;
        report.agent_id = agent_id;
        report.report_timestamp = std::chrono::system_clock::now();
        
        double total_time = 0.0;
        size_t successful = 0;
        size_t failed = 0;
        
        for (const auto& execution : it->second) {
            if (execution.timestamp < cutoff_time) {
                continue;
            }
            
            total_time += execution.execution_time_ms;
            if (execution.success) {
                successful++;
            } else {
                failed++;
            }
        }
        
        size_t total = successful + failed;
        if (total > 0) {
            report.average_execution_time_ms = total_time / total;
            report.successful_executions = successful;
            report.failed_executions = failed;
            report.success_rate = static_cast<double>(successful) / total;
        }
        
        reports.push_back(report);
    }
    
    return reports;
}

// System optimization analysis
std::vector<AgentService::OptimizationSuggestion> AgentService::analyzeSystem_Optimization() {
    std::vector<OptimizationSuggestion> suggestions;
    
    // This is a placeholder implementation
    // In a real system, you would analyze agent performance, resource usage, etc.
    
    OptimizationSuggestion memory_suggestion;
    memory_suggestion.type = "memory";
    memory_suggestion.description = "Consider optimizing memory usage for high-frequency agents";
    memory_suggestion.potential_improvement_percent = 15.0;
    suggestions.push_back(memory_suggestion);
    
    return suggestions;
}

bool AgentService::applyOptimization_Suggestion(const OptimizationSuggestion& suggestion) {
    // This is a placeholder implementation
    // In a real system, you would apply the actual optimization
    return true;
}

// Private helper methods

void AgentService::healthMonitoringLoop(std::chrono::seconds interval) {
    while (health_monitoring_active_.load()) {
        try {
            // Check agent health
            auto unhealthy_agents = getUnhealthyAgents();
            
            if (!unhealthy_agents.empty()) {
                for (const auto& agent_id : unhealthy_agents) {
                    notifyEvent("agent_unhealthy", agent_id, std::string("health_check_failed"));
                }
            }
            
            // Sleep for the specified interval
            auto sleep_start = std::chrono::steady_clock::now();
            while (health_monitoring_active_.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (std::chrono::steady_clock::now() - sleep_start >= interval) {
                    break;
                }
            }
        } catch (const std::exception&) {
            // Continue monitoring even if individual checks fail
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void AgentService::notifyEvent(const std::string& event_type, const std::string& agent_id, const std::any& data) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    
    auto it = notification_callbacks_.find(event_type);
    if (it != notification_callbacks_.end()) {
        try {
            it->second(event_type, agent_id, data);
        } catch (const std::exception&) {
            // Swallow callback exceptions to prevent system disruption
        }
    }
}

void AgentService::recordExecution(const std::string& agent_id, const ExecutionResult& result) {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    
    auto& history = execution_history_[agent_id];
    history.push_back(result);
    
    // Limit history size to prevent memory growth
    const size_t max_history_size = 1000;
    if (history.size() > max_history_size) {
        history.erase(history.begin(), history.begin() + (history.size() - max_history_size));
    }
}

AgentService::AgentInfo AgentService::createAgentInfo(
    const std::string& agent_id, 
    std::shared_ptr<kolosal::agents::AgentCore> agent) {
    
    AgentInfo info;
    info.id = agent_id;
    info.name = agent->get__agent_name().empty() ? agent_id : agent->get__agent_name();
    info.type = agent->get__agent_type();
    info.role = agent->get__role();
    info.specializations = agent->get__specializations();
    info.capabilities = agent->get__capabilities();
    info.running = agent->is__running();
    info.created_at = std::chrono::system_clock::now(); // Placeholder
    info.last_modified = std::chrono::system_clock::now(); // Placeholder
    
    try {
        info.statistics = agent->get__statistics();
    } catch (const std::exception&) {
        // Use default statistics if retrieval fails
        info.statistics = kolosal::agents::AgentCore::AgentStats{};
    }
    
    return info;
}

std::string AgentService::generateExecutionId() {
    static std::atomic<uint64_t> counter{0};
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return "exec_" + std::to_string(timestamp) + "_" + std::to_string(counter.fetch_add(1));
}

#ifdef MCP_PROTOCOL_ENABLED

// MCP Integration implementation

bool AgentService::createMCPAdapter(const std::string& agent_id, 
                                   const kolosal::services::MCPAgentAdapter::MCPConfig& config) {
    if (!agent_manager_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mcp_mutex_);
    
    // Check if adapter already exists
    if (mcp_adapters_.find(agent_id) != mcp_adapters_.end()) {
        return true; // Already exists
    }
    
    try {
        // Get the agent
        auto agent = agent_manager_->get__agent(agent_id);
        if (!agent) {
            return false;
        }
        
        // Create MCP adapter
        auto adapter = std::make_shared<kolosal::services::MCPAgentAdapter>(agent, config);
        
        // Store the adapter
        mcp_adapters_[agent_id] = adapter;
        
        return true;
        
    } catch (const std::exception& e) {
        // Log error if logger is available
        return false;
    }
}

bool AgentService::startAgentMCPServer(const std::string& agent_id, const std::string& transport_name) {
    std::lock_guard<std::mutex> lock(mcp_mutex_);
    
    auto it = mcp_adapters_.find(agent_id);
    if (it == mcp_adapters_.end()) {
        // Try to create adapter first
        if (!createMCPAdapter(agent_id)) {
            return false;
        }
        it = mcp_adapters_.find(agent_id);
    }
    
    if (it == mcp_adapters_.end()) {
        return false;
    }
    
    try {
        // Create transport based on name
        std::shared_ptr<mcp::transport::Transport> transport;
        
        if (transport_name == "stdio" || transport_name.empty()) {
            transport = std::make_shared<mcp::transport::StdioTransport>();
        } else {
            // Add support for other transport types here
            return false;
        }
        
        // Start the server
        return it->second->startServer(transport);
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool AgentService::stopAgentMCPServer(const std::string& agent_id) {
    std::lock_guard<std::mutex> lock(mcp_mutex_);
    
    auto it = mcp_adapters_.find(agent_id);
    if (it == mcp_adapters_.end()) {
        return false;
    }
    
    try {
        it->second->stopServer();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::future<bool> AgentService::connectAgentToMCPServer(const std::string& agent_id, 
                                                       const std::string& server_endpoint) {
    return std::async(std::launch::async, [this, agent_id, server_endpoint]() -> bool {
        std::lock_guard<std::mutex> lock(mcp_mutex_);
        
        auto it = mcp_adapters_.find(agent_id);
        if (it == mcp_adapters_.end()) {
            // Try to create adapter first
            if (!createMCPAdapter(agent_id)) {
                return false;
            }
            it = mcp_adapters_.find(agent_id);
        }
        
        if (it == mcp_adapters_.end()) {
            return false;
        }
        
        try {
            // Create transport based on endpoint
            std::shared_ptr<mcp::transport::Transport> transport;
            
            // For simplicity, assume stdio transport
            // In a real implementation, you'd parse the endpoint to determine transport type
            transport = std::make_shared<mcp::transport::StdioTransport>();
            
            // Initialize client connection
            auto future = it->second->initializeClient(transport);
            
            // Wait for connection
            auto status = future.wait_for(std::chrono::seconds(30));
            if (status == std::future_status::ready) {
                try {
                    auto result = future.get();
                    return true; // Connection successful
                } catch (const std::exception& e) {
                    return false;
                }
            }
            
            return false; // Timeout
            
        } catch (const std::exception& e) {
            return false;
        }
    });
}

std::shared_ptr<kolosal::services::MCPAgentAdapter> AgentService::getMCPAdapter(const std::string& agent_id) {
    std::lock_guard<std::mutex> lock(mcp_mutex_);
    
    auto it = mcp_adapters_.find(agent_id);
    if (it != mcp_adapters_.end()) {
        return it->second;
    }
    
    return nullptr;
}

std::vector<std::string> AgentService::getAgentsWithMCP() const {
    std::lock_guard<std::mutex> lock(mcp_mutex_);
    
    std::vector<std::string> result;
    result.reserve(mcp_adapters_.size());
    
    for (const auto& [agent_id, adapter] : mcp_adapters_) {
        result.push_back(agent_id);
    }
    
    return result;
}

size_t AgentService::autoSetupMCPForAllAgents(bool auto_register_functions) {
    if (!agent_manager_) {
        return 0;
    }
    
    size_t setup_count = 0;
    auto agent_ids = agent_manager_->list_agents();
    
    for (const auto& agent_id : agent_ids) {
        try {
            kolosal::services::MCPAgentAdapter::MCPConfig config;
            config.server_name = "kolosal-agent-" + agent_id;
            
            if (createMCPAdapter(agent_id, config)) {
                // Get the adapter and auto-register if requested
                if (auto_register_functions) {
                    auto adapter = getMCPAdapter(agent_id);
                    if (adapter) {
                        adapter->autoRegisterAgentFunctions(false);
                        adapter->autoRegisterAgentMemory();
                        adapter->autoRegisterAgentCapabilities();
                    }
                }
                setup_count++;
            }
            
        } catch (const std::exception& e) {
            // Continue with next agent
            continue;
        }
    }
    
    return setup_count;
}

#endif // MCP_PROTOCOL_ENABLED

} // namespace kolosal::services