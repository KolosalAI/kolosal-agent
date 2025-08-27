#pragma once

#include "agent_manager.hpp"
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <json.hpp>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

using json = nlohmann::json;

/**
 * @brief HTTP Server for Agent API
 */
class HTTPServer {
private:
    std::shared_ptr<AgentManager> agent_manager_;
    std::string host_;
    int port_;
    socket_t server_socket_;
    std::atomic<bool> running_{false};
    std::thread server_thread_;
    
    // HTTP handling
    void server_loop();
    void handle_client(socket_t client_socket);
    std::string parse_http_request(const std::string& request, std::string& method, std::string& path, std::string& body);
    void send_response(socket_t client_socket, int status_code, const std::string& body, const std::string& content_type = "application/json");
    void send_error(socket_t client_socket, int status_code, const std::string& message);
    
    // Route handlers
    void handle_list_agents(socket_t client_socket);
    void handle_create_agent(socket_t client_socket, const std::string& body);
    void handle_get_agent(socket_t client_socket, const std::string& agent_id);
    void handle_start_agent(socket_t client_socket, const std::string& agent_id);
    void handle_stop_agent(socket_t client_socket, const std::string& agent_id);
    void handle_delete_agent(socket_t client_socket, const std::string& agent_id);
    void handle_execute_function(socket_t client_socket, const std::string& agent_id, const std::string& body);
    void handle_execute_all_tools(socket_t client_socket, const std::string& agent_id, const json& params);
    void handle_system_status(socket_t client_socket);
    
    // Utility
    std::string extract_path_parameter(const std::string& path, const std::string& prefix);
    
public:
    HTTPServer(std::shared_ptr<AgentManager> agent_manager, 
               const std::string& host = "127.0.0.1", 
               int port = 8080);
    ~HTTPServer();
    
    bool start();
    void stop();
    bool is_running() const { return running_.load(); }
    
    const std::string& get_host() const { return host_; }
    int get_port() const { return port_; }
};
