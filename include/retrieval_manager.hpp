#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>
#include <json.hpp>

using json = nlohmann::json;

#ifdef BUILD_WITH_RETRIEVAL
#include <kolosal/retrieval/document_service.hpp>
#endif

/**
 * @brief Simple manager for retrieval and search operations
 */
class RetrievalManager {
public:
    struct Config {
        // Vector database config
        std::string vector_db_type = "faiss";  // "faiss" or "qdrant"
        std::string db_host = "localhost";
        int db_port = 6333;
        std::string collection_name = "documents";
        
        // Search config
        bool search_enabled = false;
        std::string searxng_url = "http://localhost:8888";
        int max_results = 10;
        int timeout = 30;
    };
    
    explicit RetrievalManager(const Config& config);
    ~RetrievalManager();
    
    // Document operations
    json add_document(const json& params);
    json search_documents(const json& params);
    json list_documents(const json& params);
    json remove_document(const json& params);
    
    // Search operations
    json internet_search(const json& params);
    json combined_search(const json& params);
    
    // Utility
    bool is_available() const;
    json get_status() const;

private:
    Config config_;
    bool available_ = false;
    
#ifdef BUILD_WITH_RETRIEVAL
    std::unique_ptr<kolosal::retrieval::DocumentService> doc_service_;
#endif
    
    void initialize();
};
