#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <future>
#include <json.hpp>

using json = nlohmann::json;

/**
 * @brief Model types supported by the system
 */
enum class ModelType {
    LANGUAGE_MODEL,
    EMBEDDING_MODEL,
    MULTIMODAL_MODEL,
    CLASSIFICATION_MODEL,
    UNKNOWN
};

/**
 * @brief Model capabilities and features
 */
struct ModelCapabilities {
    bool supports_chat = false;
    bool supports_completion = false;
    bool supports_embedding = false;
    bool supports_function_calling = false;
    bool supports_multimodal = false;
    bool supports_streaming = false;
    bool supports_batching = false;
    int max_tokens = 4096;
    int context_window = 4096;
    std::vector<std::string> supported_formats;
};

/**
 * @brief Model configuration and metadata
 */
struct ModelConfig {
    std::string id;
    std::string name;
    std::string description;
    ModelType type;
    std::string file_path;
    std::string server_url;
    ModelCapabilities capabilities;
    
    // Runtime parameters
    int max_tokens = 1024;
    double temperature = 0.7;
    double top_p = 0.9;
    int top_k = 50;
    double frequency_penalty = 0.0;
    double presence_penalty = 0.0;
    
    // Resource requirements
    size_t memory_required_mb = 0;
    bool gpu_required = false;
    int gpu_layers = 0;
    
    // Load balancing and scaling
    int max_concurrent_requests = 4;
    int request_timeout_seconds = 30;
    bool auto_reload = false;
    double cpu_threshold = 0.8;
    double memory_threshold = 0.9;
};

/**
 * @brief Model execution statistics
 */
struct ModelStats {
    uint64_t total_requests = 0;
    uint64_t successful_requests = 0;
    uint64_t failed_requests = 0;
    uint64_t total_tokens_processed = 0;
    double average_response_time_ms = 0.0;
    double tokens_per_second = 0.0;
    std::chrono::system_clock::time_point last_request_time;
    std::chrono::system_clock::time_point load_time;
    bool is_loaded = false;
    size_t current_memory_usage_mb = 0;
};

/**
 * @brief Base interface for all model implementations
 */
class IModelInterface {
public:
    virtual ~IModelInterface() = default;
    
    // Lifecycle
    virtual bool load(const ModelConfig& config) = 0;
    virtual void unload() = 0;
    virtual bool is_loaded() const = 0;
    
    // Core operations
    virtual json generate(const json& request) = 0;
    virtual json generate_streaming(const json& request) = 0;
    virtual json generate_batch(const std::vector<json>& requests) = 0;
    
    // Model information
    virtual ModelConfig get_config() const = 0;
    virtual ModelStats get_stats() const = 0;
    virtual ModelCapabilities get_capabilities() const = 0;
    
    // Health and diagnostics
    virtual bool health_check() = 0;
    virtual json get_diagnostics() = 0;
};

/**
 * @brief Language model implementation
 */
class LanguageModelInterface : public IModelInterface {
public:
    LanguageModelInterface(const ModelConfig& config);
    ~LanguageModelInterface() override;
    
    // IModelInterface implementation
    bool load(const ModelConfig& config) override;
    void unload() override;
    bool is_loaded() const override;
    json generate(const json& request) override;
    json generate_streaming(const json& request) override;
    json generate_batch(const std::vector<json>& requests) override;
    ModelConfig get_config() const override;
    ModelStats get_stats() const override;
    ModelCapabilities get_capabilities() const override;
    bool health_check() override;
    json get_diagnostics() override;
    
    // Language model specific operations
    json chat_completion(const json& messages, const json& options = json{});
    json text_completion(const std::string& prompt, const json& options = json{});
    json tokenize(const std::string& text);
    json detokenize(const std::vector<int>& tokens);
    
private:
    ModelConfig config_;
    ModelStats stats_;
    mutable std::mutex stats_mutex_;
    bool loaded_ = false;
    
    void update_stats(bool success, int tokens_processed, double response_time_ms);
};

/**
 * @brief Embedding model implementation
 */
class EmbeddingModelInterface : public IModelInterface {
public:
    EmbeddingModelInterface(const ModelConfig& config);
    ~EmbeddingModelInterface() override;
    
    // IModelInterface implementation
    bool load(const ModelConfig& config) override;
    void unload() override;
    bool is_loaded() const override;
    json generate(const json& request) override;
    json generate_streaming(const json& request) override;
    json generate_batch(const std::vector<json>& requests) override;
    ModelConfig get_config() const override;
    ModelStats get_stats() const override;
    ModelCapabilities get_capabilities() const override;
    bool health_check() override;
    json get_diagnostics() override;
    
    // Embedding specific operations
    std::vector<float> create_embedding(const std::string& text);
    std::vector<std::vector<float>> create_embeddings_batch(const std::vector<std::string>& texts);
    int get_embedding_dimensions() const;
    double compute_similarity(const std::vector<float>& embedding1, const std::vector<float>& embedding2);
    
private:
    ModelConfig config_;
    ModelStats stats_;
    mutable std::mutex stats_mutex_;
    bool loaded_ = false;
    int embedding_dimensions_ = 384;
    
    void update_stats(bool success, int texts_processed, double response_time_ms);
};

/**
 * @brief Model registry and factory
 */
class ModelRegistry {
public:
    static ModelRegistry& instance();
    
    // Model registration
    void register_model_type(const std::string& type_name, 
                           std::function<std::unique_ptr<IModelInterface>(const ModelConfig&)> factory);
    
    // Model creation
    std::unique_ptr<IModelInterface> create_model(const ModelConfig& config);
    std::vector<std::string> get_supported_types() const;
    bool is_type_supported(const std::string& type_name) const;
    
    // Built-in model types
    void register_builtin_types();
    
private:
    ModelRegistry();
    std::unordered_map<std::string, std::function<std::unique_ptr<IModelInterface>(const ModelConfig&)>> factories_;
    mutable std::mutex registry_mutex_;
};

/**
 * @brief Advanced model manager supporting multiple models and load balancing
 */
class AdvancedModelManager {
public:
    AdvancedModelManager();
    ~AdvancedModelManager();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    
    // Model management
    bool load_model(const ModelConfig& config);
    bool unload_model(const std::string& model_id);
    bool reload_model(const std::string& model_id);
    std::vector<std::string> get_loaded_models() const;
    
    // Model operations
    json generate(const std::string& model_id, const json& request);
    json generate_with_fallback(const std::vector<std::string>& model_ids, const json& request);
    json generate_batch(const std::string& model_id, const std::vector<json>& requests);
    
    // Load balancing and routing
    std::string select_best_model(ModelType type, const json& requirements = json{});
    json distribute_batch_requests(const std::vector<json>& requests);
    
    // Model information
    json get_model_info(const std::string& model_id) const;
    json get_all_models_info() const;
    ModelStats get_model_stats(const std::string& model_id) const;
    json get_system_stats() const;
    
    // Health monitoring
    json health_check_all_models();
    json get_model_health(const std::string& model_id);
    void set_health_check_interval(int seconds);
    
    // Configuration management
    bool update_model_config(const std::string& model_id, const ModelConfig& config);
    bool set_model_parameter(const std::string& model_id, const std::string& parameter, const json& value);
    
    // Hot-swapping and updates
    bool hot_swap_model(const std::string& model_id, const ModelConfig& new_config);
    bool schedule_model_reload(const std::string& model_id, const std::chrono::system_clock::time_point& when);
    
    // Resource management
    json get_resource_usage() const;
    bool optimize_memory_usage();
    bool scale_model_instances(const std::string& model_id, int target_instances);
    
    // Event callbacks
    using ModelEventCallback = std::function<void(const std::string&, const std::string&, const json&)>;
    void set_event_callback(ModelEventCallback callback);
    
private:
    struct ModelInstance {
        std::unique_ptr<IModelInterface> model;
        ModelConfig config;
        ModelStats stats;
        std::mutex instance_mutex;
        bool healthy = true;
        std::chrono::system_clock::time_point last_health_check;
    };
    
    mutable std::mutex manager_mutex_;
    std::unordered_map<std::string, std::unique_ptr<ModelInstance>> models_;
    
    // Load balancing
    std::unordered_map<std::string, std::vector<std::string>> type_to_models_;
    std::unordered_map<std::string, size_t> model_request_counts_;
    
    // Health monitoring
    std::thread health_monitor_thread_;
    std::atomic<bool> health_monitoring_active_{false};
    int health_check_interval_seconds_ = 60;
    
    // Event system
    ModelEventCallback event_callback_;
    
    // Resource monitoring
    std::atomic<size_t> total_memory_usage_mb_{0};
    std::atomic<size_t> total_active_requests_{0};
    
    // Helper methods
    void health_monitor_loop();
    void emit_event(const std::string& model_id, const std::string& event_type, const json& data = json{});
    bool check_resource_limits(const ModelConfig& config);
    void update_load_balancing_info();
    ModelInstance* get_least_loaded_instance(const std::vector<std::string>& model_ids);
    void cleanup_failed_models();
};

/**
 * @brief RAG (Retrieval-Augmented Generation) integration
 */
class RAGModelInterface {
public:
    RAGModelInterface(std::shared_ptr<AdvancedModelManager> model_manager,
                     std::shared_ptr<class RetrievalManager> retrieval_manager);
    ~RAGModelInterface();
    
    // RAG operations
    json rag_generate(const std::string& query, const json& options = json{});
    json rag_chat(const json& messages, const json& options = json{});
    json rag_batch_generate(const std::vector<std::string>& queries, const json& options = json{});
    
    // Configuration
    void set_retrieval_config(const json& config);
    void set_generation_config(const json& config);
    
    // Analytics
    json get_rag_metrics() const;
    json analyze_retrieval_quality(const std::string& query, const json& retrieved_docs) const;
    
private:
    std::shared_ptr<AdvancedModelManager> model_manager_;
    std::shared_ptr<class RetrievalManager> retrieval_manager_;
    
    struct RAGConfig {
        std::string retrieval_model_id;
        std::string generation_model_id;
        int max_retrieved_docs = 5;
        double similarity_threshold = 0.7;
        int context_window = 2048;
        bool include_sources = true;
        bool rerank_results = true;
    } rag_config_;
    
    json retrieve_relevant_context(const std::string& query);
    json build_augmented_prompt(const std::string& query, const json& context);
    json post_process_rag_response(const json& response, const json& context);
};
