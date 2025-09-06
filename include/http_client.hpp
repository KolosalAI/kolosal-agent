#pragma once

#include <string>
#include <map>
#include <memory>

/**
 * @brief Safe HTTP client with buffer overflow protection and structured error handling
 * 
 * Features:
 * - Buffer overflow protection with size limits
 * - Input validation and sanitization
 * - Structured error responses with user-friendly messages
 * - Automatic retry with exponential backoff
 * - Cross-platform implementation (Windows/Unix)
 */
class HttpClient {
public:
    /**
     * @brief Configuration for HTTP client
     */
    struct Config {
        std::string base_url = "http://127.0.0.1:8081";
        int timeout_seconds = 30;
        int max_retries = 3;
        int retry_delay_ms = 1000;
        bool verify_ssl = true;
    };

    /**
     * @brief Structured HTTP response with error handling
     */
    struct Result {
        long status_code;
        std::string body;
        std::string error_message;
        bool retry_recommended;
        
        bool is_success() const { 
            return status_code >= 200 && status_code < 300; 
        }
        
        bool is_client_error() const { 
            return status_code >= 400 && status_code < 500; 
        }
        
        bool is_server_error() const { 
            return status_code >= 500; 
        }
    };

    /**
     * @brief Constructor with configuration validation
     * @param config HTTP client configuration
     */
    explicit HttpClient(const Config& config = Config{});

    /**
     * @brief Destructor
     */
    ~HttpClient();

    /**
     * @brief Make HTTP request with safety checks and retry logic
     * @param method HTTP method (GET, POST, PUT, DELETE, PATCH)
     * @param endpoint API endpoint (relative to base_url)
     * @param body Request body (for POST/PUT/PATCH)
     * @param headers Additional HTTP headers
     * @return Structured result with status, body, and error information
     */
    Result request(const std::string& method,
                  const std::string& endpoint,
                  const std::string& body = "",
                  const std::map<std::string, std::string>& headers = {});

    /**
     * @brief Update client configuration
     * @param new_config New configuration (validated)
     */
    void update_config(const Config& new_config);

    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    const Config& get_config() const { return config_; }

private:
    Config config_;

    /**
     * @brief Make request with retry logic and exponential backoff
     */
    Result request_with_retry(const std::string& method,
                             const std::string& url,
                             const std::string& body,
                             const std::map<std::string, std::string>& headers);

    /**
     * @brief Perform actual HTTP request (platform-specific)
     */
    Result perform_request(const std::string& method,
                          const std::string& url,
                          const std::string& body,
                          const std::map<std::string, std::string>& headers);

#ifdef _WIN32
    /**
     * @brief Windows-specific HTTP implementation using WinHTTP
     */
    Result perform_winhttp_request(const std::string& method,
                                  const std::string& url,
                                  const std::string& body,
                                  const std::map<std::string, std::string>& headers);

    /**
     * @brief Safely parse URL components for WinHTTP
     */
    bool parse_url_components(const std::string& url, std::wstring& host, 
                             std::wstring& path, unsigned short& port);
#else
    /**
     * @brief Unix-specific HTTP implementation using libcurl
     */
    Result perform_curl_request(const std::string& method,
                               const std::string& url,
                               const std::string& body,
                               const std::map<std::string, std::string>& headers);
#endif

    /**
     * @brief Build complete URL from base and endpoint
     * @param endpoint API endpoint
     * @return Complete URL (validated)
     */
    std::string build_url(const std::string& endpoint) const;
};
