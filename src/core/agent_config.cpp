#include "../include/agent_config.hpp"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <filesystem>

AgentConfigManager::AgentConfigManager() {
    set_default_config();
}

bool AgentConfigManager::load_config(const std::string& file_path) {
    // If a specific file path is provided, only try that file
    if (!file_path.empty() && file_path != "agent.yaml") {
        if (std::filesystem::exists(file_path)) {
            if (load_from_file(file_path)) {
                config_file_path_ = std::filesystem::absolute(file_path).string();
                std::cout << "Loaded agent configuration from: " << config_file_path_ << std::endl;
                return true;
            }
        }
        // For tests expecting failure, don't fall back to default files
        std::cout << "Could not load configuration from: " << file_path << std::endl;
        return false;
    }
    
    // Default behavior: search for agent.yaml in multiple locations
    std::vector<std::string> search_paths = {
        "agent.yaml",
        "./agent.yaml",
        "../agent.yaml"
    };
    
    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            if (load_from_file(path)) {
                config_file_path_ = std::filesystem::absolute(path).string();
                std::cout << "Loaded agent configuration from: " << config_file_path_ << std::endl;
                return true;
            }
        }
    }
    
    std::cout << "Could not find agent.yaml, using default configuration" << std::endl;
    return false;
}

bool AgentConfigManager::reload_config() {
    if (config_file_path_.empty()) {
        return load_config();
    }
    return load_from_file(config_file_path_);
}

bool AgentConfigManager::load_from_file(const std::string& file_path) {
    try {
        YAML::Node config = YAML::LoadFile(file_path);
        
        // Check if config is empty or null
        if (!config || config.IsNull() || config.size() == 0) {
            std::cout << "Configuration file is empty or invalid: " << file_path << std::endl;
            return false;
        }
        
        // Load system configuration
        if (config["system"]) {
            auto system_node = config["system"];
            config_.system.name = system_node["name"].as<std::string>("Kolosal Agent System");
            config_.system.version = system_node["version"].as<std::string>("1.0.0");
            config_.system.host = system_node["host"].as<std::string>("127.0.0.1");
            config_.system.port = system_node["port"].as<int>(8080);
            config_.system.log_level = system_node["log_level"].as<std::string>("info");
            config_.system.max_concurrent_requests = system_node["max_concurrent_requests"].as<int>(100);
        }
        
        // Load system instruction
        if (config["system_instruction"]) {
            config_.system_instruction = config["system_instruction"].as<std::string>();
        }
        
        // Load agent configurations
        config_.agents.clear();
        if (config["agents"]) {
            for (const auto& agent_node : config["agents"]) {
                AgentSystemConfig::AgentConfig agent_config;
                agent_config.name = agent_node["name"].as<std::string>();
                agent_config.auto_start = agent_node["auto_start"].as<bool>(true);
                agent_config.model = agent_node["model"].as<std::string>("default");
                agent_config.system_prompt = agent_node["system_prompt"].as<std::string>("");
                
                if (agent_node["capabilities"]) {
                    for (const auto& capability : agent_node["capabilities"]) {
                        agent_config.capabilities.push_back(capability.as<std::string>());
                    }
                }
                
                config_.agents.push_back(agent_config);
            }
        }
        
        // Load model configurations
        config_.models.clear();
        if (config["models"]) {
            for (const auto& model_node : config["models"]) {
                std::string model_id = model_node.first.as<std::string>();
                AgentSystemConfig::ModelConfig model_config;
                
                auto model_data = model_node.second;
                model_config.id = model_id;
                model_config.actual_name = model_data["actual_name"].as<std::string>(model_id);
                model_config.type = model_data["type"].as<std::string>("llm");
                model_config.description = model_data["description"].as<std::string>("");
                
                config_.models[model_id] = model_config;
            }
        }
        
        // Load function configurations
        config_.functions.clear();
        if (config["functions"]) {
            for (const auto& func_node : config["functions"]) {
                std::string func_name = func_node.first.as<std::string>();
                AgentSystemConfig::FunctionConfig func_config;
                
                auto func_data = func_node.second;
                func_config.description = func_data["description"].as<std::string>("");
                func_config.timeout = func_data["timeout"].as<int>(30000);
                
                if (func_data["parameters"]) {
                    for (const auto& param : func_data["parameters"]) {
                        json param_json;
                        param_json["name"] = param["name"].as<std::string>("");
                        param_json["type"] = param["type"].as<std::string>("string");
                        param_json["required"] = param["required"].as<bool>(false);
                        param_json["description"] = param["description"].as<std::string>("");
                        func_config.parameters.push_back(param_json);
                    }
                }
                
                config_.functions[func_name] = func_config;
            }
        }
        
        // Load performance configuration
        if (config["performance"]) {
            auto perf_node = config["performance"];
            config_.performance.max_memory_usage = perf_node["max_memory_usage"].as<std::string>("2GB");
            config_.performance.cache_size = perf_node["cache_size"].as<std::string>("512MB");
            config_.performance.worker_threads = perf_node["worker_threads"].as<int>(4);
            config_.performance.request_timeout = perf_node["request_timeout"].as<int>(30000);
            config_.performance.max_request_size = perf_node["max_request_size"].as<std::string>("10MB");
        }
        
        // Load logging configuration
        if (config["logging"]) {
            auto log_node = config["logging"];
            config_.logging.level = log_node["level"].as<std::string>("info");
            config_.logging.file = log_node["file"].as<std::string>("agent_system.log");
            config_.logging.max_file_size = log_node["max_file_size"].as<std::string>("100MB");
            config_.logging.max_files = log_node["max_files"].as<int>(10);
            config_.logging.console_output = log_node["console_output"].as<bool>(true);
        }
        
        // Load security configuration
        if (config["security"]) {
            auto sec_node = config["security"];
            config_.security.enable_cors = sec_node["enable_cors"].as<bool>(true);
            config_.security.max_request_rate = sec_node["max_request_rate"].as<int>(100);
            config_.security.enable_auth = sec_node["enable_auth"].as<bool>(false);
            config_.security.api_key = sec_node["api_key"].as<std::string>("");
            
            if (sec_node["allowed_origins"]) {
                config_.security.allowed_origins.clear();
                for (const auto& origin : sec_node["allowed_origins"]) {
                    config_.security.allowed_origins.push_back(origin.as<std::string>());
                }
            }
        }
        
        return true;
        
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML parsing error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Configuration loading error: " << e.what() << std::endl;
        return false;
    }
}

void AgentConfigManager::set_default_config() {
    // Set default system configuration
    config_.system.name = "Kolosal Agent System";
    config_.system.version = "1.0.0";
    config_.system.host = "127.0.0.1";
    config_.system.port = 8080;
    config_.system.log_level = "info";
    config_.system.max_concurrent_requests = 100;
    
    // Set default system instruction
    config_.system_instruction = R"(You are a helpful AI assistant that is part of the Kolosal Agent System. You have been designed to assist users with various tasks including:

- Answering questions and providing information
- Analyzing text and data
- Helping with research and problem-solving
- Providing explanations and tutorials
- Assisting with creative tasks

You should always:
- Be helpful, accurate, and honest
- Admit when you don't know something
- Provide clear and well-structured responses
- Be respectful and professional
- Follow ethical guidelines

Your responses should be informative and helpful while being concise when appropriate.)";
    
    // Set default agents
    config_.agents.clear();
    
    AgentSystemConfig::AgentConfig assistant;
    assistant.name = "Assistant";
    assistant.capabilities = {"chat", "analysis", "reasoning"};
    assistant.auto_start = true;
    assistant.model = "default";
    assistant.system_prompt = "You are an AI assistant specialized in general conversation and help. You excel at answering questions, providing explanations, and helping users with various tasks. Be friendly, helpful, and informative in your responses.";
    config_.agents.push_back(assistant);
    
    AgentSystemConfig::AgentConfig analyzer;
    analyzer.name = "Analyzer";
    analyzer.capabilities = {"analysis", "data_processing", "summarization"};
    analyzer.auto_start = true;
    analyzer.model = "default";
    analyzer.system_prompt = "You are an AI analyst specialized in text and data analysis. Your role is to examine, process, and summarize information effectively. Provide detailed analysis with clear insights and actionable conclusions.";
    config_.agents.push_back(analyzer);
    
    // Set default functions
    config_.functions.clear();
    
    AgentSystemConfig::FunctionConfig chat_func;
    chat_func.description = "Interactive chat functionality";
    chat_func.timeout = 30000;
    json chat_param;
    chat_param["name"] = "message";
    chat_param["type"] = "string";
    chat_param["required"] = true;
    chat_param["description"] = "Message to send to the agent";
    chat_func.parameters.push_back(chat_param);
    config_.functions["chat"] = chat_func;
    
    AgentSystemConfig::FunctionConfig analyze_func;
    analyze_func.description = "Text and data analysis functionality";
    analyze_func.timeout = 60000;
    json analyze_param;
    analyze_param["name"] = "text";
    analyze_param["type"] = "string";
    analyze_param["required"] = true;
    analyze_param["description"] = "Text to analyze";
    analyze_func.parameters.push_back(analyze_param);
    config_.functions["analyze"] = analyze_func;
    
    AgentSystemConfig::FunctionConfig status_func;
    status_func.description = "Agent status information";
    status_func.timeout = 5000;
    config_.functions["status"] = status_func;
    
    // Set default performance settings
    config_.performance.max_memory_usage = "2GB";
    config_.performance.cache_size = "512MB";
    config_.performance.worker_threads = 4;
    config_.performance.request_timeout = 30000;
    config_.performance.max_request_size = "10MB";
    
    // Set default logging settings
    config_.logging.level = "info";
    config_.logging.file = "agent_system.log";
    config_.logging.max_file_size = "100MB";
    config_.logging.max_files = 10;
    config_.logging.console_output = true;
    
    // Set default security settings
    config_.security.enable_cors = true;
    config_.security.allowed_origins = {"http://localhost:3000", "http://127.0.0.1:3000"};
    config_.security.max_request_rate = 100;
    config_.security.enable_auth = false;
    config_.security.api_key = "";
}

bool AgentConfigManager::validate_config() const {
    // Basic validation
    if (config_.system.name.empty()) {
        std::cerr << "System name cannot be empty" << std::endl;
        return false;
    }
    
    if (config_.system.port <= 0 || config_.system.port > 65535) {
        std::cerr << "Invalid port number: " << config_.system.port << std::endl;
        return false;
    }
    
    if (config_.agents.empty()) {
        std::cout << "Warning: No agents configured" << std::endl;
    }
    
    if (config_.functions.empty()) {
        std::cout << "Warning: No functions configured" << std::endl;
    }
    
    return true;
}

void AgentConfigManager::print_config_summary() const {
    std::cout << "\n=== Agent System Configuration ===" << std::endl;
    std::cout << "System: " << config_.system.name << " v" << config_.system.version << std::endl;
    std::cout << "Server: " << config_.system.host << ":" << config_.system.port << std::endl;
    std::cout << "Agents: " << config_.agents.size() << " configured" << std::endl;
    std::cout << "Functions: " << config_.functions.size() << " available" << std::endl;
    std::cout << "Config file: " << (config_file_path_.empty() ? "default" : config_file_path_) << std::endl;
    std::cout << "=================================" << std::endl;
}

json AgentConfigManager::to_json() const {
    json config_json;
    
    // System info
    config_json["system"]["name"] = config_.system.name;
    config_json["system"]["version"] = config_.system.version;
    config_json["system"]["host"] = config_.system.host;
    config_json["system"]["port"] = config_.system.port;
    
    // System instruction
    config_json["system_instruction"] = config_.system_instruction;
    
    // Agents
    config_json["agents"] = json::array();
    for (const auto& agent : config_.agents) {
        json agent_json;
        agent_json["name"] = agent.name;
        agent_json["capabilities"] = agent.capabilities;
        agent_json["auto_start"] = agent.auto_start;
        agent_json["system_prompt"] = agent.system_prompt;
        config_json["agents"].push_back(agent_json);
    }
    
    // Functions
    config_json["functions"] = json::object();
    for (const auto& [name, func] : config_.functions) {
        json func_json;
        func_json["description"] = func.description;
        func_json["timeout"] = func.timeout;
        func_json["parameters"] = func.parameters;
        config_json["functions"][name] = func_json;
    }
    
    // Performance
    config_json["performance"]["max_memory_usage"] = config_.performance.max_memory_usage;
    config_json["performance"]["cache_size"] = config_.performance.cache_size;
    config_json["performance"]["worker_threads"] = config_.performance.worker_threads;
    config_json["performance"]["request_timeout"] = config_.performance.request_timeout;
    config_json["performance"]["max_request_size"] = config_.performance.max_request_size;
    
    // Logging
    config_json["logging"]["level"] = config_.logging.level;
    config_json["logging"]["file"] = config_.logging.file;
    config_json["logging"]["max_file_size"] = config_.logging.max_file_size;
    config_json["logging"]["max_files"] = config_.logging.max_files;
    config_json["logging"]["console_output"] = config_.logging.console_output;
    
    // Security
    config_json["security"]["enable_cors"] = config_.security.enable_cors;
    config_json["security"]["allowed_origins"] = config_.security.allowed_origins;
    config_json["security"]["max_request_rate"] = config_.security.max_request_rate;
    config_json["security"]["enable_auth"] = config_.security.enable_auth;
    config_json["security"]["api_key"] = config_.security.api_key;
    
    return config_json;
}
