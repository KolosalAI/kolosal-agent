#include "../include/workflow_manager.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <future>

WorkflowManager::WorkflowManager(std::shared_ptr<AgentManager> agent_manager, 
                                size_t max_workers, 
                                size_t max_queue_size,
                                size_t max_completed_history)
    : agent_manager_(agent_manager), 
      max_workers_(max_workers), 
      max_queue_size_(max_queue_size),
      max_completed_history_(max_completed_history) {
}

WorkflowManager::~WorkflowManager() {
    stop();
}

bool WorkflowManager::start() {
    if (running_.load()) {
        return true;
    }
    
    running_ = true;
    
    // Start worker threads
    for (size_t i = 0; i < max_workers_; ++i) {
        worker_threads_.emplace_back(&WorkflowManager::worker_thread, this);
    }
    
    return true;
}

void WorkflowManager::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_ = false;
    
    // Notify all workers to wake up and exit
    queue_condition_.notify_all();
    
    // Wait for all worker threads to finish
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    worker_threads_.clear();
}

void WorkflowManager::load_function_configs(const json& config) {
    if (config.contains("functions")) {
        function_configs_ = config["functions"];
    }
}

void WorkflowManager::set_max_workers(size_t workers) {
    max_workers_ = workers;
}

void WorkflowManager::set_max_queue_size(size_t size) {
    max_queue_size_ = size;
}

std::string WorkflowManager::submit_request(const std::string& agent_name,
                                           const std::string& function_name,
                                           const json& parameters) {
    // Get default timeout from function config
    int timeout_ms = 30000; // default 30 seconds
    if (function_configs_.find(function_name) != function_configs_.end() && 
        function_configs_[function_name].contains("timeout")) {
        timeout_ms = function_configs_[function_name]["timeout"];
    }
    
    return submit_request_with_timeout(agent_name, function_name, parameters, timeout_ms);
}

std::string WorkflowManager::submit_request_with_timeout(const std::string& agent_name,
                                                        const std::string& function_name,
                                                        const json& parameters,
                                                        int timeout_ms) {
    // Validate request
    if (!validate_request(agent_name, function_name, parameters)) {
        throw std::invalid_argument("Invalid request parameters");
    }

    // Check queue size
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (request_queue_.size() >= max_queue_size_) {
            throw std::runtime_error("Request queue is full");
        }
    }

    // Resolve agent identifier to ensure we store the agent name
    std::string actual_agent_name = agent_name;
    
    // Check if the agent_name is actually an agent ID (UUID format)
    // If so, resolve it to the actual agent name
    if (agent_manager_->agent_exists(agent_name)) {
        std::string resolved_name = agent_manager_->get_agent_name_by_id(agent_name);
        if (!resolved_name.empty()) {
            actual_agent_name = resolved_name;
        }
    }
    
    // Create request
    std::string request_id = generate_request_id();
    auto request = std::make_shared<WorkflowRequest>(
        request_id, actual_agent_name, function_name, parameters, timeout_ms
    );    // Add to queue
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        request_queue_.push(request);
        stats_.queue_size = request_queue_.size();
    }
    
    // Add to active requests
    {
        std::lock_guard<std::mutex> lock(requests_mutex_);
        active_requests_[request_id] = request;
    }
    
    // Update statistics
    stats_.total_requests++;
    stats_.active_requests++;
    
    // Notify workers
    queue_condition_.notify_one();
    
    return request_id;
}

std::shared_ptr<WorkflowRequest> WorkflowManager::get_request_status(const std::string& request_id) {
    std::lock_guard<std::mutex> lock(requests_mutex_);
    
    // Check active requests first
    auto it = active_requests_.find(request_id);
    if (it != active_requests_.end()) {
        return it->second;
    }
    
    // Check completed requests
    auto completed_it = completed_requests_.find(request_id);
    if (completed_it != completed_requests_.end()) {
        return completed_it->second;
    }
    
    return nullptr;
}

json WorkflowManager::get_request_result(const std::string& request_id) {
    auto request = get_request_status(request_id);
    if (!request) {
        return json{{"error", "Request not found"}};
    }
    
    return json{
        {"request_id", request->id},
        {"state", WorkflowUtils::state_to_string(request->state)},
        {"result", request->result},
        {"error", request->error},
        {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(
            request->timestamp.time_since_epoch()).count()}
    };
}

bool WorkflowManager::cancel_request(const std::string& request_id) {
    std::lock_guard<std::mutex> lock(requests_mutex_);
    
    auto it = active_requests_.find(request_id);
    if (it != active_requests_.end() && it->second->state == WorkflowState::PENDING) {
        it->second->state = WorkflowState::CANCELLED;
        it->second->error = "Request cancelled by user";
        move_to_completed(it->second);
        return true;
    }
    
    return false;
}

json WorkflowManager::list_active_requests() {
    std::lock_guard<std::mutex> lock(requests_mutex_);
    
    json result = json::array();
    for (const auto& pair : active_requests_) {
        result.push_back(WorkflowUtils::request_to_json(*pair.second));
    }
    
    return result;
}

json WorkflowManager::list_recent_requests(size_t limit) {
    std::lock_guard<std::mutex> lock(requests_mutex_);
    
    json result = json::array();
    size_t count = 0;
    
    // Add active requests first
    for (const auto& pair : active_requests_) {
        if (count >= limit) break;
        result.push_back(WorkflowUtils::request_to_json(*pair.second));
        count++;
    }
    
    // Add recent completed requests
    for (const auto& pair : completed_requests_) {
        if (count >= limit) break;
        result.push_back(WorkflowUtils::request_to_json(*pair.second));
        count++;
    }
    
    return result;
}

void WorkflowManager::cleanup_completed_requests(size_t keep_count) {
    std::lock_guard<std::mutex> lock(requests_mutex_);
    
    if (completed_requests_.size() <= keep_count) {
        return;
    }
    
    // Create a vector of pairs for sorting by timestamp
    std::vector<std::pair<std::chrono::system_clock::time_point, std::string>> requests_by_time;
    for (const auto& pair : completed_requests_) {
        requests_by_time.emplace_back(pair.second->timestamp, pair.first);
    }
    
    // Sort by timestamp (newest first)
    std::sort(requests_by_time.begin(), requests_by_time.end(), 
              std::greater<std::pair<std::chrono::system_clock::time_point, std::string>>());
    
    // Remove old requests
    for (size_t i = keep_count; i < requests_by_time.size(); ++i) {
        completed_requests_.erase(requests_by_time[i].second);
    }
}

WorkflowStats WorkflowManager::get_statistics() const {
    WorkflowStats stats;
    stats.total_requests = stats_.total_requests.load();
    stats.completed_requests = stats_.completed_requests.load();
    stats.failed_requests = stats_.failed_requests.load();
    stats.timeout_requests = stats_.timeout_requests.load();
    stats.active_requests = stats_.active_requests.load();
    stats.queue_size = stats_.queue_size.load();
    return stats;
}

json WorkflowManager::get_system_status() {
    auto stats = get_statistics();
    
    return json{
        {"running", running_.load()},
        {"worker_threads", worker_threads_.size()},
        {"max_workers", max_workers_},
        {"max_queue_size", max_queue_size_},
        {"statistics", {
            {"total_requests", stats.total_requests.load()},
            {"completed_requests", stats.completed_requests.load()},
            {"failed_requests", stats.failed_requests.load()},
            {"timeout_requests", stats.timeout_requests.load()},
            {"active_requests", stats.active_requests.load()},
            {"queue_size", stats.queue_size.load()}
        }}
    };
}

bool WorkflowManager::validate_request(const std::string& agent_name, 
                                      const std::string& function_name,
                                      const json& parameters) {
    // Check if agent exists
    if (!agent_manager_->agent_exists(agent_name)) {
        return false;
    }
    
    // Check if function is configured
    if (function_configs_.find(function_name) == function_configs_.end()) {
        return false;
    }
    
    try {
        validate_function_parameters(function_name, parameters);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void WorkflowManager::worker_thread() {
    while (running_.load()) {
        std::shared_ptr<WorkflowRequest> request;
        
        // Get next request from queue
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_condition_.wait(lock, [this] { 
                return !request_queue_.empty() || !running_.load(); 
            });
            
            if (!running_.load()) {
                break;
            }
            
            if (!request_queue_.empty()) {
                request = request_queue_.front();
                request_queue_.pop();
                stats_.queue_size = request_queue_.size();
            }
        }
        
        if (request) {
            process_request(request);
        }
    }
}

void WorkflowManager::process_request(std::shared_ptr<WorkflowRequest> request) {
    request->state = WorkflowState::PROCESSING;
    
    try {
        execute_request_with_timeout(request);
    } catch (const std::exception& e) {
        request->state = WorkflowState::FAILED;
        request->error = e.what();
        stats_.failed_requests++;
    }
    
    // Move to completed requests
    move_to_completed(request);
    stats_.active_requests--;
}

void WorkflowManager::execute_request_with_timeout(std::shared_ptr<WorkflowRequest> request) {
    // Create a future for the execution
    auto future = std::async(std::launch::async, [this, request]() -> json {
        // Resolve agent name to agent ID if needed
        std::string agent_identifier = request->agent_name;
        
        // Check if the agent_name is actually an agent ID (UUID format)
        // If not, try to resolve it as an agent name
        if (!agent_manager_->agent_exists(agent_identifier)) {
            std::string resolved_id = agent_manager_->get_agent_id_by_name(agent_identifier);
            if (!resolved_id.empty()) {
                agent_identifier = resolved_id;
            } else {
                throw std::runtime_error("Agent not found: " + agent_identifier);
            }
        }
        
        return agent_manager_->execute_agent_function(
            agent_identifier, 
            request->function_name, 
            request->parameters
        );
    });
    
    // Wait for completion or timeout
    auto status = future.wait_for(std::chrono::milliseconds(request->timeout_ms));
    
    if (status == std::future_status::ready) {
        try {
            request->result = future.get();
            request->state = WorkflowState::COMPLETED;
            stats_.completed_requests++;
        } catch (const std::exception& e) {
            request->state = WorkflowState::FAILED;
            request->error = e.what();
            stats_.failed_requests++;
        }
    } else {
        request->state = WorkflowState::TIMEOUT;
        request->error = "Request execution timed out";
        stats_.timeout_requests++;
    }
}

std::string WorkflowManager::generate_request_id() {
    // Generate a simple UUID-like string
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 32; ++i) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            ss << "-";
        }
        ss << dis(gen);
    }
    
    return ss.str();
}

void WorkflowManager::validate_function_parameters(const std::string& function_name, const json& parameters) {
    if (function_configs_.find(function_name) == function_configs_.end()) {
        throw std::invalid_argument("Unknown function: " + function_name);
    }
    
    const auto& function_config = function_configs_[function_name];
    if (!function_config.contains("parameters")) {
        return; // No parameters required
    }
    
    for (const auto& param_config : function_config["parameters"]) {
        std::string param_name = param_config["name"];
        bool required = param_config.value("required", false);
        
        if (required && !parameters.contains(param_name)) {
            throw std::invalid_argument("Required parameter missing: " + param_name);
        }
    }
}

void WorkflowManager::move_to_completed(std::shared_ptr<WorkflowRequest> request) {
    std::lock_guard<std::mutex> lock(requests_mutex_);
    
    // Remove from active requests
    active_requests_.erase(request->id);
    
    // Add to completed requests
    completed_requests_[request->id] = request;
    
    // Cleanup if needed
    if (completed_requests_.size() > max_completed_history_) {
        cleanup_old_requests();
    }
}

void WorkflowManager::cleanup_old_requests() {
    // This is called while holding the requests_mutex_
    // Keep only the most recent requests
    size_t target_size = max_completed_history_ * 0.8; // Keep 80% of max
    
    if (completed_requests_.size() <= target_size) {
        return;
    }
    
    // Create vector for sorting
    std::vector<std::pair<std::chrono::system_clock::time_point, std::string>> requests_by_time;
    for (const auto& pair : completed_requests_) {
        requests_by_time.emplace_back(pair.second->timestamp, pair.first);
    }
    
    // Sort by timestamp (oldest first)
    std::sort(requests_by_time.begin(), requests_by_time.end());
    
    // Remove oldest requests
    size_t to_remove = completed_requests_.size() - target_size;
    for (size_t i = 0; i < to_remove; ++i) {
        completed_requests_.erase(requests_by_time[i].second);
    }
}

// Workflow utilities implementation
namespace WorkflowUtils {
    std::string state_to_string(WorkflowState state) {
        switch (state) {
            case WorkflowState::PENDING: return "pending";
            case WorkflowState::PROCESSING: return "processing";
            case WorkflowState::COMPLETED: return "completed";
            case WorkflowState::FAILED: return "failed";
            case WorkflowState::TIMEOUT: return "timeout";
            case WorkflowState::CANCELLED: return "cancelled";
            default: return "unknown";
        }
    }
    
    WorkflowState string_to_state(const std::string& state_str) {
        if (state_str == "pending") return WorkflowState::PENDING;
        if (state_str == "processing") return WorkflowState::PROCESSING;
        if (state_str == "completed") return WorkflowState::COMPLETED;
        if (state_str == "failed") return WorkflowState::FAILED;
        if (state_str == "timeout") return WorkflowState::TIMEOUT;
        if (state_str == "cancelled") return WorkflowState::CANCELLED;
        return WorkflowState::PENDING;
    }
    
    json request_to_json(const WorkflowRequest& request) {
        return json{
            {"id", request.id},
            {"agent_name", request.agent_name},
            {"function_name", request.function_name},
            {"parameters", request.parameters},
            {"state", state_to_string(request.state)},
            {"result", request.result},
            {"error", request.error},
            {"timeout_ms", request.timeout_ms},
            {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(
                request.timestamp.time_since_epoch()).count()}
        };
    }
    
    std::string format_duration(const std::chrono::system_clock::time_point& start) {
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        return std::to_string(duration.count()) + "ms";
    }
}
