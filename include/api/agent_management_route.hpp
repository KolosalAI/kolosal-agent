#pragma once

#include "../export.hpp"
#include "../kolosal-server/include/kolosal/routes/route_interface.hpp"
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
class KOLOSAL_AGENT_API AgentManagementRoute : public IRoute {
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
    void handleListAgents(SocketType sock);
    void handleCreateAgent(SocketType sock, const std::string& body);
    void handleGetAgent(SocketType sock, const std::string& agent_id);
    void handleStartAgent(SocketType sock, const std::string& agent_id);
    void handleStopAgent(SocketType sock, const std::string& agent_id);
    void handleDeleteAgent(SocketType sock, const std::string& agent_id);
    void handleExecuteFunction(SocketType sock, const std::string& agent_id, const std::string& body);
    void handleSystemStatus(SocketType sock);
    void handleSystemReload(SocketType sock, const std::string& body);

    // Utility methods
    void sendJsonResponse(SocketType sock, int status_code, const std::string& json_body);
    void sendErrorResponse(SocketType sock, int status_code, const std::string& error, const std::string& message = "");
    std::string extractAgentIdFromPath(const std::string& path);
    std::string agentToJson(const std::string& agent_id, std::shared_ptr<kolosal::agents::AgentCore> agent);
};

} // namespace kolosal::api
