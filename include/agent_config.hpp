#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <json.hpp>

using json = nlohmann::json;

/**
 * @brief System resource information
 */
struct SystemResources {
    size_t total_memory_mb;
    size_t available_memory_mb;
    size_t free_disk_space_mb;
    int cpu_cores;
    double cpu_usage_percent;
    double memory_usage_percent;
    double disk_usage_percent;
};

/**
 * @brief Configuration validation results
 */
struct ValidationResult {
    bool is_valid;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    
    bool has_errors() const { return !errors.empty(); }
    bool has_warnings() const { return !warnings.empty(); }
    void add_error(const std::string& error) { errors.push_back(error); is_valid = false; }
    void add_warning(const std::string& warning) { warnings.push_back(warning); }
};

/**
 * @brief Configuration structure for agent system
 */
struct AgentSystemConfig {
    // Validation configuration
    struct ValidationConfig {
        bool enabled = true;
        bool strict_mode = false;
        std::string schema_version = "1.0.0";
        
        struct {
            int min_port = 1024;
            int max_port = 65535;
        } port_ranges;
        
        struct {
            std::string min_system_memory = "1GB";
            int max_memory_percent = 90;
            std::string min_cache_size = "64MB";
        } memory_limits;
        
        struct {
            int min_timeout = 1000;
            int max_timeout = 300000;
            int default_timeout = 30000;
        } timeout_limits;
        
        struct {
            bool require_default_model = true;
            bool require_embedding_model = true;
            bool validate_model_files = true;
            bool check_model_compatibility = true;
        } model_requirements;
        
        struct {
            int min_agents = 1;
            int max_agents = 50;
            std::vector<std::string> required_capabilities = {"chat"};
        } agent_requirements;
        
        struct {
            int min_functions = 3;
            std::vector<std::string> required_functions = {"chat", "status", "analyze"};
            bool validate_parameters = true;
        } function_requirements;
    } validation;
    
    // System settings
    struct {
        std::string name;
        std::string version;
        std::string host;
        int port;
        std::string log_level;
        int max_concurrent_requests;
    } system;
    
    // Agent configurations
    struct AgentConfig {
        std::string name;
        std::vector<std::string> capabilities;
        bool auto_start;
        std::string model;
        std::string system_prompt;
        
        // Retrieval configuration
        struct RetrievalConfig {
            std::string server_url;
            int timeout_seconds = 30;
            int max_retries = 3;
            bool search_enabled = false;
            int max_results = 10;
        } retrieval;
    };
    std::vector<AgentConfig> agents;
    
    // Model configurations
    struct ModelConfig {
        std::string id;
        std::string actual_name;
        std::string model_file;
        std::string type;
        std::string server_url;
        std::string description;
        bool preload = true;
        int context_size = 2048;
        int max_tokens = 1024;
        double temperature = 0.7;
        double top_p = 0.9;
        int embedding_size = 384;
    };
    std::map<std::string, ModelConfig> models;
    
    // Function definitions
    struct FunctionConfig {
        std::string description;
        int timeout;
        std::vector<json> parameters;
    };
    std::map<std::string, FunctionConfig> functions;
    
    // Performance settings with resource management
    struct PerformanceConfig {
        std::string max_memory_usage = "auto";
        std::string min_memory_required = "512MB";
        int max_memory_percent = 75;
        std::string cache_size = "auto";
        std::string min_cache_size = "128MB";
        std::string max_cache_size = "1GB";
        std::string worker_threads = "auto";
        int min_worker_threads = 2;
        int max_worker_threads = 16;
        int request_timeout = 30000;
        std::string max_request_size = "10MB";
        
        struct DiskMonitoring {
            bool enabled = true;
            std::string min_free_space = "1GB";
            std::string warning_threshold = "2GB";
            int check_interval = 300;
        } disk_space_monitoring;
        
        struct ResourceLimits {
            int cpu_usage_threshold = 80;
            int memory_usage_threshold = 85;
            int disk_usage_threshold = 90;
        } resource_limits;
        
        struct GracefulDegradation {
            bool enabled = true;
            bool reduce_cache_on_memory_pressure = true;
            bool reduce_workers_on_cpu_pressure = true;
            int queue_limit_on_resource_pressure = 50;
        } graceful_degradation;
    } performance;
    
    // Kolosal Server configuration
    struct KolosalServerConfig {
        bool auto_start = true;
        int startup_timeout = 60;
        int health_check_interval = 10;
        int max_retries = 3;
        int retry_delay = 2000;
        
        struct ResourceLimits {
            std::string max_memory = "1.5GB";
            int max_cpu_percent = 80;
        } resource_limits;
        
        std::string models_directory = "./models";
        
        struct RequiredModel {
            std::string name;
            std::string file;
            std::string type;
            bool required = true;
        };
        std::vector<RequiredModel> required_models;
        
        int model_preload_timeout = 120;
        int graceful_shutdown_timeout = 30;
    } kolosal_server;
    
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
    
    // Error handling configuration
    struct ErrorHandlingConfig {
        bool enable_fallbacks = true;
        bool fallback_responses = true;
        int max_retry_attempts = 3;
        double retry_backoff_multiplier = 2.0;
        bool timeout_escalation = true;
        bool graceful_degradation = true;
        
        struct OfflineMode {
            bool enable = true;
            bool cache_responses = true;
            std::string max_cache_size = "100MB";
        } offline_mode;
    } error_handling;
    
    // Circuit breaker configuration
    struct CircuitBreakerConfig {
        int failure_threshold = 5;
        int recovery_timeout = 30;
        int half_open_max_calls = 3;
        int metrics_window = 60;
    } circuit_breaker;
};

/**
 * @brief Resource monitor interface
 */
class ResourceMonitor {
public:
    virtual ~ResourceMonitor() = default;
    virtual SystemResources get_system_resources() = 0;
    virtual bool check_resource_thresholds(const AgentSystemConfig::PerformanceConfig& config) = 0;
    virtual void start_monitoring(std::function<void(const SystemResources&)> callback) = 0;
    virtual void stop_monitoring() = 0;
};

/**
 * @brief Configuration validator
 */
class ConfigValidator {
public:
    static ValidationResult validate_config(const AgentSystemConfig& config);
    static ValidationResult validate_models(const std::map<std::string, AgentSystemConfig::ModelConfig>& models, 
                                           const std::string& models_directory);
    static ValidationResult validate_ports(const AgentSystemConfig& config);
    static ValidationResult validate_resource_settings(const AgentSystemConfig::PerformanceConfig& performance);
    static ValidationResult validate_agents(const std::vector<AgentSystemConfig::AgentConfig>& agents);
    static ValidationResult validate_functions(const std::map<std::string, AgentSystemConfig::FunctionConfig>& functions);
    static size_t parse_memory_string(const std::string& memory_str);
    
private:
    static bool validate_timeout_range(int timeout, int min_timeout, int max_timeout);
    static bool file_exists(const std::string& path);
};

/**
 * @brief Resource-aware configuration manager
 */
class AgentConfigManager {
private:
    AgentSystemConfig config_;
    std::string config_file_path_;
    std::unique_ptr<ResourceMonitor> resource_monitor_;
    SystemResources current_resources_;
    bool resource_monitoring_active_;
    
    // Internal methods
    bool load_from_file(const std::string& file_path);
    void set_default_config();
    void apply_resource_based_defaults();
    void adjust_performance_settings();
    ValidationResult validate_and_adjust_config();
    
public:
    AgentConfigManager();
    ~AgentConfigManager() = default;
    
    // Resource monitoring
    void set_resource_monitor(std::unique_ptr<ResourceMonitor> monitor);
    void start_resource_monitoring();
    void stop_resource_monitoring();
    const SystemResources& get_current_resources() const { return current_resources_; }
    
    // Configuration loading with validation
    ValidationResult load_config(const std::string& file_path = "agent.yaml");
    ValidationResult reload_config();
    
    // Configuration access
    const AgentSystemConfig& get_config() const { return config_; }
    const std::string& get_config_file_path() const { return config_file_path_; }
    
    // Specific configuration getters
    std::string get_host() const { return config_.system.host; }
    int get_port() const { return config_.system.port; }
    const std::vector<AgentSystemConfig::AgentConfig>& get_agent_configs() const { return config_.agents; }
    const std::map<std::string, AgentSystemConfig::FunctionConfig>& get_function_configs() const { return config_.functions; }
    const std::map<std::string, AgentSystemConfig::ModelConfig>& get_model_configs() const { return config_.models; }
    
    // Resource-aware getters
    size_t get_optimal_memory_usage() const;
    int get_optimal_worker_threads() const;
    size_t get_optimal_cache_size() const;
    bool should_reduce_resource_usage() const;
    
    // Configuration validation
    ValidationResult validate_config() const;
    
    // Configuration info
    void print_config_summary() const;
    void print_validation_results(const ValidationResult& result) const;
    json to_json() const;
    
    // Dynamic configuration adjustment
    void adjust_for_resource_pressure();
    void restore_optimal_settings();
};

/**
 * @brief Default resource monitor implementation for Windows/Linux
 */
class DefaultResourceMonitor : public ResourceMonitor {
private:
    bool monitoring_active_;
    std::function<void(const SystemResources&)> callback_;
    
public:
    DefaultResourceMonitor();
    ~DefaultResourceMonitor() override;
    
    SystemResources get_system_resources() override;
    bool check_resource_thresholds(const AgentSystemConfig::PerformanceConfig& config) override;
    void start_monitoring(std::function<void(const SystemResources&)> callback) override;
    void stop_monitoring() override;
    
private:
    SystemResources get_windows_resources();
    SystemResources get_linux_resources();
};
