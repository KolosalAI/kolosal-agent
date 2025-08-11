/**
 * @file http_client.hpp
 * @brief Core functionality for http client
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_ROUTING_HTTP_CLIENT_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_ROUTING_HTTP_CLIENT_HPP_INCLUDED
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
    static HttpClient& get_Instance();
    
    bool makeStreaming_Request(
        const std::string& url, 
        const std::string& body, 
        const std::map<std::string, std::string>& headers,
        std::function<bool(const std::string&)> callback);
    
    bool get(const std::string& url, std::string& response);
    bool get(const std::string& url, std::string& response, const std::vector<std::string>& headers);
    bool post(const std::string& url, const std::string& body, std::string& response);
    bool post(const std::string& url, const std::string& body, std::string& response, const std::vector<std::string>& headers);
    bool delete_Request(const std::string& url, const std::vector<std::string>& headers);
    bool put(const std::string& url, const std::string& body, std::string& response, const std::vector<std::string>& headers);

private:
    // Private members if needed
};

#endif // KOLOSAL_AGENT_INCLUDE_ROUTING_HTTP_CLIENT_HPP_INCLUDED
