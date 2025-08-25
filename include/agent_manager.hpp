#pragma once

#include "agent.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <json.hpp>

using json = nlohmann::json;

/**
 * @brief Agent Manager - Agent lifecycle management
 */
class AgentManager {
private:
    std::map<std::string, std::unique_ptr<Agent>> agents_;
    
public:
    AgentManager() = default;
    ~AgentManager() = default;
    
    // Agent lifecycle
    std::string create_agent(const std::string& name, const std::vector<std::string>& capabilities = {});
    bool start_agent(const std::string& agent_id);
    void stop_agent(const std::string& agent_id);
    bool delete_agent(const std::string& agent_id);
    
    // Agent access
    Agent* get_agent(const std::string& agent_id);
    bool agent_exists(const std::string& agent_id) const;
    
    // Bulk operations
    json list_agents() const;
    void stop_all_agents();
    
    // Function execution
    json execute_agent_function(const std::string& agent_id, 
                                const std::string& function_name, 
                                const json& params);
};
