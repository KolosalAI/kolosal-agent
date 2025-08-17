/**
 * @file kolosal_server_functions.hpp
 * @brief Kolosal Server integrated function definitions
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for functions that integrate with kolosal-server endpoints.
 * These functions provide real web search and retrieval capabilities.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_KOLOSAL_SERVER_FUNCTIONS_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_KOLOSAL_SERVER_FUNCTIONS_HPP_INCLUDED

#include "export.hpp"
#include "agent/core/agent_interfaces.hpp"
#include "agent/core/agent_data.hpp"
#include "api/http_client.hpp"
#include "execution/function_execution_manager.hpp"
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace kolosal::agents {

// Forward declarations
class FunctionManager;

/**
 * @brief Real web search function that integrates with kolosal-server internet search
 * 
 * This function provides actual web search capabilities by communicating with
 * the kolosal-server's internet search endpoint, which uses SearXNG backend.
 */
class KOLOSAL_SERVER_API InternetSearchFunction : public AgentFunction {
private:
    std::string server_url;
    std::shared_ptr<HttpClient> http_client;
    int default_timeout;
    
public:
    InternetSearchFunction(const std::string& server_endpoint = "http://localhost:8080", 
                           int timeout_seconds = 30);
    
    std::string get__name() const override { return "internet_search"; }
    std::string get__description() const override { return "Search the internet using SearXNG backend for real-time web results"; }
    std::string get__type() const override { return "web_search"; }
    FunctionResult execute(const AgentData& parameters) override;
    
    // Configuration methods
    void set_server_url(const std::string& url) { server_url = url; }
    void set_timeout(int timeout_seconds) { default_timeout = timeout_seconds; }
    const std::string& get_server_url() const { return server_url; }
};

/**
 * @brief Enhanced web search function with post-processing
 * 
 * This function extends the basic internet search with additional processing,
 * filtering, and formatting of search results for better agent consumption.
 */
class KOLOSAL_SERVER_API EnhancedWebSearchFunction : public AgentFunction {
private:
    std::shared_ptr<InternetSearchFunction> base_search;
    bool enable_content_filtering;
    bool enable_summary_extraction;
    
public:
    EnhancedWebSearchFunction(const std::string& server_endpoint = "http://localhost:8080");
    
    std::string get__name() const override { return "enhanced_web_search"; }
    std::string get__description() const override { return "Advanced web search with content filtering and summary extraction"; }
    std::string get__type() const override { return "web_search"; }
    FunctionResult execute(const AgentData& parameters) override;
    
    // Configuration methods
    void enable_filtering(bool enable) { enable_content_filtering = enable; }
    void enable_summaries(bool enable) { enable_summary_extraction = enable; }
};

/**
 * @brief Document retrieval function with kolosal-server integration
 * 
 * This function provides enhanced document retrieval capabilities by communicating
 * with the kolosal-server's document retrieval endpoints for semantic search.
 */
class KOLOSAL_SERVER_API ServerDocumentRetrievalFunction : public AgentFunction {
private:
    std::string server_url;
    std::shared_ptr<HttpClient> http_client;
    std::string collection_name;
    int default_limit;
    
public:
    ServerDocumentRetrievalFunction(const std::string& server_endpoint = "http://localhost:8080",
                                    const std::string& collection = "documents");
    
    std::string get__name() const override { return "server_document_retrieval"; }
    std::string get__description() const override { return "Retrieve documents from server knowledge base using semantic search"; }
    std::string get__type() const override { return "document_retrieval"; }
    FunctionResult execute(const AgentData& parameters) override;
    
    // Configuration methods
    void set_server_url(const std::string& url) { server_url = url; }
    void set_collection(const std::string& collection) { collection_name = collection; }
    void set_default_limit(int limit) { default_limit = limit; }
};

/**
 * @brief Document addition function with kolosal-server integration
 * 
 * This function allows adding documents to the server's knowledge base through
 * the kolosal-server's document management endpoints.
 */
class KOLOSAL_SERVER_API ServerDocumentAddFunction : public AgentFunction {
private:
    std::string server_url;
    std::shared_ptr<HttpClient> http_client;
    std::string collection_name;
    
public:
    ServerDocumentAddFunction(const std::string& server_endpoint = "http://localhost:8080",
                              const std::string& collection = "documents");
    
    std::string get__name() const override { return "server_add_document"; }
    std::string get__description() const override { return "Add documents to server knowledge base with automatic processing"; }
    std::string get__type() const override { return "document_management"; }
    FunctionResult execute(const AgentData& parameters) override;
    
    // Configuration methods
    void set_server_url(const std::string& url) { server_url = url; }
    void set_collection(const std::string& collection) { collection_name = collection; }
};

/**
 * @brief Document parsing function with kolosal-server integration
 * 
 * This function provides document parsing capabilities by communicating with
 * the kolosal-server's document parsing endpoints for various file formats.
 */
class KOLOSAL_SERVER_API ServerDocumentParserFunction : public AgentFunction {
private:
    std::string server_url;
    std::shared_ptr<HttpClient> http_client;
    
public:
    ServerDocumentParserFunction(const std::string& server_endpoint = "http://localhost:8080");
    
    std::string get__name() const override { return "server_parse_document"; }
    std::string get__description() const override { return "Parse various document formats (PDF, DOCX, HTML, etc.) using server endpoints"; }
    std::string get__type() const override { return "document_processing"; }
    FunctionResult execute(const AgentData& parameters) override;
    
    // Configuration methods
    void set_server_url(const std::string& url) { server_url = url; }
};

/**
 * @brief Embedding generation function with kolosal-server integration
 * 
 * This function generates embeddings for text content using the kolosal-server's
 * embedding model endpoints.
 */
class KOLOSAL_SERVER_API ServerEmbeddingFunction : public AgentFunction {
private:
    std::string server_url;
    std::shared_ptr<HttpClient> http_client;
    std::string model_name;
    
public:
    ServerEmbeddingFunction(const std::string& server_endpoint = "http://localhost:8080",
                           const std::string& model = "");
    
    std::string get__name() const override { return "server_generate_embedding"; }
    std::string get__description() const override { return "Generate embeddings using server-side embedding models"; }
    std::string get__type() const override { return "embedding"; }
    FunctionResult execute(const AgentData& parameters) override;
    
    // Configuration methods
    void set_server_url(const std::string& url) { server_url = url; }
    void set_model(const std::string& model) { model_name = model; }
};

/**
 * @brief Comprehensive knowledge retrieval function
 * 
 * This function combines internet search and document retrieval to provide
 * comprehensive knowledge gathering from both web sources and local knowledge base.
 */
class KOLOSAL_SERVER_API KnowledgeRetrievalFunction : public AgentFunction {
private:
    std::shared_ptr<InternetSearchFunction> web_search;
    std::shared_ptr<ServerDocumentRetrievalFunction> doc_retrieval;
    bool prefer_local_first;
    double relevance_threshold;
    
public:
    KnowledgeRetrievalFunction(const std::string& server_endpoint = "http://localhost:8080");
    
    std::string get__name() const override { return "knowledge_retrieval"; }
    std::string get__description() const override { return "Comprehensive knowledge retrieval from both web and local sources"; }
    std::string get__type() const override { return "hybrid_retrieval"; }
    FunctionResult execute(const AgentData& parameters) override;
    
    // Configuration methods
    void set_prefer_local_first(bool prefer) { prefer_local_first = prefer; }
    void set_relevance_threshold(double threshold) { relevance_threshold = threshold; }
};

/**
 * @brief Function registry for kolosal-server integrated functions
 * 
 * This class provides a centralized way to register and manage all
 * kolosal-server integrated functions.
 */
class KOLOSAL_SERVER_API KolosalServerFunctionRegistry {
private:
    std::string server_url;
    std::map<std::string, std::unique_ptr<AgentFunction>> functions;
    
public:
    KolosalServerFunctionRegistry(const std::string& server_endpoint = "http://localhost:8080");
    ~KolosalServerFunctionRegistry() = default;
    
    // Delete copy constructor and assignment to prevent copying of unique_ptr members
    KolosalServerFunctionRegistry(const KolosalServerFunctionRegistry&) = delete;
    KolosalServerFunctionRegistry& operator=(const KolosalServerFunctionRegistry&) = delete;
    
    // Allow move operations
    KolosalServerFunctionRegistry(KolosalServerFunctionRegistry&&) = default;
    KolosalServerFunctionRegistry& operator=(KolosalServerFunctionRegistry&&) = default;
    
    // Registry methods
    void initialize_all_functions();
    void register_function(std::unique_ptr<AgentFunction> function);
    AgentFunction* get_function(const std::string& name);
    std::vector<std::string> list_available_functions() const;
    
    // Configuration methods
    void set_server_url(const std::string& url);
    const std::string& get_server_url() const { return server_url; }
    
    // Helper method to register all functions with a function manager
    void register_with_manager(std::shared_ptr<FunctionManager> manager);
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_KOLOSAL_SERVER_FUNCTIONS_HPP_INCLUDED
