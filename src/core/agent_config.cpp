#include "../include/agent_config.hpp"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <algorithm>
#include <thread>
#include <iomanip>
#include <set>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fstream>
#endif

// Utility functions for parsing memory strings
size_t ConfigValidator::parse_memory_string(const std::string& memory_str) {
    std::regex memory_regex(R"((\d+(?:\.\d+)?)\s*(B|KB|MB|GB|TB))", std::regex_constants::icase);
    std::smatch matches;
    
    if (std::regex_match(memory_str, matches, memory_regex)) {
        double value = std::stod(matches[1].str());
        std::string unit = matches[2].str();
        std::transform(unit.begin(), unit.end(), unit.begin(), ::toupper);
        
        if (unit == "B") return static_cast<size_t>(value);
        else if (unit == "KB") return static_cast<size_t>(value * 1024);
        else if (unit == "MB") return static_cast<size_t>(value * 1024 * 1024);
        else if (unit == "GB") return static_cast<size_t>(value * 1024 * 1024 * 1024);
        else if (unit == "TB") return static_cast<size_t>(value * 1024ULL * 1024 * 1024 * 1024);
    }
    
    return 0; // Invalid format
}

bool ConfigValidator::validate_timeout_range(int timeout, int min_timeout, int max_timeout) {
    return timeout >= min_timeout && timeout <= max_timeout;
}

bool ConfigValidator::file_exists(const std::string& path) {
    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

// ConfigValidator implementation
ValidationResult ConfigValidator::validate_config(const AgentSystemConfig& config) {
    ValidationResult result;
    result.is_valid = true;
    
    if (!config.validation.enabled) {
        result.add_warning("Configuration validation is disabled");
        return result;
    }
    
    // Validate ports
    auto port_result = validate_ports(config);
    result.errors.insert(result.errors.end(), port_result.errors.begin(), port_result.errors.end());
    result.warnings.insert(result.warnings.end(), port_result.warnings.begin(), port_result.warnings.end());
    
    // Validate models
    auto models_result = validate_models(config.models, config.kolosal_server.models_directory);
    result.errors.insert(result.errors.end(), models_result.errors.begin(), models_result.errors.end());
    result.warnings.insert(result.warnings.end(), models_result.warnings.begin(), models_result.warnings.end());
    
    // Validate resource settings
    auto resource_result = validate_resource_settings(config.performance);
    result.errors.insert(result.errors.end(), resource_result.errors.begin(), resource_result.errors.end());
    result.warnings.insert(result.warnings.end(), resource_result.warnings.begin(), resource_result.warnings.end());
    
    // Validate agents
    auto agents_result = validate_agents(config.agents);
    result.errors.insert(result.errors.end(), agents_result.errors.begin(), agents_result.errors.end());
    result.warnings.insert(result.warnings.end(), agents_result.warnings.begin(), agents_result.warnings.end());
    
    // Validate functions
    auto functions_result = validate_functions(config.functions);
    result.errors.insert(result.errors.end(), functions_result.errors.begin(), functions_result.errors.end());
    result.warnings.insert(result.warnings.end(), functions_result.warnings.begin(), functions_result.warnings.end());
    
    result.is_valid = result.errors.empty();
    
    if (config.validation.strict_mode && result.has_warnings()) {
        result.is_valid = false;
        result.add_error("Strict mode enabled: warnings treated as errors");
    }
    
    return result;
}

ValidationResult ConfigValidator::validate_models(const std::map<std::string, AgentSystemConfig::ModelConfig>& models, 
                                                 const std::string& models_directory) {
    ValidationResult result;
    result.is_valid = true;
    
    bool has_default = false;
    bool has_embedding = false;
    
    for (const auto& [name, model] : models) {
        if (name == "default") has_default = true;
        if (model.type == "embedding") has_embedding = true;
        
        // Validate model file exists
        if (!model.model_file.empty()) {
            std::string model_path = models_directory + "/" + model.model_file;
            if (!file_exists(model_path)) {
                result.add_error("Model file not found: " + model_path);
            }
        }
        
        // Validate model configuration
        if (model.context_size <= 0) {
            result.add_error("Invalid context size for model " + name + ": " + std::to_string(model.context_size));
        }
        
        if (model.type == "llm" && (model.temperature < 0.0 || model.temperature > 2.0)) {
            result.add_warning("Temperature for model " + name + " is outside typical range (0.0-2.0): " + std::to_string(model.temperature));
        }
        
        if (model.type == "embedding" && model.embedding_size <= 0) {
            result.add_error("Invalid embedding size for model " + name + ": " + std::to_string(model.embedding_size));
        }
    }
    
    if (!has_default) {
        result.add_error("No default model configured");
    }
    
    if (!has_embedding) {
        result.add_warning("No embedding model configured - retrieval features may not work");
    }
    
    result.is_valid = result.errors.empty();
    return result;
}

ValidationResult ConfigValidator::validate_ports(const AgentSystemConfig& config) {
    ValidationResult result;
    result.is_valid = true;
    
    int system_port = config.system.port;
    
    if (system_port < config.validation.port_ranges.min_port || 
        system_port > config.validation.port_ranges.max_port) {
        result.add_error("System port " + std::to_string(system_port) + 
                        " is outside valid range (" + 
                        std::to_string(config.validation.port_ranges.min_port) + "-" +
                        std::to_string(config.validation.port_ranges.max_port) + ")");
    }
    
    // Check if port is likely to be in use (common ports)
    std::vector<int> common_ports = {80, 443, 8080, 3000, 5000, 8000};
    if (std::find(common_ports.begin(), common_ports.end(), system_port) != common_ports.end()) {
        result.add_warning("Port " + std::to_string(system_port) + " is a commonly used port and may be occupied");
    }
    
    result.is_valid = result.errors.empty();
    return result;
}

ValidationResult ConfigValidator::validate_resource_settings(const AgentSystemConfig::PerformanceConfig& performance) {
    ValidationResult result;
    result.is_valid = true;
    
    // Validate memory settings
    if (performance.max_memory_usage != "auto") {
        size_t max_memory = parse_memory_string(performance.max_memory_usage);
        size_t min_memory = parse_memory_string(performance.min_memory_required);
        
        if (max_memory == 0) {
            result.add_error("Invalid max_memory_usage format: " + performance.max_memory_usage);
        }
        
        if (min_memory == 0) {
            result.add_error("Invalid min_memory_required format: " + performance.min_memory_required);
        }
        
        if (max_memory > 0 && min_memory > 0 && max_memory < min_memory) {
            result.add_error("max_memory_usage cannot be less than min_memory_required");
        }
    }
    
    // Validate worker threads
    if (performance.worker_threads != "auto") {
        try {
            int threads = std::stoi(performance.worker_threads);
            if (threads < performance.min_worker_threads || threads > performance.max_worker_threads) {
                result.add_error("Worker threads " + std::to_string(threads) + 
                                " is outside valid range (" + 
                                std::to_string(performance.min_worker_threads) + "-" +
                                std::to_string(performance.max_worker_threads) + ")");
            }
        } catch (const std::exception& /* e */) {
            result.add_error("Invalid worker_threads value: " + performance.worker_threads);
        }
    }
    
    // Validate thresholds
    if (performance.resource_limits.cpu_usage_threshold < 10 || performance.resource_limits.cpu_usage_threshold > 100) {
        result.add_error("CPU usage threshold must be between 10-100%");
    }
    
    if (performance.resource_limits.memory_usage_threshold < 10 || performance.resource_limits.memory_usage_threshold > 100) {
        result.add_error("Memory usage threshold must be between 10-100%");
    }
    
    if (performance.resource_limits.disk_usage_threshold < 10 || performance.resource_limits.disk_usage_threshold > 100) {
        result.add_error("Disk usage threshold must be between 10-100%");
    }
    
    result.is_valid = result.errors.empty();
    return result;
}

ValidationResult ConfigValidator::validate_agents(const std::vector<AgentSystemConfig::AgentConfig>& agents) {
    ValidationResult result;
    result.is_valid = true;
    
    if (agents.empty()) {
        result.add_error("No agents configured");
        return result;
    }
    
    std::set<std::string> agent_names;
    bool has_chat_capability = false;
    
    for (const auto& agent : agents) {
        if (agent.name.empty()) {
            result.add_error("Agent name cannot be empty");
            continue;
        }
        
        if (agent_names.count(agent.name)) {
            result.add_error("Duplicate agent name: " + agent.name);
        }
        agent_names.insert(agent.name);
        
        if (agent.capabilities.empty()) {
            result.add_warning("Agent " + agent.name + " has no capabilities defined");
        }
        
        for (const auto& capability : agent.capabilities) {
            if (capability == "chat") {
                has_chat_capability = true;
            }
        }
        
        if (agent.model.empty()) {
            result.add_warning("Agent " + agent.name + " has no model specified, will use default");
        }
    }
    
    if (!has_chat_capability) {
        result.add_warning("No agent has 'chat' capability - basic chat functionality may not work");
    }
    
    result.is_valid = result.errors.empty();
    return result;
}

ValidationResult ConfigValidator::validate_functions(const std::map<std::string, AgentSystemConfig::FunctionConfig>& functions) {
    ValidationResult result;
    result.is_valid = true;
    
    if (functions.empty()) {
        result.add_error("No functions configured");
        return result;
    }
    
    std::vector<std::string> required_functions = {"chat", "status"};
    for (const auto& required_func : required_functions) {
        if (functions.find(required_func) == functions.end()) {
            result.add_error("Required function not found: " + required_func);
        }
    }
    
    for (const auto& [name, func] : functions) {
        if (func.timeout <= 0) {
            result.add_error("Invalid timeout for function " + name + ": " + std::to_string(func.timeout));
        }
        
        if (func.timeout > 300000) { // 5 minutes
            result.add_warning("Very long timeout for function " + name + ": " + std::to_string(func.timeout) + "ms");
        }
        
        if (func.description.empty()) {
            result.add_warning("Function " + name + " has no description");
        }
    }
    
    result.is_valid = result.errors.empty();
    return result;
}

// DefaultResourceMonitor implementation
DefaultResourceMonitor::DefaultResourceMonitor() : monitoring_active_(false) {}

DefaultResourceMonitor::~DefaultResourceMonitor() {
    stop_monitoring();
}

SystemResources DefaultResourceMonitor::get_system_resources() {
#ifdef _WIN32
    return get_windows_resources();
#else
    return get_linux_resources();
#endif
}

SystemResources DefaultResourceMonitor::get_windows_resources() {
    SystemResources resources = {};
    
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        resources.total_memory_mb = static_cast<size_t>(memInfo.ullTotalPhys / (1024 * 1024));
        resources.available_memory_mb = static_cast<size_t>(memInfo.ullAvailPhys / (1024 * 1024));
        resources.memory_usage_percent = 100.0 - (static_cast<double>(memInfo.ullAvailPhys) / memInfo.ullTotalPhys * 100.0);
    }
    
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    resources.cpu_cores = sysInfo.dwNumberOfProcessors;
    
    // Get disk space for current directory
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes;
    if (GetDiskFreeSpaceEx(NULL, &freeBytesAvailable, &totalNumberOfBytes, NULL)) {
        resources.free_disk_space_mb = static_cast<size_t>(freeBytesAvailable.QuadPart / (1024 * 1024));
        resources.disk_usage_percent = 100.0 - (static_cast<double>(freeBytesAvailable.QuadPart) / totalNumberOfBytes.QuadPart * 100.0);
    }
#endif
    
    return resources;
}

SystemResources DefaultResourceMonitor::get_linux_resources() {
    SystemResources resources = {};
    
#ifndef _WIN32
    // Get memory information
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        resources.total_memory_mb = static_cast<size_t>(si.totalram * si.mem_unit / (1024 * 1024));
        resources.available_memory_mb = static_cast<size_t>(si.freeram * si.mem_unit / (1024 * 1024));
        resources.memory_usage_percent = 100.0 - (static_cast<double>(si.freeram) / si.totalram * 100.0);
    }
    
    // Get CPU count
    resources.cpu_cores = std::thread::hardware_concurrency();
    
    // Get disk space
    struct statvfs stat;
    if (statvfs(".", &stat) == 0) {
        resources.free_disk_space_mb = static_cast<size_t>((stat.f_bavail * stat.f_frsize) / (1024 * 1024));
        size_t total_space_mb = static_cast<size_t>((stat.f_blocks * stat.f_frsize) / (1024 * 1024));
        resources.disk_usage_percent = 100.0 - (static_cast<double>(resources.free_disk_space_mb) / total_space_mb * 100.0);
    }
    
    // Try to get CPU usage from /proc/loadavg
    std::ifstream loadavg("/proc/loadavg");
    if (loadavg.is_open()) {
        double load1;
        loadavg >> load1;
        resources.cpu_usage_percent = (load1 / resources.cpu_cores) * 100.0;
        loadavg.close();
    }
#endif
    
    return resources;
}

bool DefaultResourceMonitor::check_resource_thresholds(const AgentSystemConfig::PerformanceConfig& config) {
    SystemResources resources = get_system_resources();
    
    return resources.cpu_usage_percent < config.resource_limits.cpu_usage_threshold &&
           resources.memory_usage_percent < config.resource_limits.memory_usage_threshold &&
           resources.disk_usage_percent < config.resource_limits.disk_usage_threshold;
}

void DefaultResourceMonitor::start_monitoring(std::function<void(const SystemResources&)> callback) {
    callback_ = callback;
    monitoring_active_ = true;
    
    // In a real implementation, this would run in a separate thread
    // For now, we'll just call it once
    if (callback_) {
        callback_(get_system_resources());
    }
}

void DefaultResourceMonitor::stop_monitoring() {
    monitoring_active_ = false;
}

// AgentConfigManager implementation
AgentConfigManager::AgentConfigManager() : resource_monitoring_active_(false) {
    set_default_config();
    resource_monitor_ = std::make_unique<DefaultResourceMonitor>();
}

void AgentConfigManager::set_resource_monitor(std::unique_ptr<ResourceMonitor> monitor) {
    resource_monitor_ = std::move(monitor);
}

void AgentConfigManager::start_resource_monitoring() {
    if (resource_monitor_ && !resource_monitoring_active_) {
        resource_monitoring_active_ = true;
        resource_monitor_->start_monitoring([this](const SystemResources& resources) {
            current_resources_ = resources;
            
            // Check if we need to adjust performance settings
            if (!resource_monitor_->check_resource_thresholds(config_.performance)) {
                adjust_for_resource_pressure();
            }
        });
    }
}

void AgentConfigManager::stop_resource_monitoring() {
    if (resource_monitor_ && resource_monitoring_active_) {
        resource_monitor_->stop_monitoring();
        resource_monitoring_active_ = false;
    }
}

ValidationResult AgentConfigManager::load_config(const std::string& file_path) {
    ValidationResult result;
    result.is_valid = true;
    
    // If a specific file path is provided, only try that file
    if (!file_path.empty() && file_path != "agent.yaml") {
        if (std::filesystem::exists(file_path)) {
            if (load_from_file(file_path)) {
                config_file_path_ = std::filesystem::absolute(file_path).string();
                std::cout << "Loaded agent configuration from: " << config_file_path_ << std::endl;
                result = validate_and_adjust_config();
                return result;
            }
        }
        result.add_error("Could not load configuration from: " + file_path);
        return result;
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
                result = validate_and_adjust_config();
                return result;
            }
        }
    }
    
    std::cout << "Could not find agent.yaml, using default configuration" << std::endl;
    apply_resource_based_defaults();
    result = validate_and_adjust_config();
    return result;
}

ValidationResult AgentConfigManager::reload_config() {
    if (config_file_path_.empty()) {
        return load_config();
    }
    return load_config(config_file_path_);
}

ValidationResult AgentConfigManager::validate_and_adjust_config() {
    ValidationResult result = ConfigValidator::validate_config(config_);
    
    if (result.is_valid) {
        apply_resource_based_defaults();
        adjust_performance_settings();
    }
    
    return result;
}

void AgentConfigManager::apply_resource_based_defaults() {
    if (!resource_monitor_) return;
    
    SystemResources resources = resource_monitor_->get_system_resources();
    current_resources_ = resources;
    
    // Auto-configure memory settings
    if (config_.performance.max_memory_usage == "auto") {
        size_t max_memory_mb = static_cast<size_t>(resources.total_memory_mb * config_.performance.max_memory_percent / 100.0);
        config_.performance.max_memory_usage = std::to_string(max_memory_mb) + "MB";
    }
    
    // Auto-configure cache size
    if (config_.performance.cache_size == "auto") {
        size_t total_memory_mb = resources.total_memory_mb;
        size_t cache_size_mb = std::max(static_cast<size_t>(128), std::min(static_cast<size_t>(1024), total_memory_mb / 8));
        config_.performance.cache_size = std::to_string(cache_size_mb) + "MB";
    }
    
    // Auto-configure worker threads
    if (config_.performance.worker_threads == "auto") {
        int optimal_threads = std::max(config_.performance.min_worker_threads, 
                                     std::min(config_.performance.max_worker_threads, 
                                            static_cast<int>(resources.cpu_cores)));
        config_.performance.worker_threads = std::to_string(optimal_threads);
    }
}

void AgentConfigManager::adjust_performance_settings() {
    // This method can be called to dynamically adjust settings based on current conditions
    // Implementation depends on specific performance monitoring needs
}

size_t AgentConfigManager::get_optimal_memory_usage() const {
    return ConfigValidator::parse_memory_string(config_.performance.max_memory_usage);
}

int AgentConfigManager::get_optimal_worker_threads() const {
    if (config_.performance.worker_threads == "auto") {
        return std::max(config_.performance.min_worker_threads, 
                       std::min(config_.performance.max_worker_threads, 
                              static_cast<int>(current_resources_.cpu_cores)));
    }
    
    try {
        return std::stoi(config_.performance.worker_threads);
    } catch (const std::exception& /* e */) {
        return config_.performance.min_worker_threads;
    }
}

size_t AgentConfigManager::get_optimal_cache_size() const {
    return ConfigValidator::parse_memory_string(config_.performance.cache_size);
}

bool AgentConfigManager::should_reduce_resource_usage() const {
    return current_resources_.memory_usage_percent > config_.performance.resource_limits.memory_usage_threshold ||
           current_resources_.cpu_usage_percent > config_.performance.resource_limits.cpu_usage_threshold ||
           current_resources_.disk_usage_percent > config_.performance.resource_limits.disk_usage_threshold;
}

void AgentConfigManager::adjust_for_resource_pressure() {
    if (!config_.performance.graceful_degradation.enabled) {
        return;
    }
    
    // Reduce cache size if under memory pressure
    if (config_.performance.graceful_degradation.reduce_cache_on_memory_pressure &&
        current_resources_.memory_usage_percent > config_.performance.resource_limits.memory_usage_threshold) {
        
        size_t current_cache = ConfigValidator::parse_memory_string(config_.performance.cache_size);
        size_t min_cache = ConfigValidator::parse_memory_string(config_.performance.min_cache_size);
        size_t new_cache = std::max(min_cache, current_cache / 2);
        
        config_.performance.cache_size = std::to_string(new_cache / (1024 * 1024)) + "MB";
        std::cout << "Reduced cache size due to memory pressure: " << config_.performance.cache_size << std::endl;
    }
    
    // Reduce worker threads if under CPU pressure
    if (config_.performance.graceful_degradation.reduce_workers_on_cpu_pressure &&
        current_resources_.cpu_usage_percent > config_.performance.resource_limits.cpu_usage_threshold) {
        
        int current_threads = get_optimal_worker_threads();
        int new_threads = std::max(config_.performance.min_worker_threads, current_threads - 1);
        
        config_.performance.worker_threads = std::to_string(new_threads);
        std::cout << "Reduced worker threads due to CPU pressure: " << new_threads << std::endl;
    }
}

void AgentConfigManager::restore_optimal_settings() {
    // Restore optimal settings when resources are available
    apply_resource_based_defaults();
    std::cout << "Restored optimal performance settings" << std::endl;
}

ValidationResult AgentConfigManager::validate_config() const {
    return ConfigValidator::validate_config(config_);
}

void AgentConfigManager::print_validation_results(const ValidationResult& result) const {
    if (result.has_errors()) {
        std::cout << "\n=== Configuration Errors ===" << std::endl;
        for (const auto& error : result.errors) {
            std::cout << "ERROR: " << error << std::endl;
        }
    }
    
    if (result.has_warnings()) {
        std::cout << "\n=== Configuration Warnings ===" << std::endl;
        for (const auto& warning : result.warnings) {
            std::cout << "WARNING: " << warning << std::endl;
        }
    }
    
    if (result.is_valid && !result.has_warnings()) {
        std::cout << "\nConfiguration validation passed successfully." << std::endl;
    }
}

void AgentConfigManager::set_default_config() {
    // Set validation defaults
    config_.validation.enabled = true;
    config_.validation.strict_mode = false;
    config_.validation.schema_version = "1.0.0";
    config_.validation.port_ranges.min_port = 1024;
    config_.validation.port_ranges.max_port = 65535;
    
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
    
    // Set default models
    config_.models.clear();
    AgentSystemConfig::ModelConfig default_model;
    default_model.id = "default";
    default_model.actual_name = "qwen2.5-0.5b-instruct-q4_k_m";
    default_model.model_file = "qwen2.5-0.5b-instruct-q4_k_m.gguf";
    default_model.type = "llm";
    default_model.server_url = "http://127.0.0.1:8081";
    default_model.description = "Default LLM model (Qwen2.5-0.5B Instruct)";
    default_model.preload = true;
    default_model.context_size = 2048;
    default_model.max_tokens = 1024;
    default_model.temperature = 0.7;
    default_model.top_p = 0.9;
    config_.models["default"] = default_model;
    
    AgentSystemConfig::ModelConfig embedding_model;
    embedding_model.id = "embedding";
    embedding_model.actual_name = "all-MiniLM-L6-v2-bf16-q4_k";
    embedding_model.model_file = "all-MiniLM-L6-v2-bf16-q4_k.gguf";
    embedding_model.type = "embedding";
    embedding_model.server_url = "http://127.0.0.1:8081";
    embedding_model.description = "Embedding model for document vectorization and semantic search";
    embedding_model.preload = true;
    embedding_model.embedding_size = 384;
    config_.models["embedding"] = embedding_model;
    
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
    json model_param;
    model_param["name"] = "model";
    model_param["type"] = "string";
    model_param["required"] = true;
    model_param["description"] = "Name of the AI model to use for chat";
    chat_func.parameters.push_back(model_param);
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
    config_.performance.max_memory_usage = "auto";
    config_.performance.min_memory_required = "512MB";
    config_.performance.max_memory_percent = 75;
    config_.performance.cache_size = "auto";
    config_.performance.min_cache_size = "128MB";
    config_.performance.max_cache_size = "1GB";
    config_.performance.worker_threads = "auto";
    config_.performance.min_worker_threads = 2;
    config_.performance.max_worker_threads = 16;
    config_.performance.request_timeout = 30000;
    config_.performance.max_request_size = "10MB";
    
    // Set disk monitoring defaults
    config_.performance.disk_space_monitoring.enabled = true;
    config_.performance.disk_space_monitoring.min_free_space = "1GB";
    config_.performance.disk_space_monitoring.warning_threshold = "2GB";
    config_.performance.disk_space_monitoring.check_interval = 300;
    
    // Set resource limits defaults
    config_.performance.resource_limits.cpu_usage_threshold = 80;
    config_.performance.resource_limits.memory_usage_threshold = 85;
    config_.performance.resource_limits.disk_usage_threshold = 90;
    
    // Set graceful degradation defaults
    config_.performance.graceful_degradation.enabled = true;
    config_.performance.graceful_degradation.reduce_cache_on_memory_pressure = true;
    config_.performance.graceful_degradation.reduce_workers_on_cpu_pressure = true;
    config_.performance.graceful_degradation.queue_limit_on_resource_pressure = 50;
    
    // Set Kolosal Server defaults
    config_.kolosal_server.auto_start = true;
    config_.kolosal_server.startup_timeout = 60;
    config_.kolosal_server.health_check_interval = 10;
    config_.kolosal_server.max_retries = 3;
    config_.kolosal_server.retry_delay = 2000;
    config_.kolosal_server.resource_limits.max_memory = "1.5GB";
    config_.kolosal_server.resource_limits.max_cpu_percent = 80;
    config_.kolosal_server.models_directory = "./models";
    config_.kolosal_server.model_preload_timeout = 120;
    config_.kolosal_server.graceful_shutdown_timeout = 30;
    
    // Set required models defaults
    config_.kolosal_server.required_models.clear();
    AgentSystemConfig::KolosalServerConfig::RequiredModel req_llm;
    req_llm.name = "qwen2.5-0.5b-instruct-q4_k_m";
    req_llm.file = "qwen2.5-0.5b-instruct-q4_k_m.gguf";
    req_llm.type = "llm";
    req_llm.required = true;
    config_.kolosal_server.required_models.push_back(req_llm);
    
    AgentSystemConfig::KolosalServerConfig::RequiredModel req_embed;
    req_embed.name = "all-MiniLM-L6-v2-bf16-q4_k";
    req_embed.file = "all-MiniLM-L6-v2-bf16-q4_k.gguf";
    req_embed.type = "embedding";
    req_embed.required = true;
    config_.kolosal_server.required_models.push_back(req_embed);
    
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
    
    // Set default error handling
    config_.error_handling.enable_fallbacks = true;
    config_.error_handling.fallback_responses = true;
    config_.error_handling.max_retry_attempts = 3;
    config_.error_handling.retry_backoff_multiplier = 2.0;
    config_.error_handling.timeout_escalation = true;
    config_.error_handling.graceful_degradation = true;
    config_.error_handling.offline_mode.enable = true;
    config_.error_handling.offline_mode.cache_responses = true;
    config_.error_handling.offline_mode.max_cache_size = "100MB";
    
    // Set default circuit breaker
    config_.circuit_breaker.failure_threshold = 5;
    config_.circuit_breaker.recovery_timeout = 30;
    config_.circuit_breaker.half_open_max_calls = 3;
    config_.circuit_breaker.metrics_window = 60;
}

bool AgentConfigManager::load_from_file(const std::string& file_path) {
    try {
        YAML::Node config = YAML::LoadFile(file_path);
        
        // Check if config is empty or null
        if (!config || config.IsNull() || config.size() == 0) {
            std::cout << "Configuration file is empty or invalid: " << file_path << std::endl;
            return false;
        }
        
        // Load validation configuration
        if (config["validation"]) {
            auto validation_node = config["validation"];
            config_.validation.enabled = validation_node["enabled"].as<bool>(true);
            config_.validation.strict_mode = validation_node["strict_mode"].as<bool>(false);
            config_.validation.schema_version = validation_node["schema_version"].as<std::string>("1.0.0");
            
            if (validation_node["port_ranges"]) {
                auto port_ranges = validation_node["port_ranges"];
                if (port_ranges["system_port"]) {
                    auto system_port_range = port_ranges["system_port"];
                    config_.validation.port_ranges.min_port = system_port_range[0].as<int>(1024);
                    config_.validation.port_ranges.max_port = system_port_range[1].as<int>(65535);
                }
            }
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
                
                // Load retrieval configuration
                if (agent_node["retrieval"]) {
                    auto retrieval_node = agent_node["retrieval"];
                    agent_config.retrieval.server_url = retrieval_node["server_url"].as<std::string>("");
                    agent_config.retrieval.timeout_seconds = retrieval_node["timeout_seconds"].as<int>(30);
                    agent_config.retrieval.max_retries = retrieval_node["max_retries"].as<int>(3);
                    agent_config.retrieval.search_enabled = retrieval_node["search_enabled"].as<bool>(false);
                    agent_config.retrieval.max_results = retrieval_node["max_results"].as<int>(10);
                }
                
                config_.agents.push_back(agent_config);
            }
        }
        
        // Load model configurations with enhanced structure
        config_.models.clear();
        if (config["models"]) {
            for (const auto& model_node : config["models"]) {
                AgentSystemConfig::ModelConfig model_config;
                
                model_config.id = model_node["name"].as<std::string>();
                model_config.actual_name = model_node["actual_name"].as<std::string>(model_config.id);
                model_config.model_file = model_node["model_file"].as<std::string>("");
                model_config.type = model_node["type"].as<std::string>("llm");
                model_config.server_url = model_node["server_url"].as<std::string>("http://127.0.0.1:8081");
                model_config.description = model_node["description"].as<std::string>("");
                model_config.preload = model_node["preload"].as<bool>(true);
                model_config.context_size = model_node["context_size"].as<int>(2048);
                model_config.max_tokens = model_node["max_tokens"].as<int>(1024);
                model_config.temperature = model_node["temperature"].as<double>(0.7);
                model_config.top_p = model_node["top_p"].as<double>(0.9);
                model_config.embedding_size = model_node["embedding_size"].as<int>(384);
                
                config_.models[model_config.id] = model_config;
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
        
        // Load enhanced performance configuration
        if (config["performance"]) {
            auto perf_node = config["performance"];
            config_.performance.max_memory_usage = perf_node["max_memory_usage"].as<std::string>("auto");
            config_.performance.min_memory_required = perf_node["min_memory_required"].as<std::string>("512MB");
            config_.performance.max_memory_percent = perf_node["max_memory_percent"].as<int>(75);
            config_.performance.cache_size = perf_node["cache_size"].as<std::string>("auto");
            config_.performance.min_cache_size = perf_node["min_cache_size"].as<std::string>("128MB");
            config_.performance.max_cache_size = perf_node["max_cache_size"].as<std::string>("1GB");
            config_.performance.worker_threads = perf_node["worker_threads"].as<std::string>("auto");
            config_.performance.min_worker_threads = perf_node["min_worker_threads"].as<int>(2);
            config_.performance.max_worker_threads = perf_node["max_worker_threads"].as<int>(16);
            config_.performance.request_timeout = perf_node["request_timeout"].as<int>(30000);
            config_.performance.max_request_size = perf_node["max_request_size"].as<std::string>("10MB");
            
            // Load disk space monitoring
            if (perf_node["disk_space_monitoring"]) {
                auto disk_node = perf_node["disk_space_monitoring"];
                config_.performance.disk_space_monitoring.enabled = disk_node["enabled"].as<bool>(true);
                config_.performance.disk_space_monitoring.min_free_space = disk_node["min_free_space"].as<std::string>("1GB");
                config_.performance.disk_space_monitoring.warning_threshold = disk_node["warning_threshold"].as<std::string>("2GB");
                config_.performance.disk_space_monitoring.check_interval = disk_node["check_interval"].as<int>(300);
            }
            
            // Load resource limits
            if (perf_node["resource_limits"]) {
                auto limits_node = perf_node["resource_limits"];
                config_.performance.resource_limits.cpu_usage_threshold = limits_node["cpu_usage_threshold"].as<int>(80);
                config_.performance.resource_limits.memory_usage_threshold = limits_node["memory_usage_threshold"].as<int>(85);
                config_.performance.resource_limits.disk_usage_threshold = limits_node["disk_usage_threshold"].as<int>(90);
            }
            
            // Load graceful degradation
            if (perf_node["graceful_degradation"]) {
                auto degradation_node = perf_node["graceful_degradation"];
                config_.performance.graceful_degradation.enabled = degradation_node["enabled"].as<bool>(true);
                config_.performance.graceful_degradation.reduce_cache_on_memory_pressure = degradation_node["reduce_cache_on_memory_pressure"].as<bool>(true);
                config_.performance.graceful_degradation.reduce_workers_on_cpu_pressure = degradation_node["reduce_workers_on_cpu_pressure"].as<bool>(true);
                config_.performance.graceful_degradation.queue_limit_on_resource_pressure = degradation_node["queue_limit_on_resource_pressure"].as<int>(50);
            }
        }
        
        // Load Kolosal Server configuration
        if (config["kolosal_server"]) {
            auto server_node = config["kolosal_server"];
            config_.kolosal_server.auto_start = server_node["auto_start"].as<bool>(true);
            config_.kolosal_server.startup_timeout = server_node["startup_timeout"].as<int>(60);
            config_.kolosal_server.health_check_interval = server_node["health_check_interval"].as<int>(10);
            config_.kolosal_server.max_retries = server_node["max_retries"].as<int>(3);
            config_.kolosal_server.retry_delay = server_node["retry_delay"].as<int>(2000);
            
            if (server_node["resource_limits"]) {
                auto limits_node = server_node["resource_limits"];
                config_.kolosal_server.resource_limits.max_memory = limits_node["max_memory"].as<std::string>("1.5GB");
                config_.kolosal_server.resource_limits.max_cpu_percent = limits_node["max_cpu_percent"].as<int>(80);
            }
            
            config_.kolosal_server.models_directory = server_node["models_directory"].as<std::string>("./models");
            
            // Load required models with enhanced structure
            if (server_node["required_models"]) {
                config_.kolosal_server.required_models.clear();
                for (const auto& model_node : server_node["required_models"]) {
                    AgentSystemConfig::KolosalServerConfig::RequiredModel model;
                    model.name = model_node["name"].as<std::string>();
                    model.file = model_node["file"].as<std::string>();
                    model.type = model_node["type"].as<std::string>("llm");
                    model.required = model_node["required"].as<bool>(true);
                    config_.kolosal_server.required_models.push_back(model);
                }
            }
            
            config_.kolosal_server.model_preload_timeout = server_node["model_preload_timeout"].as<int>(120);
            config_.kolosal_server.graceful_shutdown_timeout = server_node["graceful_shutdown_timeout"].as<int>(30);
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
        
        // Load error handling configuration
        if (config["error_handling"]) {
            auto error_node = config["error_handling"];
            config_.error_handling.enable_fallbacks = error_node["enable_fallbacks"].as<bool>(true);
            config_.error_handling.fallback_responses = error_node["fallback_responses"].as<bool>(true);
            config_.error_handling.max_retry_attempts = error_node["max_retry_attempts"].as<int>(3);
            config_.error_handling.retry_backoff_multiplier = error_node["retry_backoff_multiplier"].as<double>(2.0);
            config_.error_handling.timeout_escalation = error_node["timeout_escalation"].as<bool>(true);
            config_.error_handling.graceful_degradation = error_node["graceful_degradation"].as<bool>(true);
            
            if (error_node["offline_mode"]) {
                auto offline_node = error_node["offline_mode"];
                config_.error_handling.offline_mode.enable = offline_node["enable"].as<bool>(true);
                config_.error_handling.offline_mode.cache_responses = offline_node["cache_responses"].as<bool>(true);
                config_.error_handling.offline_mode.max_cache_size = offline_node["max_cache_size"].as<std::string>("100MB");
            }
        }
        
        // Load circuit breaker configuration
        if (config["circuit_breaker"]) {
            auto cb_node = config["circuit_breaker"];
            config_.circuit_breaker.failure_threshold = cb_node["failure_threshold"].as<int>(5);
            config_.circuit_breaker.recovery_timeout = cb_node["recovery_timeout"].as<int>(30);
            config_.circuit_breaker.half_open_max_calls = cb_node["half_open_max_calls"].as<int>(3);
            config_.circuit_breaker.metrics_window = cb_node["metrics_window"].as<int>(60);
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

void AgentConfigManager::print_config_summary() const {
    std::cout << "\n=== Agent System Configuration ===" << std::endl;
    std::cout << "System: " << config_.system.name << " v" << config_.system.version << std::endl;
    std::cout << "Server: " << config_.system.host << ":" << config_.system.port << std::endl;
    std::cout << "Agents: " << config_.agents.size() << " configured" << std::endl;
    std::cout << "Models: " << config_.models.size() << " available" << std::endl;
    std::cout << "Functions: " << config_.functions.size() << " available" << std::endl;
    std::cout << "Config file: " << (config_file_path_.empty() ? "default" : config_file_path_) << std::endl;
    std::cout << "Validation: " << (config_.validation.enabled ? "enabled" : "disabled") << std::endl;
    std::cout << "Resource monitoring: " << (resource_monitoring_active_ ? "active" : "inactive") << std::endl;
    
    // Resource information
    if (resource_monitoring_active_ && resource_monitor_) {
        std::cout << "\n--- Resource Status ---" << std::endl;
        std::cout << "Memory: " << current_resources_.available_memory_mb << "MB free / " 
                  << current_resources_.total_memory_mb << "MB total ("
                  << std::fixed << std::setprecision(1) << current_resources_.memory_usage_percent << "% used)" << std::endl;
        std::cout << "CPU: " << std::fixed << std::setprecision(1) << current_resources_.cpu_usage_percent 
                  << "% usage (" << current_resources_.cpu_cores << " cores)" << std::endl;
        std::cout << "Disk: " << current_resources_.free_disk_space_mb << "MB free ("
                  << std::fixed << std::setprecision(1) << current_resources_.disk_usage_percent << "% used)" << std::endl;
    }
    
    // Performance settings
    std::cout << "\n--- Performance Settings ---" << std::endl;
    std::cout << "Memory limit: " << config_.performance.max_memory_usage << std::endl;
    std::cout << "Cache size: " << config_.performance.cache_size << std::endl;
    std::cout << "Worker threads: " << config_.performance.worker_threads << std::endl;
    
    std::cout << "=================================" << std::endl;
}

json AgentConfigManager::to_json() const {
    json config_json;
    
    // Validation info
    config_json["validation"]["enabled"] = config_.validation.enabled;
    config_json["validation"]["strict_mode"] = config_.validation.strict_mode;
    config_json["validation"]["schema_version"] = config_.validation.schema_version;
    
    // System info
    config_json["system"]["name"] = config_.system.name;
    config_json["system"]["version"] = config_.system.version;
    config_json["system"]["host"] = config_.system.host;
    config_json["system"]["port"] = config_.system.port;
    config_json["system"]["log_level"] = config_.system.log_level;
    config_json["system"]["max_concurrent_requests"] = config_.system.max_concurrent_requests;
    
    // System instruction
    config_json["system_instruction"] = config_.system_instruction;
    
    // Agents
    config_json["agents"] = json::array();
    for (const auto& agent : config_.agents) {
        json agent_json;
        agent_json["name"] = agent.name;
        agent_json["capabilities"] = agent.capabilities;
        agent_json["auto_start"] = agent.auto_start;
        agent_json["model"] = agent.model;
        agent_json["system_prompt"] = agent.system_prompt;
        
        if (!agent.retrieval.server_url.empty()) {
            agent_json["retrieval"]["server_url"] = agent.retrieval.server_url;
            agent_json["retrieval"]["timeout_seconds"] = agent.retrieval.timeout_seconds;
            agent_json["retrieval"]["max_retries"] = agent.retrieval.max_retries;
            agent_json["retrieval"]["search_enabled"] = agent.retrieval.search_enabled;
            agent_json["retrieval"]["max_results"] = agent.retrieval.max_results;
        }
        
        config_json["agents"].push_back(agent_json);
    }
    
    // Models
    config_json["models"] = json::array();
    for (const auto& [name, model] : config_.models) {
        json model_json;
        model_json["name"] = model.id;
        model_json["actual_name"] = model.actual_name;
        model_json["model_file"] = model.model_file;
        model_json["type"] = model.type;
        model_json["server_url"] = model.server_url;
        model_json["description"] = model.description;
        model_json["preload"] = model.preload;
        model_json["context_size"] = model.context_size;
        model_json["max_tokens"] = model.max_tokens;
        model_json["temperature"] = model.temperature;
        model_json["top_p"] = model.top_p;
        if (model.type == "embedding") {
            model_json["embedding_size"] = model.embedding_size;
        }
        config_json["models"].push_back(model_json);
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
    config_json["performance"]["min_memory_required"] = config_.performance.min_memory_required;
    config_json["performance"]["max_memory_percent"] = config_.performance.max_memory_percent;
    config_json["performance"]["cache_size"] = config_.performance.cache_size;
    config_json["performance"]["min_cache_size"] = config_.performance.min_cache_size;
    config_json["performance"]["max_cache_size"] = config_.performance.max_cache_size;
    config_json["performance"]["worker_threads"] = config_.performance.worker_threads;
    config_json["performance"]["min_worker_threads"] = config_.performance.min_worker_threads;
    config_json["performance"]["max_worker_threads"] = config_.performance.max_worker_threads;
    config_json["performance"]["request_timeout"] = config_.performance.request_timeout;
    config_json["performance"]["max_request_size"] = config_.performance.max_request_size;
    
    // Resource monitoring
    if (resource_monitoring_active_) {
        config_json["resource_status"]["memory_usage_percent"] = current_resources_.memory_usage_percent;
        config_json["resource_status"]["cpu_usage_percent"] = current_resources_.cpu_usage_percent;
        config_json["resource_status"]["disk_usage_percent"] = current_resources_.disk_usage_percent;
        config_json["resource_status"]["total_memory_mb"] = current_resources_.total_memory_mb;
        config_json["resource_status"]["available_memory_mb"] = current_resources_.available_memory_mb;
        config_json["resource_status"]["free_disk_space_mb"] = current_resources_.free_disk_space_mb;
        config_json["resource_status"]["cpu_cores"] = current_resources_.cpu_cores;
    }
    
    // Kolosal Server
    config_json["kolosal_server"]["auto_start"] = config_.kolosal_server.auto_start;
    config_json["kolosal_server"]["startup_timeout"] = config_.kolosal_server.startup_timeout;
    config_json["kolosal_server"]["models_directory"] = config_.kolosal_server.models_directory;
    
    config_json["kolosal_server"]["required_models"] = json::array();
    for (const auto& model : config_.kolosal_server.required_models) {
        json model_json;
        model_json["name"] = model.name;
        model_json["file"] = model.file;
        model_json["type"] = model.type;
        model_json["required"] = model.required;
        config_json["kolosal_server"]["required_models"].push_back(model_json);
    }
    
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
    
    return config_json;
}
