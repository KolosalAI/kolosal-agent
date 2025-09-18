#include "../include/server_http.hpp"
#include <iostream>
#include <sstream>
#include <regex>
#include <thread>
#include <algorithm>
#include <ctime>
#include <mutex>

#ifdef _WIN32
static bool winsock_initialized = false;
static std::mutex winsock_mutex;

void init_winsock() {
    std::lock_guard<std::mutex> lock(winsock_mutex);
    if (!winsock_initialized) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Failed to initialize Winsock");
        }
        winsock_initialized = true;
    }
}

void cleanup_winsock() {
    std::lock_guard<std::mutex> lock(winsock_mutex);
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

HTTPServer::HTTPServer(std::shared_ptr<AgentManager> agent_manager,
                       std::shared_ptr<WorkflowManager> workflow_manager,
                       std::shared_ptr<WorkflowOrchestrator> workflow_orchestrator,
                       const std::string& host, 
                       int port)
    : agent_manager_(agent_manager), workflow_manager_(workflow_manager), 
      workflow_orchestrator_(workflow_orchestrator), host_(host), port_(port), 
      server_socket_(INVALID_SOCKET) {
    init_winsock();
}

HTTPServer::~HTTPServer() {
    try {
        stop();
        cleanup_winsock();
    } catch (...) {
        // Suppress exceptions in destructor to prevent abort()
    }
}

bool HTTPServer::start() {
    if (running_.load()) {
        return true;
    }
    
    // Validate port number
    if (port_ < 1 || port_ > 65535) {
        std::cerr << "Invalid port number: " << port_ << ". Port must be between 1 and 65535\n";
        return false;
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
    std::cout << "  GET    /agents                    - List all agents\n";
    std::cout << "  POST   /agents                    - Create new agent\n";
    std::cout << "  GET    /agents/{id_or_name}       - Get agent info\n";
    std::cout << "  PUT    /agents/{id_or_name}/start - Start agent\n";
    std::cout << "  PUT    /agents/{id_or_name}/stop  - Stop agent\n";
    std::cout << "  DELETE /agents/{id_or_name}       - Delete agent\n";
    std::cout << "  POST   /agents/{id_or_name}/execute - Execute function (with model parameter)\n";
    std::cout << "  POST   /agent/execute            - Simple agent execute (query + context)\n";
    std::cout << "  GET    /status                    - System status\n";
    std::cout << "\n";
    std::cout << "Note: {id_or_name} can be either the agent's UUID or its human-readable name\n";
    
    if (workflow_orchestrator_) {
        std::cout << "\nWorkflow Orchestration endpoints:\n";
        std::cout << "  GET    /workflows             - List workflow definitions\n";
        std::cout << "  POST   /workflows             - Register workflow definition\n";
        std::cout << "  GET    /workflows/{id}        - Get workflow definition\n";
        std::cout << "  PUT    /workflows/{id}        - Update workflow definition\n";
        std::cout << "  DELETE /workflows/{id}        - Delete workflow definition\n";
        std::cout << "  POST   /workflows/{id}/execute     - Execute workflow\n";
        std::cout << "  GET    /workflows/executions/{id} - Get execution status\n";
        std::cout << "  PUT    /workflows/executions/{id}/{action} - Control execution (pause/resume/cancel)\n";
        std::cout << "  GET    /workflows/executions  - List workflow executions\n";
        std::cout << "  GET    /workflow_templates    - List built-in workflow templates\n";
        std::cout << "  POST   /workflow_templates/{id}/execute - Execute workflow template\n";
        std::cout << "  GET    /workflow_executions/{id}/progress - Get detailed execution progress\n";
        std::cout << "  GET    /workflow_executions/{id}/logs - Get execution logs\n";
    }
    std::cout << "\n";
    
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
        try {
            server_thread_.join();
        } catch (const std::exception& e) {
            std::cerr << "Warning: Exception during server thread join: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Warning: Unknown exception during server thread join" << std::endl;
        }
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
        
        // Log the incoming request
        std::cout << "[HTTP] " << method << " " << path << "\n";
        std::cout << "[HTTP-DEBUG] Processing request from client, body size: " << body.size() << " bytes\n";
        
        // Route handling with detailed logging
        bool route_matched = false;
        
        if (path == "/agents" && method == "GET") {
            std::cout << "[HTTP-ROUTE] Matched route: List agents\n";
            route_matched = true;
            handle_list_agents(client_socket);
        } else if (path == "/agents" && method == "POST") {
            std::cout << "[HTTP-ROUTE] Matched route: Create agent\n";
            route_matched = true;
            handle_create_agent(client_socket, body);
        } else if (path.find("/agents/") == 0 && method == "GET") {
            std::string agent_identifier = extract_path_parameter(path, "/agents/");
            if (!agent_identifier.empty() && path == "/agents/" + agent_identifier) {
                std::string agent_id = resolve_agent_identifier(agent_identifier);
                if (!agent_id.empty()) {
                    std::cout << "[HTTP-ROUTE] Matched route: Get agent details for identifier: " << agent_identifier << " (resolved to ID: " << agent_id << ")\n";
                    route_matched = true;
                    handle_get_agent(client_socket, agent_id);
                } else {
                    send_agent_not_found_error(client_socket, agent_identifier, "/agents/Assistant");
                    route_matched = true;
                }
            }
        } else if (path.find("/agents/") == 0 && path.find("/start") != std::string::npos && method == "PUT") {
            std::string agent_identifier = extract_path_parameter(path, "/agents/");
            agent_identifier = agent_identifier.substr(0, agent_identifier.length() - 6); // Remove "/start"
            std::string agent_id = resolve_agent_identifier(agent_identifier);
            if (!agent_id.empty()) {
                std::cout << "[HTTP-ROUTE] Matched route: Start agent identifier: " << agent_identifier << " (resolved to ID: " << agent_id << ")\n";
                route_matched = true;
                handle_start_agent(client_socket, agent_id);
            } else {
                send_agent_not_found_error(client_socket, agent_identifier, "/agents/Assistant/start");
                route_matched = true;
            }
        } else if (path.find("/agents/") == 0 && path.find("/stop") != std::string::npos && method == "PUT") {
            std::string agent_identifier = extract_path_parameter(path, "/agents/");
            agent_identifier = agent_identifier.substr(0, agent_identifier.length() - 5); // Remove "/stop"
            std::string agent_id = resolve_agent_identifier(agent_identifier);
            if (!agent_id.empty()) {
                std::cout << "[HTTP-ROUTE] Matched route: Stop agent identifier: " << agent_identifier << " (resolved to ID: " << agent_id << ")\n";
                route_matched = true;
                handle_stop_agent(client_socket, agent_id);
            } else {
                send_agent_not_found_error(client_socket, agent_identifier, "/agents/Assistant/stop");
                route_matched = true;
            }
        } else if (path.find("/agents/") == 0 && path.find("/execute") != std::string::npos && method == "POST") {
            std::string agent_identifier = extract_path_parameter(path, "/agents/");
            agent_identifier = agent_identifier.substr(0, agent_identifier.length() - 8); // Remove "/execute"
            std::string agent_id = resolve_agent_identifier(agent_identifier);
            if (!agent_id.empty()) {
                std::cout << "[HTTP-ROUTE] Matched route: Execute function for agent identifier: " << agent_identifier << " (resolved to ID: " << agent_id << ")\n";
                route_matched = true;
                handle_execute_function(client_socket, agent_id, body);
            } else {
                send_agent_not_found_error(client_socket, agent_identifier, "/agents/Assistant/execute");
                route_matched = true;
            }
        } else if (path == "/agent/execute" && method == "POST") {
            std::cout << "[HTTP-ROUTE] Matched route: Simple agent execute endpoint\n";
            route_matched = true;
            handle_simple_agent_execute(client_socket, body);
        } else if (path.find("/agents/") == 0 && method == "DELETE") {
            std::string agent_identifier = extract_path_parameter(path, "/agents/");
            std::string agent_id = resolve_agent_identifier(agent_identifier);
            if (!agent_id.empty()) {
                std::cout << "[HTTP-ROUTE] Matched route: Delete agent identifier: " << agent_identifier << " (resolved to ID: " << agent_id << ")\n";
                route_matched = true;
                handle_delete_agent(client_socket, agent_id);
            } else {
                send_agent_not_found_error(client_socket, agent_identifier, "/agents/Assistant");
                route_matched = true;
            }
        } else if (path == "/status" && method == "GET") {
            std::cout << "[HTTP-ROUTE] Matched route: System status\n";
            route_matched = true;
            handle_system_status(client_socket);
        }
        // Kolosal Server Management routes
        else if (path == "/kolosal-server/start" && method == "POST") {
        }
        // Workflow Orchestration routes
        else if (workflow_orchestrator_ && path == "/workflows" && method == "GET") {
            std::cout << "[HTTP-ROUTE] Matched route: List workflows\n";
            route_matched = true;
            handle_list_workflows(client_socket);
        } else if (workflow_orchestrator_ && path == "/workflows" && method == "POST") {
            std::cout << "[HTTP-ROUTE] Matched route: Register new workflow\n";
            route_matched = true;
            handle_register_workflow(client_socket, body);
        } else if (workflow_orchestrator_ && path.find("/workflows/") == 0 && path.find("/execute") != std::string::npos && method == "POST") {
            std::string workflow_id = extract_path_parameter(path, "/workflows/");
            workflow_id = workflow_id.substr(0, workflow_id.length() - 8); // Remove "/execute"
            std::cout << "[HTTP-ROUTE] Matched route: Execute workflow ID: " << workflow_id << "\n";
            route_matched = true;
            handle_execute_workflow(client_socket, body, workflow_id);
        } else if (workflow_orchestrator_ && path.find("/workflows/executions/") == 0 && method == "GET") {
            std::string execution_id = extract_path_parameter(path, "/workflows/executions/");
            if (execution_id.find("/") != std::string::npos) {
                // This is a control action like /workflows/executions/{id}/pause
                size_t slash_pos = execution_id.find("/");
                std::string action = execution_id.substr(slash_pos + 1);
                execution_id = execution_id.substr(0, slash_pos);
                std::cout << "[HTTP-ROUTE] Matched route: Control workflow execution ID: " << execution_id << ", action: " << action << "\n";
                route_matched = true;
                handle_control_workflow_execution(client_socket, execution_id, action);
            } else {
                std::cout << "[HTTP-ROUTE] Matched route: Get workflow execution status for ID: " << execution_id << "\n";
                route_matched = true;
                handle_get_workflow_execution(client_socket, execution_id);
            }
        } else if (workflow_orchestrator_ && path.find("/workflows/executions/") == 0 && method == "PUT") {
            std::string path_part = extract_path_parameter(path, "/workflows/executions/");
            size_t slash_pos = path_part.find("/");
            if (slash_pos != std::string::npos) {
                std::string execution_id = path_part.substr(0, slash_pos);
                std::string action = path_part.substr(slash_pos + 1);
                std::cout << "[HTTP-ROUTE] Matched route: Control workflow execution ID: " << execution_id << ", action: " << action << "\n";
                route_matched = true;
                handle_control_workflow_execution(client_socket, execution_id, action);
            }
        } else if (workflow_orchestrator_ && path == "/workflows/executions" && method == "GET") {
            std::cout << "[HTTP-ROUTE] Matched route: List workflow executions\n";
            route_matched = true;
            handle_list_workflow_executions(client_socket);
        } else if (workflow_orchestrator_ && path.find("/workflows/") == 0 && method == "GET" && path.find("/execute") == std::string::npos) {
            std::string workflow_id = extract_path_parameter(path, "/workflows/");
            std::cout << "[HTTP-ROUTE] Matched route: Get workflow definition for ID: " << workflow_id << "\n";
            route_matched = true;
            handle_get_workflow(client_socket, workflow_id);
        } else if (workflow_orchestrator_ && path.find("/workflows/") == 0 && method == "PUT") {
            std::string workflow_id = extract_path_parameter(path, "/workflows/");
            std::cout << "[HTTP-ROUTE] Matched route: Update workflow ID: " << workflow_id << "\n";
            route_matched = true;
            handle_update_workflow(client_socket, workflow_id, body);
        } else if (workflow_orchestrator_ && path.find("/workflows/") == 0 && method == "DELETE") {
            std::string workflow_id = extract_path_parameter(path, "/workflows/");
            std::cout << "[HTTP-ROUTE] Matched route: Delete workflow ID: " << workflow_id << "\n";
            route_matched = true;
            handle_delete_workflow(client_socket, workflow_id);
        } else if (workflow_orchestrator_ && path == "/workflow_templates" && method == "GET") {
            std::cout << "[HTTP-ROUTE] Matched route: List workflow templates\n";
            route_matched = true;
            handle_get_workflow_templates(client_socket);
        } else if (workflow_orchestrator_ && path.find("/workflow_templates/") == 0 && path.find("/execute") != std::string::npos && method == "POST") {
            std::string template_id = extract_path_parameter(path, "/workflow_templates/");
            template_id = template_id.substr(0, template_id.length() - 8); // Remove "/execute"
            std::cout << "[HTTP-ROUTE] Matched route: Execute workflow template ID: " << template_id << "\n";
            route_matched = true;
            handle_execute_workflow_template(client_socket, template_id, body);
        } else if (workflow_orchestrator_ && path.find("/workflow_executions/") == 0 && path.find("/progress") != std::string::npos && method == "GET") {
            std::string execution_id = extract_path_parameter(path, "/workflow_executions/");
            execution_id = execution_id.substr(0, execution_id.length() - 9); // Remove "/progress"
            std::cout << "[HTTP-ROUTE] Matched route: Get workflow execution progress for ID: " << execution_id << "\n";
            route_matched = true;
            handle_workflow_execution_progress(client_socket, execution_id);
        } else if (workflow_orchestrator_ && path.find("/workflow_executions/") == 0 && path.find("/logs") != std::string::npos && method == "GET") {
            std::string execution_id = extract_path_parameter(path, "/workflow_executions/");
            execution_id = execution_id.substr(0, execution_id.length() - 5); // Remove "/logs"
            std::cout << "[HTTP-ROUTE] Matched route: Get workflow execution logs for ID: " << execution_id << "\n";
            route_matched = true;
            handle_workflow_execution_logs(client_socket, execution_id);
        }
        
        if (!route_matched) {
            std::cout << "[HTTP-ERROR] No route found for " << method << " " << path << " - responding with 404\n";
            send_error(client_socket, 404, "Not Found");
        } else {
            std::cout << "[HTTP-COMPLETE] Request processing completed for " << method << " " << path << "\n";
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

void HTTPServer::handle_simple_agent_execute(socket_t client_socket, const std::string& body) {
    try {
        json request = json::parse(body);
        
        std::string query = request.value("query", "");
        std::string context = request.value("context", "");
        std::string model = request.value("model", "default");
        
        if (query.empty()) {
            send_error(client_socket, 400, "Missing 'query' parameter");
            return;
        }
        
        // Get the first available agent (or a specific one if agent name/id is provided)
        std::string agent_name = request.value("agent", "");
        std::string agent_id;
        
        if (!agent_name.empty()) {
            agent_id = resolve_agent_identifier(agent_name);
            if (agent_id.empty()) {
                send_error(client_socket, 404, "Specified agent not found: " + agent_name);
                return;
            }
        } else {
            // Use the first available RUNNING agent, or start one if needed
            json agents_info = agent_manager_->list_agents();
            if (agents_info.contains("agents") && !agents_info["agents"].empty()) {
                // First, try to find a running agent
                for (const auto& agent_info : agents_info["agents"]) {
                    if (agent_info.contains("running") && agent_info["running"].get<bool>()) {
                        agent_id = agent_info["id"];
                        break;
                    }
                }
                
                // If no running agent found, use the first one and start it
                if (agent_id.empty()) {
                    agent_id = agents_info["agents"][0]["id"];
                }
            }
        }
        
        if (agent_id.empty()) {
            send_error(client_socket, 404, "No agents available");
            return;
        }
        
        auto agent = agent_manager_->get_agent(agent_id);
        if (!agent) {
            send_error(client_socket, 404, "Agent not found");
            return;
        }
        
        // Ensure the agent is running
        if (!agent->is_running()) {
            bool started = agent_manager_->start_agent(agent_id);
            if (!started) {
                send_error(client_socket, 500, "Failed to start agent: " + agent->get_name());
                return;
            }
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
        
        for (const std::string& func_name : tool_functions) {
            try {
                json func_params;
                
                // Customize parameters based on function type
                if (func_name == "analyze") {
                    func_params["text"] = query;
                    if (!model.empty()) func_params["model"] = model;
                } else if (func_name.find("search") != std::string::npos || 
                          func_name == "research" || 
                          func_name == "plan_research" ||
                          func_name == "targeted_research") {
                    func_params["query"] = query;
                    if (!model.empty()) func_params["model"] = model;
                } else if (func_name == "list_documents") {
                    // No additional parameters needed for listing
                } else if (func_name.find("document") != std::string::npos && 
                          func_name != "list_documents" && 
                          func_name != "remove_document") {
                    func_params["query"] = query;
                    if (!model.empty()) func_params["model"] = model;
                } else {
                    // For other functions, pass query and context
                    func_params["query"] = query;
                    if (!context.empty()) func_params["context"] = context;
                    if (!model.empty()) func_params["model"] = model;
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
        
        // Build enhanced context for LLM
        std::string enhanced_context = context;
        if (!enhanced_context.empty()) {
            enhanced_context += "\n\n";
        }
        enhanced_context += "Tool execution results for query: \"" + query + "\"\n\n";
        
        for (const auto& [func_name, result] : tool_results.items()) {
            enhanced_context += "=== " + func_name + " ===\n";
            if (result.contains("error")) {
                enhanced_context += "Error: " + result["error"].get<std::string>() + "\n\n";
            } else {
                enhanced_context += result.dump(2) + "\n\n";
            }
        }
        
        // Execute chat function with the accumulated context
        json chat_params;
        chat_params["message"] = query;
        chat_params["context"] = enhanced_context;
        chat_params["tool_results"] = tool_results;
        chat_params["model"] = model;
        
        json llm_response;
        try {
            llm_response = agent->execute_function("chat", chat_params);
        } catch (const std::exception& e) {
            // Fallback response if chat function fails
            llm_response["agent"] = agent->get_name();
            llm_response["response"] = "I executed " + std::to_string(tool_functions.size()) + 
                                    " tool functions for your query. Here's a summary of the results: " + enhanced_context;
            llm_response["timestamp"] = std::to_string(std::time(nullptr));
            llm_response["error"] = e.what();
            llm_response["status"] = "fallback";
        }
        
        // Build comprehensive response
        json response;
        response["query"] = query;
        response["context"] = context;
        response["model"] = model;
        response["agent_id"] = agent_id;
        response["agent_name"] = agent->get_name();
        response["tools_executed"] = tool_functions;
        response["execution_log"] = execution_log;
        response["tool_responses"] = tool_results;
        response["llm_response"] = llm_response;
        response["summary"] = {
            {"total_tools", tool_functions.size()},
            {"successful", std::count_if(execution_log.begin(), execution_log.end(), 
                [](const json& entry) { return entry.value("status", "") == "success"; })},
            {"failed", std::count_if(execution_log.begin(), execution_log.end(), 
                [](const json& entry) { return entry.value("status", "") == "failed"; })}
        };
        response["timestamp"] = std::to_string(std::time(nullptr));
        
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

std::string HTTPServer::resolve_agent_identifier(const std::string& agent_identifier) {
    // First, try to get agent directly by ID
    if (agent_manager_->agent_exists(agent_identifier)) {
        return agent_identifier; // It's already a valid agent ID
    }
    
    // If not found by ID, try to get agent ID by name
    std::string agent_id = agent_manager_->get_agent_id_by_name(agent_identifier);
    if (!agent_id.empty()) {
        return agent_id; // Found agent by name, return its ID
    }
    
    // Neither ID nor name found
    return "";
}

void HTTPServer::send_agent_not_found_error(socket_t client_socket, const std::string& agent_identifier, const std::string& endpoint_example) {
    std::cout << "[HTTP-ERROR] Agent not found: " << agent_identifier << "\n";
    
    // Get list of available agents for better error message
    json available_agents = agent_manager_->list_agents();
    json error_response;
    error_response["error"] = "Agent not found: " + agent_identifier;
    error_response["status_code"] = 404;
    
    if (available_agents.contains("agents") && available_agents["agents"].is_array()) {
        json agent_info = json::array();
        for (const auto& agent : available_agents["agents"]) {
            if (agent.contains("name") && agent.contains("id")) {
                agent_info.push_back({
                    {"name", agent["name"]},
                    {"id", agent["id"]}
                });
            }
        }
        error_response["available_agents"] = agent_info;
        
        std::string suggestion = "Use agent name or one of the available IDs above";
        if (!endpoint_example.empty()) {
            suggestion = "Use agent name (e.g., " + endpoint_example + ") or one of the available IDs above";
        }
        error_response["suggestion"] = suggestion;
    }
    
    send_response(client_socket, 404, error_response.dump(2));
}

// Workflow Management Handlers
void HTTPServer::handle_submit_workflow_request(socket_t client_socket, const std::string& body) {
    try {
        json request_data = json::parse(body);
        
        if (!request_data.contains("agent_name") || !request_data.contains("function_name")) {
            send_error(client_socket, 400, "Missing required fields: agent_name, function_name");
            return;
        }
        
        std::string agent_name = request_data["agent_name"];
        std::string function_name = request_data["function_name"];
        json parameters = request_data.value("parameters", json{});
        
        std::string request_id;
        if (request_data.contains("timeout_ms")) {
            request_id = workflow_manager_->submit_request_with_timeout(
                agent_name, function_name, parameters, request_data["timeout_ms"]
            );
        } else {
            request_id = workflow_manager_->submit_request(agent_name, function_name, parameters);
        }
        
        json response;
        response["request_id"] = request_id;
        response["status"] = "submitted";
        response["agent_name"] = agent_name;
        response["function_name"] = function_name;
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_get_request_status(socket_t client_socket, const std::string& request_id) {
    try {
        auto request_status = workflow_manager_->get_request_status(request_id);
        if (!request_status) {
            send_error(client_socket, 404, "Request not found");
            return;
        }
        
        json response = WorkflowUtils::request_to_json(*request_status);
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_cancel_request(socket_t client_socket, const std::string& request_id) {
    try {
        bool cancelled = workflow_manager_->cancel_request(request_id);
        
        json response;
        response["request_id"] = request_id;
        response["cancelled"] = cancelled;
        response["message"] = cancelled ? "Request cancelled successfully" : "Request not found or cannot be cancelled";
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_list_workflow_requests(socket_t client_socket) {
    try {
        json active_requests = workflow_manager_->list_active_requests();
        json recent_requests = workflow_manager_->list_recent_requests(50);
        
        json response;
        response["active_requests"] = active_requests;
        response["recent_requests"] = recent_requests;
        response["total_active"] = active_requests.size();
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_workflow_system_status(socket_t client_socket) {
    try {
        json system_status = workflow_manager_->get_system_status();
        send_response(client_socket, 200, system_status.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

// Workflow Orchestration Handlers
void HTTPServer::handle_list_workflows(socket_t client_socket) {
    try {
        auto workflows = workflow_orchestrator_->list_workflows();
        
        json response = json::array();
        for (const auto& workflow : workflows) {
            json workflow_info;
            workflow_info["id"] = workflow.id;
            workflow_info["name"] = workflow.name;
            workflow_info["description"] = workflow.description;
            workflow_info["type"] = static_cast<int>(workflow.type);
            workflow_info["step_count"] = workflow.steps.size();
            workflow_info["max_execution_time_ms"] = workflow.max_execution_time_ms;
            workflow_info["allow_partial_failure"] = workflow.allow_partial_failure;
            response.push_back(workflow_info);
        }
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_register_workflow(socket_t client_socket, const std::string& body) {
    try {
        json workflow_data = json::parse(body);
        
        if (!workflow_data.contains("id") || !workflow_data.contains("name") || !workflow_data.contains("steps")) {
            send_error(client_socket, 400, "Missing required fields: id, name, steps");
            return;
        }
        
        // Create workflow definition from JSON
        WorkflowDefinition workflow(
            workflow_data["id"], 
            workflow_data["name"],
            static_cast<WorkflowType>(workflow_data.value("type", 0))
        );
        
        workflow.description = workflow_data.value("description", "");
        workflow.max_execution_time_ms = workflow_data.value("max_execution_time_ms", 300000);
        workflow.allow_partial_failure = workflow_data.value("allow_partial_failure", false);
        workflow.global_context = workflow_data.value("global_context", json{});
        
        // Parse steps
        for (const auto& step_data : workflow_data["steps"]) {
            WorkflowStep step(
                step_data["id"],
                step_data["agent_name"],
                step_data["function_name"],
                step_data.value("parameters", json{})
            );
            
            step.timeout_ms = step_data.value("timeout_ms", 30000);
            step.optional = step_data.value("optional", false);
            step.conditions = step_data.value("conditions", json{});
            step.dependencies = step_data.value("dependencies", std::vector<std::string>{});
            
            workflow.steps.push_back(step);
        }
        
        workflow_orchestrator_->register_workflow(workflow);
        
        json response;
        response["message"] = "Workflow registered successfully";
        response["workflow_id"] = workflow.id;
        
        send_response(client_socket, 201, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_execute_workflow(socket_t client_socket, const std::string& body, const std::string& workflow_id) {
    try {
        json request_data = json::parse(body);
        
        json input_data = request_data.value("input_data", json{});
        bool async_execution = request_data.value("async", true);
        
        std::string execution_id;
        if (async_execution) {
            execution_id = workflow_orchestrator_->execute_workflow_async(workflow_id, input_data);
        } else {
            execution_id = workflow_orchestrator_->execute_workflow(workflow_id, input_data);
        }
        
        json response;
        response["execution_id"] = execution_id;
        response["workflow_id"] = workflow_id;
        response["async"] = async_execution;
        response["status"] = "submitted";
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_get_workflow_execution(socket_t client_socket, const std::string& execution_id) {
    try {
        auto execution = workflow_orchestrator_->get_execution_status(execution_id);
        if (!execution) {
            send_error(client_socket, 404, "Execution not found");
            return;
        }
        
        json response;
        response["execution_id"] = execution->execution_id;
        response["workflow_id"] = execution->workflow_id;
        response["state"] = static_cast<int>(execution->state);
        response["progress_percentage"] = execution->progress_percentage;
        response["start_time"] = std::chrono::duration_cast<std::chrono::seconds>(
            execution->start_time.time_since_epoch()).count();
        
        if (execution->end_time != std::chrono::system_clock::time_point{}) {
            response["end_time"] = std::chrono::duration_cast<std::chrono::seconds>(
                execution->end_time.time_since_epoch()).count();
        }
        
        response["input_data"] = execution->input_data;
        response["output_data"] = execution->output_data;
        response["context"] = execution->context;
        response["error_message"] = execution->error_message;
        response["step_results"] = execution->step_results;
        response["step_outputs"] = execution->step_outputs;
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_control_workflow_execution(socket_t client_socket, const std::string& execution_id, const std::string& action) {
    try {
        bool success = false;
        std::string message;
        
        if (action == "pause") {
            success = workflow_orchestrator_->pause_execution(execution_id);
            message = success ? "Execution paused" : "Failed to pause execution";
        } else if (action == "resume") {
            success = workflow_orchestrator_->resume_execution(execution_id);
            message = success ? "Execution resumed" : "Failed to resume execution";
        } else if (action == "cancel") {
            success = workflow_orchestrator_->cancel_execution(execution_id);
            message = success ? "Execution cancelled" : "Failed to cancel execution";
        } else {
            send_error(client_socket, 400, "Invalid action. Use: pause, resume, or cancel");
            return;
        }
        
        json response;
        response["execution_id"] = execution_id;
        response["action"] = action;
        response["success"] = success;
        response["message"] = message;
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_list_workflow_executions(socket_t client_socket) {
    try {
        auto executions = workflow_orchestrator_->list_active_executions();
        
        json response = json::array();
        for (const auto& execution : executions) {
            json execution_info;
            execution_info["execution_id"] = execution->execution_id;
            execution_info["workflow_id"] = execution->workflow_id;
            execution_info["state"] = static_cast<int>(execution->state);
            execution_info["progress_percentage"] = execution->progress_percentage;
            execution_info["start_time"] = std::chrono::duration_cast<std::chrono::seconds>(
                execution->start_time.time_since_epoch()).count();
            execution_info["error_message"] = execution->error_message;
            response.push_back(execution_info);
        }
        
        json result;
        result["active_executions"] = response;
        result["total_active"] = response.size();
        
        send_response(client_socket, 200, result.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_start_kolosal_server(socket_t client_socket) {
    try {
        if (!agent_manager_) {
            send_error(client_socket, 500, "Agent manager not available");
            return;
        }
        
        bool success = agent_manager_->start_kolosal_server();
        
        json response;
        response["success"] = success;
        response["message"] = success ? "Kolosal server started successfully" : "Failed to start Kolosal server";
        
        if (success) {
            response["server_url"] = agent_manager_->get_kolosal_server_url();
            response["status"] = agent_manager_->get_kolosal_server_status();
        }
        
        send_response(client_socket, success ? 200 : 500, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_stop_kolosal_server(socket_t client_socket) {
    try {
        if (!agent_manager_) {
            send_error(client_socket, 500, "Agent manager not available");
            return;
        }
        
        bool success = agent_manager_->stop_kolosal_server();
        
        json response;
        response["success"] = success;
        response["message"] = success ? "Kolosal server stopped successfully" : "Failed to stop Kolosal server";
        response["status"] = agent_manager_->get_kolosal_server_status();
        
        send_response(client_socket, success ? 200 : 500, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

// Metrics and Monitoring Handlers
void HTTPServer::handle_get_system_metrics(socket_t client_socket) {
    try {
        json response;
        response["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        // Get system metrics from MetricsCollector
        // This would integrate with the MetricsCollector we created
        response["system"] = {
            {"uptime_seconds", 0}, // Placeholder
            {"cpu_usage_percent", 0.0},
            {"memory_usage_mb", 0.0},
            {"thread_count", std::thread::hardware_concurrency()}
        };
        
        response["requests"] = {
            {"total_count", 0},
            {"success_count", 0},
            {"error_count", 0},
            {"avg_response_time_ms", 0.0}
        };
        
        response["agents"] = {
            {"active_count", agent_manager_ ? agent_manager_->get_active_agent_count() : 0},
            {"total_operations", 0}
        };
        
        response["workflows"] = {
            {"active_executions", workflow_orchestrator_ ? workflow_orchestrator_->list_active_executions().size() : 0},
            {"total_executions", 0}
        };
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_get_health_status(socket_t client_socket) {
    try {
        json response;
        response["status"] = "healthy";
        response["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        response["version"] = "2.0.0";
        
        // Component health checks
        json components;
        
        // Agent Manager
        components["agent_manager"] = {
            {"status", agent_manager_ ? "healthy" : "unhealthy"},
            {"active_agents", agent_manager_ ? agent_manager_->get_active_agent_count() : 0}
        };
        
        // Workflow Orchestrator
        components["workflow_orchestrator"] = {
            {"status", workflow_orchestrator_ ? "healthy" : "unhealthy"},
            {"active_executions", workflow_orchestrator_ ? workflow_orchestrator_->list_active_executions().size() : 0}
        };
        
        // Kolosal Server
        if (agent_manager_) {
            auto kolosal_status = agent_manager_->get_kolosal_server_status();
            components["kolosal_server"] = {
                {"status", kolosal_status["running"].get<bool>() ? "healthy" : "unhealthy"},
                {"url", kolosal_status.value("url", "")},
                {"models_loaded", kolosal_status.value("models_loaded", 0)}
            };
        } else {
            components["kolosal_server"] = {
                {"status", "unknown"}
            };
        }
        
        response["components"] = components;
        
        // Overall health
        bool all_healthy = true;
        for (const auto& [name, component] : components.items()) {
            if (component["status"] != "healthy") {
                all_healthy = false;
                break;
            }
        }
        
        if (!all_healthy) {
            response["status"] = "degraded";
        }
        
        send_response(client_socket, all_healthy ? 200 : 503, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_get_prometheus_metrics(socket_t client_socket) {
    try {
        std::ostringstream prometheus;
        
        // Basic system metrics in Prometheus format
        prometheus << "# HELP kolosal_uptime_seconds Total uptime in seconds\n";
        prometheus << "# TYPE kolosal_uptime_seconds counter\n";
        prometheus << "kolosal_uptime_seconds 0\n\n"; // Placeholder
        
        prometheus << "# HELP kolosal_http_requests_total Total HTTP requests\n";
        prometheus << "# TYPE kolosal_http_requests_total counter\n";
        prometheus << "kolosal_http_requests_total 0\n\n";
        
        prometheus << "# HELP kolosal_active_agents Number of active agents\n";
        prometheus << "# TYPE kolosal_active_agents gauge\n";
        prometheus << "kolosal_active_agents " << (agent_manager_ ? agent_manager_->get_active_agent_count() : 0) << "\n\n";
        
        prometheus << "# HELP kolosal_active_workflows Number of active workflow executions\n";
        prometheus << "# TYPE kolosal_active_workflows gauge\n";
        prometheus << "kolosal_active_workflows " << (workflow_orchestrator_ ? workflow_orchestrator_->list_active_executions().size() : 0) << "\n\n";
        
        // Send response with appropriate content type
        std::string response = prometheus.str();
        std::string http_response = "HTTP/1.1 200 OK\r\n";
        http_response += "Content-Type: text/plain; charset=utf-8\r\n";
        http_response += "Content-Length: " + std::to_string(response.length()) + "\r\n";
        http_response += "Connection: close\r\n\r\n";
        http_response += response;
        
        send(client_socket, http_response.c_str(), http_response.length(), 0);
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_get_performance_metrics(socket_t client_socket) {
    try {
        json response;
        response["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        // Request performance metrics
        response["requests"] = {
            {"total_count", 0},
            {"average_duration_ms", 0.0},
            {"p50_duration_ms", 0.0},
            {"p95_duration_ms", 0.0},
            {"p99_duration_ms", 0.0},
            {"requests_per_second", 0.0}
        };
        
        // Agent performance metrics
        response["agents"] = {
            {"total_operations", 0},
            {"average_execution_time_ms", 0.0},
            {"success_rate", 1.0},
            {"most_used_functions", json::array()}
        };
        
        // Workflow performance metrics
        response["workflows"] = {
            {"total_executions", 0},
            {"average_execution_time_ms", 0.0},
            {"success_rate", 1.0},
            {"most_executed_workflows", json::array()}
        };
        
        // System performance metrics
        response["system"] = {
            {"cpu_usage_percent", 0.0},
            {"memory_usage_mb", 0.0},
            {"disk_usage_percent", 0.0},
            {"network_io_bytes", 0}
        };
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

// New workflow management handlers

void HTTPServer::handle_get_workflow(socket_t client_socket, const std::string& workflow_id) {
    try {
        auto workflow = workflow_orchestrator_->get_workflow(workflow_id);
        if (!workflow) {
            send_error(client_socket, 404, "Workflow not found");
            return;
        }
        
        json response;
        response["id"] = workflow->id;
        response["name"] = workflow->name;
        response["description"] = workflow->description;
        response["type"] = static_cast<int>(workflow->type);
        response["max_execution_time_ms"] = workflow->max_execution_time_ms;
        response["allow_partial_failure"] = workflow->allow_partial_failure;
        response["global_context"] = workflow->global_context;
        
        json steps = json::array();
        for (const auto& step : workflow->steps) {
            json step_json;
            step_json["id"] = step.id;
            step_json["agent_name"] = step.agent_name;
            step_json["function_name"] = step.function_name;
            step_json["llm_model"] = step.llm_model;
            step_json["parameters"] = step.parameters;
            step_json["timeout_ms"] = step.timeout_ms;
            step_json["optional"] = step.optional;
            step_json["dependencies"] = step.dependencies;
            step_json["conditions"] = step.conditions;
            steps.push_back(step_json);
        }
        response["steps"] = steps;
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_update_workflow(socket_t client_socket, const std::string& workflow_id, const std::string& body) {
    try {
        // Check if workflow exists
        auto existing_workflow = workflow_orchestrator_->get_workflow(workflow_id);
        if (!existing_workflow) {
            send_error(client_socket, 404, "Workflow not found");
            return;
        }
        
        json workflow_data = json::parse(body);
        workflow_data["id"] = workflow_id; // Ensure ID matches URL parameter
        
        // Create updated workflow definition from JSON
        WorkflowDefinition workflow(
            workflow_id, 
            workflow_data["name"],
            static_cast<WorkflowType>(workflow_data.value("type", 0))
        );
        
        workflow.description = workflow_data.value("description", "");
        workflow.max_execution_time_ms = workflow_data.value("max_execution_time_ms", 300000);
        workflow.allow_partial_failure = workflow_data.value("allow_partial_failure", false);
        workflow.global_context = workflow_data.value("global_context", json{});
        
        // Parse steps
        for (const auto& step_data : workflow_data["steps"]) {
            WorkflowStep step(
                step_data["id"],
                step_data["agent_name"],
                step_data["function_name"],
                step_data.value("parameters", json::array()),
                step_data.value("llm_model", "")
            );
            
            step.timeout_ms = step_data.value("timeout_ms", 60000);
            step.optional = step_data.value("optional", false);
            
            if (step_data.contains("dependencies") && step_data["dependencies"].is_array()) {
                for (const auto& dep : step_data["dependencies"]) {
                    step.dependencies.push_back(dep);
                }
            }
            
            if (step_data.contains("conditions")) {
                step.conditions = step_data["conditions"];
            }
            
            workflow.steps.push_back(step);
        }
        
        // Register the updated workflow
        workflow_orchestrator_->register_workflow(workflow);
        
        json response;
        response["message"] = "Workflow updated successfully";
        response["workflow_id"] = workflow_id;
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const json::parse_error& e) {
        send_error(client_socket, 400, "Invalid JSON: " + std::string(e.what()));
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_delete_workflow(socket_t client_socket, const std::string& workflow_id) {
    try {
        bool success = workflow_orchestrator_->remove_workflow(workflow_id);
        
        if (success) {
            json response;
            response["message"] = "Workflow deleted successfully";
            response["workflow_id"] = workflow_id;
            send_response(client_socket, 200, response.dump(2));
        } else {
            send_error(client_socket, 404, "Workflow not found");
        }
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_get_workflow_templates(socket_t client_socket) {
    try {
        json response = json::array();
        
        // Built-in templates information
        json research_template;
        research_template["id"] = "research_workflow";
        research_template["name"] = "Research and Analysis Workflow";
        research_template["description"] = "Comprehensive research workflow: question -> research -> analyze -> summarize";
        research_template["type"] = "sequential";
        research_template["input_parameters"] = json::array({"query", "depth"});
        response.push_back(research_template);
        
        json analysis_template;
        analysis_template["id"] = "analysis_workflow";
        analysis_template["name"] = "Data Analysis Workflow";
        analysis_template["description"] = "Data analysis workflow: input -> preprocess -> analyze -> report";
        analysis_template["type"] = "sequential";
        analysis_template["input_parameters"] = json::array({"text", "analysis_type"});
        response.push_back(analysis_template);
        
        json pipeline_template;
        pipeline_template["id"] = "data_pipeline_workflow";
        pipeline_template["name"] = "Data Pipeline Workflow";
        pipeline_template["description"] = "Data processing pipeline: extract -> transform -> validate -> load";
        pipeline_template["type"] = "pipeline";
        pipeline_template["input_parameters"] = json::array({"data", "format"});
        response.push_back(pipeline_template);
        
        json decision_template;
        decision_template["id"] = "decision_workflow";
        decision_template["name"] = "Decision Making Workflow";
        decision_template["description"] = "Decision making workflow: gather info -> analyze options -> decide -> execute";
        decision_template["type"] = "sequential";
        decision_template["input_parameters"] = json::array({"question", "context"});
        response.push_back(decision_template);
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_execute_workflow_template(socket_t client_socket, const std::string& template_id, const std::string& body) {
    try {
        json request_data = json::parse(body);
        json input_data = request_data.value("input_data", json{});
        bool async_execution = request_data.value("async", true);
        
        std::string execution_id;
        if (async_execution) {
            execution_id = workflow_orchestrator_->execute_workflow_async(template_id, input_data);
        } else {
            execution_id = workflow_orchestrator_->execute_workflow(template_id, input_data);
        }
        
        json response;
        response["execution_id"] = execution_id;
        response["template_id"] = template_id;
        response["async"] = async_execution;
        response["status"] = "submitted";
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const json::parse_error& e) {
        send_error(client_socket, 400, "Invalid JSON: " + std::string(e.what()));
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_workflow_execution_progress(socket_t client_socket, const std::string& execution_id) {
    try {
        auto execution = workflow_orchestrator_->get_execution_status(execution_id);
        if (!execution) {
            send_error(client_socket, 404, "Execution not found");
            return;
        }
        
        json response;
        response["execution_id"] = execution->execution_id;
        response["workflow_id"] = execution->workflow_id;
        response["state"] = static_cast<int>(execution->state);
        response["progress_percentage"] = execution->progress_percentage;
        
        // Calculate step progress
        auto workflow = workflow_orchestrator_->get_workflow(execution->workflow_id);
        if (workflow) {
            response["total_steps"] = workflow->steps.size();
            response["completed_steps"] = execution->step_results.size();
            
            json step_progress = json::array();
            for (const auto& step : workflow->steps) {
                json step_info;
                step_info["id"] = step.id;
                step_info["name"] = step.function_name;
                
                if (execution->step_results.find(step.id) != execution->step_results.end()) {
                    step_info["status"] = "completed";
                    if (execution->step_outputs.find(step.id) != execution->step_outputs.end()) {
                        step_info["has_output"] = true;
                    }
                } else {
                    step_info["status"] = "pending";
                }
                
                step_progress.push_back(step_info);
            }
            response["step_progress"] = step_progress;
        }
        
        // Timing information
        response["start_time"] = std::chrono::duration_cast<std::chrono::seconds>(
            execution->start_time.time_since_epoch()).count();
        
        if (execution->end_time != std::chrono::system_clock::time_point{}) {
            response["end_time"] = std::chrono::duration_cast<std::chrono::seconds>(
                execution->end_time.time_since_epoch()).count();
            
            auto duration = execution->end_time - execution->start_time;
            response["duration_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        } else {
            auto duration = std::chrono::system_clock::now() - execution->start_time;
            response["elapsed_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        }
        
        response["error_message"] = execution->error_message;
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}

void HTTPServer::handle_workflow_execution_logs(socket_t client_socket, const std::string& execution_id) {
    try {
        auto execution = workflow_orchestrator_->get_execution_status(execution_id);
        if (!execution) {
            send_error(client_socket, 404, "Execution not found");
            return;
        }
        
        json response;
        response["execution_id"] = execution->execution_id;
        response["workflow_id"] = execution->workflow_id;
        
        json logs = json::array();
        
        // Add execution start log
        json start_log;
        start_log["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
            execution->start_time.time_since_epoch()).count();
        start_log["level"] = "INFO";
        start_log["message"] = "Workflow execution started";
        start_log["context"] = {{"workflow_id", execution->workflow_id}};
        logs.push_back(start_log);
        
        // Add step execution logs
        auto workflow = workflow_orchestrator_->get_workflow(execution->workflow_id);
        if (workflow) {
            for (const auto& step : workflow->steps) {
                if (execution->step_results.find(step.id) != execution->step_results.end()) {
                    json step_log;
                    step_log["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
                        execution->start_time.time_since_epoch()).count(); // Placeholder - would need actual timestamps
                    step_log["level"] = "INFO";
                    step_log["message"] = "Step executed: " + step.id;
                    step_log["context"] = {
                        {"step_id", step.id},
                        {"agent_name", step.agent_name},
                        {"function_name", step.function_name}
                    };
                    
                    if (execution->step_outputs.find(step.id) != execution->step_outputs.end()) {
                        step_log["context"]["has_output"] = true;
                    }
                    
                    logs.push_back(step_log);
                }
            }
        }
        
        // Add completion or error log
        if (execution->state == WorkflowExecutionState::COMPLETED) {
            json completion_log;
            completion_log["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
                execution->end_time.time_since_epoch()).count();
            completion_log["level"] = "INFO";
            completion_log["message"] = "Workflow execution completed successfully";
            logs.push_back(completion_log);
        } else if (execution->state == WorkflowExecutionState::FAILED) {
            json error_log;
            error_log["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
                execution->end_time.time_since_epoch()).count();
            error_log["level"] = "ERROR";
            error_log["message"] = "Workflow execution failed: " + execution->error_message;
            logs.push_back(error_log);
        }
        
        response["logs"] = logs;
        response["log_count"] = logs.size();
        
        send_response(client_socket, 200, response.dump(2));
        
    } catch (const std::exception& e) {
        send_error(client_socket, 500, e.what());
    }
}
