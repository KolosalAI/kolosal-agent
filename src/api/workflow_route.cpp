/**
 * @file workflow_route.cpp
 * @brief REST API routes for workflow management
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "api/workflow_route.hpp"
#include <json.hpp>
#include <regex>
#include <sstream>
#include <iostream>

namespace kolosal::api {

WorkflowRoute::WorkflowRoute(std::shared_ptr<kolosal::agents::WorkflowEngine> workflow_engine)
    : workflow_engine_(workflow_engine) {
    if (!workflow_engine_) {
        throw std::invalid_argument("Workflow engine cannot be null");
    }
}

bool WorkflowRoute::match(const std::string& method, const std::string& path) {
    // Store the method and path for later use in handle()
    matched_method_ = method;
    matched_path_ = path;
    
    // Check if the path starts with /v1/workflows
    if (path.find("/v1/workflows") == 0) {
        return true;
    }
    
    return false;
}

void WorkflowRoute::handle(SocketType sock, const std::string& body) {
    try {
        const std::string& method = matched_method_;
        const std::string& path = matched_path_;
        
        // Route to appropriate handler based on path pattern and method
        if (path == "/v1/workflows" && method == "GET") {
            handleList_Workflows(sock);
        }
        else if (path == "/v1/workflows" && method == "POST") {
            handleCreate_Workflow(sock, body);
        }
        else if (path == "/v1/workflows/metrics" && method == "GET") {
            handleGet_Metrics(sock);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/templates/([^/]+)$")) && method == "POST") {
            std::string template_type = extractTemplate_Type_From_Path(path);
            handleCreate_Template(sock, template_type, body);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/([^/]+)$")) && method == "GET") {
            std::string workflow_id = extractWorkflow_Id_From_Path(path);
            handleGet_Workflow(sock, workflow_id);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/([^/]+)$")) && method == "PUT") {
            std::string workflow_id = extractWorkflow_Id_From_Path(path);
            handleUpdate_Workflow(sock, workflow_id, body);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/([^/]+)$")) && method == "DELETE") {
            std::string workflow_id = extractWorkflow_Id_From_Path(path);
            handleDelete_Workflow(sock, workflow_id);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/([^/]+)/execute$")) && method == "POST") {
            std::string workflow_id = extractWorkflow_Id_From_Path(path);
            handleExecute_Workflow(sock, workflow_id, body);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/([^/]+)/executions$")) && method == "GET") {
            std::string workflow_id = extractWorkflow_Id_From_Path(path);
            handleList_Executions(sock, workflow_id);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/([^/]+)/executions/([^/]+)/status$")) && method == "GET") {
            std::string workflow_id = extractWorkflow_Id_From_Path(path);
            std::string execution_id = extractExecution_Id_From_Path(path);
            handleGet_Execution_Status(sock, workflow_id, execution_id);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/([^/]+)/executions/([^/]+)/result$")) && method == "GET") {
            std::string workflow_id = extractWorkflow_Id_From_Path(path);
            std::string execution_id = extractExecution_Id_From_Path(path);
            handleGet_Execution_Result(sock, workflow_id, execution_id);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/([^/]+)/executions/([^/]+)/pause$")) && method == "PUT") {
            std::string workflow_id = extractWorkflow_Id_From_Path(path);
            std::string execution_id = extractExecution_Id_From_Path(path);
            handlePause_Execution(sock, workflow_id, execution_id);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/([^/]+)/executions/([^/]+)/resume$")) && method == "PUT") {
            std::string workflow_id = extractWorkflow_Id_From_Path(path);
            std::string execution_id = extractExecution_Id_From_Path(path);
            handleResume_Execution(sock, workflow_id, execution_id);
        }
        else if (std::regex_match(path, std::regex("^/v1/workflows/([^/]+)/executions/([^/]+)/cancel$")) && method == "PUT") {
            std::string workflow_id = extractWorkflow_Id_From_Path(path);
            std::string execution_id = extractExecution_Id_From_Path(path);
            handleCancel_Execution(sock, workflow_id, execution_id);
        }
        else {
            sendError_Response(sock, 404, "Not Found", "Workflow endpoint not found");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in WorkflowRoute::handle: " << e.what() << std::endl;
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handleList_Workflows(SocketType sock) {
    try {
        auto workflow_ids = workflow_engine_->list_workflows();
        
        nlohmann::json response;
        response["workflows"] = nlohmann::json::array();
        
        for (const auto& workflow_id : workflow_ids) {
            auto workflow = workflow_engine_->get_workflow(workflow_id);
            if (workflow) {
                nlohmann::json workflow_info;
                workflow_info["id"] = workflow->workflow_id;
                workflow_info["name"] = workflow->name;
                workflow_info["description"] = workflow->description;
                workflow_info["type"] = static_cast<int>(workflow->type);
                workflow_info["status"] = statusTo_String(workflow->status);
                workflow_info["step_count"] = workflow->steps.size();
                workflow_info["created_by"] = workflow->created_by;
                response["workflows"].push_back(workflow_info);
            }
        }
        
        response["total"] = workflow_ids.size();
        sendJson_Response(sock, 200, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handleCreate_Workflow(SocketType sock, const std::string& body) {
    try {
        if (body.empty()) {
            sendError_Response(sock, 400, "Bad Request", "Request body is required");
            return;
        }
        
        auto workflow = parseWorkflow_From_Json(body);
        std::string workflow_id = workflow_engine_->create_workflow(workflow);
        
        nlohmann::json response;
        response["workflow_id"] = workflow_id;
        response["status"] = "created";
        response["message"] = "Workflow created successfully";
        
        sendJson_Response(sock, 201, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 400, "Bad Request", e.what());
    }
}

void WorkflowRoute::handleGet_Workflow(SocketType sock, const std::string& workflow_id) {
    try {
        auto workflow = workflow_engine_->get_workflow(workflow_id);
        if (!workflow) {
            sendError_Response(sock, 404, "Not Found", "Workflow not found");
            return;
        }
        
        std::string workflow_json = workflowTo_Json(*workflow);
        sendJson_Response(sock, 200, workflow_json);
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handleUpdate_Workflow(SocketType sock, const std::string& workflow_id, const std::string& body) {
    try {
        if (body.empty()) {
            sendError_Response(sock, 400, "Bad Request", "Request body is required");
            return;
        }
        
        auto workflow = parseWorkflow_From_Json(body);
        workflow.workflow_id = workflow_id;  // Ensure ID matches
        
        bool updated = workflow_engine_->update_workflow(workflow_id, workflow);
        if (!updated) {
            sendError_Response(sock, 404, "Not Found", "Workflow not found or cannot be updated");
            return;
        }
        
        nlohmann::json response;
        response["workflow_id"] = workflow_id;
        response["status"] = "updated";
        response["message"] = "Workflow updated successfully";
        
        sendJson_Response(sock, 200, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 400, "Bad Request", e.what());
    }
}

void WorkflowRoute::handleDelete_Workflow(SocketType sock, const std::string& workflow_id) {
    try {
        bool deleted = workflow_engine_->delete_workflow(workflow_id);
        if (!deleted) {
            sendError_Response(sock, 404, "Not Found", "Workflow not found");
            return;
        }
        
        nlohmann::json response;
        response["workflow_id"] = workflow_id;
        response["status"] = "deleted";
        response["message"] = "Workflow deleted successfully";
        
        sendJson_Response(sock, 200, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handleExecute_Workflow(SocketType sock, const std::string& workflow_id, const std::string& body) {
    try {
        nlohmann::json input_context;
        if (!body.empty()) {
            input_context = nlohmann::json::parse(body);
        }
        
        std::string execution_id = workflow_engine_->execute_workflow(workflow_id, input_context);
        if (execution_id.empty()) {
            sendError_Response(sock, 404, "Not Found", "Workflow not found or cannot be executed");
            return;
        }
        
        nlohmann::json response;
        response["execution_id"] = execution_id;
        response["workflow_id"] = workflow_id;
        response["status"] = "started";
        response["message"] = "Workflow execution started";
        
        sendJson_Response(sock, 202, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 400, "Bad Request", e.what());
    }
}

void WorkflowRoute::handleList_Executions(SocketType sock, const std::string& workflow_id) {
    try {
        auto executions = workflow_engine_->get_execution_history(workflow_id);
        
        nlohmann::json response;
        response["executions"] = nlohmann::json::array();
        
        for (const auto& execution : executions) {
            nlohmann::json execution_info;
            execution_info["execution_id"] = execution.execution_id;
            execution_info["workflow_id"] = execution.workflow_id;
            execution_info["status"] = statusTo_String(execution.current_status);
            execution_info["current_step"] = execution.current_step_id;
            execution_info["completed_steps"] = execution.completed_steps.size();
            execution_info["failed_steps"] = execution.failed_steps.size();
            response["executions"].push_back(execution_info);
        }
        
        response["total"] = executions.size();
        sendJson_Response(sock, 200, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handleGet_Execution_Status(SocketType sock, const std::string& workflow_id, const std::string& execution_id) {
    try {
        auto execution_context = workflow_engine_->get_execution_status(execution_id);
        if (!execution_context) {
            sendError_Response(sock, 404, "Not Found", "Execution not found");
            return;
        }
        
        if (execution_context->workflow_id != workflow_id) {
            sendError_Response(sock, 400, "Bad Request", "Execution does not belong to specified workflow");
            return;
        }
        
        nlohmann::json response;
        response["execution_id"] = execution_context->execution_id;
        response["workflow_id"] = execution_context->workflow_id;
        response["status"] = statusTo_String(execution_context->current_status);
        response["current_step"] = execution_context->current_step_id;
        response["completed_steps"] = execution_context->completed_steps.size();
        response["failed_steps"] = execution_context->failed_steps.size();
        response["total_steps"] = execution_context->step_statuses.size();
        
        // Calculate progress percentage
        if (!execution_context->step_statuses.empty()) {
            double progress = (double)execution_context->completed_steps.size() / execution_context->step_statuses.size() * 100.0;
            response["progress"] = progress;
        } else {
            response["progress"] = 0.0;
        }
        
        // Calculate execution time
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - execution_context->execution_start);
        response["execution_time"] = duration.count() / 1000.0;  // Convert to seconds
        
        sendJson_Response(sock, 200, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handleGet_Execution_Result(SocketType sock, const std::string& workflow_id, const std::string& execution_id) {
    try {
        auto execution_context = workflow_engine_->get_execution_status(execution_id);
        if (!execution_context) {
            sendError_Response(sock, 404, "Not Found", "Execution not found");
            return;
        }
        
        if (execution_context->workflow_id != workflow_id) {
            sendError_Response(sock, 400, "Bad Request", "Execution does not belong to specified workflow");
            return;
        }
        
        nlohmann::json response;
        response["execution_id"] = execution_context->execution_id;
        response["workflow_id"] = execution_context->workflow_id;
        response["success"] = (execution_context->current_status == kolosal::agents::WorkflowStatus::COMPLETED);
        response["status"] = statusTo_String(execution_context->current_status);
        response["completed_steps"] = execution_context->completed_steps.size();
        response["failed_steps"] = execution_context->failed_steps.size();
        response["total_steps"] = execution_context->step_statuses.size();
        
        // Include step outputs
        response["results"] = execution_context->step_outputs;
        
        // Calculate execution time
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - execution_context->execution_start);
        response["execution_time"] = duration.count() / 1000.0;  // Convert to seconds
        
        // Add step results details
        nlohmann::json step_results;
        for (const auto& [step_id, status] : execution_context->step_statuses) {
            nlohmann::json step_info;
            step_info["status"] = stepStatusTo_String(status);
            if (execution_context->step_outputs.find(step_id) != execution_context->step_outputs.end()) {
                step_info["output"] = execution_context->step_outputs[step_id];
            }
            step_results[step_id] = step_info;
        }
        response["step_results"] = step_results;
        
        sendJson_Response(sock, 200, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handlePause_Execution(SocketType sock, const std::string& workflow_id, const std::string& execution_id) {
    try {
        bool paused = workflow_engine_->pause_workflow(execution_id);
        if (!paused) {
            sendError_Response(sock, 400, "Bad Request", "Cannot pause execution");
            return;
        }
        
        nlohmann::json response;
        response["execution_id"] = execution_id;
        response["workflow_id"] = workflow_id;
        response["status"] = "paused";
        response["message"] = "Workflow execution paused";
        
        sendJson_Response(sock, 200, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handleResume_Execution(SocketType sock, const std::string& workflow_id, const std::string& execution_id) {
    try {
        bool resumed = workflow_engine_->resume_workflow(execution_id);
        if (!resumed) {
            sendError_Response(sock, 400, "Bad Request", "Cannot resume execution");
            return;
        }
        
        nlohmann::json response;
        response["execution_id"] = execution_id;
        response["workflow_id"] = workflow_id;
        response["status"] = "resumed";
        response["message"] = "Workflow execution resumed";
        
        sendJson_Response(sock, 200, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handleCancel_Execution(SocketType sock, const std::string& workflow_id, const std::string& execution_id) {
    try {
        bool cancelled = workflow_engine_->cancel_workflow(execution_id);
        if (!cancelled) {
            sendError_Response(sock, 400, "Bad Request", "Cannot cancel execution");
            return;
        }
        
        nlohmann::json response;
        response["execution_id"] = execution_id;
        response["workflow_id"] = workflow_id;
        response["status"] = "cancelled";
        response["message"] = "Workflow execution cancelled";
        
        sendJson_Response(sock, 200, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handleGet_Metrics(SocketType sock) {
    try {
        auto metrics = workflow_engine_->get_metrics();
        std::string metrics_json = metricsTo_Json(metrics);
        sendJson_Response(sock, 200, metrics_json);
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 500, "Internal Server Error", e.what());
    }
}

void WorkflowRoute::handleCreate_Template(SocketType sock, const std::string& template_type, const std::string& body) {
    try {
        nlohmann::json request_data;
        if (!body.empty()) {
            request_data = nlohmann::json::parse(body);
        }
        
        std::string name = request_data.value("name", "Template Workflow");
        kolosal::agents::Workflow workflow;
        
        if (template_type == "sequential") {
            std::vector<std::pair<std::string, std::string>> agent_functions;
            if (request_data.contains("steps")) {
                for (const auto& step : request_data["steps"]) {
                    agent_functions.emplace_back(
                        step.value("agent_id", "research_analyst"),
                        step.value("function", "research_topic")
                    );
                }
            } else {
                // Default sequential workflow steps
                agent_functions = {
                    {"research_analyst", "research_topic"},
                    {"research_analyst", "analyze_data"},
                    {"research_analyst", "synthesize_information"},
                    {"research_analyst", "generate_report"}
                };
            }
            workflow = workflow_engine_->create_sequential_workflow(name, agent_functions);
        }
        else if (template_type == "parallel") {
            std::vector<std::pair<std::string, std::string>> agent_functions = {
                {"research_analyst", "research_topic"},
                {"task_executor", "execute_task"},
                {"document_specialist", "parse_document"}
            };
            workflow = workflow_engine_->create_parallel_workflow(name, agent_functions);
        }
        else if (template_type == "pipeline") {
            std::vector<std::pair<std::string, std::string>> agent_functions = {
                {"research_analyst", "research_topic"},
                {"research_analyst", "analyze_data"},
                {"document_specialist", "generate_content"}
            };
            workflow = workflow_engine_->create_pipeline_workflow(name, agent_functions);
        }
        else if (template_type == "consensus") {
            std::vector<std::string> agent_ids = {"research_analyst", "quality_assurance"};
            workflow = workflow_engine_->create_consensus_workflow(name, agent_ids, "vote_on_decision");
        }
        else {
            sendError_Response(sock, 400, "Bad Request", "Unknown template type: " + template_type);
            return;
        }
        
        std::string workflow_id = workflow_engine_->create_workflow(workflow);
        
        nlohmann::json response;
        response["workflow_id"] = workflow_id;
        response["template_type"] = template_type;
        response["status"] = "created";
        response["message"] = "Workflow created from template";
        
        sendJson_Response(sock, 201, response.dump());
        
    } catch (const std::exception& e) {
        sendError_Response(sock, 400, "Bad Request", e.what());
    }
}

// Utility methods implementation

void WorkflowRoute::sendJson_Response(SocketType sock, int status_code, const std::string& json_body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " ";
    
    switch (status_code) {
        case 200: response << "OK"; break;
        case 201: response << "Created"; break;
        case 202: response << "Accepted"; break;
        case 400: response << "Bad Request"; break;
        case 404: response << "Not Found"; break;
        case 500: response << "Internal Server Error"; break;
        default: response << "Unknown"; break;
    }
    
    response << "\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response << "Content-Length: " << json_body.length() << "\r\n";
    response << "\r\n";
    response << json_body;
    
    std::string response_str = response.str();
    send(sock, response_str.c_str(), response_str.length(), 0);
}

void WorkflowRoute::sendError_Response(SocketType sock, int status_code, const std::string& error, const std::string& message) {
    nlohmann::json error_response;
    error_response["error"] = {
        {"type", error},
        {"code", status_code}
    };
    
    if (!message.empty()) {
        error_response["error"]["message"] = message;
    }
    
    sendJson_Response(sock, status_code, error_response.dump());
}

std::string WorkflowRoute::extractWorkflow_Id_From_Path(const std::string& path) {
    std::regex pattern("^/v1/workflows/([^/]+)");
    std::smatch matches;
    
    if (std::regex_search(path, matches, pattern) && matches.size() > 1) {
        return matches[1].str();
    }
    
    return "";
}

std::string WorkflowRoute::extractExecution_Id_From_Path(const std::string& path) {
    std::regex pattern("^/v1/workflows/[^/]+/executions/([^/]+)");
    std::smatch matches;
    
    if (std::regex_search(path, matches, pattern) && matches.size() > 1) {
        return matches[1].str();
    }
    
    return "";
}

std::string WorkflowRoute::extractTemplate_Type_From_Path(const std::string& path) {
    std::regex pattern("^/v1/workflows/templates/([^/]+)$");
    std::smatch matches;
    
    if (std::regex_search(path, matches, pattern) && matches.size() > 1) {
        return matches[1].str();
    }
    
    return "";
}

std::string WorkflowRoute::workflowTo_Json(const kolosal::agents::Workflow& workflow) {
    nlohmann::json json_workflow;
    
    json_workflow["id"] = workflow.workflow_id;
    json_workflow["name"] = workflow.name;
    json_workflow["description"] = workflow.description;
    json_workflow["type"] = static_cast<int>(workflow.type);
    json_workflow["global_context"] = workflow.global_context;
    json_workflow["status"] = statusTo_String(workflow.status);
    json_workflow["created_by"] = workflow.created_by;
    
    // Convert steps
    json_workflow["steps"] = nlohmann::json::array();
    for (const auto& step : workflow.steps) {
        nlohmann::json json_step;
        json_step["id"] = step.step_id;
        json_step["name"] = step.name;
        json_step["description"] = step.description;
        json_step["agent_id"] = step.agent_id;
        json_step["function"] = step.function_name;
        json_step["parameters"] = step.parameters;
        json_step["timeout"] = step.timeout_seconds;
        json_step["max_retries"] = step.max_retries;
        json_step["retry_delay"] = step.retry_delay_seconds;
        json_step["continue_on_error"] = step.continue_on_error;
        json_step["status"] = stepStatusTo_String(step.status);
        
        // Add dependencies
        json_step["depends_on"] = nlohmann::json::array();
        for (const auto& dep : step.dependencies) {
            json_step["depends_on"].push_back(dep.step_id);
        }
        
        json_workflow["steps"].push_back(json_step);
    }
    
    // Convert settings
    json_workflow["settings"] = {
        {"max_execution_time", workflow.max_execution_time_seconds},
        {"max_concurrent_steps", workflow.max_concurrent_steps},
        {"auto_cleanup", workflow.auto_cleanup},
        {"persist_state", workflow.persist_state}
    };
    
    // Convert error handling
    json_workflow["error_handling"] = {
        {"retry_on_failure", workflow.error_handling.retry_on_failure},
        {"max_retries", workflow.error_handling.max_retries},
        {"retry_delay_seconds", workflow.error_handling.retry_delay_seconds},
        {"continue_on_error", workflow.error_handling.continue_on_error},
        {"use_fallback_agent", workflow.error_handling.use_fallback_agent},
        {"fallback_agent_id", workflow.error_handling.fallback_agent_id}
    };
    
    return json_workflow.dump();
}

std::string WorkflowRoute::executionContextTo_Json(const kolosal::agents::WorkflowExecutionContext& context) {
    nlohmann::json json_context;
    
    json_context["execution_id"] = context.execution_id;
    json_context["workflow_id"] = context.workflow_id;
    json_context["status"] = statusTo_String(context.current_status);
    json_context["current_step"] = context.current_step_id;
    json_context["completed_steps"] = context.completed_steps;
    json_context["failed_steps"] = context.failed_steps;
    json_context["global_variables"] = context.global_variables;
    json_context["step_outputs"] = context.step_outputs;
    
    return json_context.dump();
}

std::string WorkflowRoute::metricsTo_Json(const kolosal::agents::WorkflowMetrics& metrics) {
    nlohmann::json json_metrics;
    
    json_metrics["total_workflows"] = metrics.total_workflows;
    json_metrics["running_workflows"] = metrics.running_workflows;
    json_metrics["completed_workflows"] = metrics.completed_workflows;
    json_metrics["failed_workflows"] = metrics.failed_workflows;
    json_metrics["cancelled_workflows"] = metrics.cancelled_workflows;
    json_metrics["average_execution_time_ms"] = metrics.average_execution_time_ms;
    json_metrics["success_rate"] = metrics.success_rate;
    json_metrics["error_counts"] = metrics.error_counts;
    
    return json_metrics.dump();
}

kolosal::agents::Workflow WorkflowRoute::parseWorkflow_From_Json(const std::string& json_body) {
    nlohmann::json json_data = nlohmann::json::parse(json_body);
    
    kolosal::agents::Workflow workflow;
    workflow.workflow_id = json_data.value("id", "");
    workflow.name = json_data.value("name", "");
    workflow.description = json_data.value("description", "");
    workflow.type = static_cast<kolosal::agents::WorkflowType>(json_data.value("type", 0));
    workflow.global_context = json_data.value("global_context", nlohmann::json::object());
    workflow.created_by = json_data.value("created_by", "api");
    
    // Parse steps
    if (json_data.contains("steps")) {
        for (const auto& json_step : json_data["steps"]) {
            kolosal::agents::WorkflowStep step;
            step.step_id = json_step.value("id", "");
            step.name = json_step.value("name", "");
            step.description = json_step.value("description", "");
            step.agent_id = json_step.value("agent_id", "");
            step.function_name = json_step.value("function", "");
            step.parameters = json_step.value("parameters", nlohmann::json::object());
            step.timeout_seconds = json_step.value("timeout", 30);
            step.max_retries = json_step.value("max_retries", 3);
            step.retry_delay_seconds = json_step.value("retry_delay", 1);
            step.continue_on_error = json_step.value("continue_on_error", false);
            
            // Parse dependencies
            if (json_step.contains("depends_on")) {
                for (const auto& dep_id : json_step["depends_on"]) {
                    kolosal::agents::StepDependency dep;
                    dep.step_id = dep_id.get<std::string>();
                    dep.condition = "success";  // Default condition
                    dep.required = true;
                    step.dependencies.push_back(dep);
                }
            }
            
            workflow.steps.push_back(step);
        }
    }
    
    // Parse settings
    if (json_data.contains("settings")) {
        const auto& settings = json_data["settings"];
        workflow.max_execution_time_seconds = settings.value("max_execution_time", 300);
        workflow.max_concurrent_steps = settings.value("max_concurrent_steps", 4);
        workflow.auto_cleanup = settings.value("auto_cleanup", true);
        workflow.persist_state = settings.value("persist_state", true);
    }
    
    // Parse error handling
    if (json_data.contains("error_handling")) {
        const auto& error_handling = json_data["error_handling"];
        workflow.error_handling.retry_on_failure = error_handling.value("retry_on_failure", true);
        workflow.error_handling.max_retries = error_handling.value("max_retries", 3);
        workflow.error_handling.retry_delay_seconds = error_handling.value("retry_delay_seconds", 1);
        workflow.error_handling.continue_on_error = error_handling.value("continue_on_error", false);
        workflow.error_handling.use_fallback_agent = error_handling.value("use_fallback_agent", false);
        workflow.error_handling.fallback_agent_id = error_handling.value("fallback_agent_id", "");
    }
    
    return workflow;
}

std::string WorkflowRoute::statusTo_String(kolosal::agents::WorkflowStatus status) {
    switch (status) {
        case kolosal::agents::WorkflowStatus::PENDING: return "pending";
        case kolosal::agents::WorkflowStatus::RUNNING: return "running";
        case kolosal::agents::WorkflowStatus::PAUSED: return "paused";
        case kolosal::agents::WorkflowStatus::COMPLETED: return "completed";
        case kolosal::agents::WorkflowStatus::FAILED: return "failed";
        case kolosal::agents::WorkflowStatus::CANCELLED: return "cancelled";
        case kolosal::agents::WorkflowStatus::TIMEOUT: return "timeout";
        default: return "unknown";
    }
}

std::string WorkflowRoute::stepStatusTo_String(kolosal::agents::StepStatus status) {
    switch (status) {
        case kolosal::agents::StepStatus::PENDING: return "pending";
        case kolosal::agents::StepStatus::RUNNING: return "running";
        case kolosal::agents::StepStatus::COMPLETED: return "completed";
        case kolosal::agents::StepStatus::FAILED: return "failed";
        case kolosal::agents::StepStatus::SKIPPED: return "skipped";
        case kolosal::agents::StepStatus::RETRYING: return "retrying";
        default: return "unknown";
    }
}

} // namespace kolosal::api
