/**
 * @file agent_management_route.hpp
 * @brief REST API routes for agent management
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_REST_API_AGENT_MANAGEMENT_ROUTE_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_REST_API_AGENT_MANAGEMENT_ROUTE_HPP_INCLUDED

#include "../export.hpp"
#include "route_interface.hpp"
#include "../agent/multi_agent_system.hpp"
#include <memory>
#include <string>

namespace kolosal::api {

/**
 * @brief REST API route for managing the multi-agent system
 * 
 * Provides comprehensive agent management capabilities through HTTP endpoints:
 * - GET /v1/agents - List all agents and their status
 * - POST /v1/agents - Create a new agent from configuration
 * - GET /v1/agents/{id} - Get specific agent details
 * - PUT /v1/agents/{id}/start - Start an agent
 * - PUT /v1/agents/{id}/stop - Stop an agent
 * - DELETE /v1/agents/{id} - Remove an agent
 * - POST /v1/agents/{id}/execute - Execute function on agent
 * - GET /v1/system/status - Get system-wide status
 * - POST /v1/system/reload - Reload configuration
 */
class KOLOSAL_AGENT_API AgentManagementRoute : public kolosal::api::IRoute {
public:
    /**
     * @brief Constructor
     * @param agent_manager Shared pointer to the agent manager instance
     */
    explicit AgentManagementRoute(std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager);

    /**
     * @brief Check if this route should handle the request
     * @param method HTTP method (GET, POST, PUT, DELETE)
     * @param path Request path
     * @return true if this route handles the request
     */
    bool match(const std::string& method, const std::string& path) override;

    /**
     * @brief Handle the HTTP request
     * @param sock Socket for sending response
     * @param body Request body (for POST/PUT requests)
     */
    void handle(SocketType sock, const std::string& body) override;

private:
    std::shared_ptr<kolosal::agents::YAMLConfigurableAgentManager> agent_manager_;
    mutable std::string matched_method_;
    mutable std::string matched_path_;

    // Handler methods for different endpoints
    void handleList_Agents(SocketType sock);
    void handleCreate_Agent(SocketType sock, const std::string& body);
    void handleGet_Agent(SocketType sock, const std::string& agent_id);
    void handleStart_Agent(SocketType sock, const std::string& agent_id);
    void handleStop_Agent(SocketType sock, const std::string& agent_id);
    void handleDelete_Agent(SocketType sock, const std::string& agent_id);
    void handleExecute_Function(SocketType sock, const std::string& agent_id, const std::string& body);
    void handleSystem_Status(SocketType sock);
    void handleSystem_Reload(SocketType sock, const std::string& body);

    // Utility methods
    void sendJson_Response(SocketType sock, int status_code, const std::string& json_body);
    void sendError_Response(SocketType sock, int status_code, const std::string& error, const std::string& message = "");
    std::string extractAgentIdFrom_Path(const std::string& path);
    std::string agentTo_Json(const std::string& agent_id, std::shared_ptr<kolosal::agents::AgentCore> agent);
};

} // namespace kolosal::api

#endif // KOLOSAL_AGENT_INCLUDE_REST_API_AGENT_MANAGEMENT_ROUTE_HPP_INCLUDED
