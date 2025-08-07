#pragma once

#include <kolosal/retrieval/document_service.hpp>
#include <kolosal/server_config.hpp>
#include <memory>
#include <stdexcept>

namespace kolosal {
namespace agents {

/**
 * @brief Manager for DocumentService instances in the agent context
 */
class DocumentServiceManager {
public:
    static DocumentServiceManager& getInstance();
    
    /**
     * @brief Initialize the document service with configuration
     */
    bool initialize(const DatabaseConfig& config);
    
    /**
     * @brief Get the document service instance
     * @throws std::runtime_error if service is not available
     */
    kolosal::retrieval::DocumentService& getDocumentService();
    
    /**
     * @brief Check if document service is available
     */
    bool isAvailable() const;

private:
    DocumentServiceManager() = default;
    std::unique_ptr<kolosal::retrieval::DocumentService> document_service_;
    bool initialized_ = false;
};

} // namespace agents
} // namespace kolosal
