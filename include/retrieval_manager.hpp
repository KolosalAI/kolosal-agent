#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>
#include <mutex>
#include <json.hpp>
#include "client.hpp"

using json = nlohmann::json;

/**
 * @brief Vector database backend types
 */
enum class VectorDatabaseType {
    FAISS,
    QDRANT,
    AUTO_DETECT
};

/**
 * @brief Web search provider types
 */
enum class SearchProvider {
    DUCKDUCKGO,
    BING,
    GOOGLE,
    SEARX,
    CUSTOM
};

/**
 * @brief Enhanced retrieval manager with vector database integration
 * and comprehensive search capabilities
 */
class RetrievalManager {
public:
    struct Config {
        // Server connection config (loaded from agent.yaml)
        std::string server_url;
        int timeout_seconds = 30;
        int max_retries = 3;
        
        // Vector database configuration
        VectorDatabaseType vector_db_type = VectorDatabaseType::AUTO_DETECT;
        std::string embedding_model = "all-MiniLM-L6-v2";
        int embedding_batch_size = 32;
        
        // Qdrant specific settings
        std::string qdrant_host;
        int qdrant_port = 6333;
        std::string qdrant_api_key = "";
        std::string qdrant_collection = "documents";
        
        // FAISS specific settings
        std::string faiss_index_path = "./faiss_index";
        std::string faiss_metric_type = "cosine";
        
        // Search config  
        bool search_enabled = true;
        SearchProvider search_provider = SearchProvider::DUCKDUCKGO;
        std::string search_api_key = "";
        int max_search_results = 10;
        int max_document_results = 5;
        int max_results = 10;
        
        // Performance settings
        bool enable_caching = true;
        int cache_size_mb = 100;
        int max_concurrent_operations = 4;
        double similarity_threshold = 0.7;
        
        // RAG settings
        bool enable_rag = true;
        int rag_context_window = 2048;
        int rag_max_documents = 5;
        bool rag_include_metadata = true;
    };
    
    explicit RetrievalManager(const Config& config);
    ~RetrievalManager();
    
    // Lifecycle management
    bool initialize();
    void shutdown();
    bool is_available() const;
    json get_status() const;
    
    // Document operations
    json add_document(const json& params);
    json add_documents_batch(const std::vector<json>& documents);
    json update_document(const json& params);
    json remove_document(const json& params);
    json remove_documents_batch(const std::vector<std::string>& document_ids);
    json list_documents(const json& params);
    json get_document_metadata(const std::string& document_id);
    
    // Search and retrieval operations
    json search_documents(const json& params);
    json semantic_search(const json& params);
    json hybrid_search(const json& params);
    json similarity_search(const json& params);
    
    // Web search operations
    json internet_search(const json& params);
    json multi_source_search(const json& params);
    json combined_search(const json& params);
    
    // Advanced retrieval operations
    json retrieve_and_rank(const json& params);
    json contextual_retrieval(const json& params);
    json query_expansion_search(const json& params);
    
    // RAG (Retrieval-Augmented Generation) operations
    json rag_query(const json& params);
    json prepare_rag_context(const json& params);
    json evaluate_rag_response(const json& params);
    
    // Analytics and maintenance
    json get_collection_statistics();
    json get_search_analytics();
    json optimize_index();
    json backup_data(const std::string& backup_path);
    json restore_data(const std::string& backup_path);
    
    // Async operations
    std::future<json> add_documents_batch_async(const std::vector<json>& documents);
    std::future<json> search_documents_async(const json& params);
    std::future<json> combined_search_async(const json& params);
    
    // Configuration management
    void update_config(const Config& new_config);
    Config get_config() const;

private:
    struct EmbeddingCache {
        std::string text_hash;
        std::vector<float> embedding;
        std::chrono::system_clock::time_point timestamp;
    };
    
    struct SearchCache {
        std::string query_hash;
        json results;
        std::chrono::system_clock::time_point timestamp;
    };
    
    Config config_;
    std::unique_ptr<KolosalClient> kolosal_client_;
    mutable std::mutex mutex_;
    bool available_ = false;
    bool initialized_ = false;
    
    // Caching
    std::vector<EmbeddingCache> embedding_cache_;
    std::vector<SearchCache> search_cache_;
    mutable std::mutex cache_mutex_;
    
    // Performance monitoring
    std::atomic<size_t> total_searches_{0};
    std::atomic<size_t> cache_hits_{0};
    std::atomic<size_t> total_documents_{0};
    
    // Initialization helpers
    bool test_vector_db_connection();
    bool test_search_service();
    void setup_default_collection();
    
    // Vector database operations
    json create_embeddings(const std::vector<std::string>& texts);
    json store_embeddings(const std::vector<std::string>& ids, 
                         const std::vector<std::vector<float>>& embeddings,
                         const std::vector<json>& metadata);
    json query_embeddings(const std::vector<float>& query_embedding, int top_k);
    
    // Search helpers
    json perform_web_search(const std::string& query, int max_results);
    json parse_search_results(const json& raw_results);
    json rank_results(const json& results, const std::string& query);
    
    // RAG helpers
    json extract_text_chunks(const std::string& text, int chunk_size = 512, int overlap = 50);
    json build_rag_context(const json& search_results, const std::string& query);
    json evaluate_answer_relevance(const std::string& question, 
                                  const std::string& answer, 
                                  const json& context);
    
    // Caching helpers
    std::vector<float> get_cached_embedding(const std::string& text);
    void cache_embedding(const std::string& text, const std::vector<float>& embedding);
    json get_cached_search(const std::string& query);
    void cache_search_results(const std::string& query, const json& results);
    void cleanup_cache();
    
    // Utility methods
    std::string compute_text_hash(const std::string& text);
    json merge_search_results(const std::vector<json>& result_sets);
    json filter_results_by_relevance(const json& results, double threshold);
    json deduplicate_results(const json& results);
    
    // Error handling
    json create_error_response(const std::string& error_message);
    bool handle_retrieval_error(const std::exception& e, const std::string& operation);
};
