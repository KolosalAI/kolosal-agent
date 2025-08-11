/**
 * @file agent_service.hpp
 * @brief High-level agent service operations
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_AGENT_SERVICES_AGENT_SERVICE_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_AGENT_SERVICES_AGENT_SERVICE_HPP_INCLUDED

#include "../../export.hpp"
#include "../core/agent_core.hpp"
#include "../core/multi_agent_system.hpp"
#include "../../config/yaml_configuration_parser.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <future>
#include <optional>
#include <any>
#include <atomic>
#include <thread>
#include <chrono>

namespace kolosal::services {

/**
 * @brief Service layer for agent lifecycle and operation management
 * 
 * Provides high-level operations for agent management, including:
 * - Agent creation, configuration, and lifecycle management
 * - Bulk operations and batch processing
 * - Health monitoring and diagnostics
 * - Performance metrics and analytics
 * - Event-driven notifications
 */
/**
 * @brief Represents k o l o s a l_ a g e n t_ a p i functionality
 */
class KOLOSAL_AGENT_API AgentService {
public:
    struct AgentInfo {
        std::string id;
        std::string name;
        std::string type;
        kolosal::agents::AgentRole role;
        std::vector<kolosal::agents::AgentSpecialization> specializations;
        std::vector<std::string> capabilities;
        bool running;
        kolosal::agents::AgentCore::AgentStats statistics;
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point last_modified;
    };

    struct SystemMetrics {
        size_t total_agents;
        size_t running_agents;
        size_t total_functions_executed;
        size_t total_plans_created;
        double average_response_time_ms;
        size_t active_jobs;
        size_t queued_jobs;
        std::chrono::system_clock::time_point last_updated;
    };

    struct ExecutionResult {
        bool success;
        std::string message;
        std::string execution_id;
        std::any result_data;
        double execution_time_ms;
        std::chrono::system_clock::time_point timestamp;
    };

    using NotificationCallback = std::function<void(const std::string&, const std::string&, const std::any&)>;
    /**
     * @brief Constructor
     * @param agent_manager Shared pointer to the agent manager
     */
    explicit AgentService(std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager);
    ~AgentService();

    // Agent lifecycle management
    std::future<std::string> createAgentAsync(const kolosal::agents::AgentConfig& config);
    std::future<bool> startAgentAsync(const std::string& agent_id);
    std::future<bool> stopAgentAsync(const std::string& agent_id);
    std::future<bool> deleteAgentAsync(const std::string& agent_id);
    std::future<bool> restartAgentAsync(const std::string& agent_id);

    // Bulk operations
    std::future<std::vector<std::string>> createMultipleAgentsAsync(
        const std::vector<kolosal::agents::AgentConfig>& configs);
    std::future<std::vector<bool>> startMultipleAgentsAsync(const std::vector<std::string>& agent_ids);
    std::future<std::vector<bool>> stopMultipleAgentsAsync(const std::vector<std::string>& agent_ids);

    // Information and status
    std::vector<AgentInfo> getAllAgentInfo();
    std::optional<AgentInfo> getAgentInfo(const std::string& agent_id);
    SystemMetrics getSystemMetrics();
    bool isAgentHealthy(const std::string& agent_id);
    std::vector<std::string> getUnhealthyAgents();

    // Function execution
    std::future<ExecutionResult> executeFunctionAsync(
        const std::string& agent_id,
        const std::string& function_name,
        const kolosal::agents::AgentData& parameters,
        const int priority = 0);
    std::future<std::vector<ExecutionResult>> executeFunctionOnMultipleAgentsAsync(
        const std::vector<std::string>& agent_ids,
        const std::string& function_name,
        const kolosal::agents::AgentData& parameters);

    // Configuration and templates
    bool saveAgentTemplate(const std::string& template_name, const kolosal::agents::AgentConfig& config);
    std::optional<kolosal::agents::AgentConfig> getAgentTemplate(const std::string& template_name);
    std::vector<std::string> getAvailableTemplates();
    bool deleteAgentTemplate(const std::string& template_name);

    // Event notifications
    void registerNotificationCallback(const std::string& event_type, NotificationCallback callback);
    void unregisterNotificationCallback(const std::string& event_type);

    // Health monitoring
    void startHealthMonitoring(std::chrono::seconds interval = std::chrono::seconds(30));
    void stopHealthMonitoring();
    bool isHealthMonitoringActive() const;

    // Performance analysis
    struct PerformanceReport {
        std::string agent_id;
        double average_execution_time_ms;
        size_t successful_executions;
        size_t failed_executions;
        double success_rate;
        std::vector<std::string> most_used_functions;
        std::chrono::system_clock::time_point report_timestamp;
    };

    std::vector<PerformanceReport> generatePerformanceReport(
        const std::vector<std::string>& agent_ids = {},
        std::chrono::hours time_window = std::chrono::hours(24));
    // Resource optimization
    struct OptimizationSuggestion {
        std::string type; // "memory", "cpu", "redundancy", etc.
        std::string description;
        std::vector<std::string> affected_agents;
        double potential_improvement_percent;
    };

    std::vector<OptimizationSuggestion> analyzeSystem_Optimization();
    bool applyOptimization_Suggestion(const OptimizationSuggestion& suggestion);

private:
    std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager_;
    std::unordered_map<std::string, kolosal::agents::AgentConfig> agent_templates_;
    std::unordered_map<std::string, NotificationCallback> notification_callbacks_;
    
    // Health monitoring
    std::atomic<bool> health_monitoring_active_ {false};
    std::thread health_monitoring_thread_;
    
    // Performance tracking
    mutable std::mutex performance_mutex_;
    std::unordered_map<std::string, std::vector<ExecutionResult>> execution_history_;
    
    // Templates storage
    mutable std::mutex templates_mutex_;
    
    // Notification system
    mutable std::mutex callbacks_mutex_;

    // Internal methods
    void healthMonitoringLoop(std::chrono::seconds interval);
    void notifyEvent(const std::string& event_type, const std::string& agent_id, const std::any& data);
    void recordExecution(const std::string& agent_id, const ExecutionResult& result);
    AgentInfo createAgentInfo(const std::string& agent_id, std::shared_ptr<kolosal::agents::AgentCore> agent);
    std::string generateExecutionId();
};

} // namespace kolosal::services

#endif // KOLOSAL_AGENT_INCLUDE_AGENT_SERVICES_AGENT_SERVICE_HPP_INCLUDED
