#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>
#include <json.hpp>
#include "kolosal_client.hpp"

using json = nlohmann::json;

/**
 * @brief Client-based manager for retrieval and search operations
 * Uses KolosalClient to communicate with the Kolosal Server for retrieval tasks
 */
class RetrievalManager {
public:
    struct Config {
        // Server connection config
        std::string server_url = "http://127.0.0.1:8081";
        int timeout_seconds = 30;
        int max_retries = 3;
        
        // Search config  
        bool search_enabled = false;
        int max_results = 10;
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
    std::unique_ptr<KolosalClient> kolosal_client_;
    bool available_ = false;
    
    void initialize();
};
