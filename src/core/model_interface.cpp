#include "../include/model_interface.hpp"
#include <iostream>
#include <stdexcept>

// Simple HTTP client implementation without curl dependency
// This is a placeholder implementation - in production, you would use curl or another HTTP library

namespace {
    // Placeholder function for HTTP requests
    // In a real implementation, this would use curl or another HTTP library
    std::string make_http_request(const std::string& url, const std::string& method, const std::string& body) {
        // Placeholder implementation that returns mock responses
        // TODO: Replace with actual HTTP client implementation using curl
        
        std::cout << "[ModelInterface] Mock HTTP request: " << method << " " << url << std::endl;
        std::cout << "[ModelInterface] Request body: " << body << std::endl;
        
        // Mock responses for different endpoints
        if (url.find("/models") != std::string::npos) {
            return R"({
                "models": [
                    {
                        "model_id": "qwen2.5-0.5b-instruct-q4_k_m",
                        "available": true,
                        "inference_ready": true,
                        "model_type": "llm"
                    },
                    {
                        "model_id": "all-MiniLM-L6-v2-bf16-q4_k",
                        "available": true,
                        "inference_ready": true,
                        "model_type": "embedding"
                    }
                ]
            })";
        } else if (url.find("/completion") != std::string::npos) {
            return R"({
                "content": "This is a mock response from the model interface. In a real implementation, this would connect to the kolosal-server to get actual AI responses."
            })";
        } else if (url.find("/chat/completions") != std::string::npos) {
            return R"({
                "choices": [
                    {
                        "message": {
                            "content": "This is a mock chat response. In a real implementation, this would connect to the kolosal-server for actual AI chat responses."
                        }
                    }
                ]
            })";
        }
        
        throw std::runtime_error("Unknown endpoint: " + url);
    }
}

ModelInterface::ModelInterface(const std::string& server_url) : server_url_(server_url) {
    std::cout << "[ModelInterface] Initialized with server URL: " << server_url_ << std::endl;
    std::cout << "[ModelInterface] WARNING: Using mock implementation. For production, implement actual HTTP client." << std::endl;
}

std::string ModelInterface::generate_completion(const std::string& model_name, 
                                               const std::string& prompt,
                                               const std::string& system_prompt,
                                               int max_tokens,
                                               float temperature) {
    try {
        json request_body;
        request_body["model"] = model_name;
        request_body["prompt"] = prompt;
        request_body["max_tokens"] = max_tokens;
        request_body["temperature"] = temperature;
        request_body["stream"] = false;
        
        if (!system_prompt.empty()) {
            request_body["system"] = system_prompt;
        }
        
        std::string response_str = make_http_request(server_url_ + "/completion", "POST", request_body.dump());
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
        json request_body;
        request_body["model"] = model_name;
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
        
        std::string response_str = make_http_request(server_url_ + "/chat/completions", "POST", request_body.dump());
        json response = json::parse(response_str);
        
        if (response.contains("choices") && !response["choices"].empty()) {
            auto& choice = response["choices"][0];
            if (choice.contains("message") && choice["message"].contains("content")) {
                return choice["message"]["content"].get<std::string>();
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
