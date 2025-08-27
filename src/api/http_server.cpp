#include "../include/http_server.hpp"
#include <iostream>
#include <sstream>
#include <regex>
#include <thread>
#include <algorithm>
#include <ctime>

#ifdef _WIN32
static bool winsock_initialized = false;

void init_winsock() {
    if (!winsock_initialized) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Failed to initialize Winsock");
        }
        winsock_initialized = true;
    }
}

void cleanup_winsock() {
    if (winsock_initialized) {
        WSACleanup();
        winsock_initialized = false;
    }
}
#else
void init_winsock() {}
void cleanup_winsock() {}
#endif

HTTPServer::HTTPServer(std::shared_ptr<AgentManager> agent_manager, 
                       const std::string& host, 
                       int port)
    : agent_manager_(agent_manager), host_(host), port_(port), server_socket_(INVALID_SOCKET) {
    init_winsock();
}

HTTPServer::~HTTPServer() {
    stop();
    cleanup_winsock();
}

bool HTTPServer::start() {
    if (running_.load()) {
        return true;
    }
    
    // Create socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        return false;
    }
    
    // Set socket options
    int opt = 1;
#ifdef _WIN32
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    
    if (host_ == "0.0.0.0" || host_ == "127.0.0.1") {
        address.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, host_.c_str(), &address.sin_addr);
    }
    
    if (bind(server_socket_, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket to " << host_ << ":" << port_ << "\n";
        closesocket(server_socket_);
        return false;
    }
    
    // Listen
    if (listen(server_socket_, 10) == SOCKET_ERROR) {
        std::cerr << "Failed to listen on socket\n";
        closesocket(server_socket_);
        return false;
    }
    
    running_.store(true);
    server_thread_ = std::thread(&HTTPServer::server_loop, this);
    
    std::cout << "HTTP Server started on " << host_ << ":" << port_ << "\n";
    std::cout << "Available endpoints:\n";
    std::cout << "  GET    /agents              - List all agents\n";
    std::cout << "  POST   /agents              - Create new agent\n";
    std::cout << "  GET    /agents/{id}         - Get agent info\n";
    std::cout << "  PUT    /agents/{id}/start   - Start agent\n";
    std::cout << "  PUT    /agents/{id}/stop    - Stop agent\n";
    std::cout << "  DELETE /agents/{id}         - Delete agent\n";
    std::cout << "  POST   /agents/{id}/execute - Execute function (with model parameter)\n";
    std::cout << "  GET    /status              - System status\n";
    std::cout << "\n";
    std::cout << "Execute function format:\n";
    std::cout << "  {\n";
    std::cout << "    \"function\": \"chat\",\n";
    std::cout << "    \"model\": \"model_name\",\n";
    std::cout << "    \"params\": {\n";
    std::cout << "      \"message\": \"your message\"\n";
    std::cout << "    }\n";
    std::cout << "  }\n";
    std::cout << "\n";
    std::cout << "Special execute functions:\n";
    std::cout << "  execute_all_tools - Run all tool functions and use results as LLM context\n";
    std::cout << "  Note: Model parameter is required for AI-powered functions\n";
    
    return true;
}

void HTTPServer::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_.store(false);
    
    if (server_socket_ != INVALID_SOCKET) {
        closesocket(server_socket_);
        server_socket_ = INVALID_SOCKET;
    }
    
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    
    std::cout << "HTTP Server stopped\n";
}

void HTTPServer::server_loop() {
    while (running_.load()) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        socket_t client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            if (running_.load()) {
                std::cerr << "Failed to accept client connection\n";
            }
            continue;
        }
        
        // Handle client in a separate thread
        std::thread(&HTTPServer::handle_client, this, client_socket).detach();
    }
}

void HTTPServer::handle_client(socket_t client_socket) {
    char buffer[4096] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        closesocket(client_socket);
        return;
    }
    
    std::string request(buffer, bytes_received);
    std::string method, path, body;
    
    try {
        parse_http_request(request, method, path, body);
        
        std::cout << "[HTTP] " << method << " " << path << "\n";
        
        // Route handling
        if (path == "/agents" && method == "GET") {
            handle_list_agents(client_socket);
        } else if (path == "/agents" && method == "POST") {
            handle_create_agent(client_socket, body);
        } else if (path.find("/agents/") == 0 && method == "GET") {
            std::string agent_id = extract_path_parameter(path, "/agents/");
            if (!agent_id.empty() && path == "/agents/" + agent_id) {
                handle_get_agent(client_socket, agent_id);
            }
        } else if (path.find("/agents/") == 0 && path.find("/start") != std::string::npos && method == "PUT") {
            std::string agent_id = extract_path_parameter(path, "/agents/");
            agent_id = agent_id.substr(0, agent_id.length() - 6); // Remove "/start"
            handle_start_agent(client_socket, agent_id);
        } else if (path.find("/agents/") == 0 && path.find("/stop") != std::string::npos && method == "PUT") {
            std::string agent_id = extract_path_parameter(path, "/agents/");
            agent_id = agent_id.substr(0, agent_id.length() - 5); // Remove "/stop"
            handle_stop_agent(client_socket, agent_id);
        } else if (path.find("/agents/") == 0 && path.find("/execute") != std::string::npos && method == "POST") {
            std::string agent_id = extract_path_parameter(path, "/agents/");
            agent_id = agent_id.substr(0, agent_id.length() - 8); // Remove "/execute"
            handle_execute_function(client_socket, agent_id, body);
        } else if (path.find("/agents/") == 0 && method == "DELETE") {
            std::string agent_id = extract_path_parameter(path, "/agents/");
            handle_delete_agent(client_socket, agent_id);
        } else if (path == "/status" && method == "GET") {
            handle_system_status(client_socket);
        } else {
            send_error(client_socket, 404, "Not Found");
        }
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
    
    closesocket(client_socket);
}

std::string HTTPServer::parse_http_request(const std::string& request, std::string& method, std::string& path, std::string& body) {
    std::istringstream stream(request);
    std::string line;
    
    // Parse request line
    if (std::getline(stream, line)) {
        std::istringstream request_line(line);
        request_line >> method >> path;
    }
    
    // Skip headers and find body
    bool in_body = false;
    while (std::getline(stream, line)) {
        if (line.empty() || line == "\r") {
            in_body = true;
            continue;
        }
        if (in_body) {
            body += line;
        }
    }
    
    return "";
}

void HTTPServer::send_response(socket_t client_socket, int status_code, const std::string& body, const std::string& content_type) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " OK\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type\r\n";
    response << "\r\n";
    response << body;
    
    std::string response_str = response.str();
    send(client_socket, response_str.c_str(), response_str.length(), 0);
}

void HTTPServer::send_error(socket_t client_socket, int status_code, const std::string& message) {
    json error_response;
    error_response["error"] = message;
    error_response["status_code"] = status_code;
    
    send_response(client_socket, status_code, error_response.dump(2));
}

void HTTPServer::handle_list_agents(socket_t client_socket) {
    try {
        json response = agent_manager_->list_agents();
        send_response(client_socket, 200, response.dump(2));
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_create_agent(socket_t client_socket, const std::string& body) {
    try {
        json request = json::parse(body);
        
        std::string name = request.value("name", "");
        std::vector<std::string> capabilities;
        
        if (request.contains("capabilities") && request["capabilities"].is_array()) {
            for (const auto& cap : request["capabilities"]) {
                if (cap.is_string()) {
                    capabilities.push_back(cap.get<std::string>());
                }
            }
        }
        
        std::string agent_id = agent_manager_->create_agent(name, capabilities);
        
        json response;
        response["agent_id"] = agent_id;
        response["message"] = "Agent created successfully";
        
        send_response(client_socket, 201, response.dump(2));
    } catch (const std::exception& e) {
        send_error(client_socket, 400, e.what());
    }
}

void HTTPServer::handle_get_agent(socket_t client_socket, const std::string& agent_id) {
    try {
        auto agent = agent_manager_->get_agent(agent_id);
        if (!agent) {
            send_error(client_socket, 404, "Agent not found");
            return;
        }
        
        json response = agent->get_info();
        send_response(client_socket, 200, response.dump(2));
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_start_agent(socket_t client_socket, const std::string& agent_id) {
    try {
        bool success = agent_manager_->start_agent(agent_id);
        if (!success) {
            send_error(client_socket, 404, "Agent not found");
            return;
        }
        
        json response;
        response["message"] = "Agent started successfully";
        response["agent_id"] = agent_id;
        
        send_response(client_socket, 200, response.dump(2));
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_stop_agent(socket_t client_socket, const std::string& agent_id) {
    try {
        agent_manager_->stop_agent(agent_id);
        
        json response;
        response["message"] = "Agent stopped successfully";
        response["agent_id"] = agent_id;
        
        send_response(client_socket, 200, response.dump(2));
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_delete_agent(socket_t client_socket, const std::string& agent_id) {
    try {
        bool success = agent_manager_->delete_agent(agent_id);
        if (!success) {
            send_error(client_socket, 404, "Agent not found");
            return;
        }
        
        json response;
        response["message"] = "Agent deleted successfully";
        response["agent_id"] = agent_id;
        
        send_response(client_socket, 200, response.dump(2));
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_execute_function(socket_t client_socket, const std::string& agent_id, const std::string& body) {
    try {
        json request = json::parse(body);
        
        std::string function_name = request.value("function", "");
        json params = request.value("params", json::object());
        std::string model = request.value("model", "");
        
        if (function_name.empty()) {
            send_error(client_socket, 400, "Missing 'function' parameter");
            return;
        }
        
        // Add model parameter to the function parameters if provided
        if (!model.empty()) {
            params["model"] = model;
        }
        
        // Check if this is a special "execute_all_tools" request
        if (function_name == "execute_all_tools") {
            handle_execute_all_tools(client_socket, agent_id, params);
            return;
        }
        
        json result = agent_manager_->execute_agent_function(agent_id, function_name, params);
        
        json response;
        response["result"] = result;
        response["agent_id"] = agent_id;
        response["function"] = function_name;
        if (!model.empty()) {
            response["model"] = model;
        }
        
        send_response(client_socket, 200, response.dump(2));
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_execute_all_tools(socket_t client_socket, const std::string& agent_id, const json& params) {
    try {
        auto agent = agent_manager_->get_agent(agent_id);
        if (!agent) {
            send_error(client_socket, 404, "Agent not found");
            return;
        }
        
        // Get agent info to find all available functions
        json agent_info = agent->get_info();
        json available_functions = agent_info.value("functions", json::array());
        
        // Filter out basic functions to focus on tool functions
        std::vector<std::string> tool_functions;
        std::vector<std::string> exclude_functions = {"chat", "echo", "status"};
        
        for (const auto& func : available_functions) {
            std::string func_name = func.get<std::string>();
            if (std::find(exclude_functions.begin(), exclude_functions.end(), func_name) == exclude_functions.end()) {
                tool_functions.push_back(func_name);
            }
        }
        
        // Execute all tool functions and collect results
        json tool_results = json::object();
        json execution_log = json::array();
        std::string user_message = params.value("message", "");
        std::string user_query = params.value("query", user_message);
        
        for (const std::string& func_name : tool_functions) {
            try {
                json func_params;
                
                // Customize parameters based on function type
                if (func_name == "analyze") {
                    func_params["text"] = user_query;
                } else if (func_name == "search_documents" || func_name == "internet_search" || func_name == "research") {
                    func_params["query"] = user_query;
                } else if (func_name == "list_documents") {
                    // No parameters needed for listing
                } else {
                    // For other functions, pass the user query or message
                    func_params = params;
                }
                
                // Execute the function
                json result = agent->execute_function(func_name, func_params);
                tool_results[func_name] = result;
                
                json log_entry;
                log_entry["function"] = func_name;
                log_entry["status"] = "success";
                log_entry["result_summary"] = result.size() > 0 ? "Data retrieved" : "No data";
                execution_log.push_back(log_entry);
                
            } catch (const std::exception& e) {
                // Continue with other functions even if one fails
                json error_result;
                error_result["error"] = e.what();
                error_result["status"] = "failed";
                tool_results[func_name] = error_result;
                
                json log_entry;
                log_entry["function"] = func_name;
                log_entry["status"] = "failed";
                log_entry["error"] = e.what();
                execution_log.push_back(log_entry);
            }
        }
        
        // Create context for LLM
        std::string context = "Tool execution results for query: \"" + user_query + "\"\n\n";
        for (const auto& [func_name, result] : tool_results.items()) {
            context += "=== " + func_name + " ===\n";
            context += result.dump(2) + "\n\n";
        }
        
        // Execute chat function with the accumulated context
        json chat_params;
        chat_params["message"] = user_message;
        chat_params["context"] = context;
        chat_params["tool_results"] = tool_results;
        
        // Pass model parameter if provided
        if (params.contains("model") && !params["model"].empty()) {
            chat_params["model"] = params["model"];
        }
        
        json chat_result;
        try {
            chat_result = agent->execute_function("chat", chat_params);
        } catch (const std::exception& e) {
            // Fallback response if chat function fails
            chat_result["agent"] = agent->get_name();
            chat_result["response"] = "I executed " + std::to_string(tool_functions.size()) + 
                                    " tool functions for your query. Here's a summary of the results: " + context;
            chat_result["timestamp"] = std::to_string(std::time(nullptr));
            chat_result["error"] = e.what();
        }
        
        // Build comprehensive response
        json response;
        response["agent_id"] = agent_id;
        response["function"] = "execute_all_tools";
        response["user_query"] = user_query;
        response["tools_executed"] = tool_functions;
        response["execution_log"] = execution_log;
        response["tool_results"] = tool_results;
        response["context"] = context;
        response["llm_response"] = chat_result;
        response["summary"] = {
            {"total_tools", tool_functions.size()},
            {"successful", std::count_if(execution_log.begin(), execution_log.end(), 
                [](const json& entry) { return entry.value("status", "") == "success"; })},
            {"failed", std::count_if(execution_log.begin(), execution_log.end(), 
                [](const json& entry) { return entry.value("status", "") == "failed"; })}
        };
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_system_status(socket_t client_socket) {
    try {
        json agents_info = agent_manager_->list_agents();
        
        json response;
        response["system"] = "Kolosal Agent System";
        response["version"] = "1.0.0";
        response["status"] = "running";
        response["agents"] = agents_info;
        response["server"] = {
            {"host", host_},
            {"port", port_},
            {"running", running_.load()}
        };
        
        send_response(client_socket, 200, response.dump(2));
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

std::string HTTPServer::extract_path_parameter(const std::string& path, const std::string& prefix) {
    if (path.find(prefix) == 0) {
        return path.substr(prefix.length());
    }
    return "";
}
