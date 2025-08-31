#include "kolosal_client.hpp"
#include <stdexcept>
#include <chrono>
#include <thread>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
#include <curl/curl.h>
#endif

namespace {
    // Response callback for curl
    #ifndef _WIN32
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    #endif
}

KolosalClient::KolosalClient(const Config& config) : config_(config) {
    TRACE_FUNCTION();
    LOG_INFO_F("KolosalClient initialized with server URL: %s", config_.server_url.c_str());
    
    #ifndef _WIN32
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    #endif
}

KolosalClient::~KolosalClient() {
    TRACE_FUNCTION();
    
    #ifndef _WIN32
    // Cleanup libcurl
    curl_global_cleanup();
    #endif
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
        return make_request_with_retry("POST", "/documents", document_data);
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
        request_data["limit"] = limit;
        if (!filters.empty()) {
            request_data["filters"] = filters;
        }
        
        return make_request_with_retry("POST", "/documents/search", request_data);
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to search documents: %s", e.what());
        throw std::runtime_error("Failed to search documents: " + std::string(e.what()));
    }
}

json KolosalClient::remove_document(const std::string& document_id) {
    TRACE_FUNCTION();
    SCOPED_TIMER("remove_document");
    
    try {
        return make_request_with_retry("DELETE", "/documents/" + document_id);
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to remove document: %s", e.what());
        throw std::runtime_error("Failed to remove document: " + std::string(e.what()));
    }
}

json KolosalClient::list_documents(int offset, int limit) {
    TRACE_FUNCTION();
    SCOPED_TIMER("list_documents");
    
    try {
        std::string endpoint = "/documents?offset=" + std::to_string(offset) + "&limit=" + std::to_string(limit);
        return make_request_with_retry("GET", endpoint);
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
        LOG_ERROR_F("Failed to perform internet search: %s", e.what());
        throw std::runtime_error("Failed to perform internet search: " + std::string(e.what()));
    }
}

bool KolosalClient::is_server_healthy() {
    TRACE_FUNCTION();
    
    try {
        auto response = make_request("GET", "/health");
        return response.contains("status") && response["status"] == "ok";
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
    LOG_INFO_F("KolosalClient configuration updated, server URL: %s", config_.server_url.c_str());
}

json KolosalClient::make_request(const std::string& method,
                               const std::string& endpoint,
                               const json& data,
                               const json& headers) {
    TRACE_FUNCTION();
    
    std::string url = build_url(endpoint);
    std::string request_body;
    
    if (!data.empty() && (method == "POST" || method == "PUT" || method == "PATCH")) {
        request_body = data.dump();
    }
    
    LOG_DEBUG_F("Making %s request to: %s", method.c_str(), url.c_str());
    if (!request_body.empty()) {
        LOG_DEBUG_F("Request body: %s", request_body.c_str());
    }

#ifdef _WIN32
    // Windows implementation using WinHTTP
    HINTERNET hSession = WinHttpOpen(L"KolosalClient/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        throw std::runtime_error("Failed to initialize WinHTTP session");
    }

    // Parse URL
    std::wstring w_host;
    std::wstring w_path;
    INTERNET_PORT port;
    
    // Simple URL parsing - extract host, port, and path
    std::string url_without_protocol = url;
    if (url.find("http://") == 0) {
        url_without_protocol = url.substr(7);
    } else if (url.find("https://") == 0) {
        url_without_protocol = url.substr(8);
    }
    
    size_t slash_pos = url_without_protocol.find('/');
    std::string host_port = url_without_protocol.substr(0, slash_pos);
    std::string path = slash_pos != std::string::npos ? url_without_protocol.substr(slash_pos) : "/";
    
    size_t colon_pos = host_port.find(':');
    std::string host = colon_pos != std::string::npos ? host_port.substr(0, colon_pos) : host_port;
    port = colon_pos != std::string::npos ? static_cast<INTERNET_PORT>(std::stoi(host_port.substr(colon_pos + 1))) : 80;
    
    w_host = std::wstring(host.begin(), host.end());
    w_path = std::wstring(path.begin(), path.end());
    
    HINTERNET hConnect = WinHttpConnect(hSession, w_host.c_str(), port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("Failed to connect to server");
    }
    
    std::wstring w_method(method.begin(), method.end());
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, w_method.c_str(), w_path.c_str(),
                                           NULL, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("Failed to create HTTP request");
    }
    
    // Set timeout
    DWORD timeout = config_.timeout_seconds * 1000;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
    
    // Add headers
    std::wstring additional_headers = L"Content-Type: application/json\r\n";
    if (!headers.empty()) {
        for (auto& [key, value] : headers.items()) {
            additional_headers += std::wstring(key.begin(), key.end()) + L": " + 
                                std::wstring(value.get<std::string>().begin(), value.get<std::string>().end()) + L"\r\n";
        }
    }
    
    // Send request
    BOOL result = WinHttpSendRequest(hRequest, additional_headers.c_str(), -1,
                                    (LPVOID)request_body.c_str(), request_body.length(),
                                    request_body.length(), 0);
    
    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("Failed to send HTTP request");
    }
    
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        throw std::runtime_error("Failed to receive HTTP response");
    }
    
    // Get status code
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                       WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
    
    // Read response body
    std::string response_body;
    DWORD bytesAvailable, bytesRead;
    char buffer[4096];
    
    do {
        bytesAvailable = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable)) break;
        
        if (bytesAvailable > 0) {
            DWORD bytesToRead = std::min(bytesAvailable, (DWORD)sizeof(buffer));
            if (WinHttpReadData(hRequest, buffer, bytesToRead, &bytesRead)) {
                response_body.append(buffer, bytesRead);
            }
        }
    } while (bytesAvailable > 0);
    
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
    return parse_response(response_body, statusCode);

#else
    // Unix implementation using libcurl
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize libcurl");
    }
    
    std::string response_body;
    long response_code = 0;
    
    // Set basic options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config_.timeout_seconds);
    
    // Set method and body
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (!request_body.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
        }
    } else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (!request_body.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
        }
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (method == "PATCH") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        if (!request_body.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
        }
    }
    
    // Set headers
    struct curl_slist* header_list = NULL;
    header_list = curl_slist_append(header_list, "Content-Type: application/json");
    
    if (!headers.empty()) {
        for (auto& [key, value] : headers.items()) {
            std::string header = key + ": " + value.get<std::string>();
            header_list = curl_slist_append(header_list, header.c_str());
        }
    }
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    
    // Make request
    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("HTTP request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    return parse_response(response_body, response_code);
#endif
}

json KolosalClient::make_request_with_retry(const std::string& method,
                                           const std::string& endpoint,
                                           const json& data,
                                           const json& headers) {
    TRACE_FUNCTION();
    
    int attempts = 0;
    std::exception_ptr last_exception;
    
    while (attempts < config_.max_retries) {
        try {
            return make_request(method, endpoint, data, headers);
        } catch (const std::exception& e) {
            last_exception = std::current_exception();
            attempts++;
            
            LOG_WARN_F("Request failed (attempt %d/%d): %s", attempts, config_.max_retries, e.what());
            
            if (attempts < config_.max_retries) {
                LOG_DEBUG_F("Retrying in %d ms...", config_.retry_delay_ms);
                std::this_thread::sleep_for(std::chrono::milliseconds(config_.retry_delay_ms));
            }
        }
    }
    
    LOG_ERROR_F("Request failed after %d attempts", config_.max_retries);
    std::rethrow_exception(last_exception);
}

json KolosalClient::parse_response(const std::string& response_body, long status_code) {
    LOG_DEBUG_F("HTTP response: status=%ld, body_size=%zu", status_code, response_body.size());
    
    if (status_code < 200 || status_code >= 300) {
        std::string error_msg = "HTTP error " + std::to_string(status_code);
        if (!response_body.empty()) {
            try {
                auto error_json = json::parse(response_body);
                if (error_json.contains("error")) {
                    error_msg += ": " + error_json["error"].get<std::string>();
                }
            } catch (...) {
                error_msg += ": " + response_body;
            }
        }
        throw std::runtime_error(error_msg);
    }
    
    if (response_body.empty()) {
        return json::object();
    }
    
    try {
        return json::parse(response_body);
    } catch (const json::parse_error& e) {
        LOG_ERROR_F("Failed to parse JSON response: %s", e.what());
        LOG_DEBUG_F("Response body: %s", response_body.c_str());
        throw std::runtime_error("Invalid JSON response from server");
    }
}

std::string KolosalClient::build_url(const std::string& endpoint) const {
    std::string url = config_.server_url;
    if (url.back() == '/' && endpoint.front() == '/') {
        url.pop_back();
    } else if (url.back() != '/' && endpoint.front() != '/') {
        url += '/';
    }
    url += endpoint;
    return url;
}
