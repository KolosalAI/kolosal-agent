#include "../../include/metrics_handler.hpp"
#include "../../include/logger.hpp"
#include <chrono>
#include <sstream>
#include <thread>
#include <numeric>

MetricsCollector::MetricsCollector() : running_(false) {
    start_time_ = std::chrono::steady_clock::now();
}

MetricsCollector::~MetricsCollector() {
    stop();
}

void MetricsCollector::start() {
    if (running_.exchange(true)) {
        return; // Already running
    }
    
    collection_thread_ = std::thread(&MetricsCollector::collect_metrics, this);
    std::cout << "[MetricsCollector] Started metrics collection" << std::endl;
}

void MetricsCollector::stop() {
    if (!running_.exchange(false)) {
        return; // Already stopped
    }
    
    if (collection_thread_.joinable()) {
        collection_thread_.join();
    }
    std::cout << "[MetricsCollector] Stopped metrics collection" << std::endl;
}

void MetricsCollector::record_request(const std::string& endpoint, 
                                    std::chrono::milliseconds duration,
                                    int status_code) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    request_count_++;
    request_durations_.push_back(duration.count());
    
    // Keep only last 1000 request durations for memory efficiency
    if (request_durations_.size() > 1000) {
        request_durations_.erase(request_durations_.begin());
    }
    
    endpoint_stats_[endpoint].request_count++;
    endpoint_stats_[endpoint].total_duration_ms += duration.count();
    endpoint_stats_[endpoint].last_request_time = std::chrono::system_clock::now();
    
    if (status_code >= 200 && status_code < 300) {
        success_count_++;
    } else {
        error_count_++;
        error_stats_[status_code]++;
    }
}

void MetricsCollector::record_agent_operation(const std::string& agent_id,
                                             const std::string& operation,
                                             std::chrono::milliseconds duration) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    agent_stats_[agent_id].operation_count++;
    agent_stats_[agent_id].total_duration_ms += duration.count();
    agent_stats_[agent_id].last_operation_time = std::chrono::system_clock::now();
    agent_stats_[agent_id].operations[operation]++;
}

void MetricsCollector::record_workflow_execution(const std::string& workflow_id,
                                                const std::string& execution_id,
                                                WorkflowExecutionState state,
                                                std::chrono::milliseconds duration) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    workflow_stats_[workflow_id].execution_count++;
    workflow_stats_[workflow_id].total_duration_ms += duration.count();
    workflow_stats_[workflow_id].last_execution_time = std::chrono::system_clock::now();
    
    switch (state) {
        case WorkflowExecutionState::COMPLETED:
            workflow_stats_[workflow_id].success_count++;
            break;
        case WorkflowExecutionState::FAILED:
            workflow_stats_[workflow_id].error_count++;
            break;
        case WorkflowExecutionState::CANCELLED:
            workflow_stats_[workflow_id].cancelled_count++;
            break;
        case WorkflowExecutionState::TIMEOUT:
            workflow_stats_[workflow_id].timeout_count++;
            break;
        default:
            break;
    }
}

json MetricsCollector::get_system_metrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    json metrics;
    
    // System information
    auto now = std::chrono::steady_clock::now();
    auto uptime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
    
    metrics["system"]["uptime_ms"] = uptime_ms;
    metrics["system"]["uptime_hours"] = uptime_ms / (1000 * 60 * 60);
    metrics["system"]["cpu_usage_percent"] = get_cpu_usage();
    metrics["system"]["memory_usage_mb"] = get_memory_usage();
    metrics["system"]["thread_count"] = std::thread::hardware_concurrency();
    
    // Request metrics
    uint64_t total_count = request_count_.load();
    uint64_t success_count = success_count_.load();
    uint64_t error_count = error_count_.load();
    
    metrics["requests"]["total_count"] = total_count;
    metrics["requests"]["success_count"] = success_count;
    metrics["requests"]["error_count"] = error_count;
    metrics["requests"]["success_rate"] = total_count > 0 ? 
        static_cast<double>(success_count) / total_count : 0.0;
    
    if (!request_durations_.empty()) {
        auto sorted_durations = request_durations_;
        std::sort(sorted_durations.begin(), sorted_durations.end());
        
        metrics["requests"]["avg_duration_ms"] = 
            std::accumulate(sorted_durations.begin(), sorted_durations.end(), 0.0) / sorted_durations.size();
        metrics["requests"]["p50_duration_ms"] = 
            sorted_durations[sorted_durations.size() / 2];
        metrics["requests"]["p95_duration_ms"] = 
            sorted_durations[static_cast<size_t>(sorted_durations.size() * 0.95)];
        metrics["requests"]["p99_duration_ms"] = 
            sorted_durations[static_cast<size_t>(sorted_durations.size() * 0.99)];
    }
    
    // Endpoint metrics
    json endpoint_metrics = json::array();
    for (const auto& [endpoint, stats] : endpoint_stats_) {
        json endpoint_data;
        endpoint_data["endpoint"] = endpoint;
        endpoint_data["request_count"] = stats.request_count;
        endpoint_data["avg_duration_ms"] = stats.request_count > 0 ? 
            static_cast<double>(stats.total_duration_ms) / stats.request_count : 0.0;
        endpoint_data["last_request"] = std::chrono::duration_cast<std::chrono::seconds>(
            stats.last_request_time.time_since_epoch()).count();
        endpoint_metrics.push_back(endpoint_data);
    }
    metrics["endpoints"] = endpoint_metrics;
    
    // Error metrics
    json error_metrics = json::object();
    for (const auto& [code, count] : error_stats_) {
        error_metrics[std::to_string(code)] = count;
    }
    metrics["errors"] = error_metrics;
    
    return metrics;
}

json MetricsCollector::get_agent_metrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    json metrics = json::array();
    for (const auto& [agent_id, stats] : agent_stats_) {
        json agent_data;
        agent_data["agent_id"] = agent_id;
        agent_data["operation_count"] = stats.operation_count;
        agent_data["avg_duration_ms"] = stats.operation_count > 0 ? 
            static_cast<double>(stats.total_duration_ms) / stats.operation_count : 0.0;
        agent_data["last_operation"] = std::chrono::duration_cast<std::chrono::seconds>(
            stats.last_operation_time.time_since_epoch()).count();
        agent_data["operations"] = stats.operations;
        metrics.push_back(agent_data);
    }
    
    return metrics;
}

json MetricsCollector::get_workflow_metrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    json metrics = json::array();
    for (const auto& [workflow_id, stats] : workflow_stats_) {
        json workflow_data;
        workflow_data["workflow_id"] = workflow_id;
        workflow_data["execution_count"] = stats.execution_count;
        workflow_data["success_count"] = stats.success_count;
        workflow_data["error_count"] = stats.error_count;
        workflow_data["cancelled_count"] = stats.cancelled_count;
        workflow_data["timeout_count"] = stats.timeout_count;
        workflow_data["success_rate"] = stats.execution_count > 0 ? 
            static_cast<double>(stats.success_count) / stats.execution_count : 0.0;
        workflow_data["avg_duration_ms"] = stats.execution_count > 0 ? 
            static_cast<double>(stats.total_duration_ms) / stats.execution_count : 0.0;
        workflow_data["last_execution"] = std::chrono::duration_cast<std::chrono::seconds>(
            stats.last_execution_time.time_since_epoch()).count();
        metrics.push_back(workflow_data);
    }
    
    return metrics;
}

std::string MetricsCollector::get_prometheus_metrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::ostringstream prometheus;
    
    // Help and type declarations
    prometheus << "# HELP kolosal_requests_total Total number of HTTP requests\n";
    prometheus << "# TYPE kolosal_requests_total counter\n";
    prometheus << "kolosal_requests_total " << request_count_ << "\n\n";
    
    prometheus << "# HELP kolosal_requests_success_total Total number of successful HTTP requests\n";
    prometheus << "# TYPE kolosal_requests_success_total counter\n";
    prometheus << "kolosal_requests_success_total " << success_count_ << "\n\n";
    
    prometheus << "# HELP kolosal_requests_error_total Total number of failed HTTP requests\n";
    prometheus << "# TYPE kolosal_requests_error_total counter\n";
    prometheus << "kolosal_requests_error_total " << error_count_ << "\n\n";
    
    prometheus << "# HELP kolosal_uptime_seconds System uptime in seconds\n";
    prometheus << "# TYPE kolosal_uptime_seconds gauge\n";
    auto uptime_seconds = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start_time_).count();
    prometheus << "kolosal_uptime_seconds " << uptime_seconds << "\n\n";
    
    prometheus << "# HELP kolosal_cpu_usage_percent CPU usage percentage\n";
    prometheus << "# TYPE kolosal_cpu_usage_percent gauge\n";
    prometheus << "kolosal_cpu_usage_percent " << get_cpu_usage() << "\n\n";
    
    prometheus << "# HELP kolosal_memory_usage_bytes Memory usage in bytes\n";
    prometheus << "# TYPE kolosal_memory_usage_bytes gauge\n";
    prometheus << "kolosal_memory_usage_bytes " << (get_memory_usage() * 1024 * 1024) << "\n\n";
    
    // Request durations histogram
    if (!request_durations_.empty()) {
        prometheus << "# HELP kolosal_request_duration_ms HTTP request duration in milliseconds\n";
        prometheus << "# TYPE kolosal_request_duration_ms histogram\n";
        
        std::vector<int> buckets = {1, 5, 10, 25, 50, 100, 250, 500, 1000, 2500, 5000, 10000};
        std::vector<int> bucket_counts(buckets.size(), 0);
        
        for (int duration : request_durations_) {
            for (size_t i = 0; i < buckets.size(); ++i) {
                if (duration <= buckets[i]) {
                    for (size_t j = i; j < buckets.size(); ++j) {
                        bucket_counts[j]++;
                    }
                    break;
                }
            }
        }
        
        for (size_t i = 0; i < buckets.size(); ++i) {
            prometheus << "kolosal_request_duration_ms_bucket{le=\"" << buckets[i] << "\"} " 
                      << bucket_counts[i] << "\n";
        }
        prometheus << "kolosal_request_duration_ms_bucket{le=\"+Inf\"} " << request_durations_.size() << "\n";
        prometheus << "kolosal_request_duration_ms_count " << request_durations_.size() << "\n";
        prometheus << "kolosal_request_duration_ms_sum " << 
            std::accumulate(request_durations_.begin(), request_durations_.end(), 0) << "\n\n";
    }
    
    // Agent metrics
    prometheus << "# HELP kolosal_agent_operations_total Total number of agent operations\n";
    prometheus << "# TYPE kolosal_agent_operations_total counter\n";
    for (const auto& [agent_id, stats] : agent_stats_) {
        prometheus << "kolosal_agent_operations_total{agent_id=\"" << agent_id << "\"} " 
                  << stats.operation_count << "\n";
    }
    prometheus << "\n";
    
    // Workflow metrics
    prometheus << "# HELP kolosal_workflow_executions_total Total number of workflow executions\n";
    prometheus << "# TYPE kolosal_workflow_executions_total counter\n";
    for (const auto& [workflow_id, stats] : workflow_stats_) {
        prometheus << "kolosal_workflow_executions_total{workflow_id=\"" << workflow_id 
                  << "\",status=\"success\"} " << stats.success_count << "\n";
        prometheus << "kolosal_workflow_executions_total{workflow_id=\"" << workflow_id 
                  << "\",status=\"error\"} " << stats.error_count << "\n";
        prometheus << "kolosal_workflow_executions_total{workflow_id=\"" << workflow_id 
                  << "\",status=\"cancelled\"} " << stats.cancelled_count << "\n";
        prometheus << "kolosal_workflow_executions_total{workflow_id=\"" << workflow_id 
                  << "\",status=\"timeout\"} " << stats.timeout_count << "\n";
    }
    
    return prometheus.str();
}

void MetricsCollector::collect_metrics() {
    while (running_) {
        // Update system metrics periodically
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        // Here we could add more system-level metric collection
        // For now, the metrics are updated on-demand
    }
}

double MetricsCollector::get_cpu_usage() const {
    // Simplified CPU usage calculation
    // In a real implementation, you'd use platform-specific APIs
#ifdef _WIN32
    // Windows implementation would use GetSystemTimes() or PDH
    return 0.0; // Placeholder
#else
    // Linux implementation would read from /proc/stat
    return 0.0; // Placeholder
#endif
}

double MetricsCollector::get_memory_usage() const {
    // Simplified memory usage calculation
    // In a real implementation, you'd use platform-specific APIs
#ifdef _WIN32
    // Windows implementation would use GlobalMemoryStatusEx()
    return 0.0; // Placeholder
#else
    // Linux implementation would read from /proc/meminfo
    return 0.0; // Placeholder
#endif
}
