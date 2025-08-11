/**
 * @file agent_management_route.cpp
 * @brief REST API routes for agent management
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "api/agent_management_route.hpp"
#include "config/yaml_configuration_parser.hpp"
#include "../external/nlohmann/json.hpp"
#include <sstream>
#include <regex>

using json = nlohmann::json;
using namespace kolosal::agents;

namespace kolosal::api {

AgentManagementRoute::AgentManagementRoute(std::shared_ptr<YAMLConfigurableAgentManager> agent_manager)
    : agent_manager_(agent_manager) {}

bool AgentManagementRoute::match(const std::string& method, const std::string& path) {
    // Agent management endpoints
    const std::vector<std::regex> agent_patterns = {
        std::regex("^/v1/agents/?$"),
        std::regex("^/v1/agents/([^/]+)/?$"),
        std::regex("^/v1/agents/([^/]+)/(start|stop|execute)/?$"),
        std::regex("^/v1/system/(status|reload)/?$")
    };

    for (const auto& pattern : agent_patterns) {
        if (std::regex_match(path, pattern)) {
            matched_method_ = method;
            matched_path_ = path;
            return true;
        }
    }
    return false;
}

void AgentManagementRoute::handle(SocketType sock, const std::string& body) {
    try {
        if (matched_path_ == "/v1/agents" || matched_path_ == "/v1/agents/") {
            if (matched_method_ == "GET") {
                handleList_Agents(sock);
            } else if (matched_method_ == "POST") {
                handleCreate_Agent(sock, body);
            } else {
                sendError_Response(sock, 405, "Method not allowed");
            }
        } else if (matched_path_.find("/v1/system/status") == 0) {
            if (matched_method_ == "GET") {
                handleSystem_Status(sock);
            } else {
                sendError_Response(sock, 405, "Method not allowed");
            }
        } else if (matched_path_.find("/v1/system/reload") == 0) {
            if (matched_method_ == "POST") {
                handleSystem_Reload(sock, body);
            } else {
                sendError_Response(sock, 405, "Method not allowed");
            }
        } else {
            // Individual agent operations
            std::regex agent_regex("^/v1/agents/([^/]+)(?:/([^/]+))?/?$");
            std::smatch matches;
            
            if (std::regex_match(matched_path_, matches, agent_regex)) {
                std::string agent_id = matches[1].str();
                std::string action = matches.size() > 2 ? matches[2].str() : "";
                if (action.empty()) {
                    if (matched_method_ == "GET") {
                        handleGet_Agent(sock, agent_id);
                    } else if (matched_method_ == "DELETE") {
                        handleDelete_Agent(sock, agent_id);
                    } else {
                        sendError_Response(sock, 405, "Method not allowed");
                    }
                } else if (action == "start") {
                    if (matched_method_ == "PUT") {
                        handleStart_Agent(sock, agent_id);
                    } else {
                        sendError_Response(sock, 405, "Method not allowed");
                    }
                } else if (action == "stop") {
                    if (matched_method_ == "PUT") {
                        handleStop_Agent(sock, agent_id);
                    } else {
                        sendError_Response(sock, 405, "Method not allowed");
                    }
                } else if (action == "execute") {
                    if (matched_method_ == "POST") {
                        handleExecute_Function(sock, agent_id, body);
                    } else {
                        sendError_Response(sock, 405, "Method not allowed");
                    }
                } else {
                    sendError_Response(sock, 404, "Endpoint not found");
                }
            } else {
                sendError_Response(sock, 404, "Endpoint not found");
            }
        }
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal server error", e.what());
    }
}

void AgentManagementRoute::handleList_Agents(SocketType sock) {
    try {
        json response;
        response["agents"] = json::array();
        
        const auto agent_ids = agent_manager_->list_agents();
        for (const auto& agent_id : agent_ids) {
            const auto agent = agent_manager_->get__agent(agent_id);
            if (agent) {
                json agent_json;
                agent_json["id"] = agent_id;
                agent_json["name"] = agent->get__agent_name();
                agent_json["type"] = agent->get__agent_type();
                agent_json["running"] = agent->is__running();
                agent_json["role"] = static_cast<int>(agent->get__role());
                agent_json["capabilities"] = agent->get__capabilities();
                
                const auto stats = agent->get__statistics();
                agent_json["statistics"] = {
                    {"total_functions_executed", stats.total_functions_executed},
                    {"total_tools_executed", stats.total_tools_executed},
                    {"total_plans_created", stats.total_plans_created},
                    {"memory_entries_count", stats.memory_entries_count},
                    {"average_execution_time_ms", stats.average_execution_time_ms}
                };
                
                response["agents"].push_back(agent_json);
            }
        }
        
        response["total_count"] = agent_ids.size();
        response["system_running"] = agent_manager_->is__running();
        
        sendJson_Response(sock, 200, response.dump(2));
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Failed to list agents", e.what());
    }
}

void AgentManagementRoute::handleCreate_Agent(SocketType sock, const std::string& body) {
    try {
        if (body.empty()) {
            sendError_Response(sock, 400, "Request body required");
            return;
        }

    const auto request_json = json::parse(body);
        // Create AgentConfig from JSON
        AgentConfig config;
        config.name = request_json.value("name", "");
        config.id = request_json.value("id", "");
        config.type = request_json.value("type", "generic");
        
        if (request_json.contains("role")) {
            const AgentRole role = static_cast<AgentRole>(request_json["role"].get<int>());
            // Convert role enum to string (assuming we have a role manager or utility function)
            // For now, just use the role as-is or convert it to string representation
            config.role = std::to_string(static_cast<int>(role));
        }
        
        if (request_json.contains("capabilities")) {
            for (const auto& cap : request_json["capabilities"]) {
                config.capabilities.push_back(cap.get<std::string>());
            }
        }
        
        if (request_json.contains("functions")) {
            for (const auto& func : request_json["functions"]) {
                config.functions.push_back(func.get<std::string>());
            }
        }
        
        // Additional config parameters - use existing AgentConfig fields
        if (request_json.contains("config")) {
            const auto& cfg = request_json["config"];
            config.auto_start = cfg.value("auto_start", true);
            config.max_concurrent_jobs = cfg.value("max_concurrent_tasks", 5);
            config.heartbeat_interval_seconds = cfg.value("heartbeat_interval", 5);
        }

    std::string agent_id = agent_manager_->create__agent_from_config(config);
        if (agent_id.empty()) {
            sendError_Response(sock, 400, "Failed to create agent");
            return;
        }

        // Start the agent if auto_start is true
        const bool auto_start = config.auto_start;
        if (auto_start) {
            agent_manager_->start_agent(agent_id);
        }

        json response;
        response["agent_id"] = agent_id;
        response["message"] = "Agent created successfully";
        response["started"] = auto_start;
        
        sendJson_Response(sock, 201, response.dump(2));
    } catch (const json::parse_error& e) {
        sendError_Response(sock, 400, "Invalid JSON", e.what());
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Failed to create agent", e.what());
    }
}

void AgentManagementRoute::handleGet_Agent(SocketType sock, const std::string& agent_id) {
    try {
        auto agent = agent_manager_->get__agent(agent_id);
        if (!agent) {
            sendError_Response(sock, 404, "Agent not found");
            return;
        }

        json response;
        response["id"] = agent_id;
        response["name"] = agent->get__agent_name();
        response["type"] = agent->get__agent_type();
        response["running"] = agent->is__running();
        response["role"] = static_cast<int>(agent->get__role());
        response["capabilities"] = agent->get__capabilities();
        response["specializations"] = json::array();
        
        for (const auto& spec : agent->get__specializations()) {
            response["specializations"].push_back(static_cast<int>(spec));
        }
        
        auto stats = agent->get__statistics();
        response["statistics"] = {
            {"total_functions_executed", stats.total_functions_executed},
            {"total_tools_executed", stats.total_tools_executed},
            {"total_plans_created", stats.total_plans_created},
            {"memory_entries_count", stats.memory_entries_count},
            {"average_execution_time_ms", stats.average_execution_time_ms},
            {"last_activity", std::chrono::duration_cast<std::chrono::seconds>(
                stats.last_activity.time_since_epoch()).count()}
        };
        
        sendJson_Response(sock, 200, response.dump(2));
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Failed to get agent details", e.what());
    }
}

void AgentManagementRoute::handleStart_Agent(SocketType sock, const std::string& agent_id) {
    try {
        if (!agent_manager_->start_agent(agent_id)) {
            sendError_Response(sock, 400, "Failed to start agent");
            return;
        }

        json response;
        response["message"] = "Agent started successfully";
        response["agent_id"] = agent_id;
        
        sendJson_Response(sock, 200, response.dump(2));
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Failed to start agent", e.what());
    }
}

void AgentManagementRoute::handleStop_Agent(SocketType sock, const std::string& agent_id) {
    try {
        if (!agent_manager_->stop_agent(agent_id)) {
            sendError_Response(sock, 400, "Failed to stop agent");
            return;
        }

        json response;
        response["message"] = "Agent stopped successfully";
        response["agent_id"] = agent_id;
        
        sendJson_Response(sock, 200, response.dump(2));
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Failed to stop agent", e.what());
    }
}

void AgentManagementRoute::handleDelete_Agent(SocketType sock, const std::string& agent_id) {
    try {
        if (!agent_manager_->delete_agent(agent_id)) {
            sendError_Response(sock, 400, "Failed to delete agent");
            return;
        }

        json response;
        response["message"] = "Agent deleted successfully";
        response["agent_id"] = agent_id;
        
        sendJson_Response(sock, 200, response.dump(2));
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Failed to delete agent", e.what());
    }
}

void AgentManagementRoute::handleExecute_Function(SocketType sock, const std::string& agent_id, const std::string& body) {
    try {
        auto agent = agent_manager_->get__agent(agent_id);
        if (!agent) {
            sendError_Response(sock, 404, "Agent not found");
            return;
        }

        if (body.empty()) {
            sendError_Response(sock, 400, "Request body required");
            return;
        }

        auto request_json = json::parse(body);
    std::string function_name = request_json.value("function", "");
        if (function_name.empty()) {
            sendError_Response(sock, 400, "Function name required");
            return;
        }

        // Convert JSON parameters to AgentData
        AgentData parameters;
        if (request_json.contains("parameters")) {
            // Store JSON parameters as string representation
            parameters.set("json", request_json["parameters"].dump());
        }

        // Execute the function
        const auto result = agent->execute_function(function_name, parameters);
        json response;
        response["success"] = result.success;
        response["message"] = result.error_message;
        response["function"] = function_name;
        response["agent_id"] = agent_id;
        
        if (result.success && result.result_data.has__key("result")) {
            response["result"] = result.result_data.get__string("result");
        }
        
        const int status_code = result.success ? 200 : 400;
        sendJson_Response(sock, status_code, response.dump(2));
        
    } catch (const json::parse_error& e) {
        sendError_Response(sock, 400, "Invalid JSON", e.what());
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Failed to execute function", e.what());
    }
}

void AgentManagementRoute::handleSystem_Status(SocketType sock) {
    try {
        json response;
        response["system_running"] = agent_manager_->is__running();
        response["status"] = agent_manager_->get__system_status();
        response["total_agents"] = agent_manager_->list_agents().size();
        response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        sendJson_Response(sock, 200, response.dump(2));
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Failed to get system status", e.what());
    }
}

void AgentManagementRoute::handleSystem_Reload(SocketType sock, const std::string& body) {
    try {
        if (body.empty()) {
            sendError_Response(sock, 400, "Configuration file path required");
            return;
        }

    auto request_json = json::parse(body);
    std::string config_path = request_json.value("config_file", "config.yaml");
        if (!agent_manager_->reload_configuration(config_path)) {
            sendError_Response(sock, 400, "Failed to reload configuration");
            return;
        }

        json response;
        response["message"] = "Configuration reloaded successfully";
        response["config_file"] = config_path;
        
        sendJson_Response(sock, 200, response.dump(2));
    } catch (const json::parse_error& e) {
        sendError_Response(sock, 400, "Invalid JSON", e.what());
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Failed to reload configuration", e.what());
    }
}

void AgentManagementRoute::sendJson_Response(SocketType sock, int status_code, const std::string& json_body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " ";
    
    switch (status_code) {
        case 200: response << "OK"; break;
        case 201: response << "Created"; break;
        case 400: response << "Bad Request"; break;
        case 404: response << "Not Found"; break;
        case 405: response << "Method Not Allowed"; break;
        case 500: response << "Internal Server Error"; break;
        default: response << "Unknown"; break;
    }
    
    response << "\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Content-Length: " << json_body.length() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response << "\r\n";
    response << json_body;

    std::string response_str = response.str();
    send(sock, response_str.c_str(), response_str.length(), 0);
}

void AgentManagementRoute::sendError_Response(SocketType sock, int status_code, const std::string& error, const std::string& message) {
    json error_json;
    error_json["error"] = {
        {"type", error},
        {"message", message.empty() ? error : message},
        {"code", status_code}
    };
    
    sendJson_Response(sock, status_code, error_json.dump(2));
}

std::string AgentManagementRoute::extractAgentIdFrom_Path(const std::string& path) {
    std::regex agent_regex("^/v1/agents/([^/]+)");
    std::smatch matches;
    
    if (std::regex_search(path, matches, agent_regex)) {
        return matches[1].str();
    }
    
    return "";
}

} // namespace kolosal::api
