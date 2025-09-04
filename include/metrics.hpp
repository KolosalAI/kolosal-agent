#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <map>
#include <vector>
#include <chrono>
#include <atomic>
#include <thread>
#include <json.hpp>
#include "workflow_types.hpp"

using json = nlohmann::json;

/**
 * @brief Statistics for HTTP endpoints
 */
struct EndpointStats {
    uint64_t request_count = 0;
    uint64_t total_duration_ms = 0;
    std::chrono::system_clock::time_point last_request_time;
};

/**
 * @brief Statistics for agents
 */
struct AgentStats {
    uint64_t operation_count = 0;
    uint64_t total_duration_ms = 0;
    std::chrono::system_clock::time_point last_operation_time;
    std::map<std::string, uint64_t> operations; // operation_name -> count
};

/**
 * @brief Statistics for workflows
 */
struct WorkflowStats {
    uint64_t execution_count = 0;
    uint64_t success_count = 0;
    uint64_t error_count = 0;
    uint64_t cancelled_count = 0;
    uint64_t timeout_count = 0;
    uint64_t total_duration_ms = 0;
    std::chrono::system_clock::time_point last_execution_time;
};

/**
 * @brief Comprehensive metrics collection and monitoring system
 */
class MetricsCollector {
public:
    MetricsCollector();
    ~MetricsCollector();
    
    // Lifecycle
    void start();
    void stop();
    bool is_running() const { return running_; }
    
    // Record metrics
    void record_request(const std::string& endpoint, 
                       std::chrono::milliseconds duration,
                       int status_code);
    
    void record_agent_operation(const std::string& agent_id,
                               const std::string& operation,
                               std::chrono::milliseconds duration);
    
    void record_workflow_execution(const std::string& workflow_id,
                                  const std::string& execution_id,
                                  WorkflowExecutionState state,
                                  std::chrono::milliseconds duration);
    
    // Get metrics
    json get_system_metrics() const;
    json get_agent_metrics() const;
    json get_workflow_metrics() const;
    json get_health_status() const;
    
    // Prometheus format
    std::string get_prometheus_metrics() const;
    
private:
    void collect_metrics();
    double get_cpu_usage() const;
    double get_memory_usage() const;
    
    mutable std::mutex metrics_mutex_;
    std::atomic<bool> running_;
    std::thread collection_thread_;
    std::chrono::steady_clock::time_point start_time_;
    
    // Request metrics
    std::atomic<uint64_t> request_count_{0};
    std::atomic<uint64_t> success_count_{0};
    std::atomic<uint64_t> error_count_{0};
    std::vector<int> request_durations_; // in milliseconds
    
    // Detailed statistics
    std::map<std::string, EndpointStats> endpoint_stats_;
    std::map<std::string, AgentStats> agent_stats_;
    std::map<std::string, WorkflowStats> workflow_stats_;
    std::map<int, uint64_t> error_stats_; // status_code -> count
};

/**
 * @brief Health check service for monitoring system components
 */
class HealthCheckService {
public:
    HealthCheckService();
    ~HealthCheckService();
    
    // Component health checks
    bool check_database_connection();
    bool check_retrieval_service();
    bool check_model_service();
    bool check_workflow_engine();
    
    // Overall health status
    json get_health_status() const;
    json get_detailed_health_report() const;
    
    // Dependency checks
    void register_dependency(const std::string& name, 
                           std::function<bool()> check_function);
    void remove_dependency(const std::string& name);
    
private:
    mutable std::mutex health_mutex_;
    std::map<std::string, std::function<bool()>> dependency_checks_;
    std::map<std::string, bool> last_check_results_;
    std::map<std::string, std::chrono::system_clock::time_point> last_check_times_;
};

/**
 * @brief Performance monitoring service
 */
class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor();
    
    void start_monitoring();
    void stop_monitoring();
    
    // Performance tracking
    void start_operation(const std::string& operation_id, const std::string& operation_type);
    void end_operation(const std::string& operation_id);
    void record_custom_metric(const std::string& name, double value, const std::string& unit = "");
    
    // Performance reports
    json get_performance_report() const;
    json get_slow_operations(int limit = 10) const;
    json get_custom_metrics() const;
    
private:
    struct OperationInfo {
        std::string type;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::milliseconds duration{0};
        bool completed = false;
    };
    
    struct CustomMetric {
        double value;
        std::string unit;
        std::chrono::system_clock::time_point timestamp;
    };
    
    mutable std::mutex perf_mutex_;
    std::atomic<bool> monitoring_{false};
    std::thread monitoring_thread_;
    
    std::map<std::string, OperationInfo> active_operations_;
    std::vector<std::pair<std::string, OperationInfo>> completed_operations_;
    std::map<std::string, std::vector<CustomMetric>> custom_metrics_;
    
    void monitoring_loop();
    void cleanup_old_operations();
};

/**
 * @brief Singleton metrics manager that coordinates all monitoring services
 */
class MetricsManager {
public:
    static MetricsManager& instance();
    
    // Component access
    MetricsCollector& metrics() { return *metrics_collector_; }
    HealthCheckService& health() { return *health_service_; }
    PerformanceMonitor& performance() { return *performance_monitor_; }
    
    // Lifecycle
    void start();
    void stop();
    
    // Unified reporting
    json get_all_metrics() const;
    json get_dashboard_data() const;
    std::string get_prometheus_export() const;
    
private:
    MetricsManager();
    ~MetricsManager();
    
    std::unique_ptr<MetricsCollector> metrics_collector_;
    std::unique_ptr<HealthCheckService> health_service_;
    std::unique_ptr<PerformanceMonitor> performance_monitor_;
    
    bool initialized_ = false;
};
