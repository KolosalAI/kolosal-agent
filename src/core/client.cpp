#include "client.hpp"
#include "http_client.hpp"
#include "logger.hpp"
#include <stdexcept>
#include <chrono>
#include <thread>
#include <sstream>
#include <algorithm>

KolosalClient::KolosalClient(const Config& config) : config_(config) {
    TRACE_FUNCTION();
    
    // Validate configuration
    if (config_.server_url.empty()) {
        LOG_ERROR("Server URL is empty - must be configured in agent.yaml");
        throw std::runtime_error("Server URL not configured - check ./configs/agent.yaml");
    }
    
    LOG_INFO_F("KolosalClient initialized with server URL: %s", config_.server_url.c_str());
    
    // Initialize HTTP client with configuration
    HttpClient::Config http_config;
    http_config.base_url = config_.server_url;
    http_config.timeout_seconds = config_.timeout_seconds;
    http_config.max_retries = config_.max_retries;
    http_config.retry_delay_ms = config_.retry_delay_ms;
    http_config.verify_ssl = config_.verify_ssl;
    
    http_client_ = std::make_unique<HttpClient>(http_config);
}

KolosalClient::~KolosalClient() {
    TRACE_FUNCTION();
}

bool KolosalClient::is_model_available(const std::string& model_name) {
    TRACE_FUNCTION();
    
    try {
        auto models = get_available_models();
        
        if (models.is_array()) {
            for (const auto& model : models) {
                if (model.contains("model_id") && model["model_id"] == model_name) {
                    return true;
                }
                if (model.contains("id") && model["id"] == model_name) {
                    return true;
                }
                if (model.contains("name") && model["name"] == model_name) {
                    return true;
                }
            }
        }
        
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to check model availability: %s", e.what());
        return false;
    }
}

json KolosalClient::get_available_models() {
    TRACE_FUNCTION();
    SCOPED_TIMER("get_available_models");
    
    try {
        return make_request_with_retry("GET", "/models");
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to get available models: %s", e.what());
        return json::array();
    }
}

std::string KolosalClient::chat_with_model(const std::string& model_name, 
                                          const std::string& message, 
                                          const std::string& system_prompt) {
    TRACE_FUNCTION();
    SCOPED_TIMER("chat_with_model");
    
    try {
        json request_data;
        request_data["model"] = model_name;
        request_data["messages"] = json::array();
        
        if (!system_prompt.empty()) {
            json system_msg;
            system_msg["role"] = "system";
            system_msg["content"] = system_prompt;
            request_data["messages"].push_back(system_msg);
        }
        
        json user_msg;
        user_msg["role"] = "user";
        user_msg["content"] = message;
        request_data["messages"].push_back(user_msg);
        
        auto response = make_request_with_retry("POST", "/chat/completions", request_data);
        
        // Parse OpenAI-compatible response
        if (response.contains("choices") && response["choices"].is_array() && !response["choices"].empty()) {
            const auto& first_choice = response["choices"][0];
            if (first_choice.contains("message") && first_choice["message"].contains("content")) {
                return first_choice["message"]["content"].get<std::string>();
            }
        }
        
        // Fallback: check if response has direct content
        if (response.contains("content")) {
            return response["content"].get<std::string>();
        }
        
        LOG_WARN("Unexpected response format from chat endpoint");
        return "Response received but in unexpected format";
        
    } catch (const std::exception& e) {
        LOG_ERROR_F("Chat request failed: %s", e.what());
        throw std::runtime_error("Failed to communicate with model: " + std::string(e.what()));
    }
}

json KolosalClient::completion_request(const std::string& model_name, 
                                      const std::string& prompt, 
                                      const json& params) {
    TRACE_FUNCTION();
    SCOPED_TIMER("completion_request");
    
    try {
        json request_data = params;  // Start with provided parameters
        request_data["model"] = model_name;
        request_data["prompt"] = prompt;
        
        return make_request_with_retry("POST", "/completions", request_data);
        
    } catch (const std::exception& e) {
        LOG_ERROR_F("Completion request failed: %s", e.what());
        throw std::runtime_error("Failed to get completion from model: " + std::string(e.what()));
    }
}

json KolosalClient::add_document(const json& document_data) {
    TRACE_FUNCTION();
    SCOPED_TIMER("add_document");
    
    try {
        // Wrap the document in the format expected by /add_documents endpoint
        json request_body;
        request_body["documents"] = json::array({document_data});
        
        return make_request_with_retry("POST", "/add_documents", request_body);
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to add document: %s", e.what());
        throw std::runtime_error("Failed to add document: " + std::string(e.what()));
    }
}

json KolosalClient::search_documents(const std::string& query, 
                                    int limit, 
                                    const json& filters) {
    TRACE_FUNCTION();
    SCOPED_TIMER("search_documents");
    
    try {
        json request_data;
        request_data["query"] = query;
        request_data["k"] = limit;  // Server expects 'k', not 'limit'
        if (!filters.empty()) {
            request_data["filters"] = filters;
        }
        
        return make_request_with_retry("POST", "/retrieve", request_data);
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to search documents: %s", e.what());
        throw std::runtime_error("Failed to search documents: " + std::string(e.what()));
    }
}

json KolosalClient::remove_document(const std::string& document_id) {
    TRACE_FUNCTION();
    SCOPED_TIMER("remove_document");
    
    try {
        // Wrap the document ID in the format expected by /remove_documents endpoint
        json request_body;
        request_body["ids"] = json::array({document_id});
        
        return make_request_with_retry("POST", "/remove_documents", request_body);
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to remove document: %s", e.what());
        throw std::runtime_error("Failed to remove document: " + std::string(e.what()));
    }
}

json KolosalClient::list_documents(int offset, int limit) {
    TRACE_FUNCTION();
    SCOPED_TIMER("list_documents");
    
    try {
        // The /list_documents endpoint doesn't take parameters, just returns all document IDs
        return make_request_with_retry("GET", "/list_documents");
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to list documents: %s", e.what());
        throw std::runtime_error("Failed to list documents: " + std::string(e.what()));
    }
}

json KolosalClient::internet_search(const std::string& query, int num_results) {
    TRACE_FUNCTION();
    SCOPED_TIMER("internet_search");
    
    try {
        json request_data;
        request_data["query"] = query;
        request_data["num_results"] = num_results;
        
        return make_request_with_retry("POST", "/search", request_data);
    } catch (const std::exception& e) {
        std::string error_msg = e.what();
        
        // Check if this is a 404 error indicating search functionality is not available
        if (error_msg.find("HTTP error 404") != std::string::npos || 
            error_msg.find("Not found") != std::string::npos) {
            LOG_WARN_F("Internet search endpoint not available on server: %s", error_msg.c_str());
            
            // Return a mock response indicating search is not available
            json mock_response;
            mock_response["status"] = "search_not_available";
            mock_response["message"] = "Internet search functionality is not available on this server";
            mock_response["query"] = query;
            mock_response["results"] = json::array();
            mock_response["suggestion"] = "Please enable the internet search feature on the Kolosal server or use alternative research methods";
            
            return mock_response;
        }
        
        LOG_ERROR_F("Failed to perform internet search: %s", error_msg.c_str());
        throw std::runtime_error("Failed to perform internet search: " + error_msg);
    }
}

bool KolosalClient::is_server_healthy() {
    TRACE_FUNCTION();
    
    try {
        auto response = make_request("GET", "/health");
        return response.contains("status") && 
               (response["status"] == "ok" || response["status"] == "healthy");
    } catch (const std::exception& e) {
        LOG_DEBUG_F("Server health check failed: %s", e.what());
        return false;
    }
}

json KolosalClient::get_server_status() {
    TRACE_FUNCTION();
    
    try {
        return make_request_with_retry("GET", "/status");
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to get server status: %s", e.what());
        throw std::runtime_error("Failed to get server status: " + std::string(e.what()));
    }
}

json KolosalClient::get_server_config() {
    TRACE_FUNCTION();
    
    try {
        return make_request_with_retry("GET", "/config");
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to get server config: %s", e.what());
        throw std::runtime_error("Failed to get server config: " + std::string(e.what()));
    }
}

void KolosalClient::update_config(const Config& new_config) {
    TRACE_FUNCTION();
    
    config_ = new_config;
    
    // Update HTTP client configuration
    if (http_client_) {
        HttpClient::Config http_config;
        http_config.base_url = config_.server_url;
        http_config.timeout_seconds = config_.timeout_seconds;
        http_config.max_retries = config_.max_retries;
        http_config.retry_delay_ms = config_.retry_delay_ms;
        http_config.verify_ssl = config_.verify_ssl;
        
        http_client_->update_config(http_config);
    }
    
    LOG_INFO_F("KolosalClient configuration updated, server URL: %s", config_.server_url.c_str());
}

json KolosalClient::make_request(const std::string& method,
                               const std::string& endpoint,
                               const json& data,
                               const json& headers) {
    TRACE_FUNCTION();
    
    if (!http_client_) {
        throw std::runtime_error("HTTP client not initialized");
    }
    
    // Convert headers to map
    std::map<std::string, std::string> header_map;
    if (!headers.empty() && headers.is_object()) {
        for (auto& [key, value] : headers.items()) {
            if (value.is_string()) {
                header_map[key] = value.get<std::string>();
            }
        }
    }
    
    // Convert data to string
    std::string body;
    if (!data.empty()) {
        body = data.dump();
    }
    
    // Make request using safe HTTP client
    auto result = http_client_->request(method, endpoint, body, header_map);
    
    if (!result.is_success()) {
        throw std::runtime_error(result.error_message);
    }
    
    // Parse response
    if (result.body.empty()) {
        return json::object();
    }
    
    try {
        return json::parse(result.body);
    } catch (const json::parse_error& e) {
        LOG_ERROR_F("Failed to parse JSON response: %s", e.what());
        throw std::runtime_error("Invalid JSON response from server");
    }
}

json KolosalClient::make_request_with_retry(const std::string& method,
                                           const std::string& endpoint,
                                           const json& data,
                                           const json& headers) {
    TRACE_FUNCTION();
    
    // Use the safe HTTP client's retry mechanism
    return make_request(method, endpoint, data, headers);
}

json KolosalClient::parse_response(const std::string& response_body, long status_code) {
    // This method is no longer needed as HttpClient handles response parsing
    // Kept for compatibility but should not be called
    throw std::runtime_error("parse_response should not be called directly");
}

std::string KolosalClient::build_url(const std::string& endpoint) const {
    // This method is no longer needed as HttpClient handles URL building
    // Kept for compatibility but should not be called
    throw std::runtime_error("build_url should not be called directly");
}
