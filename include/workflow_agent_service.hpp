/**
 * @file workflow_agent_service.hpp
 * @brief Service layer implementation for workflow agent
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_WORKFLOW_AGENT_SERVICE_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_WORKFLOW_AGENT_SERVICE_HPP_INCLUDED

#include "agent/agent_interfaces.hpp"
#include <json.hpp>
#include <memory>
#include <future>
#include <vector>
#include <string>
#include <map>
#include <thread>

namespace kolosal::agents {

using json = nlohmann::json;
/**
 * @brief Provides workflow agent services
 */
class WorkflowAgentService {
public:
    WorkflowAgentService() = default;
    ~WorkflowAgentService() = default;

    // Workflow management structures
    struct WorkflowRequest {
        std::string name;
        std::string description;
        json workflow_definition;
        json parameters;
        std::string type = "sequential"; // sequential, parallel, conditional
        
        void from_json(const json& j);
        bool validate() const;
    };

    struct WorkflowResponse {
        bool success = false;
        std::string message;
        std::string workflow_id;
        std::string status;
        json result;
        std::vector<std::string> errors;
        
        json to_json() const;
    };

    struct WorkflowExecutionRequest {
        std::string workflow_id;
        json input_parameters;
        bool async_execution = true;
        void from_json(const json& j);
        bool validate() const;
    };

    struct WorkflowExecutionResponse {
        bool success = false;
        std::string message;
        std::string execution_id;
        std::string status; // pending, running, completed, failed
        json output;
        std::vector<std::string> step_results;
        
        json to_json() const;
    };

    struct WorkflowStatusRequest {
        std::string workflow_id;
        std::string execution_id;
        
        void from_json(const json& j);
        bool validate() const;
    };

    struct RAGWorkflowRequest {
        std::string query;
        std::string collection_name = "documents";
        int retrieve_k = 5;
        double score_threshold = 0.6;
        std::string completion_model;
        json rag_config;
        
        void from_json(const json& j);
        bool validate() const;
    };

    struct RAGWorkflowResponse {
        bool success = false;
        std::string message;
        std::string response_text;
        std::vector<json> retrieved_documents;
        json metadata;
        
        json to_json() const;
    };

    // Session management
    struct SessionRequest {
        std::string session_name;
        json session_config;
        std::string workflow_type;
        
        void from_json(const json& j);
        bool validate() const;
    };

    struct SessionResponse {
        bool success = false;
        std::string message;
        std::string session_id;
        json session_info;
        
        json to_json() const;
    };

    // Core service methods
    std::future<WorkflowResponse> create_Workflow(const WorkflowRequest& request);
    std::future<WorkflowExecutionResponse> execute_Workflow(const WorkflowExecutionRequest& request);
    std::future<WorkflowResponse> getWorkflow_Status(const WorkflowStatusRequest& request);
    std::future<json> list_Workflows();
    std::future<WorkflowResponse> delete_Workflow(const std::string& workflow_id);
    
    // RAG workflow operations
    std::future<RAGWorkflowResponse> execute_RAGWorkflow(const RAGWorkflowRequest& request);
    std::future<RAGWorkflowResponse> search_RAGContext(const RAGWorkflowRequest& request);
    
    // Session management
    std::future<SessionResponse> create_Session(const SessionRequest& request);
    std::future<SessionResponse> get_Session(const std::string& session_id);
    std::future<json> list_Sessions();
    std::future<SessionResponse> delete_Session(const std::string& session_id);
    std::future<json> getSession_History(const std::string& session_id);

    // Orchestration operations
    std::future<json> createOrchestration_Plan(const json& request);
    std::future<json> executeOrchestration_Plan(const std::string& plan_id, const json& parameters);
    std::future<json> getOrchestration_Status(const std::string& plan_id);

private:
    std::string generateWorkflow_Id();
    std::string generateExecution_Id();
    std::string generateSession_Id();
    std::string generateError_Message(const std::string& operation, const std::exception& e);
    
    // Internal workflow storage (in a real implementation, this would be persistent)
    std::map<std::string, json> workflows_;
    std::map<std::string, json> executions_;
    std::map<std::string, json> sessions_;
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_WORKFLOW_AGENT_SERVICE_HPP_INCLUDED
