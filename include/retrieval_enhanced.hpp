#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>
#include <chrono>
#include <future>
#include <unordered_set>
#include <nlohmann/json.hpp>

namespace kolosal {

// Forward declarations
struct Document;
struct SearchResult;
struct VectorSearchResult;

// Utility functions
std::vector<float> normalize_vector(const std::vector<float>& vector);
float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b);

// Document representation
struct Document {
    std::string id;
    std::string content;
    std::string source;
    std::unordered_map<std::string, std::string> metadata;
    std::chrono::system_clock::time_point created_at;
    
    Document() = default;
    Document(const std::string& id, const std::string& content, const std::string& source = "");
    
    nlohmann::json to_json() const;
    static Document from_json(const nlohmann::json& json_doc);
};

// Search result
struct SearchResult {
    Document document;
    float similarity_score;
    
    SearchResult() : similarity_score(0.0f) {}
    SearchResult(const Document& doc, float score);
};

// Vector search result (internal)
struct VectorSearchResult {
    std::string id;
    float score;
    nlohmann::json payload;
};

// Search options
struct SearchOptions {
    size_t limit = 10;
    float threshold = 0.0f;
    bool include_metadata = true;
    std::vector<std::string> filters;
};

// Retrieval statistics
struct RetrievalStats {
    size_t total_documents = 0;
    size_t cache_size = 0;
    double avg_query_time_ms = 0.0;
};

// Configuration for retrieval system
struct RetrievalConfig {
    // Vector store settings
    bool use_qdrant = false;
    bool use_faiss = true;
    
    // Qdrant settings
    std::string qdrant_host = "localhost";
    int qdrant_port = 6333;
    std::string collection_name = "documents";
    
    // FAISS settings
    std::string faiss_index_type = "Flat";
    
    // General settings
    size_t embedding_dimension = 768;
    size_t max_cache_size = 10000;
};

// Abstract vector store interface
class IVectorStore {
public:
    virtual ~IVectorStore() = default;
    virtual std::string add_document(const Document& document, const std::vector<float>& embedding) = 0;
    virtual std::vector<VectorSearchResult> search(const std::vector<float>& query_vector, 
                                                  size_t limit = 10, float threshold = 0.0f) = 0;
    virtual bool delete_document(const std::string& document_id) = 0;
};

// Qdrant vector store implementation
class QdrantVectorStore : public IVectorStore {
private:
    std::string host_;
    int port_;
    std::string collection_name_;
    bool connected_;
    
    // Local storage for demo purposes (replace with actual Qdrant client)
    struct VectorPoint {
        std::string id;
        std::vector<float> vector;
        nlohmann::json payload;
    };
    
    std::unordered_map<std::string, VectorPoint> points_;
    std::mutex data_mutex_;
    
    std::string generate_uuid();

public:
    QdrantVectorStore(const std::string& host, int port, const std::string& collection);
    
    bool connect();
    void disconnect();
    bool create_collection(const std::string& collection_name, size_t vector_size);
    
    std::string add_document(const Document& document, const std::vector<float>& embedding) override;
    std::vector<VectorSearchResult> search(const std::vector<float>& query_vector, 
                                          size_t limit = 10, float threshold = 0.0f) override;
    bool delete_document(const std::string& document_id) override;
    
    // Batch operations
    std::vector<VectorSearchResult> batch_search(const std::vector<std::vector<float>>& query_vectors, 
                                                size_t limit = 10, float threshold = 0.0f);
};

// FAISS vector store implementation
class FAISSVectorStore : public IVectorStore {
private:
    size_t dimension_;
    std::string index_type_;
    void* index_; // FAISS index pointer
    bool initialized_;
    
    // Document storage
    std::unordered_map<std::string, Document> documents_;
    std::unordered_map<std::string, std::vector<float>> embeddings_;
    std::mutex data_mutex_;
    
    std::string generate_uuid();

public:
    FAISSVectorStore(size_t dimension, const std::string& index_type = "Flat");
    ~FAISSVectorStore();
    
    bool initialize();
    
    std::string add_document(const Document& document, const std::vector<float>& embedding) override;
    std::vector<VectorSearchResult> search(const std::vector<float>& query_vector, 
                                          size_t k = 10, float threshold = 0.0f) override;
    bool delete_document(const std::string& document_id) override;
    
    // Index management
    void save_index(const std::string& filepath);
    bool load_index(const std::string& filepath);
};

// Enhanced retrieval manager with advanced features
class EnhancedRetrievalManager {
private:
    RetrievalConfig config_;
    bool initialized_;
    
    // Vector stores
    std::unique_ptr<QdrantVectorStore> qdrant_store_;
    std::unique_ptr<FAISSVectorStore> faiss_store_;
    
    // Document cache
    std::unordered_map<std::string, Document> document_cache_;
    mutable std::mutex cache_mutex_;
    
    // Helper methods
    std::vector<float> generate_embedding(const std::string& text);
    std::string to_lowercase(const std::string& str);

public:
    EnhancedRetrievalManager();
    
    // Initialization
    bool initialize(const RetrievalConfig& config);
    
    // Document management
    std::string add_document(const Document& document);
    std::vector<std::string> batch_add_documents(const std::vector<Document>& documents);
    bool delete_document(const std::string& document_id);
    std::optional<Document> get_document(const std::string& document_id);
    
    // Search operations
    std::vector<SearchResult> search(const std::string& query, const SearchOptions& options = {});
    std::vector<SearchResult> semantic_search(const std::string& query, const SearchOptions& options = {});
    std::vector<SearchResult> hybrid_search(const std::string& query, const SearchOptions& options = {});
    
    // System management
    RetrievalStats get_stats() const;
    void clear_cache();
};

} // namespace kolosal
