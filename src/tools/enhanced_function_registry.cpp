/**
 * @file enhanced_function_registry.cpp
 * @brief Implementation of enhanced function registry
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for enhanced function registration.
 */

#include "tools/enhanced_function_registry.hpp"
#include "logger/kolosal_logger.hpp"
#include "kolosal/logger.hpp"
#include "api/http_client.hpp"
#include <algorithm>

namespace kolosal::agents {

// EnhancedFunctionRegistry Implementation
EnhancedFunctionRegistry::EnhancedFunctionRegistry(const std::string& server_endpoint) 
    : server_url(server_endpoint), server_functions_enabled(true) {
    server_registry = std::make_shared<KolosalServerFunctionRegistry>(server_endpoint);
}

void EnhancedFunctionRegistry::register_builtin_functions(std::shared_ptr<FunctionManager> manager) {
    if (!manager) {
        ServerLogger::logError("EnhancedFunctionRegistry: Cannot register built-in functions with null manager");
        return;
    }
    
    // Register all built-in functions
    manager->register_function(std::make_unique<AddFunction>());
    manager->register_function(std::make_unique<EchoFunction>());    
    manager->register_function(std::make_unique<DelayFunction>());
    manager->register_function(std::make_unique<TextAnalysisFunction>());
    manager->register_function(std::make_unique<TextProcessingFunction>());
    manager->register_function(std::make_unique<DataTransformFunction>());
    manager->register_function(std::make_unique<DataAnalysisFunction>());
    manager->register_function(std::make_unique<InferenceFunction>());
    
    // Register built-in retrieval functions (these are basic/simulation versions)
    manager->register_function(std::make_unique<RetrievalFunction>());
    manager->register_function(std::make_unique<ContextRetrievalFunction>());
    manager->register_function(std::make_unique<WebSearchFunction>()); // This is the simulation version
    manager->register_function(std::make_unique<CodeGenerationFunction>());
    
    // Register document management functions
    manager->register_function(std::make_unique<AddDocumentFunction>());
    manager->register_function(std::make_unique<RemoveDocumentFunction>());
    
    // Register document parsing functions
    manager->register_function(std::make_unique<ParsePdfFunction>());
    manager->register_function(std::make_unique<ParseDocxFunction>());
    
    // Register embedding and utility functions
    manager->register_function(std::make_unique<GetEmbeddingFunction>());
    manager->register_function(std::make_unique<TestDocumentServiceFunction>());
    
    ServerLogger::logInfo("EnhancedFunctionRegistry: Registered built-in functions");
}

void EnhancedFunctionRegistry::register_server_functions(std::shared_ptr<FunctionManager> manager) {
    if (!manager) {
        ServerLogger::logError("EnhancedFunctionRegistry: Cannot register server functions with null manager");
        return;
    }
    
    if (!server_functions_enabled) {
        ServerLogger::logInfo("EnhancedFunctionRegistry: Server functions are disabled, skipping registration");
        return;
    }
    
    // Register kolosal-server integrated functions
    server_registry->register_with_manager(manager);
    
    ServerLogger::logInfo("EnhancedFunctionRegistry: Registered kolosal-server integrated functions");
}

void EnhancedFunctionRegistry::register_all_functions(std::shared_ptr<FunctionManager> manager, 
                                                     bool include_server_functions) {
    if (!manager) {
        ServerLogger::logError("EnhancedFunctionRegistry: Cannot register functions with null manager");
        return;
    }
    
    // Register built-in functions first
    register_builtin_functions(manager);
    
    // Register server-integrated functions if enabled
    if (include_server_functions && server_functions_enabled) {
        register_server_functions(manager);
    }
    
    // Also register the ToolDiscoveryFunction which needs the manager
    manager->register_function(std::make_unique<ToolDiscoveryFunction>(manager));
    
    ServerLogger::logInfo("EnhancedFunctionRegistry: Registered all functions (server functions: {})", 
                      include_server_functions && server_functions_enabled ? "enabled" : "disabled");
}

void EnhancedFunctionRegistry::set_server_url(const std::string& url) {
    server_url = url;
    server_registry->set_server_url(url);
    ServerLogger::logInfo("EnhancedFunctionRegistry: Updated server URL to '{}'", url);
}

std::vector<std::string> EnhancedFunctionRegistry::get_available_functions(bool include_server_functions) const {
    std::vector<std::string> functions;
    
    // Built-in functions
    std::vector<std::string> builtin_functions = {
        "add", "echo", "delay", "text_analysis", "text_processing", 
        "data_transform", "data_analysis", "inference", "retrieval", 
        "context_retrieval", "web_search", "code_generation", "add_document", 
        "remove_document", "parse_pdf", "parse_docx", "get_embedding", 
        "test_document_service", "list_tools"
    };
    
    functions.insert(functions.end(), builtin_functions.begin(), builtin_functions.end());
    
    // Server-integrated functions
    if (include_server_functions && server_functions_enabled) {
        auto server_functions = server_registry->list_available_functions();
        functions.insert(functions.end(), server_functions.begin(), server_functions.end());
    }
    
    return functions;
}

bool EnhancedFunctionRegistry::test_server_connection() {
    try {
        HttpClient client;
        std::string response;
        
        // Try to connect to the server health endpoint
        std::string health_url = server_url + "/health";
        bool success = client.get(health_url, response);
        
        if (success) {
            ServerLogger::logInfo("EnhancedFunctionRegistry: Server connection test successful");
            return true;
        } else {
                        ServerLogger::logWarning("EnhancedFunctionRegistry: Server connection test failed - no response");
            return false;
        }
    } catch (const std::exception& e) {
        ServerLogger::logError("EnhancedFunctionRegistry: Server connection test failed - {}", e.what());
        return false;
    }
}

// FunctionRegistryUtils Implementation
namespace FunctionRegistryUtils {

void register_web_search_functions(std::shared_ptr<FunctionManager> manager,
                                  const std::string& server_url,
                                  bool enable_real_search) {
    if (!manager) {
        ServerLogger::logError("FunctionRegistryUtils: Cannot register web search functions with null manager");
        return;
    }
    
    if (enable_real_search) {
        // Register real internet search functions
        manager->register_function(std::make_unique<InternetSearchFunction>(server_url));
        manager->register_function(std::make_unique<EnhancedWebSearchFunction>(server_url));
        ServerLogger::logInfo("FunctionRegistryUtils: Registered real web search functions");
    } else {
        // Register simulation function only
        manager->register_function(std::make_unique<WebSearchFunction>());
        ServerLogger::logInfo("FunctionRegistryUtils: Registered simulated web search function");
    }
}

void register_document_functions(std::shared_ptr<FunctionManager> manager,
                                const std::string& server_url,
                                const std::string& collection_name) {
    if (!manager) {
        ServerLogger::logError("FunctionRegistryUtils: Cannot register document functions with null manager");
        return;
    }
    
    // Register server-integrated document functions
    manager->register_function(std::make_unique<ServerDocumentRetrievalFunction>(server_url, collection_name));
    manager->register_function(std::make_unique<ServerDocumentAddFunction>(server_url, collection_name));
    manager->register_function(std::make_unique<ServerDocumentParserFunction>(server_url));
    manager->register_function(std::make_unique<ServerEmbeddingFunction>(server_url));
    
    // Also register built-in document functions for backward compatibility
    manager->register_function(std::make_unique<AddDocumentFunction>());
    manager->register_function(std::make_unique<RemoveDocumentFunction>());
    manager->register_function(std::make_unique<ParsePdfFunction>());
    manager->register_function(std::make_unique<ParseDocxFunction>());
    
    ServerLogger::logInfo("FunctionRegistryUtils: Registered document functions with collection '{}'", collection_name);
}

void register_knowledge_functions(std::shared_ptr<FunctionManager> manager,
                                 const std::string& server_url) {
    if (!manager) {
        ServerLogger::logError("FunctionRegistryUtils: Cannot register knowledge functions with null manager");
        return;
    }
    
    // Register comprehensive knowledge retrieval function
    manager->register_function(std::make_unique<KnowledgeRetrievalFunction>(server_url));
    
    // Register both types of search and retrieval for flexibility
    manager->register_function(std::make_unique<InternetSearchFunction>(server_url));
    manager->register_function(std::make_unique<ServerDocumentRetrievalFunction>(server_url));
    
    ServerLogger::logInfo("FunctionRegistryUtils: Registered hybrid knowledge functions");
}

std::vector<std::string> get_recommended_functions_for_role(const std::string& role) {
    std::vector<std::string> recommended;
    
    // Base functions for all roles
    std::vector<std::string> base_functions = {
        "echo", "text_analysis", "list_tools"
    };
    recommended.insert(recommended.end(), base_functions.begin(), base_functions.end());
    
    if (role == "researcher" || role == "analyst") {
        std::vector<std::string> research_functions = {
            "internet_search", "enhanced_web_search", "server_document_retrieval", 
            "knowledge_retrieval", "parse_pdf", "parse_docx", "context_retrieval"
        };
        recommended.insert(recommended.end(), research_functions.begin(), research_functions.end());
        
    } else if (role == "developer" || role == "programmer") {
        std::vector<std::string> dev_functions = {
            "code_generation", "internet_search", "server_document_retrieval", 
            "text_processing", "data_analysis"
        };
        recommended.insert(recommended.end(), dev_functions.begin(), dev_functions.end());
        
    } else if (role == "assistant" || role == "general") {
        std::vector<std::string> assistant_functions = {
            "internet_search", "knowledge_retrieval", "text_processing", 
            "data_transform", "inference", "add_document"
        };
        recommended.insert(recommended.end(), assistant_functions.begin(), assistant_functions.end());
        
    } else if (role == "specialist" || role == "expert") {
        std::vector<std::string> specialist_functions = {
            "enhanced_web_search", "server_document_retrieval", "knowledge_retrieval",
            "server_generate_embedding", "inference", "data_analysis"
        };
        recommended.insert(recommended.end(), specialist_functions.begin(), specialist_functions.end());
        
    } else {
        // Default role - include basic set
        std::vector<std::string> default_functions = {
            "internet_search", "server_document_retrieval", "text_processing", "inference"
        };
        recommended.insert(recommended.end(), default_functions.begin(), default_functions.end());
    }
    
    return recommended;
}

} // namespace FunctionRegistryUtils

} // namespace kolosal::agents
