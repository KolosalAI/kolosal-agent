#pragma once

#include <json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

/**
 * @brief Enhanced retrieval functions for specialized retrieval agents
 */
namespace RetrievalFunctions {
    
    /**
     * @brief Chunk text into smaller pieces for better vector storage
     */
    std::vector<std::string> chunk_text(const std::string& text, int chunk_size = 512, int overlap = 50);
    
    /**
     * @brief Extract metadata from document content
     */
    json extract_metadata(const std::string& content);
    
    /**
     * @brief Generate embeddings preview/summary for documents
     */
    json analyze_document_structure(const std::string& content);
    
    /**
     * @brief Similarity search with advanced filtering
     */
    json advanced_similarity_search(const json& query_params, const json& filters);
    
    /**
     * @brief Batch document processing
     */
    json batch_add_documents(const json& documents);
    
    /**
     * @brief Generate search suggestions based on query
     */
    std::vector<std::string> generate_search_suggestions(const std::string& query);
    
    /**
     * @brief Document clustering and organization
     */
    json organize_documents_by_similarity(const json& params);
    
    /**
     * @brief Knowledge graph extraction from documents
     */
    json extract_knowledge_graph(const json& documents);
}
