/**
 * @file document_service_manager.cpp
 * @brief Management and coordination system for document service
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "utils/document_service_manager.hpp"
#include <kolosal/retrieval/document_service.hpp>
#include <kolosal/server_config.hpp>
#include <kolosal/logger.hpp>
#include "logger/server_logger_integration.hpp"
#include <iostream>

namespace kolosal {
namespace agents {

DocumentServiceManager& DocumentServiceManager::get_Instance() {
    static DocumentServiceManager instance;
    return instance;
}

bool DocumentServiceManager::initialize(const kolosal::DatabaseConfig& config) {
    try {
    document_service_ = std::make_unique<kolosal::retrieval::DocumentService>(config);
        initialized_ = true;
        
        // Test initialization
    auto init_future = document_service_->initialize();
        const bool success = init_future.get();
        if (!success) {
            std::cerr << "Warning: DocumentService initialization failed" << std::endl;
            initialized_ = false;
        }
        
        return success;
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to initialize DocumentService: " << e.what() << std::endl;
        initialized_ = false;
        return false;
    }
}

kolosal::retrieval::DocumentService& DocumentServiceManager::getDocument_Service() {
    if (!initialized_ || !document_service_) {
        throw std::runtime_error("DocumentService not available - not initialized or failed to initialize");
    }
    return *document_service_;
}

bool DocumentServiceManager::is_Available() const {
    return initialized_ && document_service_ != nullptr;
}

} // namespace agents
} // namespace kolosal
