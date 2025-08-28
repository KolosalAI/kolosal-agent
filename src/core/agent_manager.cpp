#include "../include/agent_manager.hpp"
#include <iostream>
#include <algorithm>

AgentManager::AgentManager() {
    config_manager_ = std::make_shared<AgentConfigManager>();
}

AgentManager::AgentManager(std::shared_ptr<AgentConfigManager> config_manager) 
    : config_manager_(config_manager) {
    if (!config_manager_) {
        config_manager_ = std::make_shared<AgentConfigManager>();
    }
}

bool AgentManager::load_configuration(const std::string& config_file) {
    if (!config_manager_) {
        config_manager_ = std::make_shared<AgentConfigManager>();
    }
    
    bool loaded = config_manager_->load_config(config_file);
    if (loaded && config_manager_->validate_config()) {
        config_manager_->print_config_summary();
        return true;
    }
    return loaded; // Still return true even if validation shows warnings
}

std::string AgentManager::create_agent(const std::string& name, const std::vector<std::string>& capabilities) {
    auto agent = std::make_unique<Agent>(name);
    std::string agent_id = agent->get_id();
    
    // Add capabilities
    for (const auto& capability : capabilities) {
        agent->add_capability(capability);
    }
    
    // Apply system instruction if available
    if (config_manager_) {
        const std::string& system_instruction = config_manager_->get_system_instruction();
        if (!system_instruction.empty()) {
            agent->set_system_instruction(system_instruction);
        }
    }
    
    agents_[agent_id] = std::move(agent);
    
    std::cout << "Created agent '" << name << "' with ID: " << agent_id << "\n";
    return agent_id;
}

std::string AgentManager::create_agent_with_config(const std::string& name, const json& config) {
    auto agent = std::make_unique<Agent>(name);
    std::string agent_id = agent->get_id();
    
#ifdef BUILD_WITH_RETRIEVAL
    // Configure retrieval if specified
    agent->configure_retrieval(config);
#endif
    
    // Add capabilities from config
    if (config.contains("capabilities") && config["capabilities"].is_array()) {
        for (const auto& capability : config["capabilities"]) {
            if (capability.is_string() && !capability.is_null()) {
                agent->add_capability(capability.get<std::string>());
            }
        }
    }
    
    // Apply system instruction if available
    if (config_manager_) {
        const std::string& system_instruction = config_manager_->get_system_instruction();
        if (!system_instruction.empty()) {
            agent->set_system_instruction(system_instruction);
        }
    }
    
    // Apply agent-specific system prompt if provided
    if (config.contains("system_prompt") && !config["system_prompt"].is_null() && !config["system_prompt"].empty()) {
        std::string system_prompt = config.value("system_prompt", "");
        if (!system_prompt.empty()) {
            agent->set_agent_specific_prompt(system_prompt);
        }
    }
    
    agents_[agent_id] = std::move(agent);
    
    std::cout << "Created agent '" << name << "' with config, ID: " << agent_id << "\n";
    return agent_id;
}

std::string AgentManager::create_agent_from_config(const AgentSystemConfig::AgentConfig& agent_config) {
    json config_json;
    config_json["capabilities"] = agent_config.capabilities;
    config_json["system_prompt"] = agent_config.system_prompt;
    
    std::string agent_id = create_agent_with_config(agent_config.name, config_json);
    
    // Auto-start if configured
    if (agent_config.auto_start) {
        start_agent(agent_id);
    }
    
    return agent_id;
}

void AgentManager::initialize_default_agents() {
    if (!config_manager_) {
        std::cout << "No configuration manager available for default agents\n";
        return;
    }
    
    const auto& agent_configs = config_manager_->get_agent_configs();
    
    std::cout << "Initializing " << agent_configs.size() << " default agents from configuration...\n";
    
    for (const auto& agent_config : agent_configs) {
        try {
            std::string agent_id = create_agent_from_config(agent_config);
            std::cout << "  - " << agent_config.name << " (" << agent_id << ")"
                      << (agent_config.auto_start ? " [auto-started]" : "") << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Failed to create agent '" << agent_config.name << "': " << e.what() << "\n";
        }
    }
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

std::string AgentManager::get_agent_id_by_name(const std::string& agent_name) const {
    for (const auto& [agent_id, agent] : agents_) {
        if (agent->get_name() == agent_name) {
            return agent_id;
        }
    }
    return ""; // Empty string if not found
}

std::string AgentManager::get_agent_name_by_id(const std::string& agent_id) const {
    auto it = agents_.find(agent_id);
    if (it != agents_.end()) {
        return it->second->get_name();
    }
    return ""; // Empty string if not found
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
