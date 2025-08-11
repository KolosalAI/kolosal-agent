/**
 * @file document_service_manager.hpp
 * @brief Management and coordination system for document service
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_DOCUMENT_SERVICE_MANAGER_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_DOCUMENT_SERVICE_MANAGER_HPP_INCLUDED

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
    static DocumentServiceManager& get_Instance();
    
    /**
     * @brief Initialize the document service with configuration
     */
    bool initialize(const kolosal::DatabaseConfig& config);
    
    /**
     * @brief Get the document service instance
     * @throws std::runtime_error if service is not available
     */
    kolosal::retrieval::DocumentService& getDocument_Service();
    
    /**
     * @brief Check if document service is available
     */
    bool is_Available() const;

private:
    DocumentServiceManager() = default;
    std::unique_ptr<kolosal::retrieval::DocumentService> document_service_;
    bool initialized_ = false;
};

} // namespace agents
} // namespace kolosal

#endif // KOLOSAL_AGENT_INCLUDE_DOCUMENT_SERVICE_MANAGER_HPP_INCLUDED
