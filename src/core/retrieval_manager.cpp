#include "../../include/retrieval_manager.hpp"
#include <iostream>

RetrievalManager::RetrievalManager(const Config& config) : config_(config) {
    initialize();
}

RetrievalManager::~RetrievalManager() = default;

void RetrievalManager::initialize() {
    try {
        // Create KolosalClient with server configuration
        KolosalClient::Config client_config;
        client_config.server_url = config_.server_url;
        client_config.timeout_seconds = config_.timeout_seconds;
        client_config.max_retries = config_.max_retries;
        
        kolosal_client_ = std::make_unique<KolosalClient>(client_config);
        
        // Test connection to server
        if (kolosal_client_->is_server_healthy()) {
            available_ = true;
            std::cout << "RetrievalManager initialized successfully with Kolosal Server at " << config_.server_url << std::endl;
        } else {
            std::cout << "RetrievalManager: Kolosal Server not available at " << config_.server_url << std::endl;
            available_ = false;
        }
    } catch (const std::exception& e) {
        std::cout << "RetrievalManager initialization failed: " << e.what() << std::endl;
        available_ = false;
    }
}

bool RetrievalManager::is_available() const {
    return available_;
}

json RetrievalManager::get_status() const {
    json status;
    status["available"] = available_;
    status["server_url"] = config_.server_url;
    status["search_enabled"] = config_.search_enabled;
    
    if (available_ && kolosal_client_) {
        try {
            status["server_healthy"] = kolosal_client_->is_server_healthy();
        } catch (...) {
            status["server_healthy"] = false;
        }
    }
    
    return status;
}

json RetrievalManager::add_document(const json& params) {
    if (!available_ || !kolosal_client_) {
        throw std::runtime_error("Retrieval system not available");
    }
    
    try {
        return kolosal_client_->add_document(params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to add document: " + std::string(e.what()));
    }
}

json RetrievalManager::search_documents(const json& params) {
    if (!available_ || !kolosal_client_) {
        throw std::runtime_error("Retrieval system not available");
    }
    
    try {
        std::string query = params.value("query", "");
        int limit = params.value("limit", 10);
        
        return kolosal_client_->search_documents(query, limit);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to search documents: " + std::string(e.what()));
    }
}

json RetrievalManager::list_documents(const json& params) {
    if (!available_ || !kolosal_client_) {
        throw std::runtime_error("Retrieval system not available");
    }
    
    try {
        int offset = params.value("offset", 0);
        int limit = params.value("limit", 50);
        
        return kolosal_client_->list_documents(offset, limit);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to list documents: " + std::string(e.what()));
    }
}

json RetrievalManager::remove_document(const json& params) {
    if (!available_ || !kolosal_client_) {
        throw std::runtime_error("Retrieval system not available");
    }
    
    try {
        std::string doc_id = params.value("id", "");
        if (doc_id.empty()) {
            throw std::runtime_error("Document ID is required");
        }
        
        return kolosal_client_->remove_document(doc_id);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to remove document: " + std::string(e.what()));
    }
}

json RetrievalManager::internet_search(const json& params) {
    if (!available_ || !kolosal_client_) {
        throw std::runtime_error("Search system not available");
    }
    
    if (!config_.search_enabled) {
        throw std::runtime_error("Internet search not enabled");
    }
    
    try {
        std::string query = params.value("query", "");
        int results = params.value("results", config_.max_results);
        
        return kolosal_client_->internet_search(query, results);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to perform internet search: " + std::string(e.what()));
    }
}

json RetrievalManager::combined_search(const json& params) {
    if (!available_ || !kolosal_client_) {
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
