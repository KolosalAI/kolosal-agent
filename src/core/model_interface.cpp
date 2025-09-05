#include "../include/model_interface.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>

ModelInterface::ModelInterface(const std::string& server_url) {
    // Create KolosalClient with the server URL
    KolosalClient::Config client_config;
    client_config.server_url = server_url;
    
    kolosal_client_ = std::make_unique<KolosalClient>(client_config);
    
    std::cout << "[ModelInterface] Initialized with server URL: " << server_url << std::endl;
    std::cout << "[ModelInterface] Using KolosalClient for server communication." << std::endl;
    
    // Set up default model configurations if none are provided
    json default_models = json::array();
    json default_model;
    default_model["id"] = "default";
    default_model["actual_name"] = "qwen3-0.6b:UD-Q4_K_XL";
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
        
        json params;
        params["max_tokens"] = max_tokens;
        params["temperature"] = temperature;
        params["stream"] = false;
        
        if (!system_prompt.empty()) {
            params["system"] = system_prompt;
        }
        
        json response = kolosal_client_->completion_request(actual_model_name, prompt, params);
        
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
        
        // For now, ignore conversation_history and just call the 3-parameter version
        return kolosal_client_->chat_with_model(actual_model_name, message, system_prompt);
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
        
        // Fallback to checking via KolosalClient
        return kolosal_client_->is_model_available(model_name);
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
        
        return kolosal_client_->get_available_models();
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
