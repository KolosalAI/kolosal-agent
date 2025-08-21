/**
 * @file workflow_route.hpp
 * @brief REST API routes for workflow management
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_API_WORKFLOW_ROUTE_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_API_WORKFLOW_ROUTE_HPP_INCLUDED

#include "../export.hpp"
#include "route_interface.hpp"
#include "../workflow/workflow_engine.hpp"
#include <memory>
#include <string>

namespace kolosal::api {

/**
 * @brief REST API route for managing workflows
 * 
 * Provides comprehensive workflow management capabilities through HTTP endpoints:
 * - GET /v1/workflows - List all workflows
 * - POST /v1/workflows - Create a new workflow
 * - GET /v1/workflows/{id} - Get specific workflow details
 * - PUT /v1/workflows/{id} - Update a workflow
 * - DELETE /v1/workflows/{id} - Delete a workflow
 * - POST /v1/workflows/{id}/execute - Execute a workflow
 * - GET /v1/workflows/{id}/executions - List workflow executions
 * - GET /v1/workflows/{id}/executions/{execution_id}/status - Get execution status
 * - GET /v1/workflows/{id}/executions/{execution_id}/result - Get execution result
 * - PUT /v1/workflows/{id}/executions/{execution_id}/pause - Pause execution
 * - PUT /v1/workflows/{id}/executions/{execution_id}/resume - Resume execution
 * - PUT /v1/workflows/{id}/executions/{execution_id}/cancel - Cancel execution
 * - GET /v1/workflows/metrics - Get workflow system metrics
 * - POST /v1/workflows/templates/{type} - Create workflow from template
 */
class KOLOSAL_AGENT_API WorkflowRoute : public kolosal::api::IRoute {
public:
    /**
     * @brief Constructor
     * @param workflow_engine Shared pointer to the workflow engine instance
     */
    explicit WorkflowRoute(std::shared_ptr<kolosal::agents::WorkflowEngine> workflow_engine);

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
    std::shared_ptr<kolosal::agents::WorkflowEngine> workflow_engine_;
    mutable std::string matched_method_;
    mutable std::string matched_path_;

    // Handler methods for different endpoints
    void handleList_Workflows(SocketType sock);
    void handleCreate_Workflow(SocketType sock, const std::string& body);
    void handleGet_Workflow(SocketType sock, const std::string& workflow_id);
    void handleUpdate_Workflow(SocketType sock, const std::string& workflow_id, const std::string& body);
    void handleDelete_Workflow(SocketType sock, const std::string& workflow_id);
    void handleExecute_Workflow(SocketType sock, const std::string& workflow_id, const std::string& body);
    void handleList_Executions(SocketType sock, const std::string& workflow_id);
    void handleGet_Execution_Status(SocketType sock, const std::string& workflow_id, const std::string& execution_id);
    void handleGet_Execution_Result(SocketType sock, const std::string& workflow_id, const std::string& execution_id);
    void handlePause_Execution(SocketType sock, const std::string& workflow_id, const std::string& execution_id);
    void handleResume_Execution(SocketType sock, const std::string& workflow_id, const std::string& execution_id);
    void handleCancel_Execution(SocketType sock, const std::string& workflow_id, const std::string& execution_id);
    void handleGet_Metrics(SocketType sock);
    void handleCreate_Template(SocketType sock, const std::string& template_type, const std::string& body);

    // Utility methods
    void sendJson_Response(SocketType sock, int status_code, const std::string& json_body);
    void sendError_Response(SocketType sock, int status_code, const std::string& error, const std::string& message = "");
    std::string extractWorkflow_Id_From_Path(const std::string& path);
    std::string extractExecution_Id_From_Path(const std::string& path);
    std::string extractTemplate_Type_From_Path(const std::string& path);
    std::string workflowTo_Json(const kolosal::agents::Workflow& workflow);
    std::string executionContextTo_Json(const kolosal::agents::WorkflowExecutionContext& context);
    std::string metricsTo_Json(const kolosal::agents::WorkflowMetrics& metrics);
    kolosal::agents::Workflow parseWorkflow_From_Json(const std::string& json_body);
    std::string statusTo_String(kolosal::agents::WorkflowStatus status);
    std::string stepStatusTo_String(kolosal::agents::StepStatus status);
};

} // namespace kolosal::api

#endif // KOLOSAL_AGENT_INCLUDE_API_WORKFLOW_ROUTE_HPP_INCLUDED
