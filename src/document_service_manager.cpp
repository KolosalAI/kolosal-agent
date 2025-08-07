#include "document_service_manager.hpp"
#include <kolosal/logger.hpp>
#include "server_logger_adapter.hpp"
#include <iostream>

namespace kolosal {
namespace agents {

DocumentServiceManager& DocumentServiceManager::getInstance() {
    static DocumentServiceManager instance;
    return instance;
}

bool DocumentServiceManager::initialize(const DatabaseConfig& config) {
    try {
        document_service_ = std::make_unique<kolosal::retrieval::DocumentService>(config);
        initialized_ = true;
        
        // Test initialization
        auto init_future = document_service_->initialize();
        bool success = init_future.get();
        
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

kolosal::retrieval::DocumentService& DocumentServiceManager::getDocumentService() {
    if (!initialized_ || !document_service_) {
        throw std::runtime_error("DocumentService not available - not initialized or failed to initialize");
    }
    return *document_service_;
}

bool DocumentServiceManager::isAvailable() const {
    return initialized_ && document_service_ != nullptr;
}

} // namespace agents
} // namespace kolosal
