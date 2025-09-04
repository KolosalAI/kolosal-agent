#pragma once

#include "agent.hpp"
#include "agent_config.hpp"
#include "server_launcher.hpp"
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
    std::unique_ptr<KolosalServerLauncher> server_launcher_;
    
public:
    AgentManager();
    explicit AgentManager(std::shared_ptr<AgentConfigManager> config_manager);
    ~AgentManager() = default;
    
    // Configuration
    bool load_configuration(const std::string& config_file = "agent.yaml");
    std::shared_ptr<AgentConfigManager> get_config_manager() const { return config_manager_; }
    
    // Server management
    bool start_kolosal_server();
    bool stop_kolosal_server();
    bool is_kolosal_server_running() const;
    std::string get_kolosal_server_url() const;
    json get_kolosal_server_status() const;
    
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
    size_t get_active_agent_count() const;
    
    // Function execution
    json execute_agent_function(const std::string& agent_id, 
                                const std::string& function_name, 
                                const json& params);
                                
private:
    // Internal methods
    void load_model_configurations();
    void initialize_server_launcher();
    void setup_server_status_callback();
};
