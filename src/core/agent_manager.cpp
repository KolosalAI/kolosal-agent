#include "agent_manager.hpp"
#include "logger.hpp"
#include <filesystem>

namespace fs = std::filesystem;
#include <iostream>
#include <algorithm>

AgentManager::AgentManager() {
    TRACE_FUNCTION();
    config_manager_ = std::make_shared<AgentConfigManager>();
    initialize_server_launcher();
    LOG_DEBUG("AgentManager created with default configuration");
}

AgentManager::AgentManager(std::shared_ptr<AgentConfigManager> config_manager) 
    : config_manager_(config_manager) {
    TRACE_FUNCTION();
    if (!config_manager_) {
        config_manager_ = std::make_shared<AgentConfigManager>();
        LOG_WARN("Null config_manager provided, created default one");
    } else {
        LOG_DEBUG("AgentManager created with provided configuration");
    }
    initialize_server_launcher();
}

bool AgentManager::load_configuration(const std::string& config_file) {
    if (!config_manager_) {
        config_manager_ = std::make_shared<AgentConfigManager>();
    }
    
    bool loaded = config_manager_->load_config(config_file);
    if (loaded && config_manager_->validate_config()) {
        config_manager_->print_config_summary();
        
        // Load model configurations for all agents after config is loaded
        load_model_configurations();
        
        return true;
    }
    return loaded; // Still return true even if validation shows warnings
}

std::string AgentManager::create_agent(const std::string& name, const std::vector<std::string>& capabilities) {
    TRACE_FUNCTION();
    SCOPED_TIMER("create_agent_" + name);
    
    LOG_DEBUG_F("Creating agent '%s' with %zu capabilities", name.c_str(), capabilities.size());
    
    auto agent = std::make_unique<Agent>(name);
    std::string agent_id = agent->get_id();
    
    // Add capabilities
    for (const auto& capability : capabilities) {
        agent->add_capability(capability);
        LOG_DEBUG_F("Added capability '%s' to agent '%s'", capability.c_str(), name.c_str());
    }
    
    // Apply system instruction if available
    if (config_manager_) {
        const std::string& system_instruction = config_manager_->get_system_instruction();
        if (!system_instruction.empty()) {
            agent->set_system_instruction(system_instruction);
            LOG_DEBUG_F("Applied system instruction to agent '%s' (length: %zu)", name.c_str(), system_instruction.length());
        }
    }
    
    agents_[agent_id] = std::move(agent);
    
    LOG_INFO_F("Created agent '%s' with ID: %s", name.c_str(), agent_id.c_str());
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
    
    // Configure models if available
    if (config_manager_) {
        const auto& system_config = config_manager_->get_config();
        json model_configs = json::array();
        
        // Use model configurations from agent.yaml if available
        if (!system_config.models.empty()) {
            for (const auto& [model_id, model_config] : system_config.models) {
                json model_json;
                model_json["id"] = model_config.id;
                model_json["actual_name"] = model_config.actual_name;
                model_json["name"] = model_config.actual_name;  // For backward compatibility
                model_json["type"] = model_config.type;
                if (!model_config.description.empty()) {
                    model_json["description"] = model_config.description;
                }
                model_configs.push_back(model_json);
            }
        } else {
            // Fall back to using agent model names
            for (const auto& agent_config : system_config.agents) {
                if (!agent_config.model.empty()) {
                    json model_json;
                    model_json["id"] = agent_config.model;
                    model_json["name"] = agent_config.model;
                    model_json["type"] = "llm";
                    model_configs.push_back(model_json);
                }
            }
        }
        
        if (!model_configs.empty()) {
            agent->configure_models(model_configs);
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
    
    // Ensure all agents have model configurations loaded
    load_model_configurations();
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

void AgentManager::load_model_configurations() {
    if (!config_manager_) {
        LOG_WARN("No config manager available for loading model configurations");
        return;
    }
    
    // Get model configurations from config manager
    const auto& system_config = config_manager_->get_config();
    json model_configs = json::array();
    
    // Use model configurations from agent.yaml if available
    if (!system_config.models.empty()) {
        for (const auto& [model_id, model_config] : system_config.models) {
            json model_json;
            model_json["id"] = model_config.id;
            model_json["actual_name"] = model_config.actual_name;
            model_json["name"] = model_config.actual_name;  // For backward compatibility
            model_json["type"] = model_config.type;
            if (!model_config.description.empty()) {
                model_json["description"] = model_config.description;
            }
            model_configs.push_back(model_json);
        }
    } else {
        // Fall back to using agent model names
        for (const auto& agent_config : system_config.agents) {
            if (!agent_config.model.empty()) {
                json model_json;
                model_json["id"] = agent_config.model;
                model_json["name"] = agent_config.model;
                model_json["type"] = "llm";
                model_configs.push_back(model_json);
            }
        }
    }
    
    // Pass model configurations to all existing agents
    for (auto& [agent_id, agent] : agents_) {
        agent->configure_models(model_configs);
    }
    
    LOG_INFO_F("Loaded model configurations for %zu agents", agents_.size());
}

bool AgentManager::start_kolosal_server() {
    TRACE_FUNCTION();
    
    if (!server_launcher_) {
        LOG_ERROR("Server launcher not initialized");
        return false;
    }
    
    if (server_launcher_->is_running()) {
        LOG_INFO("Kolosal server is already running");
        return true;
    }
    
    LOG_INFO("Starting Kolosal server");
    bool success = server_launcher_->start();
    
    if (success) {
        LOG_INFO_F("Kolosal server started successfully at %s", server_launcher_->get_server_url().c_str());
        
        // Update all agents with the correct server URL
        for (auto& [agent_id, agent] : agents_) {
            if (agent) {
                // Re-configure model interface with correct server URL
                const auto& system_config = config_manager_->get_config();
                json model_configs = json::array();
                
                for (const auto& [model_id, model_config] : system_config.models) {
                    json model_json;
                    model_json["id"] = model_config.id;
                    model_json["actual_name"] = model_config.actual_name;
                    model_json["name"] = model_config.actual_name;
                    model_json["type"] = model_config.type;
                    model_json["server_url"] = server_launcher_->get_server_url();
                    if (!model_config.description.empty()) {
                        model_json["description"] = model_config.description;
                    }
                    model_configs.push_back(model_json);
                }
                
                if (!model_configs.empty()) {
                    agent->configure_models(model_configs);
                }
            }
        }
    } else {
        LOG_ERROR("Failed to start Kolosal server");
    }
    
    return success;
}

bool AgentManager::stop_kolosal_server() {
    TRACE_FUNCTION();
    
    if (!server_launcher_) {
        LOG_DEBUG("Server launcher not initialized");
        return true;
    }
    
    if (!server_launcher_->is_running()) {
        LOG_DEBUG("Kolosal server is already stopped");
        return true;
    }
    
    LOG_INFO("Stopping Kolosal server");
    bool success = server_launcher_->stop();
    
    if (success) {
        LOG_INFO("Kolosal server stopped successfully");
    } else {
        LOG_ERROR("Failed to stop Kolosal server");
    }
    
    return success;
}

bool AgentManager::is_kolosal_server_running() const {
    return server_launcher_ && server_launcher_->is_running();
}

std::string AgentManager::get_kolosal_server_url() const {
    if (server_launcher_) {
        return server_launcher_->get_server_url();
    }
    return "";
}

KolosalServerLauncher::Status AgentManager::get_kolosal_server_status() const {
    if (server_launcher_) {
        return server_launcher_->get_status();
    }
    return KolosalServerLauncher::Status::STOPPED;
}

void AgentManager::initialize_server_launcher() {
    TRACE_FUNCTION();
    
    // Get current working directory
    std::string workspace_path = fs::current_path().string();
    
    // Create default server configuration
    auto server_config = create_default_server_config(workspace_path);
    
    // Override with configuration from config manager if available
    if (config_manager_) {
        const auto& system_config = config_manager_->get_config();
        
        // Check if there's server configuration in the system config
        // For now, we use hardcoded defaults but this could be extended
        // to read from the config.yaml file
        server_config.port = 8081;  // Use different port from main agent system
        server_config.host = "127.0.0.1";
        server_config.log_level = "INFO";
        server_config.quiet_mode = false;
        
        // Look for kolosal-server executable in the build directory
        std::vector<std::string> search_paths = {
            workspace_path + "/build/Debug/kolosal-server.exe",
            workspace_path + "/build/Release/kolosal-server.exe", 
            workspace_path + "/build/kolosal-server/Debug/kolosal-server.exe",
            workspace_path + "/build/kolosal-server/Release/kolosal-server.exe",
            workspace_path + "/kolosal-server/build/Debug/kolosal-server.exe",
            workspace_path + "/kolosal-server/build/Release/kolosal-server.exe"
        };
        
        for (const auto& path : search_paths) {
            if (fs::exists(path)) {
                server_config.executable_path = path;
                LOG_DEBUG_F("Found kolosal-server executable: %s", path.c_str());
                break;
            }
        }
        
        // Set config file path if it exists
        std::string config_file_path = workspace_path + "/config.yaml";
        if (fs::exists(config_file_path)) {
            server_config.config_file = config_file_path;
            LOG_DEBUG_F("Using config file: %s", config_file_path.c_str());
        }
    }
    
    // Create the server launcher
    server_launcher_ = std::make_unique<KolosalServerLauncher>(server_config);
    
    // Set up status callback
    setup_server_status_callback();
    
    LOG_INFO("Kolosal server launcher initialized");
}

void AgentManager::setup_server_status_callback() {
    if (!server_launcher_) {
        return;
    }
    
    server_launcher_->set_status_callback([this](KolosalServerLauncher::Status status, const std::string& message) {
        LOG_INFO_F("Kolosal server status changed: %s (%s)", 
                   server_launcher_->get_status_string().c_str(), 
                   message.c_str());
        
        // If server becomes ready, update all agents
        if (status == KolosalServerLauncher::Status::RUNNING) {
            load_model_configurations();
        }
    });
}
