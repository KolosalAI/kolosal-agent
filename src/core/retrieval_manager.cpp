#include "../include/retrieval_manager.hpp"
#include <iostream>

RetrievalManager::RetrievalManager(const Config& config) : config_(config) {
    initialize();
}

RetrievalManager::~RetrievalManager() = default;

void RetrievalManager::initialize() {
#ifdef BUILD_WITH_RETRIEVAL
    try {
        // Initialize document service
        kolosal::DatabaseConfig db_config;
        db_config.type = config_.vector_db_type;
        db_config.host = config_.db_host;
        db_config.port = config_.db_port;
        db_config.collection_name = config_.collection_name;
        
        doc_service_ = std::make_unique<kolosal::retrieval::DocumentService>(db_config);
        
        // Initialize search route if enabled
        if (config_.search_enabled) {
            kolosal::SearchConfig search_config;
            search_config.enabled = true;
            search_config.searxng_url = config_.searxng_url;
            search_config.max_results = config_.max_results;
            search_config.timeout = config_.timeout;
            
            search_route_ = std::make_unique<kolosal::InternetSearchRoute>(search_config);
        }
        
        available_ = true;
        std::cout << "RetrievalManager initialized successfully\n";
    } catch (const std::exception& e) {
        std::cout << "RetrievalManager initialization failed: " << e.what() << "\n";
        available_ = false;
    }
#else
    std::cout << "RetrievalManager not available (kolosal-server not built)\n";
    available_ = false;
#endif
}

bool RetrievalManager::is_available() const {
    return available_;
}

json RetrievalManager::get_status() const {
    json status;
    status["available"] = available_;
    status["vector_db_type"] = config_.vector_db_type;
    status["search_enabled"] = config_.search_enabled;
    return status;
}

json RetrievalManager::add_document(const json& params) {
    if (!available_) {
        throw std::runtime_error("Retrieval system not available");
    }
    
#ifdef BUILD_WITH_RETRIEVAL
    // Implementation for adding documents
    // This would use doc_service_->addDocuments()
    json result;
    result["status"] = "success";
    result["message"] = "Document added (placeholder implementation)";
    return result;
#else
    throw std::runtime_error("Retrieval system not built");
#endif
}

json RetrievalManager::search_documents(const json& params) {
    if (!available_) {
        throw std::runtime_error("Retrieval system not available");
    }
    
#ifdef BUILD_WITH_RETRIEVAL
    std::string query = params.value("query", "");
    int limit = params.value("limit", 10);
    
    // Implementation for document search
    // This would use doc_service_->retrieveDocuments()
    json result;
    result["query"] = query;
    result["results"] = json::array();
    result["message"] = "Document search completed (placeholder implementation)";
    return result;
#else
    throw std::runtime_error("Retrieval system not built");
#endif
}

json RetrievalManager::list_documents(const json& params) {
    if (!available_) {
        throw std::runtime_error("Retrieval system not available");
    }
    
#ifdef BUILD_WITH_RETRIEVAL
    // Implementation for listing documents
    json result;
    result["documents"] = json::array();
    result["count"] = 0;
    result["message"] = "Document list retrieved (placeholder implementation)";
    return result;
#else
    throw std::runtime_error("Retrieval system not built");
#endif
}

json RetrievalManager::remove_document(const json& params) {
    if (!available_) {
        throw std::runtime_error("Retrieval system not available");
    }
    
#ifdef BUILD_WITH_RETRIEVAL
    std::string doc_id = params.value("id", "");
    
    // Implementation for removing documents
    json result;
    result["id"] = doc_id;
    result["status"] = "success";
    result["message"] = "Document removed (placeholder implementation)";
    return result;
#else
    throw std::runtime_error("Retrieval system not built");
#endif
}

json RetrievalManager::internet_search(const json& params) {
    if (!available_ || !config_.search_enabled) {
        throw std::runtime_error("Internet search not available");
    }
    
#ifdef BUILD_WITH_RETRIEVAL
    std::string query = params.value("query", "");
    int results = params.value("results", config_.max_results);
    
    // Implementation for internet search
    // This would use search_route_->handle()
    json result;
    result["query"] = query;
    result["results"] = json::array();
    result["message"] = "Internet search completed (placeholder implementation)";
    return result;
#else
    throw std::runtime_error("Search system not built");
#endif
}

json RetrievalManager::combined_search(const json& params) {
    if (!available_) {
        throw std::runtime_error("Retrieval system not available");
    }
    
    std::string query = params.value("query", "");
    
    json result;
    result["query"] = query;
    result["local_results"] = json::array();
    result["web_results"] = json::array();
    
    try {
        // Search local documents
        json doc_params;
        doc_params["query"] = query;
        doc_params["limit"] = 5;
        result["local_results"] = search_documents(doc_params);
    } catch (const std::exception& e) {
        result["local_error"] = e.what();
    }
    
    try {
        // Search internet if enabled
        if (config_.search_enabled) {
            json search_params;
            search_params["query"] = query;
            search_params["results"] = 5;
            result["web_results"] = internet_search(search_params);
        }
    } catch (const std::exception& e) {
        result["web_error"] = e.what();
    }
    
    return result;
}
