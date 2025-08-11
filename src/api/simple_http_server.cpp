/**
 * @file simple_http_server.cpp
 * @brief Core functionality for simple http server
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "api/simple_http_server.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace kolosal::api {

SimpleHttpServer::SimpleHttpServer(const ServerConfig& configuration)
    : config_(configuration) {
    
#ifdef _WIN32
    listen_socket_ = INVALID_SOCKET;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize WinSock" << std::endl;
    }
#else
    listen_socket_ = -1;
#endif
}

SimpleHttpServer::~SimpleHttpServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool SimpleHttpServer::start() {
    if (running_.load()) {
        return true;
    }

    if (!initialize_Socket()) {
        return false;
    }

    running_ = true;
    server_thread_ = std::thread(&SimpleHttpServer::server_Loop, this);
    
    std::cout << "🌐 Agent HTTP Server started on " << config_.host << ":" << config_.port << std::endl;
    return true;
}

void SimpleHttpServer::stop() {
    if (!running_.load()) {
        return;
    }

    running_ = false;
    cleanup_Socket();

    if (server_thread_.joinable()) {
        server_thread_.join();
    }

    std::cout << "🛑 Agent HTTP Server stopped" << std::endl;
}

bool SimpleHttpServer::is_Running() const {
    return running_.load();
}

void SimpleHttpServer::add_Route(std::shared_ptr<IRoute> route) {
    routes_.push_back(route);
}

void SimpleHttpServer::remove_Route(std::shared_ptr<IRoute> route) {
    routes_.erase(
        std::remove(routes_.begin(), routes_.end(), route),
        routes_.end()
    );
}

bool SimpleHttpServer::initialize_Socket() {
    listen_socket_ = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (listen_socket_ == INVALID_SOCKET) {
#else
    if (listen_socket_ < 0) {
#endif
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    // Enable socket reuse
    int opt = 1;
#ifdef _WIN32
    setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, 
               reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(config_.host.c_str());
    address.sin_port = htons(config_.port);

#ifdef _WIN32
    if (bind(listen_socket_, reinterpret_cast<struct sockaddr*>(&address), 
             sizeof(address)) == SOCKET_ERROR) {
#else
    if (bind(listen_socket_, reinterpret_cast<struct sockaddr*>(&address), 
             sizeof(address)) < 0) {
#endif
        std::cerr << "Failed to bind socket to " << config_.host << ":" << config_.port << std::endl;
        cleanup_Socket();
        return false;
    }

    // Listen for connections
#ifdef _WIN32
    if (listen(listen_socket_, config_.backlog) == SOCKET_ERROR) {
#else
    if (listen(listen_socket_, config_.backlog) < 0) {
#endif
        std::cerr << "Failed to listen on socket" << std::endl;
        cleanup_Socket();
        return false;
    }

    return true;
}

void SimpleHttpServer::cleanup_Socket() {
#ifdef _WIN32
    if (listen_socket_ != INVALID_SOCKET) {
        closesocket(listen_socket_);
        listen_socket_ = INVALID_SOCKET;
    }
#else
    if (listen_socket_ >= 0) {
        close(listen_socket_);
        listen_socket_ = -1;
    }
#endif
}

void SimpleHttpServer::server_Loop() {
    while (running_.load()) {
        struct sockaddr_in client_addr;
#ifdef _WIN32
        int client_len = sizeof(client_addr);
#else
        socklen_t client_len = sizeof(client_addr);
#endif
        SocketType client_socket = accept(listen_socket_, 
                                        reinterpret_cast<struct sockaddr*>(&client_addr), 
                                        &client_len);
#ifdef _WIN32
        if (client_socket == INVALID_SOCKET) {
#else
        if (client_socket < 0) {
#endif
            if (running_.load()) {
                std::cerr << "Failed to accept client connection" << std::endl;
            }
            continue;
        }

        // Handle client in a separate thread for better performance
        std::thread client_thread(&SimpleHttpServer::handle_Client, this, client_socket);
        client_thread.detach();
    }
}

void SimpleHttpServer::handle_Client(SocketType client_socket) {
    constexpr size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    
    // Read request
#ifdef _WIN32
    int bytes_received = recv(client_socket, buffer, static_cast<int>(BUFFER_SIZE - 1), 0);
#else
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
#endif
    if (bytes_received <= 0) {
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
        return;
    }
    
    buffer[bytes_received] = '\0';
    std::string request(buffer);
    
    // Parse HTTP request
    auto [method, path] = parseHttp_Request(request);
    std::string body = getRequest_Body(request);
    
    // Find matching route
    bool route_found = false;
    for (auto& route : routes_) {
        if (route->match(method, path)) {
            route_found = true;
            try {
                route->handle(client_socket, body);
            } catch (const std::exception& e) {
                std::cerr << "Error handling route: " << e.what() << std::endl;
                send_Response(client_socket, 
                    "HTTP/1.1 500 Internal Server Error\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: 54\r\n"
                    "\r\n"
                    "{\"error\":{\"type\":\"Internal Server Error\",\"code\":500}}"
                );
            }
            break;
        }
    }
    
    if (!route_found) {
        if (method == "OPTIONS" && config_.enable_cors) {
            // Handle CORS preflight
            send_Response(client_socket, 
                "HTTP/1.1 200 OK\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
                "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
                "Content-Length: 0\r\n"
                "\r\n"
            );
        } else {
            sendNotFound_Response(client_socket);
        }
    }
    
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

std::pair<std::string, std::string> SimpleHttpServer::parseHttp_Request(const std::string& request) {
    std::istringstream iss(request);
    std::string method, path, version;
    
    if (iss >> method >> path >> version) {
        return {method, path};
    }
    
    return {"", ""};
}

std::string SimpleHttpServer::getRequest_Body(const std::string& request) {
    const size_t body_start = request.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        return request.substr(body_start + 4);
    }
    return "";
}

void SimpleHttpServer::send_Response(SocketType sock, const std::string& response) {
    send(sock, response.c_str(), response.length(), 0);
}

void SimpleHttpServer::sendNotFound_Response(SocketType sock) {
    send_Response(sock, 
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 46\r\n"
        "\r\n"
        "{\"error\":{\"type\":\"Not Found\",\"code\":404}}"
    );
}

void SimpleHttpServer::sendMethodNotAllowed_Response(SocketType sock) {
    send_Response(sock, 
        "HTTP/1.1 405 Method Not Allowed\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 57\r\n"
        "\r\n"
        "{\"error\":{\"type\":\"Method Not Allowed\",\"code\":405}}"
    );
}

} // namespace kolosal::api
