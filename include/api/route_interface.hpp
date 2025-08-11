/**
 * @file route_interface.hpp
 * @brief Core functionality for route interface
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_REST_API_ROUTE_INTERFACE_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_REST_API_ROUTE_INTERFACE_HPP_INCLUDED

#include "../export.hpp"
#include <string>

#ifdef _WIN32
#include <winsock2.h>
using SocketType = SOCKET;
#else
#include <sys/socket.h>
#include <unistd.h>
using SocketType = int;
#endif

namespace kolosal::api {

/**
 * @brief Base interface for HTTP route handlers
 * 
 * This is a standalone route interface that doesn't depend on kolosal-server.
 * It provides the basic functionality needed for handling HTTP requests.
 */
class KOLOSAL_AGENT_API IRoute {
public:
    /**
     * @brief Check if this route should handle the request
     * @param method HTTP method (GET, POST, PUT, DELETE)
     * @param path Request path
     * @return true if this route handles the request
     */
    virtual bool match(const std::string& method, const std::string& path) = 0;
    
    /**
     * @brief Handle the HTTP request
     * @param sock Socket for sending response
     * @param body Request body (for POST/PUT requests)
     */
    virtual void handle(SocketType sock, const std::string& body) = 0;
    
    virtual ~IRoute() = default;
};

} // namespace kolosal::api

#endif // KOLOSAL_AGENT_INCLUDE_REST_API_ROUTE_INTERFACE_HPP_INCLUDED
