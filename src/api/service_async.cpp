#include "../../include/service_async.hpp"
#include "../../include/logger.hpp"
#include <random>
#include <iomanip>
#include <sstream>
#include <algorithm>

AsyncServiceLayer::AsyncServiceLayer(size_t worker_threads) 
    : worker_count_(worker_threads),
      task_queue_(std::priority_queue<std::shared_ptr<AsyncTask>, 
                                     std::vector<std::shared_ptr<AsyncTask>>,
                                     std::function<bool(const std::shared_ptr<AsyncTask>&, 
                                                      const std::shared_ptr<AsyncTask>&)>>(
          [](const std::shared_ptr<AsyncTask>& a, const std::shared_ptr<AsyncTask>& b) {
              return a->priority < b->priority; // Higher priority first
          })) {
}

AsyncServiceLayer::~AsyncServiceLayer() {
    stop();
}

void AsyncServiceLayer::start() {
    if (running_.exchange(true)) {
        return; // Already running
    }
    
    // Start worker threads
    worker_threads_.clear();
    for (size_t i = 0; i < worker_count_; ++i) {
        worker_threads_.emplace_back(&AsyncServiceLayer::worker_thread, this);
    }
    
    // Start cleanup thread
    cleanup_thread_ = std::thread([this]() {
        while (running_) {
            cleanup_completed_operations();
            std::this_thread::sleep_for(std::chrono::minutes(5));
        }
    });
    
    std::cout << "[AsyncServiceLayer] Started with " << worker_count_ << " worker threads" << std::endl;
}

void AsyncServiceLayer::stop() {
    if (!running_.exchange(false)) {
        return; // Already stopped
    }
    
    // Notify all workers to stop
    queue_condition_.notify_all();
    
    // Wait for all worker threads to finish
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_.clear();
    
    // Wait for cleanup thread
    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }
    
    std::cout << "[AsyncServiceLayer] Stopped all worker threads" << std::endl;
}

std::future<json> AsyncServiceLayer::submit_batch_operation(const std::string& operation_type,
                                                           const std::vector<std::function<json()>>& tasks) {
    return submit_operation(operation_type, [tasks]() -> json {
        json results = json::array();
        for (size_t i = 0; i < tasks.size(); ++i) {
            try {
                results.push_back({
                    {"index", i},
                    {"success", true},
                    {"result", tasks[i]()}
                });
            } catch (const std::exception& e) {
                results.push_back({
                    {"index", i},
                    {"success", false},
                    {"error", e.what()}
                });
            }
        }
        return results;
    }, 5); // Higher priority for batch operations
}

bool AsyncServiceLayer::cancel_operation(const std::string& operation_id) {
    std::lock_guard<std::mutex> lock(operations_mutex_);
    auto it = operations_.find(operation_id);
    if (it != operations_.end() && it->second->status == AsyncOperationStatus::PENDING) {
        it->second->status = AsyncOperationStatus::CANCELLED;
        it->second->end_time = std::chrono::system_clock::now();
        cancelled_operations_++;
        
        notify_subscribers(AsyncEvent(AsyncEvent::OPERATION_CANCELLED, operation_id));
        return true;
    }
    return false;
}

std::shared_ptr<AsyncOperationResult> AsyncServiceLayer::get_operation_status(const std::string& operation_id) {
    std::lock_guard<std::mutex> lock(operations_mutex_);
    auto it = operations_.find(operation_id);
    if (it != operations_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<AsyncOperationResult>> AsyncServiceLayer::get_all_operations() const {
    std::lock_guard<std::mutex> lock(operations_mutex_);
    std::vector<std::shared_ptr<AsyncOperationResult>> results;
    for (const auto& [id, operation] : operations_) {
        results.push_back(operation);
    }
    return results;
}

std::vector<std::shared_ptr<AsyncOperationResult>> AsyncServiceLayer::get_operations_by_type(const std::string& operation_type) const {
    std::lock_guard<std::mutex> lock(operations_mutex_);
    std::vector<std::shared_ptr<AsyncOperationResult>> results;
    for (const auto& [id, operation] : operations_) {
        // Note: We'd need to store operation type in AsyncOperationResult for this to work properly
        results.push_back(operation);
    }
    return results;
}

void AsyncServiceLayer::subscribe_to_events(const std::string& subscriber_id, EventCallback callback) {
    std::lock_guard<std::mutex> lock(subscribers_mutex_);
    event_subscribers_[subscriber_id] = std::move(callback);
}

void AsyncServiceLayer::unsubscribe_from_events(const std::string& subscriber_id) {
    std::lock_guard<std::mutex> lock(subscribers_mutex_);
    event_subscribers_.erase(subscriber_id);
}

size_t AsyncServiceLayer::get_queue_size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return task_queue_.size();
}

json AsyncServiceLayer::get_queue_statistics() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    json stats;
    stats["current_queue_size"] = task_queue_.size();
    stats["max_queue_size"] = max_queue_size_;
    stats["completed_operations"] = completed_operations_.load();
    stats["failed_operations"] = failed_operations_.load();
    stats["cancelled_operations"] = cancelled_operations_.load();
    stats["worker_count"] = worker_count_;
    
    return stats;
}

void AsyncServiceLayer::set_max_queue_size(size_t max_size) {
    max_queue_size_ = max_size;
}

void AsyncServiceLayer::adjust_worker_count(size_t worker_count) {
    if (!running_) {
        worker_count_ = worker_count;
        return;
    }
    
    // For a running system, this would require more complex logic
    // to safely add/remove worker threads
    std::cout << "[AsyncServiceLayer] Dynamic worker adjustment not implemented for running system" << std::endl;
}

json AsyncServiceLayer::get_worker_statistics() const {
    json stats;
    stats["worker_count"] = worker_count_;
    stats["running"] = running_.load();
    stats["total_operations_completed"] = completed_operations_.load();
    stats["total_operations_failed"] = failed_operations_.load();
    stats["total_operations_cancelled"] = cancelled_operations_.load();
    
    return stats;
}

void AsyncServiceLayer::worker_thread() {
    while (running_) {
        std::shared_ptr<AsyncTask> task;
        
        // Get next task from queue
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_condition_.wait(lock, [this] { return !running_ || !task_queue_.empty(); });
            
            if (!running_) break;
            
            if (!task_queue_.empty()) {
                task = task_queue_.top();
                task_queue_.pop();
            }
        }
        
        if (!task) continue;
        
        // Check if operation was cancelled
        if (task->result->status == AsyncOperationStatus::CANCELLED) {
            continue;
        }
        
        // Execute the task
        task->result->status = AsyncOperationStatus::RUNNING;
        notify_subscribers(AsyncEvent(AsyncEvent::OPERATION_STARTED, task->operation_id));
        
        try {
            json result = task->task_function();
            task->result->result_data = result;
            task->result->status = AsyncOperationStatus::COMPLETED;
            task->result->end_time = std::chrono::system_clock::now();
            
            task->promise.set_value(result);
            completed_operations_++;
            
            notify_subscribers(AsyncEvent(AsyncEvent::OPERATION_COMPLETED, task->operation_id, result));
            
        } catch (const std::exception& e) {
            task->result->status = AsyncOperationStatus::FAILED;
            task->result->error_message = e.what();
            task->result->end_time = std::chrono::system_clock::now();
            
            task->promise.set_exception(std::current_exception());
            failed_operations_++;
            
            json error_data;
            error_data["error"] = e.what();
            notify_subscribers(AsyncEvent(AsyncEvent::OPERATION_FAILED, task->operation_id, error_data));
        }
    }
}

void AsyncServiceLayer::cleanup_completed_operations() {
    std::lock_guard<std::mutex> lock(operations_mutex_);
    
    auto now = std::chrono::system_clock::now();
    auto one_hour_ago = now - std::chrono::hours(1);
    
    auto it = operations_.begin();
    while (it != operations_.end()) {
        if (it->second->status == AsyncOperationStatus::COMPLETED ||
            it->second->status == AsyncOperationStatus::FAILED ||
            it->second->status == AsyncOperationStatus::CANCELLED) {
            
            if (it->second->end_time < one_hour_ago) {
                it = operations_.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}

void AsyncServiceLayer::notify_subscribers(const AsyncEvent& event) {
    std::lock_guard<std::mutex> lock(subscribers_mutex_);
    
    for (const auto& [id, callback] : event_subscribers_) {
        try {
            callback(event);
        } catch (const std::exception& e) {
            std::cout << "[AsyncServiceLayer] Error notifying subscriber " << id << ": " << e.what() << std::endl;
        }
    }
}

std::string AsyncServiceLayer::generate_operation_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "op_";
    
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << dis(gen);
    }
    
    ss << "_" << std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return ss.str();
}

// AsyncAgentService implementation

AsyncAgentService::AsyncAgentService(std::shared_ptr<AsyncServiceLayer> service_layer)
    : service_layer_(service_layer) {
}

AsyncAgentService::~AsyncAgentService() = default;

std::future<json> AsyncAgentService::create_agent_async(const json& agent_config) {
    return service_layer_->submit_operation("create_agent", [agent_config]() -> json {
        // This would integrate with your existing AgentManager
        json result;
        result["success"] = true;
        result["agent_id"] = "agent_" + std::to_string(std::rand());
        result["config"] = agent_config;
        return result;
    });
}

std::future<json> AsyncAgentService::execute_function_async(const std::string& agent_id,
                                                           const std::string& function_name,
                                                           const json& parameters) {
    return service_layer_->submit_operation("execute_function", 
        [agent_id, function_name, parameters]() -> json {
            // This would integrate with your existing AgentManager
            json result;
            result["success"] = true;
            result["agent_id"] = agent_id;
            result["function"] = function_name;
            result["parameters"] = parameters;
            result["output"] = "Function executed successfully";
            return result;
        });
}

std::future<json> AsyncAgentService::batch_execute_functions(
    const std::vector<std::tuple<std::string, std::string, json>>& requests) {
    
    std::vector<std::function<json()>> tasks;
    for (const auto& [agent_id, function_name, parameters] : requests) {
        tasks.push_back([agent_id, function_name, parameters]() -> json {
            json result;
            result["agent_id"] = agent_id;
            result["function"] = function_name;
            result["success"] = true;
            result["output"] = "Batch function executed";
            return result;
        });
    }
    
    return service_layer_->submit_batch_operation("batch_execute_functions", tasks);
}

// AsyncWorkflowService implementation

AsyncWorkflowService::AsyncWorkflowService(std::shared_ptr<AsyncServiceLayer> service_layer)
    : service_layer_(service_layer) {
}

AsyncWorkflowService::~AsyncWorkflowService() = default;

std::future<json> AsyncWorkflowService::execute_workflow_async(const std::string& workflow_id,
                                                              const json& input_data) {
    return service_layer_->submit_operation("execute_workflow", 
        [workflow_id, input_data]() -> json {
            // This would integrate with your existing WorkflowOrchestrator
            json result;
            result["success"] = true;
            result["workflow_id"] = workflow_id;
            result["execution_id"] = "exec_" + std::to_string(std::rand());
            result["input_data"] = input_data;
            result["status"] = "completed";
            return result;
        }, 3); // Medium priority
}

// EventNotificationService implementation

EventNotificationService::EventNotificationService() = default;
EventNotificationService::~EventNotificationService() = default;

void EventNotificationService::add_client(const std::string& client_id, 
                                         std::function<void(const json&)> send_callback) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    clients_[client_id] = ClientInfo{std::move(send_callback), {}, std::chrono::system_clock::now()};
    total_clients_served_++;
}

void EventNotificationService::remove_client(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    clients_.erase(client_id);
}

void EventNotificationService::broadcast_event(const AsyncEvent& event) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    json event_json;
    event_json["type"] = static_cast<int>(event.type);
    event_json["operation_id"] = event.operation_id;
    event_json["data"] = event.event_data;
    event_json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        event.timestamp.time_since_epoch()).count();
    
    for (const auto& [client_id, client_info] : clients_) {
        try {
            client_info.send_callback(event_json);
            total_events_sent_++;
        } catch (const std::exception& e) {
            std::cout << "[EventNotificationService] Failed to send event to client " 
                     << client_id << ": " << e.what() << std::endl;
        }
    }
}

json EventNotificationService::get_notification_statistics() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    json stats;
    stats["connected_clients"] = clients_.size();
    stats["total_events_sent"] = total_events_sent_.load();
    stats["total_clients_served"] = total_clients_served_.load();
    
    return stats;
}
