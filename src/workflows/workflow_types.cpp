#include "../include/workflow_types.hpp"
#include "../include/workflow_manager.hpp"
#include <random>
#include <future>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

WorkflowOrchestrator::WorkflowOrchestrator(std::shared_ptr<WorkflowManager> workflow_manager)
    : workflow_manager_(workflow_manager) {
}

WorkflowOrchestrator::~WorkflowOrchestrator() {
    stop();
}

bool WorkflowOrchestrator::start() {
    if (running_.load()) {
        return true;
    }
    
    running_ = true;
    
    // Start orchestrator threads (2 threads for handling executions)
    for (int i = 0; i < 2; ++i) {
        orchestrator_threads_.emplace_back(&WorkflowOrchestrator::orchestrator_thread, this);
    }
    
    // Register built-in workflows
    register_builtin_workflows();
    
    return true;
}

void WorkflowOrchestrator::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_ = false;
    execution_condition_.notify_all();
    
    for (auto& thread : orchestrator_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    orchestrator_threads_.clear();
}

bool WorkflowOrchestrator::load_workflow_config(const std::string& config_file_path) {
    try {
        config_file_path_ = config_file_path;
        YAML::Node yaml_config = YAML::LoadFile(config_file_path);
        
        // Load agent-LLM mappings directly from YAML
        if (yaml_config["agent_llm_mappings"]) {
            // Convert only the agent_llm_mappings section to JSON for compatibility
            std::function<json(const YAML::Node&)> yaml_to_json_simple = [&](const YAML::Node& node) -> json {
                json result;
                if (!node.IsDefined() || node.IsNull()) {
                    return json();
                }
                
                if (node.IsScalar()) {
                    try {
                        return json(node.as<std::string>());
                    } catch (...) {
                        return json("");
                    }
                } else if (node.IsSequence()) {
                    result = json::array();
                    for (auto it = node.begin(); it != node.end(); ++it) {
                        if (it->IsDefined() && !it->IsNull()) {
                            result.push_back(yaml_to_json_simple(*it));
                        }
                    }
                } else if (node.IsMap()) {
                    result = json::object();
                    for (auto it = node.begin(); it != node.end(); ++it) {
                        if (it->first.IsDefined() && !it->first.IsNull() && 
                            it->second.IsDefined() && !it->second.IsNull()) {
                            try {
                                std::string key = it->first.as<std::string>();
                                result[key] = yaml_to_json_simple(it->second);
                            } catch (...) {
                                // Skip invalid keys
                            }
                        }
                    }
                }
                return result;
            };
            
            json agent_mappings = yaml_to_json_simple(yaml_config["agent_llm_mappings"]);
            load_agent_llm_mappings(agent_mappings);
        }
        
        // Load workflow definitions directly from YAML
        if (yaml_config["workflows"] && yaml_config["workflows"].IsSequence()) {
            std::lock_guard<std::mutex> lock(orchestrator_mutex_);
            std::cout << "DEBUG: Found " << yaml_config["workflows"].size() << " workflows in YAML config" << std::endl;
            for (const auto& workflow_node : yaml_config["workflows"]) {
                try {
                    std::cout << "DEBUG: Processing workflow YAML node" << std::endl;
                    WorkflowDefinition workflow = parse_workflow_from_yaml(workflow_node);
                    if (validate_workflow_definition(workflow)) {
                        workflow_definitions_[workflow.id] = workflow;
                        std::cout << "DEBUG: Successfully registered workflow: " << workflow.id << std::endl;
                    }
                } catch (const std::exception& e) {
                    // Log error but continue with other workflows
                    std::cerr << "Error parsing workflow: " << e.what() << std::endl;
                }
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading workflow config: " << e.what() << std::endl;
        return false;
    }
}

void WorkflowOrchestrator::reload_workflow_config() {
    if (!config_file_path_.empty()) {
        load_workflow_config(config_file_path_);
    }
}

json WorkflowOrchestrator::get_workflow_config() const {
    return workflow_config_;
}

void WorkflowOrchestrator::register_workflow(const WorkflowDefinition& workflow) {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    workflow_definitions_.insert_or_assign(workflow.id, workflow);
}

bool WorkflowOrchestrator::remove_workflow(const std::string& workflow_id) {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    return workflow_definitions_.erase(workflow_id) > 0;
}

std::vector<WorkflowDefinition> WorkflowOrchestrator::list_workflows() const {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    std::vector<WorkflowDefinition> workflows;
    for (const auto& pair : workflow_definitions_) {
        workflows.push_back(pair.second);
    }
    return workflows;
}

WorkflowDefinition* WorkflowOrchestrator::get_workflow(const std::string& workflow_id) {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    auto it = workflow_definitions_.find(workflow_id);
    return (it != workflow_definitions_.end()) ? &it->second : nullptr;
}

std::string WorkflowOrchestrator::execute_workflow(const std::string& workflow_id, const json& input_data) {
    auto execution_id = execute_workflow_async(workflow_id, input_data);
    
    // Wait for completion
    while (true) {
        auto execution = get_execution_status(execution_id);
        if (!execution) break;
        
        if (execution->state == WorkflowExecutionState::COMPLETED ||
            execution->state == WorkflowExecutionState::FAILED ||
            execution->state == WorkflowExecutionState::CANCELLED ||
            execution->state == WorkflowExecutionState::TIMEOUT) {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return execution_id;
}

std::string WorkflowOrchestrator::execute_workflow_async(const std::string& workflow_id, const json& input_data) {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    
    // Check if workflow exists
    auto it = workflow_definitions_.find(workflow_id);
    if (it == workflow_definitions_.end()) {
        throw std::invalid_argument("Workflow not found: " + workflow_id);
    }
    
    // Create execution
    std::string execution_id = generate_execution_id();
    auto execution = std::make_shared<WorkflowExecution>(execution_id, workflow_id);
    execution->input_data = input_data;
    execution->context = it->second.global_context;
    execution->context["input"] = input_data;
    
    // Add to active executions
    active_executions_[execution_id] = execution;
    
    // Notify orchestrator threads
    execution_condition_.notify_one();
    
    return execution_id;
}

bool WorkflowOrchestrator::pause_execution(const std::string& execution_id) {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    auto it = active_executions_.find(execution_id);
    if (it != active_executions_.end() && it->second->state == WorkflowExecutionState::RUNNING) {
        it->second->state = WorkflowExecutionState::PAUSED;
        return true;
    }
    return false;
}

bool WorkflowOrchestrator::resume_execution(const std::string& execution_id) {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    auto it = active_executions_.find(execution_id);
    if (it != active_executions_.end() && it->second->state == WorkflowExecutionState::PAUSED) {
        it->second->state = WorkflowExecutionState::RUNNING;
        execution_condition_.notify_one();
        return true;
    }
    return false;
}

bool WorkflowOrchestrator::cancel_execution(const std::string& execution_id) {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    auto it = active_executions_.find(execution_id);
    if (it != active_executions_.end()) {
        it->second->state = WorkflowExecutionState::CANCELLED;
        it->second->error_message = "Execution cancelled by user";
        execution_condition_.notify_one();
        return true;
    }
    return false;
}

std::shared_ptr<WorkflowExecution> WorkflowOrchestrator::get_execution_status(const std::string& execution_id) {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    
    auto it = active_executions_.find(execution_id);
    if (it != active_executions_.end()) {
        return it->second;
    }
    
    auto completed_it = completed_executions_.find(execution_id);
    if (completed_it != completed_executions_.end()) {
        return completed_it->second;
    }
    
    return nullptr;
}

json WorkflowOrchestrator::get_execution_progress(const std::string& execution_id) {
    auto execution = get_execution_status(execution_id);
    if (!execution) {
        return json{{"error", "Execution not found"}};
    }
    
    return json{
        {"execution_id", execution->execution_id},
        {"workflow_id", execution->workflow_id},
        {"state", static_cast<int>(execution->state)},
        {"progress_percentage", execution->progress_percentage},
        {"start_time", std::chrono::duration_cast<std::chrono::seconds>(
            execution->start_time.time_since_epoch()).count()},
        {"error_message", execution->error_message},
        {"step_count", execution->step_results.size()},
        {"context", execution->context}
    };
}

std::vector<std::shared_ptr<WorkflowExecution>> WorkflowOrchestrator::list_active_executions() {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    std::vector<std::shared_ptr<WorkflowExecution>> executions;
    for (const auto& pair : active_executions_) {
        executions.push_back(pair.second);
    }
    return executions;
}

void WorkflowOrchestrator::register_builtin_workflows() {
    register_workflow(WorkflowTemplates::create_research_workflow());
    register_workflow(WorkflowTemplates::create_analysis_workflow());
    register_workflow(WorkflowTemplates::create_data_pipeline_workflow());
    register_workflow(WorkflowTemplates::create_decision_workflow());
}

void WorkflowOrchestrator::orchestrator_thread() {
    while (running_.load()) {
        std::shared_ptr<WorkflowExecution> execution;
        
        // Get next execution to process
        {
            std::unique_lock<std::mutex> lock(orchestrator_mutex_);
            execution_condition_.wait(lock, [this] {
                return !running_.load() || 
                       std::any_of(active_executions_.begin(), active_executions_.end(),
                                  [](const auto& pair) {
                                      return pair.second->state == WorkflowExecutionState::PENDING ||
                                             pair.second->state == WorkflowExecutionState::RUNNING;
                                  });
            });
            
            if (!running_.load()) break;
            
            // Find an execution to process
            for (auto& pair : active_executions_) {
                if (pair.second->state == WorkflowExecutionState::PENDING) {
                    execution = pair.second;
                    execution->state = WorkflowExecutionState::RUNNING;
                    break;
                }
            }
        }
        
        if (execution) {
            process_execution(execution);
        }
    }
}

void WorkflowOrchestrator::process_execution(std::shared_ptr<WorkflowExecution> execution) {
    try {
        auto workflow = get_workflow(execution->workflow_id);
        if (!workflow) {
            execution->state = WorkflowExecutionState::FAILED;
            execution->error_message = "Workflow definition not found";
            move_to_completed(execution);
            return;
        }
        
        // Execute based on workflow type
        switch (workflow->type) {
            case WorkflowType::SEQUENTIAL:
                execute_sequential_workflow(execution);
                break;
            case WorkflowType::PARALLEL:
                execute_parallel_workflow(execution);
                break;
            case WorkflowType::CONDITIONAL:
                execute_conditional_workflow(execution);
                break;
            case WorkflowType::LOOP:
                execute_loop_workflow(execution);
                break;
            case WorkflowType::PIPELINE:
                execute_pipeline_workflow(execution);
                break;
        }
        
    } catch (const std::exception& e) {
        execution->state = WorkflowExecutionState::FAILED;
        execution->error_message = e.what();
    }
    
    execution->end_time = std::chrono::system_clock::now();
    move_to_completed(execution);
}

void WorkflowOrchestrator::execute_sequential_workflow(std::shared_ptr<WorkflowExecution> execution) {
    auto workflow = get_workflow(execution->workflow_id);
    if (!workflow) return;
    
    for (size_t i = 0; i < workflow->steps.size(); ++i) {
        if (execution->state != WorkflowExecutionState::RUNNING) {
            break; // Paused, cancelled, or failed
        }
        
        const auto& step = workflow->steps[i];
        
        // Check dependencies
        bool dependencies_met = true;
        for (const auto& dep : step.dependencies) {
            if (execution->step_results.find(dep) == execution->step_results.end()) {
                dependencies_met = false;
                break;
            }
        }
        
        if (!dependencies_met) {
            if (!step.optional) {
                execution->state = WorkflowExecutionState::FAILED;
                execution->error_message = "Step dependencies not met: " + step.id;
                return;
            }
            continue;
        }
        
        // Execute step
        if (!execute_step(step, execution)) {
            if (!step.optional && !workflow->allow_partial_failure) {
                execution->state = WorkflowExecutionState::FAILED;
                return;
            }
        }
        
        // Update progress
        execution->progress_percentage = ((double)(i + 1) / workflow->steps.size()) * 100.0;
    }
    
    if (execution->state == WorkflowExecutionState::RUNNING) {
        execution->state = WorkflowExecutionState::COMPLETED;
    }
}

void WorkflowOrchestrator::execute_parallel_workflow(std::shared_ptr<WorkflowExecution> execution) {
    auto workflow = get_workflow(execution->workflow_id);
    if (!workflow) return;
    
    // Submit all steps in parallel
    std::vector<std::future<bool>> futures;
    
    for (const auto& step : workflow->steps) {
        if (execution->state != WorkflowExecutionState::RUNNING) {
            break;
        }
        
        // Skip steps with unmet dependencies for now (simplified)
        auto future = std::async(std::launch::async, [this, &step, execution]() {
            return execute_step(step, execution);
        });
        
        futures.push_back(std::move(future));
    }
    
    // Wait for all steps to complete
    bool all_succeeded = true;
    for (auto& future : futures) {
        try {
            if (!future.get()) {
                all_succeeded = false;
            }
        } catch (const std::exception&) {
            all_succeeded = false;
        }
    }
    
    execution->progress_percentage = 100.0;
    
    if (execution->state == WorkflowExecutionState::RUNNING) {
        execution->state = all_succeeded ? WorkflowExecutionState::COMPLETED : 
                          (workflow->allow_partial_failure ? WorkflowExecutionState::COMPLETED : WorkflowExecutionState::FAILED);
    }
}

void WorkflowOrchestrator::execute_conditional_workflow(std::shared_ptr<WorkflowExecution> execution) {
    auto workflow = get_workflow(execution->workflow_id);
    if (!workflow) return;
    
    for (size_t i = 0; i < workflow->steps.size(); ++i) {
        if (execution->state != WorkflowExecutionState::RUNNING) {
            break;
        }
        
        const auto& step = workflow->steps[i];
        
        // Check condition
        if (!step.conditions.empty() && !evaluate_condition(step.conditions, execution->context)) {
            continue; // Skip this step
        }
        
        // Execute step
        if (!execute_step(step, execution)) {
            if (!step.optional && !workflow->allow_partial_failure) {
                execution->state = WorkflowExecutionState::FAILED;
                return;
            }
        }
        
        execution->progress_percentage = ((double)(i + 1) / workflow->steps.size()) * 100.0;
    }
    
    if (execution->state == WorkflowExecutionState::RUNNING) {
        execution->state = WorkflowExecutionState::COMPLETED;
    }
}

void WorkflowOrchestrator::execute_loop_workflow(std::shared_ptr<WorkflowExecution> execution) {
    // Simplified loop implementation - execute steps repeatedly until condition is met
    auto workflow = get_workflow(execution->workflow_id);
    if (!workflow) return;
    
    int max_iterations = execution->context.value("max_iterations", 10);
    json loop_condition = execution->context.value("loop_condition", json{});
    
    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        if (execution->state != WorkflowExecutionState::RUNNING) {
            break;
        }
        
        // Execute all steps in this iteration
        for (const auto& step : workflow->steps) {
            if (!execute_step(step, execution)) {
                if (!workflow->allow_partial_failure) {
                    execution->state = WorkflowExecutionState::FAILED;
                    return;
                }
            }
        }
        
        // Check loop condition
        if (!loop_condition.empty() && !evaluate_condition(loop_condition, execution->context)) {
            break; // Exit loop
        }
        
        execution->progress_percentage = ((double)(iteration + 1) / max_iterations) * 100.0;
    }
    
    if (execution->state == WorkflowExecutionState::RUNNING) {
        execution->state = WorkflowExecutionState::COMPLETED;
    }
}

void WorkflowOrchestrator::execute_pipeline_workflow(std::shared_ptr<WorkflowExecution> execution) {
    // Pipeline workflow passes output of one step as input to the next
    auto workflow = get_workflow(execution->workflow_id);
    if (!workflow) return;
    
    json pipeline_data = execution->input_data;
    
    for (size_t i = 0; i < workflow->steps.size(); ++i) {
        if (execution->state != WorkflowExecutionState::RUNNING) {
            break;
        }
        
        auto step = workflow->steps[i];
        
        // For new array format, we don't merge pipeline data directly
        // Instead, we update the execution context with pipeline data
        if (step.parameters.is_array()) {
            execution->context["pipeline_input"] = pipeline_data;
        } else {
            // Legacy format: Merge pipeline data into step parameters
            step.parameters["pipeline_input"] = pipeline_data;
        }
        
        // Execute step
        if (execute_step(step, execution)) {
            // Get output and use as input for next step
            if (execution->step_outputs.find(step.id) != execution->step_outputs.end()) {
                pipeline_data = execution->step_outputs[step.id];
            }
        } else if (!workflow->allow_partial_failure) {
            execution->state = WorkflowExecutionState::FAILED;
            return;
        }
        
        execution->progress_percentage = ((double)(i + 1) / workflow->steps.size()) * 100.0;
    }
    
    execution->output_data = pipeline_data;
    
    if (execution->state == WorkflowExecutionState::RUNNING) {
        execution->state = WorkflowExecutionState::COMPLETED;
    }
}

bool WorkflowOrchestrator::execute_step(const WorkflowStep& step, std::shared_ptr<WorkflowExecution> execution) {
    try {
        // Resolve parameters with context
        json resolved_params = resolve_parameters(step.parameters, execution->context);
        
        // For new array format, create a simple parameter object for backward compatibility
        if (step.parameters.is_array()) {
            json simple_params = json::object();
            for (const auto& param : step.parameters) {
                if (param.is_string()) {
                    std::string param_name = param.get<std::string>();
                    // Set default values based on parameter name
                    if (param_name == "query") {
                        simple_params[param_name] = execution->input_data.value("query", "default query");
                    } else if (param_name == "text") {
                        simple_params[param_name] = execution->input_data.value("text", "default text");
                    } else if (param_name == "message") {
                        simple_params[param_name] = execution->input_data.value("message", "default message");
                    } else if (param_name == "model") {
                        simple_params[param_name] = step.llm_model.empty() ? "gemma3-1b" : step.llm_model;
                    } else if (param_name == "depth") {
                        simple_params[param_name] = "basic";
                    } else if (param_name == "analysis_type") {
                        simple_params[param_name] = "general";
                    } else if (param_name == "context") {
                        simple_params[param_name] = execution->context.dump();
                    } else {
                        // Generic parameter handling
                        simple_params[param_name] = execution->input_data.value(param_name, "");
                    }
                }
            }
            resolved_params = simple_params;
        }
        
        // Add LLM model to parameters if specified
        if (!step.llm_model.empty()) {
            resolved_params["model"] = step.llm_model;
        }
        
        // Submit request to workflow manager
        std::string request_id = workflow_manager_->submit_request_with_timeout(
            step.agent_name, step.function_name, resolved_params, step.timeout_ms
        );
        
        execution->step_results[step.id] = request_id;
        
        // Wait for completion
        wait_for_step_completion(request_id, execution, step);
        
        return true;
        
    } catch (const std::exception& e) {
        execution->error_message += "Step " + step.id + " failed: " + e.what() + "; ";
        return false;
    }
}

void WorkflowOrchestrator::wait_for_step_completion(const std::string& request_id,
                                                   std::shared_ptr<WorkflowExecution> execution,
                                                   const WorkflowStep& step) {
    while (execution->state == WorkflowExecutionState::RUNNING) {
        auto request_status = workflow_manager_->get_request_status(request_id);
        if (!request_status) {
            break;
        }
        
        if (request_status->state == WorkflowState::COMPLETED) {
            execution->step_outputs[step.id] = request_status->result;
            execution->context[step.id + "_output"] = request_status->result;
            break;
        } else if (request_status->state == WorkflowState::FAILED ||
                  request_status->state == WorkflowState::TIMEOUT) {
            throw std::runtime_error("Step execution failed: " + request_status->error);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::string WorkflowOrchestrator::generate_execution_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "wf-" << std::hex;
    for (int i = 0; i < 16; ++i) {
        ss << dis(gen);
    }
    
    return ss.str();
}

bool WorkflowOrchestrator::evaluate_condition(const json& condition, const json& context) {
    // Simplified condition evaluation
    if (condition.contains("field") && condition.contains("operator") && condition.contains("value")) {
        std::string field = condition["field"];
        std::string op = condition["operator"];
        json expected_value = condition["value"];
        
        if (!context.contains(field)) {
            return false;
        }
        
        json actual_value = context[field];
        
        if (op == "equals") return actual_value == expected_value;
        if (op == "not_equals") return actual_value != expected_value;
        if (op == "exists") return true;
        // Add more operators as needed
    }
    
    return true; // Default to true for invalid conditions
}

json WorkflowOrchestrator::resolve_parameters(const json& parameters, const json& context) {
    json resolved;
    
    // Handle new format: parameters as array of strings
    if (parameters.is_array()) {
        resolved = json::object();
        for (const auto& param : parameters) {
            if (param.is_string()) {
                std::string param_name = param.get<std::string>();
                // For array format, we just list the parameter names
                // The actual values will be provided during execution
                resolved[param_name] = nullptr;  // Placeholder
            }
        }
    } else {
        // Legacy format: parameters as object with template resolution
        resolved = parameters;
        
        // Simple template resolution - replace {{field}} with context values
        std::function<void(json&)> resolve_recursive = [&](json& obj) {
            if (obj.is_string()) {
                std::string str = obj;
                size_t start = str.find("{{");
                while (start != std::string::npos) {
                    size_t end = str.find("}}", start);
                    if (end != std::string::npos) {
                        std::string field = str.substr(start + 2, end - start - 2);
                        if (context.contains(field)) {
                            std::string replacement = context[field].is_string() ? 
                                                    context[field].get<std::string>() : 
                                                    context[field].dump();
                            str.replace(start, end - start + 2, replacement);
                        }
                    }
                    start = str.find("{{", start + 1);
                }
                obj = str;
            } else if (obj.is_object()) {
                for (auto& [key, value] : obj.items()) {
                    resolve_recursive(value);
                }
            } else if (obj.is_array()) {
                for (auto& item : obj) {
                    resolve_recursive(item);
                }
            }
        };
        
        resolve_recursive(resolved);
    }
    
    return resolved;
}

void WorkflowOrchestrator::load_agent_llm_mappings(const json& config) {
    agent_llm_mappings_.clear();
    
    for (const auto& [agent_name, agent_config] : config.items()) {
        if (agent_config.contains("supported_models") && agent_config["supported_models"].is_array()) {
            std::vector<std::string> models;
            for (const auto& model : agent_config["supported_models"]) {
                if (model.is_string()) {
                    models.push_back(model.get<std::string>());
                }
            }
            agent_llm_mappings_[agent_name]["supported_models"] = models;
        }
        
        if (agent_config.contains("default_model") && agent_config["default_model"].is_string()) {
            agent_llm_mappings_[agent_name]["default_model"] = {agent_config["default_model"].get<std::string>()};
        }
    }
}

bool WorkflowOrchestrator::validate_agent_llm_pairing(const std::string& agent_name, const std::string& llm_model) {
    // If no LLM model specified, it's valid (will use default)
    if (llm_model.empty()) {
        return true;
    }
    
    // Check if agent exists in mappings
    auto agent_it = agent_llm_mappings_.find(agent_name);
    if (agent_it == agent_llm_mappings_.end()) {
        return false; // Agent not found in mappings
    }
    
    // Check if model is in supported models
    auto supported_it = agent_it->second.find("supported_models");
    if (supported_it != agent_it->second.end()) {
        const auto& supported_models = supported_it->second;
        return std::find(supported_models.begin(), supported_models.end(), llm_model) != supported_models.end();
    }
    
    return false;
}

bool WorkflowOrchestrator::validate_workflow_definition(const WorkflowDefinition& workflow) {
    // Basic validation
    if (workflow.id.empty() || workflow.name.empty()) {
        return false;
    }
    
    // Validate steps
    for (const auto& step : workflow.steps) {
        if (step.id.empty() || step.agent_name.empty() || step.function_name.empty()) {
            return false;
        }
        
        // Validate agent-LLM pairing if LLM model is specified
        if (!validate_agent_llm_pairing(step.agent_name, step.llm_model)) {
            std::cerr << "Invalid agent-LLM pairing: " << step.agent_name << " with " << step.llm_model << std::endl;
            return false;
        }
        
        // Validate dependencies exist
        for (const auto& dep : step.dependencies) {
            bool found = false;
            for (const auto& other_step : workflow.steps) {
                if (other_step.id == dep) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cerr << "Invalid dependency: " << dep << " for step " << step.id << std::endl;
                return false;
            }
        }
    }
    
    return true;
}

WorkflowDefinition WorkflowOrchestrator::parse_workflow_from_config(const json& workflow_config) {
    // Safely extract required fields with null checks
    std::string id = workflow_config.value("id", "");
    std::string name = workflow_config.value("name", "");
    
    if (id.empty()) {
        throw std::runtime_error("Workflow 'id' field is required and cannot be null or empty");
    }
    if (name.empty()) {
        throw std::runtime_error("Workflow 'name' field is required and cannot be null or empty");
    }
    
    // Parse workflow type
    WorkflowType type = WorkflowType::SEQUENTIAL;
    if (workflow_config.contains("type") && !workflow_config["type"].is_null()) {
        std::string type_str = workflow_config.value("type", "sequential");
        if (type_str == "parallel") type = WorkflowType::PARALLEL;
        else if (type_str == "conditional") type = WorkflowType::CONDITIONAL;
        else if (type_str == "loop") type = WorkflowType::LOOP;
        else if (type_str == "pipeline") type = WorkflowType::PIPELINE;
    }
    
    WorkflowDefinition workflow(id, name, type);
    
    // Set optional fields
    if (workflow_config.contains("description") && !workflow_config["description"].is_null()) {
        workflow.description = workflow_config.value("description", "");
    }
    
    if (workflow_config.contains("max_execution_time_ms") && !workflow_config["max_execution_time_ms"].is_null()) {
        workflow.max_execution_time_ms = workflow_config.value("max_execution_time_ms", 300000);
    }
    
    if (workflow_config.contains("allow_partial_failure") && !workflow_config["allow_partial_failure"].is_null()) {
        workflow.allow_partial_failure = workflow_config.value("allow_partial_failure", false);
    }
    
    // Parse steps
    if (workflow_config.contains("steps") && workflow_config["steps"].is_array()) {
        for (const auto& step_config : workflow_config["steps"]) {
            // Safely extract required step fields with null checks
            std::string step_id = step_config.value("id", "");
            std::string agent_name = step_config.value("agent_name", "");
            std::string function_name = step_config.value("function_name", "");
            
            if (step_id.empty()) {
                throw std::runtime_error("Step 'id' field is required and cannot be null or empty");
            }
            if (agent_name.empty()) {
                throw std::runtime_error("Step 'agent_name' field is required and cannot be null or empty");
            }
            if (function_name.empty()) {
                throw std::runtime_error("Step 'function_name' field is required and cannot be null or empty");
            }
            
            std::string llm_model = step_config.value("llm_model", "");
            
            json parameters = step_config.value("parameters", json::array());
            
            // Handle both old object format and new array format
            if (step_config.contains("parameters") && step_config["parameters"].is_array()) {
                // New format: array of parameter names
                parameters = json::array();
                for (const auto& param : step_config["parameters"]) {
                    if (param.is_string()) {
                        parameters.push_back(param.get<std::string>());
                    }
                }
            } else if (step_config.contains("parameters") && step_config["parameters"].is_object()) {
                // Legacy format: object with key-value pairs
                parameters = step_config["parameters"];
            }
            
            WorkflowStep step(step_id, agent_name, function_name, parameters, llm_model);
            
            // Set optional step fields
            if (step_config.contains("timeout_ms") && !step_config["timeout_ms"].is_null()) {
                step.timeout_ms = step_config.value("timeout_ms", 60000);
            }
            
            if (step_config.contains("optional") && !step_config["optional"].is_null()) {
                step.optional = step_config.value("optional", false);
            }
            
            if (step_config.contains("dependencies") && step_config["dependencies"].is_array()) {
                for (const auto& dep : step_config["dependencies"]) {
                    if (!dep.is_null() && dep.is_string()) {
                        step.dependencies.push_back(dep.get<std::string>());
                    }
                }
            }
            
            if (step_config.contains("condition") && !step_config["condition"].is_null()) {
                step.conditions = step_config["condition"];
            }
            
            workflow.steps.push_back(step);
        }
    }
    
    return workflow;
}

WorkflowDefinition WorkflowOrchestrator::parse_workflow_from_yaml(const YAML::Node& workflow_config) {
    // Safely extract required fields with null checks
    std::string id;
    std::string name;
    
    if (!workflow_config["id"] || workflow_config["id"].IsNull()) {
        throw std::runtime_error("Workflow 'id' field is required and cannot be null or empty");
    }
    id = workflow_config["id"].as<std::string>();
    
    if (!workflow_config["name"] || workflow_config["name"].IsNull()) {
        throw std::runtime_error("Workflow 'name' field is required and cannot be null or empty");
    }
    name = workflow_config["name"].as<std::string>();
    
    if (id.empty()) {
        throw std::runtime_error("Workflow 'id' field is required and cannot be null or empty");
    }
    if (name.empty()) {
        throw std::runtime_error("Workflow 'name' field is required and cannot be null or empty");
    }
    
    // Parse workflow type
    WorkflowType type = WorkflowType::SEQUENTIAL;
    if (workflow_config["type"] && !workflow_config["type"].IsNull()) {
        std::string type_str = workflow_config["type"].as<std::string>("sequential");
        if (type_str == "parallel") type = WorkflowType::PARALLEL;
        else if (type_str == "conditional") type = WorkflowType::CONDITIONAL;
        else if (type_str == "loop") type = WorkflowType::LOOP;
        else if (type_str == "pipeline") type = WorkflowType::PIPELINE;
    }
    
    WorkflowDefinition workflow(id, name, type);
    
    // Set optional fields
    if (workflow_config["description"] && !workflow_config["description"].IsNull()) {
        workflow.description = workflow_config["description"].as<std::string>("");
    }
    
    if (workflow_config["max_execution_time_ms"] && !workflow_config["max_execution_time_ms"].IsNull()) {
        workflow.max_execution_time_ms = workflow_config["max_execution_time_ms"].as<int>(300000);
    }
    
    if (workflow_config["allow_partial_failure"] && !workflow_config["allow_partial_failure"].IsNull()) {
        workflow.allow_partial_failure = workflow_config["allow_partial_failure"].as<bool>(false);
    }
    
    // Parse steps
    if (workflow_config["steps"] && workflow_config["steps"].IsSequence()) {
        for (const auto& step_config : workflow_config["steps"]) {
            // Safely extract required step fields with null checks
            std::string step_id;
            std::string agent_name;
            std::string function_name;
            
            if (!step_config["id"] || step_config["id"].IsNull()) {
                throw std::runtime_error("Step 'id' field is required and cannot be null or empty");
            }
            step_id = step_config["id"].as<std::string>();
            
            if (!step_config["agent_name"] || step_config["agent_name"].IsNull()) {
                throw std::runtime_error("Step 'agent_name' field is required and cannot be null or empty");
            }
            agent_name = step_config["agent_name"].as<std::string>();
            
            if (!step_config["function_name"] || step_config["function_name"].IsNull()) {
                throw std::runtime_error("Step 'function_name' field is required and cannot be null or empty");
            }
            function_name = step_config["function_name"].as<std::string>();
            
            if (step_id.empty()) {
                throw std::runtime_error("Step 'id' field is required and cannot be null or empty");
            }
            if (agent_name.empty()) {
                throw std::runtime_error("Step 'agent_name' field is required and cannot be null or empty");
            }
            if (function_name.empty()) {
                throw std::runtime_error("Step 'function_name' field is required and cannot be null or empty");
            }
            
            std::string llm_model;
            if (step_config["llm_model"] && !step_config["llm_model"].IsNull()) {
                llm_model = step_config["llm_model"].as<std::string>();
                if (llm_model.empty()) {
                    // If llm_model is explicitly empty in YAML, leave it empty
                    // But ensure it's not due to parsing error
                    std::cout << "DEBUG: Found empty llm_model for step: " << step_id << std::endl;
                }
            } else {
                // llm_model field is missing or null, leave empty
                std::cout << "DEBUG: No llm_model field found for step: " << step_id << std::endl;
            }
            
            // Convert YAML parameters to JSON for compatibility with existing code
            json parameters;
            if (step_config["parameters"] && !step_config["parameters"].IsNull()) {
                try {
                    // Handle parameters as array of strings (new format)
                    if (step_config["parameters"].IsSequence()) {
                        parameters = json::array();
                        for (const auto& param : step_config["parameters"]) {
                            if (param && !param.IsNull()) {
                                parameters.push_back(param.as<std::string>());
                            }
                        }
                    } else {
                        // Legacy format: convert YAML to JSON directly
                        YAML::Emitter emitter;
                        emitter << step_config["parameters"];
                        parameters = json::parse(emitter.c_str());
                    }
                } catch (const std::exception& e) {
                    // If conversion fails, use empty array
                    parameters = json::array();
                }
            } else {
                parameters = json::array();
            }
            
            WorkflowStep step(step_id, agent_name, function_name, parameters, llm_model);
            
            // Set optional step fields
            if (step_config["timeout_ms"] && !step_config["timeout_ms"].IsNull()) {
                step.timeout_ms = step_config["timeout_ms"].as<int>(60000);
            }
            
            if (step_config["optional"] && !step_config["optional"].IsNull()) {
                step.optional = step_config["optional"].as<bool>(false);
            }
            
            if (step_config["dependencies"] && step_config["dependencies"].IsSequence()) {
                for (const auto& dep : step_config["dependencies"]) {
                    if (dep && !dep.IsNull()) {
                        step.dependencies.push_back(dep.as<std::string>());
                    }
                }
            }
            
            if (step_config["condition"] && !step_config["condition"].IsNull()) {
                try {
                    YAML::Emitter emitter;
                    emitter << step_config["condition"];
                    step.conditions = json::parse(emitter.c_str());
                } catch (const std::exception& e) {
                    // If conversion fails, use empty object
                    step.conditions = json{};
                }
            }
            
            workflow.steps.push_back(step);
        }
    }
    
    return workflow;
}

void WorkflowOrchestrator::move_to_completed(std::shared_ptr<WorkflowExecution> execution) {
    std::lock_guard<std::mutex> lock(orchestrator_mutex_);
    active_executions_.erase(execution->execution_id);
    completed_executions_[execution->execution_id] = execution;
}

// Workflow Builder Implementation
WorkflowBuilder::WorkflowBuilder(const std::string& workflow_id, const std::string& name)
    : workflow_(workflow_id, name) {
}

WorkflowBuilder& WorkflowBuilder::set_type(WorkflowType type) {
    workflow_.type = type;
    return *this;
}

WorkflowBuilder& WorkflowBuilder::set_description(const std::string& description) {
    workflow_.description = description;
    return *this;
}

WorkflowBuilder& WorkflowBuilder::set_max_execution_time(int timeout_ms) {
    workflow_.max_execution_time_ms = timeout_ms;
    return *this;
}

WorkflowBuilder& WorkflowBuilder::allow_partial_failure(bool allow) {
    workflow_.allow_partial_failure = allow;
    return *this;
}

WorkflowBuilder& WorkflowBuilder::set_global_context(const json& context) {
    workflow_.global_context = context;
    return *this;
}

WorkflowBuilder& WorkflowBuilder::add_step(const std::string& id, 
                                          const std::string& agent_name,
                                          const std::string& function_name,
                                          const json& parameters,
                                          const std::string& llm_model) {
    workflow_.steps.emplace_back(id, agent_name, function_name, parameters, llm_model);
    return *this;
}

WorkflowBuilder& WorkflowBuilder::add_conditional_step(const std::string& id,
                                                      const std::string& agent_name,
                                                      const std::string& function_name,
                                                      const json& condition,
                                                      const json& parameters,
                                                      const std::string& llm_model) {
    WorkflowStep step(id, agent_name, function_name, parameters, llm_model);
    step.conditions = condition;
    workflow_.steps.push_back(step);
    return *this;
}

WorkflowBuilder& WorkflowBuilder::add_step_dependency(const std::string& step_id, 
                                                     const std::string& depends_on) {
    for (auto& step : workflow_.steps) {
        if (step.id == step_id) {
            step.dependencies.push_back(depends_on);
            break;
        }
    }
    return *this;
}

WorkflowBuilder& WorkflowBuilder::set_step_timeout(const std::string& step_id, int timeout_ms) {
    for (auto& step : workflow_.steps) {
        if (step.id == step_id) {
            step.timeout_ms = timeout_ms;
            break;
        }
    }
    return *this;
}

WorkflowBuilder& WorkflowBuilder::set_step_optional(const std::string& step_id, bool optional) {
    for (auto& step : workflow_.steps) {
        if (step.id == step_id) {
            step.optional = optional;
            break;
        }
    }
    return *this;
}

WorkflowDefinition WorkflowBuilder::build() {
    return workflow_;
}

// Workflow Templates Implementation
namespace WorkflowTemplates {
    WorkflowDefinition create_research_workflow() {
        return WorkflowBuilder("research_workflow", "Research Workflow")
            .set_type(WorkflowType::SEQUENTIAL)
            .set_description("Comprehensive research workflow: question -> research -> analyze -> summarize")
            .add_step("research", "Researcher", "research", json::array({"query", "depth"}))
            .add_step("analyze", "Analyzer", "analyze", json::array({"text", "analysis_type"}))
            .add_step("summarize", "Assistant", "chat", json::array({"message", "model"}))
            .add_step_dependency("analyze", "research")
            .add_step_dependency("summarize", "analyze")
            .build();
    }
    
    WorkflowDefinition create_analysis_workflow() {
        return WorkflowBuilder("analysis_workflow", "Analysis Workflow")
            .set_type(WorkflowType::SEQUENTIAL)
            .set_description("Data analysis workflow: input -> preprocess -> analyze -> report")
            .add_step("preprocess", "Analyzer", "analyze", json::array({"text", "analysis_type"}))
            .add_step("analyze", "Analyzer", "analyze", json::array({"text", "analysis_type"}))
            .add_step("report", "Assistant", "chat", json::array({"message", "model"}))
            .add_step_dependency("analyze", "preprocess")
            .add_step_dependency("report", "analyze")
            .build();
    }
    
    WorkflowDefinition create_conversation_workflow(const std::vector<std::string>& agent_names) {
        auto builder = WorkflowBuilder("conversation_workflow", "Multi-Agent Conversation")
            .set_type(WorkflowType::SEQUENTIAL)
            .set_description("Multi-agent conversation workflow");
        
        for (size_t i = 0; i < agent_names.size(); ++i) {
            std::string step_id = "response_" + std::to_string(i);
            builder.add_step(step_id, agent_names[i], "chat", 
                           json::array({"message", "model"}));
        }
        
        return builder.build();
    }
    
    WorkflowDefinition create_data_pipeline_workflow() {
        return WorkflowBuilder("data_pipeline_workflow", "Data Pipeline Workflow")
            .set_type(WorkflowType::PIPELINE)
            .set_description("Data processing pipeline: extract -> transform -> validate -> load")
            .add_step("extract", "Analyzer", "analyze", json::array({"text", "analysis_type"}))
            .add_step("transform", "Analyzer", "analyze", json::array({"text", "analysis_type"}))
            .add_step("validate", "Analyzer", "analyze", json::array({"text", "analysis_type"}))
            .add_step("load", "Assistant", "status", json::array())
            .build();
    }
    
    WorkflowDefinition create_decision_workflow() {
        return WorkflowBuilder("decision_workflow", "Decision Making Workflow")
            .set_type(WorkflowType::SEQUENTIAL)
            .set_description("Decision making workflow: gather info -> analyze options -> decide -> execute")
            .add_step("gather_info", "Researcher", "research", json::array({"query", "depth"}))
            .add_step("analyze_options", "Analyzer", "analyze", json::array({"text", "analysis_type"}))
            .add_step("decide", "Assistant", "chat", json::array({"message", "model"}))
            .add_step("execute", "Assistant", "status", json::array())
            .add_step_dependency("analyze_options", "gather_info")
            .add_step_dependency("decide", "analyze_options")
            .add_step_dependency("execute", "decide")
            .build();
    }
}
