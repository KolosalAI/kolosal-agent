#pragma once

#include <string>
#include <memory>
#include <json.hpp>
#include "logger.hpp"

using json = nlohmann::json;

// Forward declaration
class HttpClient;

/**
 * @brief Client for communicating with Kolosal Server
 * 
 * This class provides a clean interface for making requests to the Kolosal Server
 * running in the background. It handles HTTP requests for model inference,
 * retrieval operations, and server status checks.
 */
class KolosalClient {
public:
    /**
     * @brief Configuration for the Kolosal Client
     */
    struct Config {
        std::string server_url = "http://127.0.0.1:8081";
        int timeout_seconds = 30;
        int max_retries = 3;
        int retry_delay_ms = 1000;
        bool verify_ssl = true;
    };

    /**
     * @brief Constructor
     * @param config Client configuration
     */
    explicit KolosalClient(const Config& config = Config{});

    /**
     * @brief Destructor
     */
    ~KolosalClient();

    // Model Interface Methods
    /**
     * @brief Check if a model is available on the server
     * @param model_name Name of the model to check
     * @return true if model is available, false otherwise
     */
    bool is_model_available(const std::string& model_name);

    /**
     * @brief Get list of available models
     * @return JSON array of available models
     */
    json get_available_models();

    /**
     * @brief Chat with a model
     * @param model_name Name of the model to use
     * @param message Message to send
     * @param system_prompt Optional system prompt
     * @return Model response as string
     */
    std::string chat_with_model(const std::string& model_name, 
                                const std::string& message, 
                                const std::string& system_prompt = "");

    /**
     * @brief Make a completion request to a model
     * @param model_name Name of the model to use
     * @param prompt Prompt for completion
     * @param params Additional parameters (temperature, max_tokens, etc.)
     * @return JSON response from the model
     */
    json completion_request(const std::string& model_name, 
                           const std::string& prompt, 
                           const json& params = json::object());

    // Retrieval Interface Methods
    /**
     * @brief Add a document to the retrieval system
     * @param document_data Document content and metadata
     * @return Operation result
     */
    json add_document(const json& document_data);

    /**
     * @brief Search documents in the retrieval system
     * @param query Search query
     * @param limit Maximum number of results (default: 10)
     * @param filters Optional filters
     * @return Search results
     */
    json search_documents(const std::string& query, 
                         int limit = 10, 
                         const json& filters = json::object());

    /**
     * @brief Remove a document from the retrieval system
     * @param document_id ID of the document to remove
     * @return Operation result
     */
    json remove_document(const std::string& document_id);

    /**
     * @brief List documents in the retrieval system
     * @param offset Offset for pagination (default: 0)
     * @param limit Maximum number of results (default: 50)
     * @return List of documents
     */
    json list_documents(int offset = 0, int limit = 50);

    /**
     * @brief Perform internet search via the server
     * @param query Search query
     * @param num_results Number of results to return (default: 10)
     * @return Search results
     */
    json internet_search(const std::string& query, int num_results = 10);

    // Server Status Methods
    /**
     * @brief Check if the server is healthy and responding
     * @return true if server is healthy, false otherwise
     */
    bool is_server_healthy();

    /**
     * @brief Get server status information
     * @return JSON object with server status
     */
    json get_server_status();

    /**
     * @brief Get server configuration
     * @return JSON object with server configuration
     */
    json get_server_config();

    /**
     * @brief Update client configuration
     * @param new_config New configuration to apply
     */
    void update_config(const Config& new_config);

    /**
     * @brief Get current client configuration
     * @return Current configuration
     */
    const Config& get_config() const { return config_; }

private:
    Config config_;
    std::unique_ptr<HttpClient> http_client_;
    
    /**
     * @brief Make HTTP request to the server (deprecated - use HttpClient directly)
     */
    json make_request(const std::string& method,
                     const std::string& endpoint,
                     const json& data = json::object(),
                     const json& headers = json::object());

    /**
     * @brief Make HTTP request with retry logic (deprecated - use HttpClient directly)
     */
    json make_request_with_retry(const std::string& method,
                                const std::string& endpoint,
                                const json& data = json::object(),
                                const json& headers = json::object());

    /**
     * @brief Parse response and check for errors (deprecated)
     */
    json parse_response(const std::string& response_body, long status_code);

    /**
     * @brief Build full URL for an endpoint (deprecated)
     */
    std::string build_url(const std::string& endpoint) const;
};
