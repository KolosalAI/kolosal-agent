#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <atomic>
#include <json.hpp>

using json = nlohmann::json;

#ifdef BUILD_WITH_RETRIEVAL
// Forward declaration
class RetrievalManager;
#endif

/**
 * @brief Agent Class - Core functionality
 */
class Agent {
private:
    std::string id_;
    std::string name_;
    std::vector<std::string> capabilities_;
    std::map<std::string, std::function<json(const json&)>> functions_;
    std::atomic<bool> running_{false};
    
#ifdef BUILD_WITH_RETRIEVAL
    std::unique_ptr<RetrievalManager> retrieval_manager_;
#endif
    
public:
    explicit Agent(const std::string& name);
    ~Agent() = default;
    
    // Core lifecycle
    bool start();
    void stop();
    bool is_running() const { return running_.load(); }
    
    // Function execution
    json execute_function(const std::string& function_name, const json& params);
    void register_function(const std::string& name, std::function<json(const json&)> func);
    
    // Capability management
    void add_capability(const std::string& capability);
    const std::vector<std::string>& get_capabilities() const { return capabilities_; }
    
    // Information
    json get_info() const;
    const std::string& get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    
    // Built-in functions
    void setup_builtin_functions();
    
#ifdef BUILD_WITH_RETRIEVAL
    // Retrieval functions
    void setup_retrieval_functions();
    void configure_retrieval(const json& config);
#endif
};
