#pragma once
#include <string>
#include <map>
#include <functional>
#include <vector>

/**
 * @brief Simple HTTP client stub for making requests
 */
class HttpClient {
public:
    HttpClient() = default;
    static HttpClient& getInstance();
    
    bool makeStreamingRequest(
        const std::string& url, 
        const std::string& body, 
        const std::map<std::string, std::string>& headers,
        std::function<bool(const std::string&)> callback);
    
    bool get(const std::string& url, std::string& response);
    bool get(const std::string& url, std::string& response, const std::vector<std::string>& headers);
    bool post(const std::string& url, const std::string& body, std::string& response);
    bool post(const std::string& url, const std::string& body, std::string& response, const std::vector<std::string>& headers);
    bool deleteRequest(const std::string& url, const std::vector<std::string>& headers);
    bool put(const std::string& url, const std::string& body, std::string& response, const std::vector<std::string>& headers);

private:
    // Private members if needed
};
