#pragma once

#include "agent.hpp"
#include "agent_config.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <json.hpp>

using json = nlohmann::json;

/**
 * @brief Agent Manager - Agent lifecycle management with configuration support
 */
class AgentManager {
private:
    std::map<std::string, std::unique_ptr<Agent>> agents_;
    std::shared_ptr<AgentConfigManager> config_manager_;
    
public:
    AgentManager();
    explicit AgentManager(std::shared_ptr<AgentConfigManager> config_manager);
    ~AgentManager() = default;
    
    // Configuration
    bool load_configuration(const std::string& config_file = "agent.yaml");
    std::shared_ptr<AgentConfigManager> get_config_manager() const { return config_manager_; }
    
    // Agent lifecycle
    std::string create_agent(const std::string& name, const std::vector<std::string>& capabilities = {});
    std::string create_agent_with_config(const std::string& name, const json& config);
    std::string create_agent_from_config(const AgentSystemConfig::AgentConfig& agent_config);
    bool start_agent(const std::string& agent_id);
    void stop_agent(const std::string& agent_id);
    bool delete_agent(const std::string& agent_id);
    
    // Initialize default agents from configuration
    void initialize_default_agents();
    
    // Agent access
    Agent* get_agent(const std::string& agent_id);
    bool agent_exists(const std::string& agent_id) const;
    std::string get_agent_id_by_name(const std::string& agent_name) const;
    std::string get_agent_name_by_id(const std::string& agent_id) const;
    
    // Bulk operations
    json list_agents() const;
    void stop_all_agents();
    
    // Function execution
    json execute_agent_function(const std::string& agent_id, 
                                const std::string& function_name, 
                                const json& params);
                                
private:
    // Internal methods
    void load_model_configurations();
};
