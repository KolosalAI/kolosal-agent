/**
 * @file kolosal_server_functions.cpp
 * @brief Implementation of Kolosal Server integrated functions
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for functions that integrate with kolosal-server endpoints.
 */

#include "tools/kolosal_server_functions.hpp"
#include "logger/kolosal_logger.hpp"
#include "kolosal/logger.hpp"
#include <json.hpp>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <regex>

using json = nlohmann::json;
using namespace kolosal::agents;

namespace kolosal::agents {

// InternetSearchFunction Implementation
InternetSearchFunction::InternetSearchFunction(const std::string& server_endpoint, int timeout_seconds) 
    : server_url(server_endpoint), default_timeout(timeout_seconds) {
    http_client = std::make_shared<HttpClient>();
}

FunctionResult InternetSearchFunction::execute(const AgentData& params) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        std::string query = params.get_string("query");
        if (query.empty()) {
            return FunctionResult(false, "Query parameter is required for internet search");
        }
        
        // Extract parameters with defaults
        std::string engines = params.get_string("engines", "");
        std::string categories = params.get_string("categories", "general");
        std::string language = params.get_string("language", "en");
        std::string format = params.get_string("format", "json");
        int results = params.get_int("results", 10);
        bool safe_search = params.get_bool("safe_search", true);
        int timeout = params.get_int("timeout", default_timeout);
        
        // Build the search URL
        std::string search_url = server_url + "/internet_search";
        
        // Create JSON request body
        json request_json = {
            {"query", query},
            {"format", format},
            {"results", results},
            {"safe_search", safe_search},
            {"timeout", timeout},
            {"language", language},
            {"categories", categories}
        };
        
        if (!engines.empty()) {
            request_json["engines"] = engines;
        }
        
        std::string request_body = request_json.dump();
        std::string response_body;
        
        // Set headers
        std::vector<std::string> headers = {
            "Content-Type: application/json",
            "Accept: application/json"
        };
        
        // Make HTTP POST request
        bool success = http_client->post(search_url, request_body, response_body, headers);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        if (!success) {
            FunctionResult result(false, "Failed to connect to internet search service");
            result.execution_time_ms = duration.count() / 1000.0;
            return result;
        }
        
        // Parse the response
        json response_json;
        try {
            response_json = json::parse(response_body);
        } catch (const json::parse_error& e) {
            FunctionResult result(false, "Failed to parse search response: " + std::string(e.what()));
            result.execution_time_ms = duration.count() / 1000.0;
            return result;
        }
        
        // Check if the search was successful
        if (response_json.contains("error")) {
            std::string error_msg = response_json["error"].get<std::string>();
            FunctionResult result(false, "Search error: " + error_msg);
            result.execution_time_ms = duration.count() / 1000.0;
            return result;
        }
        
        // Extract search results
        FunctionResult result(true);
        result.result_data.set("query", query);
        result.result_data.set("search_type", "internet");
        result.result_data.set("engine_used", engines.empty() ? "default" : engines);
        
        if (response_json.contains("results")) {
            auto results_array = response_json["results"];
            std::vector<std::string> titles, urls, snippets;
            
            for (const auto& item : results_array) {
                if (item.contains("title")) {
                    titles.push_back(item["title"].get<std::string>());
                }
                if (item.contains("url")) {
                    urls.push_back(item["url"].get<std::string>());
                }
                if (item.contains("content") || item.contains("snippet")) {
                    std::string content = item.contains("content") ? 
                        item["content"].get<std::string>() : 
                        item["snippet"].get<std::string>();
                    snippets.push_back(content);
                }
            }
            
            result.result_data.set("results_count", static_cast<int>(titles.size()));
            result.result_data.set("titles", titles);
            result.result_data.set("urls", urls);
            result.result_data.set("snippets", snippets);
            
            // Create formatted output
            std::ostringstream formatted;
            formatted << "Internet Search Results for: " << query << "\\n\\n";
            for (size_t i = 0; i < titles.size(); ++i) {
                formatted << (i + 1) << ". " << titles[i] << "\\n";
                if (i < urls.size()) {
                    formatted << "   URL: " << urls[i] << "\\n";
                }
                if (i < snippets.size()) {
                    formatted << "   Snippet: " << snippets[i] << "\\n";
                }
                formatted << "\\n";
            }
            
            result.result_data.set("formatted_results", formatted.str());
            result.result_data.set("result", "Found " + std::to_string(titles.size()) + " search results for: " + query);
        } else {
            result.result_data.set("results_count", 0);
            result.result_data.set("result", "No search results found for: " + query);
        }
        
        result.execution_time_ms = duration.count() / 1000.0;
        
        ServerLogger::logInfo("InternetSearchFunction: Search for '{}' returned {} results", 
                          query, result.result_data.get_int("results_count", 0));
        
        return result;
        
    } catch (const std::exception& e) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        FunctionResult result(false, "Internet search error: " + std::string(e.what()));
        result.execution_time_ms = duration.count() / 1000.0;
        
        return result;
    }
}

// EnhancedWebSearchFunction Implementation
EnhancedWebSearchFunction::EnhancedWebSearchFunction(const std::string& server_endpoint) 
    : enable_content_filtering(true), enable_summary_extraction(false) {
    base_search = std::make_shared<InternetSearchFunction>(server_endpoint);
}

FunctionResult EnhancedWebSearchFunction::execute(const AgentData& params) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // First, perform the basic search
        FunctionResult basic_result = base_search->execute(params);
        
        if (!basic_result.success) {
            return basic_result;
        }
        
        // Apply enhancements
        FunctionResult enhanced_result(true);
        
        // Copy basic results
        enhanced_result.result_data = basic_result.result_data;
        enhanced_result.result_data.set("search_type", "enhanced_internet");
        
        // Apply content filtering if enabled
        if (enable_content_filtering) {
            std::vector<std::string> snippets = basic_result.result_data.get_array_string("snippets");
            std::vector<std::string> filtered_snippets;
            
            for (const auto& snippet : snippets) {
                // Simple content filtering - remove very short snippets
                if (snippet.length() >= 50) {
                    // Remove common unwanted patterns
                    std::string filtered = snippet;
                    
                    // Remove excessive whitespace
                    filtered = std::regex_replace(filtered, std::regex("\\s+"), " ");
                    
                    // Trim
                    filtered.erase(0, filtered.find_first_not_of(" \\t\\n\\r"));
                    filtered.erase(filtered.find_last_not_of(" \\t\\n\\r") + 1);
                    
                    filtered_snippets.push_back(filtered);
                }
            }
            
            enhanced_result.result_data.set("filtered_snippets", filtered_snippets);
            enhanced_result.result_data.set("filtering_applied", true);
        }
        
        // Apply summary extraction if enabled
        if (enable_summary_extraction) {
            std::vector<std::string> snippets = basic_result.result_data.get_array_string("snippets");
            std::vector<std::string> summaries;
            
            for (const auto& snippet : snippets) {
                // Simple extractive summarization - take first sentence
                std::string summary = snippet.substr(0, std::min<size_t>(200, snippet.length()));
                size_t period_pos = summary.find_first_of(".!?");
                if (period_pos != std::string::npos && period_pos > 20) {
                    summary = summary.substr(0, period_pos + 1);
                }
                summaries.push_back(summary);
            }
            
            enhanced_result.result_data.set("summaries", summaries);
            enhanced_result.result_data.set("summary_extraction_applied", true);
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        enhanced_result.execution_time_ms = duration.count() / 1000.0;
        
        return enhanced_result;
        
    } catch (const std::exception& e) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        FunctionResult result(false, "Enhanced search error: " + std::string(e.what()));
        result.execution_time_ms = duration.count() / 1000.0;
        
        return result;
    }
}

// ServerDocumentRetrievalFunction Implementation
ServerDocumentRetrievalFunction::ServerDocumentRetrievalFunction(const std::string& server_endpoint, 
                                                                 const std::string& collection)
    : server_url(server_endpoint), collection_name(collection), default_limit(10) {
    http_client = std::make_shared<HttpClient>();
}

FunctionResult ServerDocumentRetrievalFunction::execute(const AgentData& params) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        std::string query = params.get_string("query");
        if (query.empty()) {
            return FunctionResult(false, "Query parameter is required for document retrieval");
        }
        
        int limit = params.get_int("limit", default_limit);
        std::string collection = params.get_string("collection", collection_name);
        double threshold = params.get_double("threshold", 0.7);
        
        // Build the retrieval URL
        std::string retrieval_url = server_url + "/retrieve";
        
        // Create JSON request body
        json request_json = {
            {"query", query},
            {"limit", limit},
            {"collection", collection},
            {"threshold", threshold}
        };
        
        std::string request_body = request_json.dump();
        std::string response_body;
        
        // Set headers
        std::vector<std::string> headers = {
            "Content-Type: application/json",
            "Accept: application/json"
        };
        
        // Make HTTP POST request
        bool success = http_client->post(retrieval_url, request_body, response_body, headers);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        if (!success) {
            FunctionResult result(false, "Failed to connect to document retrieval service");
            result.execution_time_ms = duration.count() / 1000.0;
            return result;
        }
        
        // Parse the response
        json response_json;
        try {
            response_json = json::parse(response_body);
        } catch (const json::parse_error& e) {
            FunctionResult result(false, "Failed to parse retrieval response: " + std::string(e.what()));
            result.execution_time_ms = duration.count() / 1000.0;
            return result;
        }
        
        // Check if the retrieval was successful
        if (response_json.contains("error")) {
            std::string error_msg = response_json["error"].get<std::string>();
            FunctionResult result(false, "Retrieval error: " + error_msg);
            result.execution_time_ms = duration.count() / 1000.0;
            return result;
        }
        
        // Extract retrieval results
        FunctionResult result(true);
        result.result_data.set("query", query);
        result.result_data.set("collection", collection);
        result.result_data.set("retrieval_type", "server_document");
        
        if (response_json.contains("documents")) {
            auto docs_array = response_json["documents"];
            std::vector<std::string> contents, ids, sources;
            std::vector<double> scores;
            
            for (const auto& doc : docs_array) {
                if (doc.contains("content")) {
                    contents.push_back(doc["content"].get<std::string>());
                }
                if (doc.contains("id")) {
                    ids.push_back(doc["id"].get<std::string>());
                }
                if (doc.contains("source") || doc.contains("metadata")) {
                    std::string source = doc.contains("source") ? 
                        doc["source"].get<std::string>() : 
                        (doc["metadata"].contains("source") ? 
                         doc["metadata"]["source"].get<std::string>() : "unknown");
                    sources.push_back(source);
                }
                if (doc.contains("score")) {
                    scores.push_back(doc["score"].get<double>());
                }
            }
            
            result.result_data.set("documents_count", static_cast<int>(contents.size()));
            result.result_data.set("contents", contents);
            result.result_data.set("document_ids", ids);
            result.result_data.set("sources", sources);
            
            // Create formatted output
            std::ostringstream formatted;
            formatted << "Document Retrieval Results for: " << query << "\\n\\n";
            for (size_t i = 0; i < contents.size(); ++i) {
                formatted << (i + 1) << ". ";
                if (i < sources.size()) {
                    formatted << "Source: " << sources[i] << "\\n";
                }
                if (i < scores.size()) {
                    formatted << "   Score: " << std::fixed << std::setprecision(3) << scores[i] << "\\n";
                }
                formatted << "   Content: " << contents[i].substr(0, 300);
                if (contents[i].length() > 300) formatted << "...";
                formatted << "\\n\\n";
            }
            
            result.result_data.set("formatted_results", formatted.str());
            result.result_data.set("result", "Retrieved " + std::to_string(contents.size()) + " documents for: " + query);
        } else {
            result.result_data.set("documents_count", 0);
            result.result_data.set("result", "No documents found for: " + query);
        }
        
        result.execution_time_ms = duration.count() / 1000.0;
        
        ServerLogger::logInfo("ServerDocumentRetrievalFunction: Retrieved {} documents for '{}'", 
                          result.result_data.get_int("documents_count", 0), query);
        
        return result;
        
    } catch (const std::exception& e) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        FunctionResult result(false, "Document retrieval error: " + std::string(e.what()));
        result.execution_time_ms = duration.count() / 1000.0;
        
        return result;
    }
}

// KnowledgeRetrievalFunction Implementation
KnowledgeRetrievalFunction::KnowledgeRetrievalFunction(const std::string& server_endpoint)
    : prefer_local_first(true), relevance_threshold(0.7) {
    web_search = std::make_shared<InternetSearchFunction>(server_endpoint);
    doc_retrieval = std::make_shared<ServerDocumentRetrievalFunction>(server_endpoint);
}

FunctionResult KnowledgeRetrievalFunction::execute(const AgentData& params) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        std::string query = params.get_string("query");
        if (query.empty()) {
            return FunctionResult(false, "Query parameter is required for knowledge retrieval");
        }
        
        bool web_only = params.get_bool("web_only", false);
        bool local_only = params.get_bool("local_only", false);
        int max_results = params.get_int("max_results", 15);
        
        FunctionResult combined_result(true);
        combined_result.result_data.set("query", query);
        combined_result.result_data.set("retrieval_type", "hybrid_knowledge");
        
        std::vector<std::string> all_contents, all_sources, all_types;
        int total_results = 0;
        
        // Local document retrieval first (if not web_only)
        if (!web_only) {
            AgentData local_params = params;
            local_params.set("limit", max_results / 2);
            
            FunctionResult local_result = doc_retrieval->execute(local_params);
            
            if (local_result.success && local_result.result_data.get_int("documents_count", 0) > 0) {
                auto local_contents = local_result.result_data.get_array_string("contents");
                auto local_sources = local_result.result_data.get_array_string("sources");
                
                for (size_t i = 0; i < local_contents.size(); ++i) {
                    all_contents.push_back(local_contents[i]);
                    all_sources.push_back(i < local_sources.size() ? local_sources[i] : "local_document");
                    all_types.push_back("document");
                    total_results++;
                }
                
                combined_result.result_data.set("local_documents_found", static_cast<int>(local_contents.size()));
            }
        }
        
        // Web search (if not local_only and we need more results)
        if (!local_only && total_results < max_results) {
            AgentData web_params = params;
            web_params.set("results", max_results - total_results);
            
            FunctionResult web_result = web_search->execute(web_params);
            
            if (web_result.success && web_result.result_data.get_int("results_count", 0) > 0) {
                auto web_snippets = web_result.result_data.get_array_string("snippets");
                auto web_urls = web_result.result_data.get_array_string("urls");
                
                for (size_t i = 0; i < web_snippets.size() && total_results < max_results; ++i) {
                    all_contents.push_back(web_snippets[i]);
                    all_sources.push_back(i < web_urls.size() ? web_urls[i] : "web_search");
                    all_types.push_back("web");
                    total_results++;
                }
                
                combined_result.result_data.set("web_results_found", static_cast<int>(web_snippets.size()));
            }
        }
        
        // Combine results
        combined_result.result_data.set("total_results", total_results);
        combined_result.result_data.set("contents", all_contents);
        combined_result.result_data.set("sources", all_sources);
        combined_result.result_data.set("types", all_types);
        
        // Create formatted output
        std::ostringstream formatted;
        formatted << "Hybrid Knowledge Retrieval Results for: " << query << "\\n\\n";
        for (int i = 0; i < total_results; ++i) {
            formatted << (i + 1) << ". [" << all_types[i] << "] ";
            formatted << all_sources[i] << "\\n";
            formatted << "   " << all_contents[i].substr(0, 250);
            if (all_contents[i].length() > 250) formatted << "...";
            formatted << "\\n\\n";
        }
        
        combined_result.result_data.set("formatted_results", formatted.str());
        combined_result.result_data.set("result", 
            "Retrieved " + std::to_string(total_results) + " knowledge sources for: " + query);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        combined_result.execution_time_ms = duration.count() / 1000.0;
        
        ServerLogger::logInfo("KnowledgeRetrievalFunction: Retrieved {} total results for '{}'", 
                          total_results, query);
        
        return combined_result;
        
    } catch (const std::exception& e) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        FunctionResult result(false, "Hybrid knowledge retrieval error: " + std::string(e.what()));
        result.execution_time_ms = duration.count() / 1000.0;
        
        return result;
    }
}

// ServerDocumentAddFunction Implementation
ServerDocumentAddFunction::ServerDocumentAddFunction(const std::string& server_endpoint,
                                                   const std::string& collection_name) 
    : server_url(server_endpoint), collection_name(collection_name) {
    http_client = std::make_shared<HttpClient>();
}

FunctionResult ServerDocumentAddFunction::execute(const AgentData& parameters) {
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Extract parameters
        std::string content = parameters.get_string("content", "");
        std::string metadata = parameters.get_string("metadata", "{}");
        
        if (content.empty()) {
            return FunctionResult(false, "Content parameter is required");
        }
        
        // For now, return a placeholder implementation
        FunctionResult result(true, "Document add functionality not yet implemented");
        result.result_data.set("status", "placeholder");
        result.result_data.set("message", "ServerDocumentAddFunction constructor created successfully");
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        result.execution_time_ms = duration.count() / 1000.0;
        
        return result;
        
    } catch (const std::exception& e) {
        return FunctionResult(false, "Document add error: " + std::string(e.what()));
    }
}

// ServerDocumentParserFunction Implementation  
ServerDocumentParserFunction::ServerDocumentParserFunction(const std::string& server_endpoint)
    : server_url(server_endpoint) {
    http_client = std::make_shared<HttpClient>();
}

FunctionResult ServerDocumentParserFunction::execute(const AgentData& parameters) {
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Extract parameters  
        std::string file_path = parameters.get_string("file_path", "");
        std::string parser_type = parameters.get_string("parser_type", "auto");
        
        if (file_path.empty()) {
            return FunctionResult(false, "file_path parameter is required");
        }
        
        // For now, return a placeholder implementation
        FunctionResult result(true, "Document parser functionality not yet implemented");
        result.result_data.set("status", "placeholder");
        result.result_data.set("message", "ServerDocumentParserFunction constructor created successfully");
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        result.execution_time_ms = duration.count() / 1000.0;
        
        return result;
        
    } catch (const std::exception& e) {
        return FunctionResult(false, "Document parser error: " + std::string(e.what()));
    }
}

// ServerEmbeddingFunction Implementation
ServerEmbeddingFunction::ServerEmbeddingFunction(const std::string& server_endpoint,
                                                const std::string& model_name)
    : server_url(server_endpoint), model_name(model_name) {
    http_client = std::make_shared<HttpClient>();
}

FunctionResult ServerEmbeddingFunction::execute(const AgentData& parameters) {
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Extract parameters
        std::string text = parameters.get_string("text", "");
        int batch_size = parameters.get_int("batch_size", 1);
        
        if (text.empty()) {
            return FunctionResult(false, "text parameter is required");
        }
        
        // For now, return a placeholder implementation
        FunctionResult result(true, "Server embedding functionality not yet implemented");
        result.result_data.set("status", "placeholder");
        result.result_data.set("message", "ServerEmbeddingFunction constructor created successfully");
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        result.execution_time_ms = duration.count() / 1000.0;
        
        return result;
        
    } catch (const std::exception& e) {
        return FunctionResult(false, "Server embedding error: " + std::string(e.what()));
    }
}

// KolosalServerFunctionRegistry Implementation
KolosalServerFunctionRegistry::KolosalServerFunctionRegistry(const std::string& server_endpoint) 
    : server_url(server_endpoint) {
    initialize_all_functions();
}

void KolosalServerFunctionRegistry::initialize_all_functions() {
    // Register all kolosal-server integrated functions
    register_function(std::make_unique<InternetSearchFunction>(server_url));
    register_function(std::make_unique<EnhancedWebSearchFunction>(server_url));
    register_function(std::make_unique<ServerDocumentRetrievalFunction>(server_url));
    register_function(std::make_unique<KnowledgeRetrievalFunction>(server_url));
    
    ServerLogger::logInfo("KolosalServerFunctionRegistry: Initialized {} functions", functions.size());
}

void KolosalServerFunctionRegistry::register_function(std::unique_ptr<AgentFunction> function) {
    if (function) {
        std::string name = function->get__name();
        functions[name] = std::move(function);
        ServerLogger::logInfo("KolosalServerFunctionRegistry: Registered function '{}'", name);
    }
}

AgentFunction* KolosalServerFunctionRegistry::get_function(const std::string& name) {
    auto it = functions.find(name);
    return (it != functions.end()) ? it->second.get() : nullptr;
}

std::vector<std::string> KolosalServerFunctionRegistry::list_available_functions() const {
    std::vector<std::string> function_names;
    for (const auto& pair : functions) {
        function_names.push_back(pair.first);
    }
    return function_names;
}

void KolosalServerFunctionRegistry::set_server_url(const std::string& url) {
    server_url = url;
    // Reinitialize functions with new URL
    functions.clear();
    initialize_all_functions();
}

void KolosalServerFunctionRegistry::register_with_manager(std::shared_ptr<FunctionManager> manager) {
    if (!manager) {
        ServerLogger::logError("KolosalServerFunctionRegistry: Cannot register with null manager");
        return;
    }
    
    for (const auto& pair : functions) {
        // Create a copy of the function for the manager
        // Note: This is a simplified approach - in a full implementation,
        // you'd want proper cloning mechanisms
        if (pair.first == "internet_search") {
            manager->register_function(std::make_unique<InternetSearchFunction>(server_url));
        } else if (pair.first == "enhanced_web_search") {
            manager->register_function(std::make_unique<EnhancedWebSearchFunction>(server_url));
        } else if (pair.first == "server_document_retrieval") {
            manager->register_function(std::make_unique<ServerDocumentRetrievalFunction>(server_url));
        } else if (pair.first == "knowledge_retrieval") {
            manager->register_function(std::make_unique<KnowledgeRetrievalFunction>(server_url));
        }
    }
    
    ServerLogger::logInfo("KolosalServerFunctionRegistry: Registered {} functions with manager", functions.size());
}

} // namespace kolosal::agents
