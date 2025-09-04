#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <atomic>
#include <json.hpp>
#include "model_interface.hpp"

using json = nlohmann::json;

#ifdef BUILD_WITH_RETRIEVAL
#include "retrieval_manager.hpp"
#endif

// Include enhanced retrieval functions
#include "functions/retrieval.hpp"

/**
 * @brief Agent Class - Core functionality with system prompt support
 */
class Agent {
private:
    std::string id_;
    std::string name_;
    std::vector<std::string> capabilities_;
    std::map<std::string, std::function<json(const json&)>> functions_;
    std::atomic<bool> running_{false};
    
    // System instructions and prompts
    std::string system_instruction_;
    std::string agent_specific_prompt_;
    
    // Model interface for AI communication
    std::unique_ptr<ModelInterface> model_interface_;
    
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
    
    // System instruction and prompt management
    void set_system_instruction(const std::string& instruction);
    void set_agent_specific_prompt(const std::string& prompt);
    const std::string& get_system_instruction() const { return system_instruction_; }
    const std::string& get_agent_specific_prompt() const { return agent_specific_prompt_; }
    std::string get_combined_prompt() const;
    
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
    
    // Model configuration
    void configure_models(const json& model_configs);
    
    // Helper functions
    json create_research_function_response(const std::string& function_name, const json& params, const std::string& task_description);
    
#ifdef BUILD_WITH_RETRIEVAL
    // Retrieval functions
    void setup_retrieval_functions();
    void configure_retrieval(const json& config);
    
    // Deep research functions
    void setup_deep_research_functions();
#endif
};
