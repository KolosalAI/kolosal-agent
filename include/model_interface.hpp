#pragma once

#include <string>
#include <json.hpp>

using json = nlohmann::json;

/**
 * @brief Interface for model communication
 * Handles communication with various AI models through the kolosal-server
 */
class ModelInterface {
private:
    std::string server_url_;
    
public:
    explicit ModelInterface(const std::string& server_url = "http://localhost:8080");
    ~ModelInterface() = default;
    
    /**
     * @brief Generate text completion using the specified model
     * @param model_name Name of the model to use
     * @param prompt The prompt to send to the model
     * @param system_prompt Optional system prompt
     * @param max_tokens Maximum tokens to generate
     * @param temperature Temperature for generation
     * @return Generated text response
     */
    std::string generate_completion(const std::string& model_name, 
                                   const std::string& prompt,
                                   const std::string& system_prompt = "",
                                   int max_tokens = 256,
                                   float temperature = 0.7);
    
    /**
     * @brief Send a chat message to the specified model
     * @param model_name Name of the model to use
     * @param message User message
     * @param system_prompt Optional system prompt
     * @param conversation_history Optional conversation context
     * @return Model response
     */
    std::string chat_with_model(const std::string& model_name,
                                const std::string& message,
                                const std::string& system_prompt = "",
                                const json& conversation_history = json::array());
    
    /**
     * @brief Check if a model is available
     * @param model_name Name of the model to check
     * @return True if model is available and ready
     */
    bool is_model_available(const std::string& model_name);
    
    /**
     * @brief Get list of available models
     * @return JSON array of available models
     */
    json get_available_models();
    
private:
    // Note: This is a placeholder implementation
    // For production use, implement actual HTTP client using curl or similar library
};
