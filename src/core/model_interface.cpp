#include "../include/model_interface.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif

// Real HTTP client implementation using WinHTTP
namespace {
#ifdef _WIN32
    std::string make_http_request(const std::string& url, const std::string& method, const std::string& body) {
        std::cout << "[ModelInterface] Making real HTTP request: " << method << " " << url << std::endl;
        
        // Parse URL
        std::string hostname, path;
        int port = 80;
        bool is_https = false;
        
        size_t proto_end = url.find("://");
        if (proto_end == std::string::npos) {
            throw std::runtime_error("Invalid URL format");
        }
        
        std::string protocol = url.substr(0, proto_end);
        if (protocol == "https") {
            port = 443;
            is_https = true;
        }
        
        std::string url_part = url.substr(proto_end + 3);
        size_t path_start = url_part.find('/');
        
        if (path_start != std::string::npos) {
            hostname = url_part.substr(0, path_start);
            path = url_part.substr(path_start);
        } else {
            hostname = url_part;
            path = "/";
        }
        
        // Extract port if present
        size_t port_pos = hostname.find(':');
        if (port_pos != std::string::npos) {
            port = std::stoi(hostname.substr(port_pos + 1));
            hostname = hostname.substr(0, port_pos);
        }
        
        // Initialize WinHTTP
        HINTERNET hSession = WinHttpOpen(L"Kolosal Agent/1.0", 
                                        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                        WINHTTP_NO_PROXY_NAME, 
                                        WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) {
            throw std::runtime_error("Failed to initialize WinHTTP");
        }
        
        // Convert hostname to wide string
        std::wstring w_hostname(hostname.begin(), hostname.end());
        
        HINTERNET hConnect = WinHttpConnect(hSession, w_hostname.c_str(), port, 0);
        if (!hConnect) {
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to connect to server");
        }
        
        // Convert method and path to wide strings
        std::wstring w_method(method.begin(), method.end());
        std::wstring w_path(path.begin(), path.end());
        
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, w_method.c_str(), w_path.c_str(),
                                               NULL, WINHTTP_NO_REFERER, 
                                               WINHTTP_DEFAULT_ACCEPT_TYPES, 
                                               is_https ? WINHTTP_FLAG_SECURE : 0);
        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to open request");
        }
        
        // Set headers for JSON content
        std::wstring headers = L"Content-Type: application/json\r\n";
        
        // Send request
        BOOL result = FALSE;
        if (!body.empty()) {
            result = WinHttpSendRequest(hRequest, headers.c_str(), -1,
                                       (LPVOID)body.c_str(), body.length(),
                                       body.length(), 0);
        } else {
            result = WinHttpSendRequest(hRequest, headers.c_str(), -1,
                                       WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        }
        
        if (!result) {
            DWORD error = GetLastError();
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            std::string error_msg = "Failed to send request. Error code: " + std::to_string(error);
            std::cout << "[ModelInterface] " << error_msg << std::endl;
            throw std::runtime_error(error_msg);
        }
        
        // Receive response
        if (!WinHttpReceiveResponse(hRequest, NULL)) {
            DWORD error = GetLastError();
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            std::string error_msg = "Failed to receive response. Error code: " + std::to_string(error);
            std::cout << "[ModelInterface] " << error_msg << std::endl;
            throw std::runtime_error(error_msg);
        }
        
        // Read response data
        std::string response;
        DWORD bytesAvailable = 0;
        
        do {
            bytesAvailable = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable)) {
                break;
            }
            
            if (bytesAvailable > 0) {
                char* buffer = new char[bytesAvailable + 1];
                DWORD bytesRead = 0;
                
                if (WinHttpReadData(hRequest, buffer, bytesAvailable, &bytesRead)) {
                    buffer[bytesRead] = '\0';
                    response += buffer;
                }
                delete[] buffer;
            }
        } while (bytesAvailable > 0);
        
        // Cleanup
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        
        std::cout << "[ModelInterface] Response received: " << response.substr(0, 200) << "..." << std::endl;
        return response;
    }
#else
    // Fallback for non-Windows systems - you could implement with curl here
    std::string make_http_request(const std::string& url, const std::string& method, const std::string& body) {
        throw std::runtime_error("HTTP client not implemented for non-Windows systems");
    }
#endif
}

ModelInterface::ModelInterface(const std::string& server_url) : server_url_(server_url) {
    std::cout << "[ModelInterface] Initialized with server URL: " << server_url_ << std::endl;
    std::cout << "[ModelInterface] Using WinHTTP client for real server communication." << std::endl;
    
    // Set up default model configurations if none are provided
    json default_models = json::array();
    json default_model;
    default_model["id"] = "gemma3-1b";
    default_model["actual_name"] = "gemma3-1b";
    default_model["type"] = "llm";
    default_models.push_back(default_model);
    
    model_configurations_ = default_models;
    std::cout << "[ModelInterface] Initialized with default model configurations" << std::endl;
}

std::string ModelInterface::resolve_model_name(const std::string& model_name) {
    // Check if we have model configurations with actual_name mapping
    if (!model_configurations_.empty()) {
        for (const auto& model_config : model_configurations_) {
            if (model_config.contains("id") && model_config["id"] == model_name) {
                if (model_config.contains("actual_name")) {
                    return model_config["actual_name"].get<std::string>();
                }
                // If no actual_name field, return the id as-is
                return model_name;
            }
        }
    }
    
    // If no configuration found, return the model name as-is
    return model_name;
}

std::string ModelInterface::generate_completion(const std::string& model_name, 
                                               const std::string& prompt,
                                               const std::string& system_prompt,
                                               int max_tokens,
                                               float temperature) {
    try {
        // Resolve the model name to actual server model name
        std::string actual_model_name = resolve_model_name(model_name);
        std::cout << "[ModelInterface] Resolving model '" << model_name << "' to '" << actual_model_name << "'" << std::endl;
        
        json request_body;
        request_body["model"] = actual_model_name;
        request_body["prompt"] = prompt;
        request_body["max_tokens"] = max_tokens;
        request_body["temperature"] = temperature;
        request_body["stream"] = false;
        
        if (!system_prompt.empty()) {
            request_body["system"] = system_prompt;
        }
        
        std::string response_str = make_http_request(server_url_ + "/v1/completions", "POST", request_body.dump());
        json response = json::parse(response_str);
        
        if (response.contains("content")) {
            return response["content"].get<std::string>();
        } else if (response.contains("choices") && !response["choices"].empty()) {
            return response["choices"][0]["text"].get<std::string>();
        } else {
            throw std::runtime_error("Unexpected response format from model server");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in generate_completion: " << e.what() << std::endl;
        throw std::runtime_error("Failed to generate completion: " + std::string(e.what()));
    }
}

std::string ModelInterface::chat_with_model(const std::string& model_name,
                                           const std::string& message,
                                           const std::string& system_prompt,
                                           const json& conversation_history) {
    try {
        // Resolve the model name to actual server model name
        std::string actual_model_name = resolve_model_name(model_name);
        std::cout << "[ModelInterface] Resolving model '" << model_name << "' to '" << actual_model_name << "'" << std::endl;
        
        json request_body;
        request_body["model"] = actual_model_name;
        request_body["stream"] = false;
        request_body["temperature"] = 0.7;
        request_body["max_tokens"] = 512;
        
        // Build messages array
        json messages = json::array();
        
        // Add system prompt if provided
        if (!system_prompt.empty()) {
            json system_msg;
            system_msg["role"] = "system";
            system_msg["content"] = system_prompt;
            messages.push_back(system_msg);
        }
        
        // Add conversation history if provided
        if (conversation_history.is_array()) {
            for (const auto& msg : conversation_history) {
                messages.push_back(msg);
            }
        }
        
        // Add current user message
        json user_msg;
        user_msg["role"] = "user";
        user_msg["content"] = message;
        messages.push_back(user_msg);
        
        request_body["messages"] = messages;
        
        std::string response_str = make_http_request(server_url_ + "/v1/chat/completions", "POST", request_body.dump());
        std::cout << "[ModelInterface] Raw response: " << response_str << std::endl;
        
        json response = json::parse(response_str);
        std::cout << "[ModelInterface] Parsed JSON: " << response.dump(2) << std::endl;
        
        if (response.contains("choices") && !response["choices"].empty()) {
            auto& choice = response["choices"][0];
            if (choice.contains("message") && choice["message"].contains("content")) {
                std::string content = choice["message"]["content"].get<std::string>();
                std::cout << "[ModelInterface] Extracted content: " << content << std::endl;
                return content;
            }
        }
        
        // Fallback: try to get content directly
        if (response.contains("content")) {
            return response["content"].get<std::string>();
        }
        
        throw std::runtime_error("Unexpected response format from chat endpoint");
    } catch (const std::exception& e) {
        std::cerr << "Error in chat_with_model: " << e.what() << std::endl;
        throw std::runtime_error("Failed to chat with model: " + std::string(e.what()));
    }
}

bool ModelInterface::is_model_available(const std::string& model_name) {
    try {
        // First check in our configured models
        if (!model_configurations_.empty()) {
            for (const auto& model_config : model_configurations_) {
                if (model_config.contains("id") && model_config["id"] == model_name) {
                    return true; // Model is configured, assume it's available
                }
            }
            return false; // Model not found in configurations
        }
        
        // Fallback to checking via HTTP request
        json models = get_available_models();
        
        if (models.is_array()) {
            for (const auto& model : models) {
                if (model.contains("model_id") && model["model_id"] == model_name) {
                    return model.value("available", false) && model.value("inference_ready", false);
                }
            }
        }
        
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error checking model availability: " << e.what() << std::endl;
        return false;
    }
}

json ModelInterface::get_available_models() {
    try {
        // If we have configured models, use them instead of making HTTP requests
        if (!model_configurations_.empty()) {
            json available_models = json::array();
            for (const auto& model_config : model_configurations_) {
                if (model_config.contains("id") && model_config.contains("type")) {
                    json model_info;
                    model_info["model_id"] = model_config["id"];
                    model_info["available"] = true;
                    model_info["inference_ready"] = true;
                    model_info["model_type"] = model_config["type"];
                    available_models.push_back(model_info);
                }
            }
            return available_models;
        }
        
        std::string response_str = make_http_request(server_url_ + "/models", "GET", "");
        json response = json::parse(response_str);
        
        // Extract the models array from the response
        if (response.contains("models") && response["models"].is_array()) {
            return response["models"];
        } else {
            // Return empty array if models key is not found
            return json::array();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting available models: " << e.what() << std::endl;
        throw std::runtime_error("Failed to get available models: " + std::string(e.what()));
    }
}

void ModelInterface::configure_models(const json& model_configs) {
    model_configurations_ = model_configs;
    std::cout << "[ModelInterface] Configured with " << model_configs.size() << " models" << std::endl;
    
    for (const auto& model : model_configs) {
        if (model.contains("id")) {
            std::cout << "[ModelInterface] - Model: " << model["id"].get<std::string>() 
                      << " (type: " << model.value("type", "unknown") << ")" << std::endl;
        }
    }
}
