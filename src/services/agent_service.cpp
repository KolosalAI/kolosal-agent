#include "../include/services/agent_service.hpp"
#include "../include/agent/multi_agent_system.hpp"
#include <algorithm>
#include <numeric>
#include <chrono>
#include <thread>
#include <optional>

using namespace kolosal::agents;

namespace kolosal::services {

AgentService::AgentService(std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager)
    : agent_manager_(agent_manager) {}

AgentService::~AgentService() {
    stopHealthMonitoring();
}

std::future<std::string> AgentService::createAgentAsync(const AgentConfig& config) {
    return std::async(std::launch::async, [this, config]() -> std::string {
        try {
            std::string agent_id = agent_manager_->create_agent_from_config(config);
            if (!agent_id.empty()) {
                notifyEvent("agent_created", agent_id, config);
            }
            return agent_id;
        } catch (const std::exception& e) {
            notifyEvent("agent_creation_failed", "", std::string(e.what()));
            return "";
        }
    });
}

std::future<bool> AgentService::startAgentAsync(const std::string& agent_id) {
    return std::async(std::launch::async, [this, agent_id]() -> bool {
        try {
            bool result = agent_manager_->start_agent(agent_id);
            if (result) {
                notifyEvent("agent_started", agent_id, std::string("started"));
            } else {
                notifyEvent("agent_start_failed", agent_id, std::string("failed to start"));
            }
            return result;
        } catch (const std::exception& e) {
            notifyEvent("agent_start_failed", agent_id, std::string(e.what()));
            return false;
        }
    });
}

std::future<bool> AgentService::stopAgentAsync(const std::string& agent_id) {
    return std::async(std::launch::async, [this, agent_id]() -> bool {
        try {
            bool result = agent_manager_->stop_agent(agent_id);
            if (result) {
                notifyEvent("agent_stopped", agent_id, std::string("stopped"));
            } else {
                notifyEvent("agent_stop_failed", agent_id, std::string("failed to stop"));
            }
            return result;
        } catch (const std::exception& e) {
            notifyEvent("agent_stop_failed", agent_id, std::string(e.what()));
            return false;
        }
    });
}

std::future<bool> AgentService::deleteAgentAsync(const std::string& agent_id) {
    return std::async(std::launch::async, [this, agent_id]() -> bool {
        try {
            bool result = agent_manager_->delete_agent(agent_id);
            if (result) {
                notifyEvent("agent_deleted", agent_id, std::string("deleted"));
            } else {
                notifyEvent("agent_deletion_failed", agent_id, std::string("failed to delete"));
            }
            return result;
        } catch (const std::exception& e) {
            notifyEvent("agent_deletion_failed", agent_id, std::string(e.what()));
            return false;
        }
    });
}

std::future<bool> AgentService::restartAgentAsync(const std::string& agent_id) {
    return std::async(std::launch::async, [this, agent_id]() -> bool {
        try {
            bool stopped = agent_manager_->stop_agent(agent_id);
            if (!stopped) return false;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            bool started = agent_manager_->start_agent(agent_id);
            if (started) {
                notifyEvent("agent_restarted", agent_id, std::string("restarted"));
            }
            return started;
        } catch (const std::exception& e) {
            notifyEvent("agent_restart_failed", agent_id, std::string(e.what()));
            return false;
        }
    });
}

std::future<std::vector<std::string>> AgentService::createMultipleAgentsAsync(
    const std::vector<AgentConfig>& configs) {
    return std::async(std::launch::async, [this, configs]() -> std::vector<std::string> {
        std::vector<std::string> results;
        results.reserve(configs.size());
        
        for (const auto& config : configs) {
            try {
                std::string agent_id = agent_manager_->create_agent_from_config(config);
                results.push_back(agent_id);
                if (!agent_id.empty()) {
                    notifyEvent("agent_created", agent_id, config);
                }
            } catch (const std::exception& e) {
                results.push_back("");
                notifyEvent("agent_creation_failed", "", std::string(e.what()));
            }
        }
        
        return results;
    });
}

std::future<std::vector<bool>> AgentService::startMultipleAgentsAsync(
    const std::vector<std::string>& agent_ids) {
    return std::async(std::launch::async, [this, agent_ids]() -> std::vector<bool> {
        std::vector<std::future<bool>> futures;
        futures.reserve(agent_ids.size());
        
        for (const auto& agent_id : agent_ids) {
            futures.push_back(startAgentAsync(agent_id));
        }
        
        std::vector<bool> results;
        results.reserve(futures.size());
        
        for (auto& future : futures) {
            results.push_back(future.get());
        }
        
        return results;
    });
}

std::future<std::vector<bool>> AgentService::stopMultipleAgentsAsync(
    const std::vector<std::string>& agent_ids) {
    return std::async(std::launch::async, [this, agent_ids]() -> std::vector<bool> {
        std::vector<std::future<bool>> futures;
        futures.reserve(agent_ids.size());
        
        for (const auto& agent_id : agent_ids) {
            futures.push_back(stopAgentAsync(agent_id));
        }
        
        std::vector<bool> results;
        results.reserve(futures.size());
        
        for (auto& future : futures) {
            results.push_back(future.get());
        }
        
        return results;
    });
}

std::vector<AgentService::AgentInfo> AgentService::getAllAgentInfo() {
    std::vector<AgentInfo> infos;
    auto agent_ids = agent_manager_->list_agents();
    infos.reserve(agent_ids.size());
    
    for (const auto& agent_id : agent_ids) {
        auto agent = agent_manager_->get_agent(agent_id);
        if (agent) {
            infos.push_back(createAgentInfo(agent_id, agent));
        }
    }
    
    return infos;
}

std::optional<AgentService::AgentInfo> AgentService::getAgentInfo(const std::string& agent_id) {
    auto agent = agent_manager_->get_agent(agent_id);
    if (!agent) {
        return std::nullopt;
    }
    
    return createAgentInfo(agent_id, agent);
}

AgentService::SystemMetrics AgentService::getSystemMetrics() {
    SystemMetrics metrics{};
    metrics.last_updated = std::chrono::system_clock::now();
    
    auto agent_ids = agent_manager_->list_agents();
    metrics.total_agents = agent_ids.size();
    metrics.running_agents = 0;
    metrics.total_functions_executed = 0;
    metrics.total_plans_created = 0;
    
    std::vector<double> response_times;
    
    for (const auto& agent_id : agent_ids) {
        auto agent = agent_manager_->get_agent(agent_id);
        if (agent) {
            if (agent->is_running()) {
                metrics.running_agents++;
            }
            
            auto stats = agent->get_statistics();
            metrics.total_functions_executed += stats.total_functions_executed;
            metrics.total_plans_created += stats.total_plans_created;
            
            if (stats.average_execution_time_ms > 0) {
                response_times.push_back(stats.average_execution_time_ms);
            }
            
            // Get job counts from job manager
            auto job_manager = agent->get_job_manager();
            if (job_manager) {
                // Note: These methods would need to be implemented in JobManager
                // metrics.active_jobs += job_manager->getActiveJobsCount();
                // metrics.queued_jobs += job_manager->getQueuedJobsCount();
            }
        }
    }
    
    if (!response_times.empty()) {
        metrics.average_response_time_ms = std::accumulate(response_times.begin(), response_times.end(), 0.0) / response_times.size();
    }
    
    return metrics;
}

bool AgentService::isAgentHealthy(const std::string& agent_id) {
    auto agent = agent_manager_->get_agent(agent_id);
    if (!agent) {
        return false;
    }
    
    // Check if agent is running
    if (!agent->is_running()) {
        return false;
    }
    
    // Check last activity (agents should have activity within reasonable time)
    auto stats = agent->get_statistics();
    auto now = std::chrono::system_clock::now();
    auto time_since_activity = now - stats.last_activity;
    
    // If no activity for more than 5 minutes, consider unhealthy
    if (time_since_activity > std::chrono::minutes(5)) {
        return false;
    }
    
    // Additional health checks could be added here
    return true;
}

std::vector<std::string> AgentService::getUnhealthyAgents() {
    std::vector<std::string> unhealthy;
    auto agent_ids = agent_manager_->list_agents();
    
    for (size_t i = 0; i < agent_ids.size(); ++i) {
        const auto& agent_id = agent_ids[i];
        if (!isAgentHealthy(agent_id)) {
            unhealthy.push_back(agent_id);
        }
    }
    
    return unhealthy;
}

std::future<AgentService::ExecutionResult> AgentService::executeFunctionAsync(
    const std::string& agent_id,
    const std::string& function_name,
    const AgentData& parameters,
    int priority) {
    
    return std::async(std::launch::async, [this, agent_id, function_name, parameters, priority]() -> ExecutionResult {
        ExecutionResult result{};
        result.timestamp = std::chrono::system_clock::now();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        try {
            auto agent = agent_manager_->get_agent(agent_id);
            if (!agent) {
                result.success = false;
                result.message = "Agent not found";
                return result;
            }
            
            if (priority > 0) {
                result.execution_id = agent->execute_function_async(function_name, parameters, priority);
                result.success = !result.execution_id.empty();
                result.message = result.success ? "Function queued for execution" : "Failed to queue function";
            } else {
                auto func_result = agent->execute_function(function_name, parameters);
                result.success = func_result.success;
                result.message = func_result.error_message;
                result.result_data = func_result.result_data;
            }
            
        } catch (const std::exception& e) {
            result.success = false;
            result.message = e.what();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        result.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        // Record execution for analytics
        recordExecution(agent_id, result);
        
        return result;
    });
}

std::future<std::vector<AgentService::ExecutionResult>> AgentService::executeFunctionOnMultipleAgentsAsync(
    const std::vector<std::string>& agent_ids,
    const std::string& function_name,
    const AgentData& parameters) {
    
    return std::async(std::launch::async, [this, agent_ids, function_name, parameters]() -> std::vector<ExecutionResult> {
        std::vector<std::future<ExecutionResult>> futures;
        futures.reserve(agent_ids.size());
        
        for (const auto& agent_id : agent_ids) {
            futures.push_back(executeFunctionAsync(agent_id, function_name, parameters));
        }
        
        std::vector<ExecutionResult> results;
        results.reserve(futures.size());
        
        for (auto& future : futures) {
            results.push_back(future.get());
        }
        
        return results;
    });
}

bool AgentService::saveAgentTemplate(const std::string& template_name, const AgentConfig& config) {
    std::lock_guard<std::mutex> lock(templates_mutex_);
    agent_templates_[template_name] = config;
    return true; // In a real implementation, this would persist to storage
}

std::optional<AgentConfig> AgentService::getAgentTemplate(const std::string& template_name) {
    std::lock_guard<std::mutex> lock(templates_mutex_);
    auto it = agent_templates_.find(template_name);
    if (it != agent_templates_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<std::string> AgentService::getAvailableTemplates() {
    std::lock_guard<std::mutex> lock(templates_mutex_);
    std::vector<std::string> templates;
    templates.reserve(agent_templates_.size());
    
    for (const auto& [name, _] : agent_templates_) {
        templates.push_back(name);
    }
    
    return templates;
}

bool AgentService::deleteAgentTemplate(const std::string& template_name) {
    std::lock_guard<std::mutex> lock(templates_mutex_);
    return agent_templates_.erase(template_name) > 0;
}

void AgentService::registerNotificationCallback(const std::string& event_type, NotificationCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    notification_callbacks_[event_type] = callback;
}

void AgentService::unregisterNotificationCallback(const std::string& event_type) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    notification_callbacks_.erase(event_type);
}

void AgentService::startHealthMonitoring(std::chrono::seconds interval) {
    if (health_monitoring_active_.load()) {
        return;
    }
    
    health_monitoring_active_ = true;
    health_monitoring_thread_ = std::thread(&AgentService::healthMonitoringLoop, this, interval);
}

void AgentService::stopHealthMonitoring() {
    if (health_monitoring_active_.load()) {
        health_monitoring_active_ = false;
        if (health_monitoring_thread_.joinable()) {
            health_monitoring_thread_.join();
        }
    }
}

bool AgentService::isHealthMonitoringActive() const {
    return health_monitoring_active_.load();
}

std::vector<AgentService::PerformanceReport> AgentService::generatePerformanceReport(
    const std::vector<std::string>& agent_ids,
    std::chrono::hours time_window) {
    
    std::lock_guard<std::mutex> lock(performance_mutex_);
    std::vector<PerformanceReport> reports;
    
    auto now = std::chrono::system_clock::now();
    auto cutoff_time = now - time_window;
    
    auto target_agents = agent_ids.empty() ? agent_manager_->list_agents() : agent_ids;
    
    for (size_t i = 0; i < target_agents.size(); ++i) {
        const auto& agent_id = target_agents[i];
        PerformanceReport report{};
        report.agent_id = agent_id;
        report.report_timestamp = now;
        
        auto it = execution_history_.find(agent_id);
        if (it != execution_history_.end()) {
            std::vector<ExecutionResult> recent_executions;
            
            for (size_t j = 0; j < it->second.size(); ++j) {
                const auto& result = it->second[j];
                if (result.timestamp >= cutoff_time) {
                    recent_executions.push_back(result);
                }
            }
            
            if (!recent_executions.empty()) {
                size_t successful = std::count_if(recent_executions.begin(), recent_executions.end(),
                    [](const ExecutionResult& r) { return r.success; });
                
                report.successful_executions = successful;
                report.failed_executions = recent_executions.size() - successful;
                report.success_rate = static_cast<double>(successful) / recent_executions.size();
                
                double total_time = std::accumulate(recent_executions.begin(), recent_executions.end(), 0.0,
                    [](double sum, const ExecutionResult& r) { return sum + r.execution_time_ms; });
                
                report.average_execution_time_ms = total_time / recent_executions.size();
            }
        }
        
        reports.push_back(report);
    }
    
    return reports;
}

void AgentService::healthMonitoringLoop(std::chrono::seconds interval) {
    while (health_monitoring_active_.load()) {
        try {
            auto unhealthy_agents = getUnhealthyAgents();
            
            if (!unhealthy_agents.empty()) {
                notifyEvent("unhealthy_agents_detected", "", unhealthy_agents);
            }
            
            auto metrics = getSystemMetrics();
            notifyEvent("system_metrics_updated", "", metrics);
            
        } catch (const std::exception& e) {
            notifyEvent("health_monitoring_error", "", std::string(e.what()));
        }
        
        std::this_thread::sleep_for(interval);
    }
}

void AgentService::notifyEvent(const std::string& event_type, const std::string& agent_id, const std::any& data) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    auto it = notification_callbacks_.find(event_type);
    if (it != notification_callbacks_.end()) {
        try {
            it->second(event_type, agent_id, data);
        } catch (const std::exception& e) {
            // Log callback error, but don't throw
        }
    }
}

void AgentService::recordExecution(const std::string& agent_id, const ExecutionResult& result) {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    auto& history = execution_history_[agent_id];
    history.push_back(result);
    
    // Keep only last 1000 executions per agent to prevent memory growth
    if (history.size() > 1000) {
        history.erase(history.begin(), history.begin() + 100);
    }
}

AgentService::AgentInfo AgentService::createAgentInfo(const std::string& agent_id, std::shared_ptr<AgentCore> agent) {
    AgentInfo info{};
    info.id = agent_id;
    info.name = agent->get_agent_name();
    info.type = agent->get_agent_type();
    info.role = agent->get_role();
    info.specializations = agent->get_specializations();
    info.capabilities = agent->get_capabilities();
    info.running = agent->is_running();
    info.statistics = agent->get_statistics();
    info.created_at = std::chrono::system_clock::now(); // Would be stored in real implementation
    info.last_modified = info.statistics.last_activity;
    
    return info;
}

} // namespace kolosal::services
