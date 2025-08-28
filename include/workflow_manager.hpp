#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <chrono>
#include <json.hpp>
#include "agent_manager.hpp"

using json = nlohmann::json;

/**
 * @brief Workflow execution states
 */
enum class WorkflowState {
    PENDING,
    PROCESSING,
    COMPLETED,
    FAILED,
    TIMEOUT,
    CANCELLED
};

/**
 * @brief Workflow request structure
 */
struct WorkflowRequest {
    std::string id;
    std::string agent_name;
    std::string function_name;
    json parameters;
    std::chrono::system_clock::time_point timestamp;
    WorkflowState state;
    json result;
    std::string error;
    int timeout_ms;
    
    WorkflowRequest(const std::string& req_id, 
                   const std::string& agent, 
                   const std::string& function,
                   const json& params,
                   int timeout = 30000)
        : id(req_id), agent_name(agent), function_name(function), 
          parameters(params), timestamp(std::chrono::system_clock::now()),
          state(WorkflowState::PENDING), timeout_ms(timeout) {}
};

/**
 * @brief Workflow execution statistics
 */
struct WorkflowStats {
    std::atomic<size_t> total_requests{0};
    std::atomic<size_t> completed_requests{0};
    std::atomic<size_t> failed_requests{0};
    std::atomic<size_t> timeout_requests{0};
    std::atomic<size_t> active_requests{0};
    std::atomic<size_t> queue_size{0};
    
    // Default constructor
    WorkflowStats() = default;
    
    // Copy constructor
    WorkflowStats(const WorkflowStats& other) 
        : total_requests(other.total_requests.load()),
          completed_requests(other.completed_requests.load()),
          failed_requests(other.failed_requests.load()),
          timeout_requests(other.timeout_requests.load()),
          active_requests(other.active_requests.load()),
          queue_size(other.queue_size.load()) {}
    
    // Copy assignment operator
    WorkflowStats& operator=(const WorkflowStats& other) {
        if (this != &other) {
            total_requests.store(other.total_requests.load());
            completed_requests.store(other.completed_requests.load());
            failed_requests.store(other.failed_requests.load());
            timeout_requests.store(other.timeout_requests.load());
            active_requests.store(other.active_requests.load());
            queue_size.store(other.queue_size.load());
        }
        return *this;
    }
};

/**
 * @brief Workflow Manager - Manages agent workflow execution
 */
class WorkflowManager {
private:
    std::shared_ptr<AgentManager> agent_manager_;
    
    // Request management
    std::queue<std::shared_ptr<WorkflowRequest>> request_queue_;
    std::map<std::string, std::shared_ptr<WorkflowRequest>> active_requests_;
    std::map<std::string, std::shared_ptr<WorkflowRequest>> completed_requests_;
    
    // Threading and synchronization
    std::vector<std::thread> worker_threads_;
    std::mutex queue_mutex_;
    std::mutex requests_mutex_;
    std::condition_variable queue_condition_;
    std::atomic<bool> running_{false};
    
    // Configuration
    size_t max_workers_;
    size_t max_queue_size_;
    size_t max_completed_history_;
    
    // Statistics
    WorkflowStats stats_;
    
    // Function configuration from agent.yaml
    std::map<std::string, json> function_configs_;
    
public:
    explicit WorkflowManager(std::shared_ptr<AgentManager> agent_manager, 
                            size_t max_workers = 4, 
                            size_t max_queue_size = 1000,
                            size_t max_completed_history = 10000);
    ~WorkflowManager();
    
    // Lifecycle management
    bool start();
    void stop();
    bool is_running() const { return running_.load(); }
    
    // Configuration
    void load_function_configs(const json& config);
    void set_max_workers(size_t workers);
    void set_max_queue_size(size_t size);
    
    // Request submission and management
    std::string submit_request(const std::string& agent_name,
                              const std::string& function_name,
                              const json& parameters);
    std::string submit_request_with_timeout(const std::string& agent_name,
                                           const std::string& function_name,
                                           const json& parameters,
                                           int timeout_ms);
    
    // Request status and results
    std::shared_ptr<WorkflowRequest> get_request_status(const std::string& request_id);
    json get_request_result(const std::string& request_id);
    bool cancel_request(const std::string& request_id);
    
    // Bulk operations
    json list_active_requests();
    json list_recent_requests(size_t limit = 100);
    void cleanup_completed_requests(size_t keep_count = 1000);
    
    // Statistics and monitoring
    WorkflowStats get_statistics() const;
    json get_system_status();
    
    // Validation
    bool validate_request(const std::string& agent_name, 
                         const std::string& function_name,
                         const json& parameters);

private:
    // Worker thread functions
    void worker_thread();
    void process_request(std::shared_ptr<WorkflowRequest> request);
    
    // Helper functions
    std::string generate_request_id();
    void validate_function_parameters(const std::string& function_name, const json& parameters);
    void move_to_completed(std::shared_ptr<WorkflowRequest> request);
    void cleanup_old_requests();
    
    // Request execution
    void execute_request_with_timeout(std::shared_ptr<WorkflowRequest> request);
};

/**
 * @brief Workflow utilities and helper functions
 */
namespace WorkflowUtils {
    std::string state_to_string(WorkflowState state);
    WorkflowState string_to_state(const std::string& state_str);
    json request_to_json(const WorkflowRequest& request);
    std::string format_duration(const std::chrono::system_clock::time_point& start);
}
