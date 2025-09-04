#pragma once

#include <future>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <set>
#include <json.hpp>

using json = nlohmann::json;

/**
 * @brief Async operation status
 */
enum class AsyncOperationStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED,
    CANCELLED
};

/**
 * @brief Async operation result
 */
struct AsyncOperationResult {
    std::string operation_id;
    AsyncOperationStatus status;
    json result_data;
    std::string error_message;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    
    AsyncOperationResult() : status(AsyncOperationStatus::PENDING) {
        start_time = std::chrono::system_clock::now();
    }
};

/**
 * @brief Async operation task
 */
struct AsyncTask {
    std::string operation_id;
    std::string operation_type;
    std::function<json()> task_function;
    std::shared_ptr<AsyncOperationResult> result;
    std::promise<json> promise;
    int priority = 0; // Higher values = higher priority
    
    AsyncTask() : result(std::make_shared<AsyncOperationResult>()) {}
};

/**
 * @brief Event notification for async operations
 */
struct AsyncEvent {
    enum Type {
        OPERATION_STARTED,
        OPERATION_COMPLETED,
        OPERATION_FAILED,
        OPERATION_CANCELLED,
        SYSTEM_STATUS_CHANGED
    };
    
    Type type;
    std::string operation_id;
    json event_data;
    std::chrono::system_clock::time_point timestamp;
    
    AsyncEvent(Type t, const std::string& id = "", const json& data = json{}) 
        : type(t), operation_id(id), event_data(data), 
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @brief Event subscriber callback
 */
using EventCallback = std::function<void(const AsyncEvent&)>;

/**
 * @brief Thread-safe async service layer
 */
class AsyncServiceLayer {
public:
    AsyncServiceLayer(size_t worker_threads = std::thread::hardware_concurrency());
    ~AsyncServiceLayer();
    
    // Lifecycle
    void start();
    void stop();
    bool is_running() const { return running_; }
    
    // Async operation management
    template<typename Func>
    std::future<json> submit_operation(const std::string& operation_type, 
                                     Func&& func, 
                                     int priority = 0);
    
    std::future<json> submit_batch_operation(const std::string& operation_type,
                                           const std::vector<std::function<json()>>& tasks);
    
    bool cancel_operation(const std::string& operation_id);
    std::shared_ptr<AsyncOperationResult> get_operation_status(const std::string& operation_id);
    std::vector<std::shared_ptr<AsyncOperationResult>> get_all_operations() const;
    std::vector<std::shared_ptr<AsyncOperationResult>> get_operations_by_type(const std::string& operation_type) const;
    
    // Event system
    void subscribe_to_events(const std::string& subscriber_id, EventCallback callback);
    void unsubscribe_from_events(const std::string& subscriber_id);
    
    // Queue management
    size_t get_queue_size() const;
    json get_queue_statistics() const;
    void set_max_queue_size(size_t max_size);
    
    // Worker management
    void adjust_worker_count(size_t worker_count);
    json get_worker_statistics() const;
    
private:
    void worker_thread();
    void cleanup_completed_operations();
    void notify_subscribers(const AsyncEvent& event);
    std::string generate_operation_id();
    
    std::atomic<bool> running_{false};
    size_t worker_count_;
    std::vector<std::thread> worker_threads_;
    std::thread cleanup_thread_;
    
    // Task queue
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_condition_;
    std::priority_queue<std::shared_ptr<AsyncTask>, 
                       std::vector<std::shared_ptr<AsyncTask>>,
                       std::function<bool(const std::shared_ptr<AsyncTask>&, 
                                        const std::shared_ptr<AsyncTask>&)>> task_queue_;
    size_t max_queue_size_ = 1000;
    
    // Operation tracking
    mutable std::mutex operations_mutex_;
    std::unordered_map<std::string, std::shared_ptr<AsyncOperationResult>> operations_;
    
    // Event system
    mutable std::mutex subscribers_mutex_;
    std::unordered_map<std::string, EventCallback> event_subscribers_;
    
    // Statistics
    std::atomic<size_t> completed_operations_{0};
    std::atomic<size_t> failed_operations_{0};
    std::atomic<size_t> cancelled_operations_{0};
};

/**
 * @brief Specialized async agent service
 */
class AsyncAgentService {
public:
    AsyncAgentService(std::shared_ptr<AsyncServiceLayer> service_layer);
    ~AsyncAgentService();
    
    // Async agent operations
    std::future<json> create_agent_async(const json& agent_config);
    std::future<json> execute_function_async(const std::string& agent_id,
                                            const std::string& function_name,
                                            const json& parameters);
    std::future<json> batch_execute_functions(const std::vector<std::tuple<std::string, std::string, json>>& requests);
    
    // Agent lifecycle management
    std::future<json> start_agent_async(const std::string& agent_id);
    std::future<json> stop_agent_async(const std::string& agent_id);
    std::future<json> restart_agent_async(const std::string& agent_id);
    
    // Bulk operations
    std::future<json> bulk_agent_operation(const std::string& operation_type,
                                         const std::vector<std::string>& agent_ids,
                                         const json& parameters = json{});
    
private:
    std::shared_ptr<AsyncServiceLayer> service_layer_;
};

/**
 * @brief Specialized async workflow service
 */
class AsyncWorkflowService {
public:
    AsyncWorkflowService(std::shared_ptr<AsyncServiceLayer> service_layer);
    ~AsyncWorkflowService();
    
    // Async workflow operations
    std::future<json> execute_workflow_async(const std::string& workflow_id,
                                            const json& input_data);
    std::future<json> batch_execute_workflows(const std::vector<std::pair<std::string, json>>& workflows);
    
    // Workflow management
    std::future<json> create_workflow_async(const json& workflow_definition);
    std::future<json> update_workflow_async(const std::string& workflow_id,
                                           const json& workflow_definition);
    std::future<json> delete_workflow_async(const std::string& workflow_id);
    
    // Execution control
    std::future<json> pause_execution_async(const std::string& execution_id);
    std::future<json> resume_execution_async(const std::string& execution_id);
    std::future<json> cancel_execution_async(const std::string& execution_id);
    
private:
    std::shared_ptr<AsyncServiceLayer> service_layer_;
};

/**
 * @brief WebSocket event notification service
 */
class EventNotificationService {
public:
    EventNotificationService();
    ~EventNotificationService();
    
    // Client management
    void add_client(const std::string& client_id, std::function<void(const json&)> send_callback);
    void remove_client(const std::string& client_id);
    
    // Event broadcasting
    void broadcast_event(const AsyncEvent& event);
    void send_to_client(const std::string& client_id, const json& message);
    
    // Subscription management
    void subscribe_client_to_events(const std::string& client_id, const std::vector<std::string>& event_types);
    void unsubscribe_client_from_events(const std::string& client_id, const std::vector<std::string>& event_types);
    
    // Statistics
    json get_notification_statistics() const;
    
private:
    struct ClientInfo {
        std::function<void(const json&)> send_callback;
        std::set<std::string> subscribed_events;
        std::chrono::system_clock::time_point last_activity;
    };
    
    mutable std::mutex clients_mutex_;
    std::unordered_map<std::string, ClientInfo> clients_;
    
    std::atomic<size_t> total_events_sent_{0};
    std::atomic<size_t> total_clients_served_{0};
};

/**
 * @brief Performance analytics service for async operations
 */
class AsyncPerformanceAnalytics {
public:
    AsyncPerformanceAnalytics();
    ~AsyncPerformanceAnalytics();
    
    void record_operation_start(const std::string& operation_id, const std::string& operation_type);
    void record_operation_end(const std::string& operation_id, bool success, const std::string& error = "");
    
    // Analytics reports
    json get_performance_summary() const;
    json get_operation_type_statistics() const;
    json get_throughput_analysis() const;
    json get_error_analysis() const;
    
    // Performance insights
    std::vector<std::string> get_slow_operations(double threshold_seconds = 5.0) const;
    std::vector<std::string> get_frequently_failing_operations(double failure_rate_threshold = 0.1) const;
    json get_performance_recommendations() const;
    
private:
    struct OperationRecord {
        std::string operation_type;
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point end_time;
        bool success;
        std::string error_message;
        
        double duration_seconds() const {
            return std::chrono::duration<double>(end_time - start_time).count();
        }
    };
    
    mutable std::mutex analytics_mutex_;
    std::unordered_map<std::string, OperationRecord> active_operations_;
    std::vector<OperationRecord> completed_operations_;
    
    void cleanup_old_records();
};

// Template implementation
template<typename Func>
std::future<json> AsyncServiceLayer::submit_operation(const std::string& operation_type, 
                                                     Func&& func, 
                                                     int priority) {
    auto task = std::make_shared<AsyncTask>();
    task->operation_id = generate_operation_id();
    task->operation_type = operation_type;
    task->priority = priority;
    task->task_function = std::forward<Func>(func);
    
    auto future = task->promise.get_future();
    
    {
        std::lock_guard<std::mutex> lock(operations_mutex_);
        operations_[task->operation_id] = task->result;
    }
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (task_queue_.size() >= max_queue_size_) {
            task->result->status = AsyncOperationStatus::FAILED;
            task->result->error_message = "Queue is full";
            task->promise.set_exception(std::make_exception_ptr(std::runtime_error("Queue is full")));
            return future;
        }
        
        task_queue_.push(task);
    }
    
    queue_condition_.notify_one();
    return future;
}
