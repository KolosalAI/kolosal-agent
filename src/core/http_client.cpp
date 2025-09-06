#include "http_client.hpp"
#include "logger.hpp"
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <thread>
#include <sstream>
#include <regex>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
#include <curl/curl.h>
#endif

namespace {
    // Safe buffer size constants
    constexpr size_t MAX_RESPONSE_SIZE = 100 * 1024 * 1024; // 100MB
    constexpr size_t MAX_URL_LENGTH = 2048;
    constexpr size_t MAX_HEADER_LENGTH = 8192;
    constexpr size_t BUFFER_CHUNK_SIZE = 8192;

    // Validate URL format and safety
    bool is_valid_url(const std::string& url) {
        if (url.empty() || url.length() > MAX_URL_LENGTH) {
            return false;
        }
        
        // Basic URL pattern validation
        static const std::regex url_pattern(
            R"(^https?://[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)*(:(\d{1,5}))?(/.*)?$)"
        );
        
        return std::regex_match(url, url_pattern);
    }

    // Sanitize header values
    std::string sanitize_header_value(const std::string& value) {
        std::string sanitized = value;
        // Remove control characters and newlines to prevent header injection
        sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(), 
            [](char c) { return c < 32 || c == 127; }), sanitized.end());
        
        if (sanitized.length() > MAX_HEADER_LENGTH) {
            sanitized.resize(MAX_HEADER_LENGTH);
        }
        
        return sanitized;
    }

    #ifndef _WIN32
    // Safe write callback with bounds checking
    size_t safe_write_callback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t total_size = size * nmemb;
        
        // Check for potential overflow
        if (size != 0 && nmemb > SIZE_MAX / size) {
            return 0; // Overflow detected
        }
        
        // Check total response size limit
        if (userp->size() + total_size > MAX_RESPONSE_SIZE) {
            return 0; // Response too large
        }
        
        try {
            userp->append(static_cast<const char*>(contents), total_size);
        } catch (const std::bad_alloc&) {
            return 0; // Memory allocation failed
        }
        
        return total_size;
    }
    #endif

    // Parse HTTP status code from response
    struct HttpResponse {
        long status_code;
        std::string body;
        std::string error_message;
        bool retry_recommended;
    };

    // Determine if error is retryable
    bool is_retryable_error(long status_code, const std::string& error_msg) {
        // Retry on temporary failures
        return status_code == 429 ||  // Too Many Requests
               status_code == 502 ||  // Bad Gateway
               status_code == 503 ||  // Service Unavailable
               status_code == 504 ||  // Gateway Timeout
               error_msg.find("timeout") != std::string::npos ||
               error_msg.find("connection") != std::string::npos;
    }

    // Get user-friendly error message
    std::string get_user_friendly_error(long status_code, const std::string& technical_error) {
        switch (status_code) {
            case 400:
                return "Bad request - Please check your input parameters";
            case 401:
                return "Unauthorized - Please check your authentication credentials";
            case 403:
                return "Access forbidden - You don't have permission to access this resource";
            case 404:
                return "Resource not found - The requested endpoint may not exist";
            case 429:
                return "Rate limit exceeded - Please reduce request frequency and try again";
            case 500:
                return "Internal server error - The server encountered an unexpected condition";
            case 502:
                return "Bad gateway - The server received an invalid response from upstream";
            case 503:
                return "Service unavailable - The server is temporarily overloaded or under maintenance";
            case 504:
                return "Gateway timeout - The server didn't receive a timely response from upstream";
            default:
                if (status_code >= 400 && status_code < 500) {
                    return "Client error (" + std::to_string(status_code) + ") - Please check your request";
                } else if (status_code >= 500) {
                    return "Server error (" + std::to_string(status_code) + ") - Please try again later";
                }
                return "Unknown error (" + std::to_string(status_code) + ") - " + technical_error;
        }
    }
}

HttpClient::HttpClient(const Config& config) : config_(config) {
    if (!is_valid_url(config_.base_url)) {
        throw std::invalid_argument("Invalid base URL format");
    }
    
    if (config_.timeout_seconds <= 0 || config_.timeout_seconds > 300) {
        throw std::invalid_argument("Timeout must be between 1 and 300 seconds");
    }
    
    if (config_.max_retries < 0 || config_.max_retries > 10) {
        throw std::invalid_argument("Max retries must be between 0 and 10");
    }
    
    #ifndef _WIN32
    curl_global_init(CURL_GLOBAL_DEFAULT);
    #endif
    
    LOG_INFO_F("HttpClient initialized with base URL: %s", config_.base_url.c_str());
}

HttpClient::~HttpClient() {
    #ifndef _WIN32
    curl_global_cleanup();
    #endif
}

HttpClient::Result HttpClient::request(const std::string& method,
                                     const std::string& endpoint,
                                     const std::string& body,
                                     const std::map<std::string, std::string>& headers) {
    
    // Input validation
    if (method.empty() || endpoint.empty()) {
        return Result{500, "", "Invalid method or endpoint", false};
    }
    
    if (body.size() > MAX_RESPONSE_SIZE) {
        return Result{400, "", "Request body too large", false};
    }
    
    std::string url = build_url(endpoint);
    if (!is_valid_url(url)) {
        return Result{400, "", "Invalid URL constructed", false};
    }
    
    return request_with_retry(method, url, body, headers);
}

HttpClient::Result HttpClient::request_with_retry(const std::string& method,
                                                const std::string& url,
                                                const std::string& body,
                                                const std::map<std::string, std::string>& headers) {
    
    int attempts = 0;
    Result last_result{500, "", "No attempts made", false};
    
    while (attempts <= config_.max_retries) {
        try {
            last_result = perform_request(method, url, body, headers);
            
            // Success case
            if (last_result.status_code >= 200 && last_result.status_code < 300) {
                return last_result;
            }
            
            // Check if we should retry
            if (!last_result.retry_recommended || attempts >= config_.max_retries) {
                break;
            }
            
            // Calculate backoff delay (exponential with jitter)
            int base_delay = config_.retry_delay_ms;
            int backoff_delay = base_delay * (1 << std::min(attempts, 5)); // Cap at 32x
            int jitter = (std::rand() % (backoff_delay / 4)) - (backoff_delay / 8);
            int total_delay = std::max(backoff_delay + jitter, base_delay);
            
            LOG_WARN_F("Request failed (attempt %d/%d), retrying in %dms: %s", 
                      attempts + 1, config_.max_retries + 1, total_delay, last_result.error_message.c_str());
            
            std::this_thread::sleep_for(std::chrono::milliseconds(total_delay));
            
        } catch (const std::exception& e) {
            last_result.status_code = 500;
            last_result.body.clear();
            last_result.error_message = e.what();
            last_result.retry_recommended = false;
            
            if (attempts >= config_.max_retries) {
                break;
            }
        }
        
        attempts++;
    }
    
    // Enhance error message with user-friendly information
    last_result.error_message = get_user_friendly_error(last_result.status_code, last_result.error_message);
    
    LOG_ERROR_F("Request failed after %d attempts: %s", attempts, last_result.error_message.c_str());
    return last_result;
}

HttpClient::Result HttpClient::perform_request(const std::string& method,
                                             const std::string& url,
                                             const std::string& body,
                                             const std::map<std::string, std::string>& headers) {
    
    LOG_DEBUG_F("Making %s request to: %s", method.c_str(), url.c_str());
    
#ifdef _WIN32
    return perform_winhttp_request(method, url, body, headers);
#else
    return perform_curl_request(method, url, body, headers);
#endif
}

#ifdef _WIN32
HttpClient::Result HttpClient::perform_winhttp_request(const std::string& method,
                                                     const std::string& url,
                                                     const std::string& body,
                                                     const std::map<std::string, std::string>& headers) {
    
    // Parse URL components safely
    std::wstring w_host, w_path;
    INTERNET_PORT port;
    
    if (!parse_url_components(url, w_host, w_path, port)) {
        return Result{400, "", "Failed to parse URL components", false};
    }
    
    // Initialize WinHTTP session
    HINTERNET hSession = WinHttpOpen(L"HttpClient/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        return Result{500, "", "Failed to initialize WinHTTP session", true};
    }
    
    // Connect to server
    HINTERNET hConnect = WinHttpConnect(hSession, w_host.c_str(), port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return Result{500, "", "Failed to connect to server", true};
    }
    
    // Create request
    std::wstring w_method(method.begin(), method.end());
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, w_method.c_str(), w_path.c_str(),
                                           NULL, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return Result{500, "", "Failed to create HTTP request", true};
    }
    
    // Set timeouts with bounds checking
    DWORD timeout = static_cast<DWORD>(std::min(config_.timeout_seconds * 1000, 300000));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
    
    // Build headers string safely
    std::wstring additional_headers = L"Content-Type: application/json\r\n";
    for (const auto& [key, value] : headers) {
        std::string sanitized_key = sanitize_header_value(key);
        std::string sanitized_value = sanitize_header_value(value);
        
        additional_headers += std::wstring(sanitized_key.begin(), sanitized_key.end()) + L": " + 
                             std::wstring(sanitized_value.begin(), sanitized_value.end()) + L"\r\n";
    }
    
    // Send request with size validation
    DWORD body_length = static_cast<DWORD>(std::min(body.length(), static_cast<size_t>(MAXDWORD)));
    BOOL result = WinHttpSendRequest(hRequest, additional_headers.c_str(), -1,
                                    const_cast<char*>(body.c_str()), body_length,
                                    body_length, 0);
    
    if (!result) {
        DWORD error = GetLastError();
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        
        bool should_retry = (error == ERROR_WINHTTP_TIMEOUT || 
                           error == ERROR_WINHTTP_CANNOT_CONNECT ||
                           error == ERROR_WINHTTP_CONNECTION_ERROR);
        
        return Result{500, "", "Failed to send HTTP request: " + std::to_string(error), should_retry};
    }
    
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return Result{500, "", "Failed to receive HTTP response", true};
    }
    
    // Get status code
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                       WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
    
    // Read response body with bounds checking
    std::string response_body;
    response_body.reserve(BUFFER_CHUNK_SIZE);
    
    DWORD bytesAvailable, bytesRead;
    std::vector<char> buffer(BUFFER_CHUNK_SIZE);
    
    do {
        bytesAvailable = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable)) break;
        
        if (bytesAvailable > 0) {
            DWORD bytesToRead = std::min(bytesAvailable, static_cast<DWORD>(buffer.size()));
            if (WinHttpReadData(hRequest, buffer.data(), bytesToRead, &bytesRead) && bytesRead > 0) {
                
                // Check total size limit
                if (response_body.size() + bytesRead > MAX_RESPONSE_SIZE) {
                    WinHttpCloseHandle(hRequest);
                    WinHttpCloseHandle(hConnect);
                    WinHttpCloseHandle(hSession);
                    return Result{413, "", "Response too large", false};
                }
                
                try {
                    response_body.append(buffer.data(), bytesRead);
                } catch (const std::bad_alloc&) {
                    WinHttpCloseHandle(hRequest);
                    WinHttpCloseHandle(hConnect);
                    WinHttpCloseHandle(hSession);
                    return Result{500, "", "Memory allocation failed", false};
                }
            }
        }
    } while (bytesAvailable > 0);
    
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
    bool should_retry = is_retryable_error(statusCode, response_body);
    std::string error_msg = (statusCode >= 400) ? response_body : "";
    
    return Result{static_cast<long>(statusCode), response_body, error_msg, should_retry};
}

bool HttpClient::parse_url_components(const std::string& url, std::wstring& host, 
                                    std::wstring& path, INTERNET_PORT& port) {
    try {
        // Remove protocol
        std::string url_without_protocol = url;
        bool is_https = false;
        
        if (url.find("https://") == 0) {
            url_without_protocol = url.substr(8);
            is_https = true;
        } else if (url.find("http://") == 0) {
            url_without_protocol = url.substr(7);
        } else {
            return false;
        }
        
        // Split host and path
        size_t slash_pos = url_without_protocol.find('/');
        std::string host_port = url_without_protocol.substr(0, slash_pos);
        std::string path_str = slash_pos != std::string::npos ? url_without_protocol.substr(slash_pos) : "/";
        
        // Split host and port
        size_t colon_pos = host_port.find(':');
        std::string host_str = colon_pos != std::string::npos ? host_port.substr(0, colon_pos) : host_port;
        
        if (colon_pos != std::string::npos) {
            int port_num = std::stoi(host_port.substr(colon_pos + 1));
            if (port_num <= 0 || port_num > 65535) {
                return false;
            }
            port = static_cast<INTERNET_PORT>(port_num);
        } else {
            port = is_https ? 443 : 80;
        }
        
        // Convert to wide strings with bounds checking
        if (host_str.length() > 253 || path_str.length() > 2000) { // DNS name + path limits
            return false;
        }
        
        host = std::wstring(host_str.begin(), host_str.end());
        path = std::wstring(path_str.begin(), path_str.end());
        
        return true;
    } catch (...) {
        return false;
    }
}

#else
HttpClient::Result HttpClient::perform_curl_request(const std::string& method,
                                                   const std::string& url,
                                                   const std::string& body,
                                                   const std::map<std::string, std::string>& headers) {
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        return Result{500, "", "Failed to initialize libcurl", true};
    }
    
    std::string response_body;
    response_body.reserve(BUFFER_CHUNK_SIZE);
    long response_code = 0;
    
    // Set basic options with bounds checking
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, safe_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(std::min(config_.timeout_seconds, 300)));
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, config_.verify_ssl ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, config_.verify_ssl ? 2L : 0L);
    
    // Set method and body
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (!body.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.length()));
        }
    } else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (!body.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.length()));
        }
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (method == "PATCH") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        if (!body.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.length()));
        }
    }
    
    // Set headers with sanitization
    struct curl_slist* header_list = NULL;
    header_list = curl_slist_append(header_list, "Content-Type: application/json");
    
    for (const auto& [key, value] : headers) {
        std::string sanitized_key = sanitize_header_value(key);
        std::string sanitized_value = sanitize_header_value(value);
        std::string header = sanitized_key + ": " + sanitized_value;
        
        if (header.length() <= MAX_HEADER_LENGTH) {
            header_list = curl_slist_append(header_list, header.c_str());
        }
    }
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    // Cleanup
    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);
    
    // Handle curl errors
    if (res != CURLE_OK) {
        std::string error_msg = curl_easy_strerror(res);
        bool should_retry = (res == CURLE_COULDNT_CONNECT ||
                           res == CURLE_OPERATION_TIMEDOUT ||
                           res == CURLE_RECV_ERROR ||
                           res == CURLE_SEND_ERROR);
        
        return Result{500, "", "HTTP request failed: " + error_msg, should_retry};
    }
    
    bool should_retry = is_retryable_error(response_code, response_body);
    std::string error_msg = (response_code >= 400) ? response_body : "";
    
    return Result{response_code, response_body, error_msg, should_retry};
}
#endif

std::string HttpClient::build_url(const std::string& endpoint) const {
    std::string url = config_.base_url;
    
    // Validate and construct URL safely
    if (!endpoint.empty()) {
        if (url.back() == '/' && endpoint.front() == '/') {
            url.pop_back();
        } else if (url.back() != '/' && endpoint.front() != '/') {
            url += '/';
        }
        url += endpoint;
    }
    
    // Final validation
    if (!is_valid_url(url)) {
        throw std::invalid_argument("Constructed URL is invalid");
    }
    
    return url;
}

void HttpClient::update_config(const Config& new_config) {
    if (!is_valid_url(new_config.base_url)) {
        throw std::invalid_argument("Invalid base URL in new configuration");
    }
    
    if (new_config.timeout_seconds <= 0 || new_config.timeout_seconds > 300) {
        throw std::invalid_argument("Invalid timeout in new configuration");
    }
    
    config_ = new_config;
    LOG_INFO_F("HttpClient configuration updated, base URL: %s", config_.base_url.c_str());
}
