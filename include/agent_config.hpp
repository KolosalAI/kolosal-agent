#pragma once

#include <string>
#include <vector>
#include <map>
#include <json.hpp>

using json = nlohmann::json;

/**
 * @brief Configuration structure for agent system
 */
struct AgentSystemConfig {
    // System settings
    struct {
        std::string name;
        std::string version;
        std::string host;
        int port;
        std::string log_level;
        int max_concurrent_requests;
    } system;
    
    // System instruction for all agents
    std::string system_instruction;
    
    // Agent configurations
    struct AgentConfig {
        std::string name;
        std::vector<std::string> capabilities;
        bool auto_start;
        std::string model;
        std::string system_prompt;
    };
    std::vector<AgentConfig> agents;
    
    // Function definitions
    struct FunctionConfig {
        std::string description;
        int timeout;
        std::vector<json> parameters;
    };
    std::map<std::string, FunctionConfig> functions;
    
    // Performance settings
    struct {
        std::string max_memory_usage;
        std::string cache_size;
        int worker_threads;
        int request_timeout;
        std::string max_request_size;
    } performance;
    
    // Logging settings
    struct {
        std::string level;
        std::string file;
        std::string max_file_size;
        int max_files;
        bool console_output;
    } logging;
    
    // Security settings
    struct {
        bool enable_cors;
        std::vector<std::string> allowed_origins;
        int max_request_rate;
        bool enable_auth;
        std::string api_key;
    } security;
};

/**
 * @brief Configuration manager for the agent system
 */
class AgentConfigManager {
private:
    AgentSystemConfig config_;
    std::string config_file_path_;
    
    // Internal methods
    bool load_from_file(const std::string& file_path);
    void set_default_config();
    
public:
    AgentConfigManager();
    ~AgentConfigManager() = default;
    
    // Configuration loading
    bool load_config(const std::string& file_path = "agent.yaml");
    bool reload_config();
    
    // Configuration access
    const AgentSystemConfig& get_config() const { return config_; }
    const std::string& get_config_file_path() const { return config_file_path_; }
    
    // Specific configuration getters
    std::string get_system_instruction() const { return config_.system_instruction; }
    std::string get_host() const { return config_.system.host; }
    int get_port() const { return config_.system.port; }
    const std::vector<AgentSystemConfig::AgentConfig>& get_agent_configs() const { return config_.agents; }
    const std::map<std::string, AgentSystemConfig::FunctionConfig>& get_function_configs() const { return config_.functions; }
    
    // Configuration validation
    bool validate_config() const;
    
    // Configuration info
    void print_config_summary() const;
    json to_json() const;
};
