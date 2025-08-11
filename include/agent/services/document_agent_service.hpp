/**
 * @file document_agent_service.hpp
 * @brief Service layer implementation for document agent
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_DOCUMENT_AGENT_SERVICE_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_DOCUMENT_AGENT_SERVICE_HPP_INCLUDED

#include "export.hpp"
#include "agent/core/agent_interfaces.hpp"
#include "kolosal/retrieval/add_document_types.hpp"
#include "kolosal/retrieval/retrieve_types.hpp"
#include "kolosal/retrieval/document_service.hpp"
#include <json.hpp>
#include <memory>
#include <future>
#include <vector>
#include <string>

namespace kolosal::agents {

using json = nlohmann::json;
// Forward declarations for types that may not exist yet
struct QueryResult {
    std::string query;
    bool success = false;
    std::string error;
    std::string error_type;
    size_t total_found = 0;
    std::vector<kolosal::retrieval::RetrievedDocument> documents;
};

struct DocumentResult {
    std::string id;
    bool success = false;
    std::string error;
};

/**
 * @brief Provides document agent services
 */
class DocumentAgentService {
public:
    DocumentAgentService() = default;
    ~DocumentAgentService() = default;

    // Bulk document operations
    struct BulkDocumentRequest {
        std::vector<kolosal::retrieval::Document> documents;
        std::string collection_name = "documents";
        int batch_size = 100;
        void from_json(const json& j);
        bool validate() const;
    };

    struct BulkDocumentResponse {
        bool success = false;
        std::string message;
        size_t total_documents = 0;
        size_t successful_count = 0;
        size_t failed_count = 0;
        std::string collection_name;
        std::vector<DocumentResult> results;
        
        json to_json() const;
    };

    // Bulk retrieval operations
    struct BulkRetrievalRequest {
        std::vector<std::string> queries;
        int k = 5;
        double score_threshold = 0.0;
        std::string collection_name = "documents";
        void from_json(const json& j);
        bool validate() const;
    };

    struct BulkRetrievalResponse {
        bool success = false;
        std::string message;
        size_t total_queries = 0;
        std::vector<QueryResult> results;
        
        json to_json() const;
    };

    // Document search operations
    struct DocumentSearchRequest {
        std::string query;
        std::string collection_name = "documents";
        int limit = 10;
        double score_threshold = 0.0;
        json filters;
        
        void from_json(const json& j);
        bool validate() const;
    };

    struct DocumentSearchResponse {
        bool success = false;
        std::string message;
        std::vector<kolosal::retrieval::RetrievedDocument> documents;
        size_t total_found = 0;
        json to_json() const;
    };

    // Document upload operations
    struct DocumentUploadRequest {
        std::string content;
        std::string filename;
        std::string collection_name = "documents";
        json metadata;
        bool chunk_document = true;
        void from_json(const json& j);
        bool validate() const;
    };

    struct DocumentUploadResponse {
        bool success = false;
        std::string message;
        std::string document_id;
        std::vector<std::string> chunk_ids;
        
        json to_json() const;
    };

    // Core service methods
    std::future<BulkDocumentResponse> processBulk_Documents(const BulkDocumentRequest& request);
    std::future<BulkRetrievalResponse> processBulk_Retrieval(const BulkRetrievalRequest& request);
    std::future<DocumentSearchResponse> search_Documents(const DocumentSearchRequest& request);
    std::future<DocumentUploadResponse> upload_Document(const DocumentUploadRequest& request);

    // Collection management
    std::future<json> list_Collections();
    std::future<json> create_Collection(const std::string& name, const json& configuration = {});
    std::future<json> delete_Collection(const std::string& name);
    std::future<json> getCollection_Info(const std::string& name);

private:
    kolosal::retrieval::DocumentService& getDocument_Service();
    std::string generateError_Message(const std::string& operation, const std::exception& e);
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_DOCUMENT_AGENT_SERVICE_HPP_INCLUDED
