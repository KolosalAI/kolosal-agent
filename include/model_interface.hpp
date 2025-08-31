#pragma once

#include <string>
#include <memory>
#include <json.hpp>
#include "kolosal_client.hpp"

using json = nlohmann::json;

/**
 * @brief Interface for model communication
 * Handles communication with various AI models through the kolosal-server
 */
class ModelInterface {
private:
    std::unique_ptr<KolosalClient> kolosal_client_;
    
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
    
    /**
     * @brief Configure model interface with model configurations
     * @param model_configs JSON array of model configurations
     */
    void configure_models(const json& model_configs);
    
    /**
     * @brief Resolve model alias to actual server model name
     * @param model_name Model alias or name to resolve
     * @return Actual model name expected by server
     */
    std::string resolve_model_name(const std::string& model_name);
    
private:
    json model_configurations_;
    
    /**
     * @brief Get the underlying Kolosal client
     * @return Reference to the Kolosal client
     */
    KolosalClient& get_client() { return *kolosal_client_; }
};
