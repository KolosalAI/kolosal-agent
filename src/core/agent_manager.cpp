#include "../include/agent_manager.hpp"
#include <iostream>
#include <algorithm>

std::string AgentManager::create_agent(const std::string& name, const std::vector<std::string>& capabilities) {
    auto agent = std::make_unique<Agent>(name);
    std::string agent_id = agent->get_id();
    
    // Add capabilities
    for (const auto& capability : capabilities) {
        agent->add_capability(capability);
    }
    
    agents_[agent_id] = std::move(agent);
    
    std::cout << "Created agent '" << name << "' with ID: " << agent_id << "\n";
    return agent_id;
}

bool AgentManager::start_agent(const std::string& agent_id) {
    auto it = agents_.find(agent_id);
    if (it == agents_.end()) {
        return false;
    }
    
    return it->second->start();
}

void AgentManager::stop_agent(const std::string& agent_id) {
    auto it = agents_.find(agent_id);
    if (it != agents_.end()) {
        it->second->stop();
    }
}

bool AgentManager::delete_agent(const std::string& agent_id) {
    auto it = agents_.find(agent_id);
    if (it == agents_.end()) {
        return false;
    }
    
    it->second->stop();
    agents_.erase(it);
    std::cout << "Deleted agent with ID: " << agent_id << "\n";
    return true;
}

Agent* AgentManager::get_agent(const std::string& agent_id) {
    auto it = agents_.find(agent_id);
    return (it != agents_.end()) ? it->second.get() : nullptr;
}

bool AgentManager::agent_exists(const std::string& agent_id) const {
    return agents_.find(agent_id) != agents_.end();
}

json AgentManager::list_agents() const {
    json agent_list = json::array();
    
    for (const auto& [agent_id, agent] : agents_) {
        agent_list.push_back(agent->get_info());
    }
    
    json response;
    response["agents"] = agent_list;
    response["total_count"] = agents_.size();
    response["running_count"] = std::count_if(agents_.begin(), agents_.end(),
        [](const auto& pair) { return pair.second->is_running(); });
    
    return response;
}

void AgentManager::stop_all_agents() {
    for (auto& [agent_id, agent] : agents_) {
        agent->stop();
    }
    std::cout << "Stopped all agents\n";
}

json AgentManager::execute_agent_function(const std::string& agent_id, 
                                          const std::string& function_name, 
                                          const json& params) {
    auto agent = get_agent(agent_id);
    if (!agent) {
        throw std::runtime_error("Agent not found: " + agent_id);
    }
    
    return agent->execute_function(function_name, params);
}
