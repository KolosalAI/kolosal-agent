/**
 * @file simple_http_server.hpp
 * @brief Core functionality for simple http server
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_REST_API_SIMPLE_HTTP_SERVER_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_REST_API_SIMPLE_HTTP_SERVER_HPP_INCLUDED

#include "../export.hpp"
#include "route_interface.hpp"
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using SocketType = SOCKET;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using SocketType = int;
#endif

namespace kolosal::api {

/**
 * @brief Simple HTTP server for the agent system
 * 
 * This is a lightweight HTTP server that can host agent management APIs
 * without depending on external server components.
 */
class KOLOSAL_AGENT_API SimpleHttpServer {
public:
    struct ServerConfig {
        std::string host = "127.0.0.1";
        int port = 8080;
        int backlog = 10;
        bool enable_cors = true;
    };

    explicit SimpleHttpServer(const ServerConfig& configuration = ServerConfig {});
    ~SimpleHttpServer();

    // Server lifecycle
    bool start();
    void stop();
    bool is_Running() const;

    // Route management
    void add_Route(std::shared_ptr<IRoute> route);
    void remove_Route(std::shared_ptr<IRoute> route);

private:
    ServerConfig config_;
    std::atomic<bool> running_ {false};
    std::thread server_thread_;
    SocketType listen_socket_;
    std::vector<std::shared_ptr<IRoute>> routes_;

    // Internal methods
    void server_Loop();
    void handle_Client(SocketType client_socket);
    bool initialize_Socket();
    void cleanup_Socket();
    std::pair<std::string, std::string> parseHttp_Request(const std::string& request);
    std::string getRequest_Body(const std::string& request);
    void send_Response(SocketType sock, const std::string& response);
    void sendNotFound_Response(SocketType sock);
    void sendMethodNotAllowed_Response(SocketType sock);
};

} // namespace kolosal::api

#endif // KOLOSAL_AGENT_INCLUDE_REST_API_SIMPLE_HTTP_SERVER_HPP_INCLUDED
