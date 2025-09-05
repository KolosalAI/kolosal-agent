#include "workflow_types.hpp"
#include "workflow_manager.hpp"
#include "logger.hpp"
#include <random>
#include <future>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <yaml-cpp/yaml.h>

WorkflowOrchestrator::WorkflowOrchestrator(std::shared_ptr<WorkflowManager> workflow_manager)
    : workflow_manager_(workflow_manager),
      workflows_dir_("workflows"),
      templates_dir_("workflows/templates") {
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
        
        // Check if file exists before attempting to load
        std::ifstream file_check(config_file_path);
        if (!file_check.good()) {
            LOG_WARN_F("Workflow config file not found: %s", config_file_path.c_str());
            return false;
        }
        file_check.close();
        
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
    
    // Wait for completion with timeout to prevent infinite loops
    auto start_time = std::chrono::steady_clock::now();
    auto timeout_duration = std::chrono::minutes(2); // 2 minute timeout for tests
    
    while (std::chrono::steady_clock::now() - start_time < timeout_duration) {
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
    
    // Check if we timed out
    auto execution = get_execution_status(execution_id);
    if (execution && execution->state == WorkflowExecutionState::RUNNING) {
        execution->state = WorkflowExecutionState::TIMEOUT;
        execution->error_message = "Workflow execution timed out";
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
            // Add timeout to wait to prevent infinite blocking
            execution_condition_.wait_for(lock, std::chrono::seconds(5), [this] {
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
            try {
                process_execution(execution);
            } catch (const std::exception& e) {
                std::cerr << "[WorkflowOrchestrator] Error processing execution " << execution->execution_id 
                          << ": " << e.what() << std::endl;
                execution->state = WorkflowExecutionState::FAILED;
                execution->error_message = e.what();
                move_to_completed(execution);
            }
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
        
        // Execute step with retry support
        if (!execute_step_with_retry(step, execution)) {
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
            return execute_step_with_retry(step, execution);
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
        
        // Execute step with retry support
        if (!execute_step_with_retry(step, execution)) {
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
            if (!execute_step_with_retry(step, execution)) {
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
        
        // Execute step with retry support
        if (execute_step_with_retry(step, execution)) {
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

bool WorkflowOrchestrator::execute_step_with_retry(const WorkflowStep& step, std::shared_ptr<WorkflowExecution> execution) {
    // Initialize step statistics
    execution->step_stats[step.id] = StepExecutionStats();
    execution->step_stats[step.id].start_time = std::chrono::system_clock::now();
    execution->current_step_id = step.id;
    
    // Determine retry policy (step-specific or workflow default)
    RetryPolicy retry_policy = step.retry_policy;
    if (retry_policy.max_retries == 0) {
        auto workflow = get_workflow(execution->workflow_id);
        if (workflow) {
            retry_policy = workflow->default_retry_policy;
        }
    }
    
    int attempt = 0;
    int delay_ms = retry_policy.initial_delay_ms;
    
    while (attempt <= retry_policy.max_retries) {
        try {
            execution->step_stats[step.id].retry_count = attempt;
            
            // Log retry attempt
            if (attempt > 0) {
                std::string log_msg = "Retrying step '" + step.id + "', attempt " + std::to_string(attempt + 1) + 
                                     " of " + std::to_string(retry_policy.max_retries + 1);
                execution->execution_log.push_back(log_msg);
                LOG_INFO_F("Retrying step '%s', attempt %d", step.id.c_str(), attempt + 1);
            }
            
            bool success = execute_step(step, execution);
            
            if (success) {
                execution->step_stats[step.id].completed_successfully = true;
                execution->step_stats[step.id].end_time = std::chrono::system_clock::now();
                return true;
            }
            
        } catch (const std::exception& e) {
            execution->step_stats[step.id].error_message = e.what();
            
            if (attempt == retry_policy.max_retries) {
                // Final attempt failed
                execution->step_stats[step.id].end_time = std::chrono::system_clock::now();
                execution->failed_step_count++;
                
                std::string log_msg = "Step '" + step.id + "' failed after " + std::to_string(attempt + 1) + 
                                     " attempts: " + e.what();
                execution->execution_log.push_back(log_msg);
                
                throw; // Re-throw the exception
            }
        }
        
        attempt++;
        
        // Wait before retry (with exponential backoff)
        if (attempt <= retry_policy.max_retries) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            delay_ms = static_cast<int>(delay_ms * retry_policy.backoff_multiplier);
            delay_ms = std::min(delay_ms, retry_policy.max_delay_ms);
        }
    }
    
    // If we get here, all retries failed
    return false;
}

bool WorkflowOrchestrator::execute_step(const WorkflowStep& step, std::shared_ptr<WorkflowExecution> execution) {
    try {
        // Build proper parameters from step definition and execution context
        json resolved_params = json::object();
        
        // Handle array format (new format) - convert to parameter object
        if (step.parameters.is_array()) {
            for (const auto& param : step.parameters) {
                if (param.is_string()) {
                    std::string param_name = param.get<std::string>();
                    
                    // Map parameter names to values from input_data and context
                    if (param_name == "query") {
                        resolved_params[param_name] = execution->input_data.value("query", "What is artificial intelligence?");
                    } else if (param_name == "text") {
                        // For pipeline workflows, use output from previous step
                        if (execution->context.contains("pipeline_input")) {
                            auto pipeline_input = execution->context["pipeline_input"];
                            if (pipeline_input.is_string()) {
                                resolved_params[param_name] = pipeline_input.get<std::string>();
                            } else {
                                resolved_params[param_name] = execution->input_data.value("text", "Sample text for analysis");
                            }
                        } else {
                            resolved_params[param_name] = execution->input_data.value("text", "Sample text for analysis");
                        }
                    } else if (param_name == "message") {
                        // For chat functions, use message or query from input
                        resolved_params[param_name] = execution->input_data.value("message", 
                            execution->input_data.value("query", "Hello, how can I help you?"));
                    } else if (param_name == "model") {
                        // Always use the step's specified LLM model
                        resolved_params[param_name] = step.llm_model.empty() ? "gemma3-1b" : step.llm_model;
                    } else if (param_name == "depth") {
                        resolved_params[param_name] = execution->input_data.value("depth", "basic");
                    } else if (param_name == "analysis_type") {
                        resolved_params[param_name] = execution->input_data.value("analysis_type", "general");
                    } else if (param_name == "context") {
                        // Provide context from previous step outputs
                        std::string context_str = "";
                        for (const auto& [step_id, output] : execution->step_outputs) {
                            if (output.is_string()) {
                                context_str += step_id + ": " + output.get<std::string>() + "\n";
                            } else {
                                context_str += step_id + ": " + output.dump() + "\n";
                            }
                        }
                        resolved_params[param_name] = context_str.empty() ? execution->context.dump() : context_str;
                    } else if (param_name == "results") {
                        resolved_params[param_name] = execution->input_data.value("results", 10);
                    } else if (param_name == "language") {
                        resolved_params[param_name] = execution->input_data.value("language", "en");
                    } else if (param_name == "limit") {
                        resolved_params[param_name] = execution->input_data.value("limit", 10);
                    } else if (param_name == "threshold") {
                        resolved_params[param_name] = execution->input_data.value("threshold", 0.7);
                    } else {
                        // Try to get from input_data, context, or use empty string
                        if (execution->input_data.contains(param_name)) {
                            resolved_params[param_name] = execution->input_data[param_name];
                        } else if (execution->context.contains(param_name)) {
                            resolved_params[param_name] = execution->context[param_name];
                        } else {
                            resolved_params[param_name] = "";
                        }
                    }
                }
            }
        } else {
            // Handle legacy object format
            resolved_params = resolve_parameters(step.parameters, execution->context);
        }
        
        // Ensure model parameter is always set for functions that need it
        if (!resolved_params.contains("model") && !step.llm_model.empty()) {
            resolved_params["model"] = step.llm_model;
        }
        
        // For chat functions, ensure model is specified
        if (step.function_name == "chat" && !resolved_params.contains("model")) {
            resolved_params["model"] = step.llm_model.empty() ? "gemma3-1b" : step.llm_model;
        }
        
        // For analyze functions with model support
        if (step.function_name == "analyze" && !resolved_params.contains("model") && !step.llm_model.empty()) {
            resolved_params["model"] = step.llm_model;
        }
        
        LOG_INFO_F("Executing step '%s' with agent '%s', function '%s'", 
                   step.id.c_str(), step.agent_name.c_str(), step.function_name.c_str());
        LOG_DEBUG_F("Step parameters: %s", resolved_params.dump().c_str());
        
        // Submit request to workflow manager
        std::string request_id = workflow_manager_->submit_request_with_timeout(
            step.agent_name, step.function_name, resolved_params, step.timeout_ms
        );
        
        execution->step_results[step.id] = request_id;
        
        // Wait for completion
        wait_for_step_completion(request_id, execution, step);
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR_F("Step '%s' failed: %s", step.id.c_str(), e.what());
        execution->error_message += "Step " + step.id + " failed: " + e.what() + "; ";
        return false;
    }
}

void WorkflowOrchestrator::wait_for_step_completion(const std::string& request_id,
                                                   std::shared_ptr<WorkflowExecution> execution,
                                                   const WorkflowStep& step) {
    auto start_time = std::chrono::steady_clock::now();
    auto timeout_duration = std::chrono::seconds(30); // Reduced to 30 second timeout for individual steps
    
    LOG_DEBUG_F("Waiting for step completion: %s (request: %s)", step.id.c_str(), request_id.c_str());
    
    while (execution->state == WorkflowExecutionState::RUNNING && 
           std::chrono::steady_clock::now() - start_time < timeout_duration) {
        auto request_status = workflow_manager_->get_request_status(request_id);
        if (!request_status) {
            // If request status is null, the request might not exist or be completed
            LOG_WARN_F("Request status is null for request: %s", request_id.c_str());
            break;
        }
        
        LOG_DEBUG_F("Step %s state: %d", step.id.c_str(), static_cast<int>(request_status->state));
        
        if (request_status->state == WorkflowState::COMPLETED) {
            execution->step_outputs[step.id] = request_status->result;
            execution->context[step.id + "_output"] = request_status->result;
            LOG_INFO_F("Step %s completed successfully", step.id.c_str());
            LOG_DEBUG_F("Step %s result: %s", step.id.c_str(), request_status->result.dump().c_str());
            break;
        } else if (request_status->state == WorkflowState::FAILED ||
                  request_status->state == WorkflowState::TIMEOUT ||
                  request_status->state == WorkflowState::CANCELLED) {
            std::string error_msg = "Step execution failed: " + request_status->error;
            LOG_ERROR_F("Step %s failed: %s", step.id.c_str(), error_msg.c_str());
            throw std::runtime_error(error_msg);
        }
        
        // Add more verbose logging for debugging
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed > std::chrono::seconds(5)) {
            LOG_WARN_F("Step %s taking longer than 5 seconds, state: %d", 
                      step.id.c_str(), static_cast<int>(request_status->state));
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Check if we timed out
    if (std::chrono::steady_clock::now() - start_time >= timeout_duration) {
        std::string timeout_msg = "Step execution timed out: " + step.id;
        LOG_ERROR_F("Step execution timed out after 30 seconds: %s", step.id.c_str());
        throw std::runtime_error(timeout_msg);
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
    // Handle empty or null conditions
    if (condition.is_null() || condition.empty()) {
        return true;
    }
    
    // Handle complex conditions with AND/OR logic
    if (condition.contains("and") || condition.contains("or")) {
        return evaluate_complex_condition(condition, context);
    }
    
    // Simple condition evaluation
    if (condition.contains("field") && condition.contains("operator") && condition.contains("value")) {
        std::string field = condition["field"];
        std::string op = condition["operator"];
        json expected_value = condition["value"];
        
        // Navigate nested field paths (e.g., "step_output.result")
        json actual_value;
        std::istringstream field_stream(field);
        std::string field_part;
        json current_context = context;
        
        while (std::getline(field_stream, field_part, '.')) {
            if (current_context.contains(field_part)) {
                current_context = current_context[field_part];
            } else {
                // Field not found
                if (op == "exists") return false;
                return false;
            }
        }
        actual_value = current_context;
        
        // Evaluate based on operator
        if (op == "equals") return actual_value == expected_value;
        if (op == "not_equals") return actual_value != expected_value;
        if (op == "exists") return true; // We found the field
        if (op == "contains" && actual_value.is_string() && expected_value.is_string()) {
            return actual_value.get<std::string>().find(expected_value.get<std::string>()) != std::string::npos;
        }
        if (op == "greater_than" && actual_value.is_number() && expected_value.is_number()) {
            return actual_value.get<double>() > expected_value.get<double>();
        }
        if (op == "less_than" && actual_value.is_number() && expected_value.is_number()) {
            return actual_value.get<double>() < expected_value.get<double>();
        }
        if (op == "greater_equal" && actual_value.is_number() && expected_value.is_number()) {
            return actual_value.get<double>() >= expected_value.get<double>();
        }
        if (op == "less_equal" && actual_value.is_number() && expected_value.is_number()) {
            return actual_value.get<double>() <= expected_value.get<double>();
        }
    }
    
    return true; // Default to true for invalid conditions
}

bool WorkflowOrchestrator::evaluate_complex_condition(const json& condition, const json& context) {
    // Handle AND conditions
    if (condition.contains("and") && condition["and"].is_array()) {
        for (const auto& sub_condition : condition["and"]) {
            if (!evaluate_condition(sub_condition, context)) {
                return false; // All must be true for AND
            }
        }
        return true;
    }
    
    // Handle OR conditions
    if (condition.contains("or") && condition["or"].is_array()) {
        for (const auto& sub_condition : condition["or"]) {
            if (evaluate_condition(sub_condition, context)) {
                return true; // Any can be true for OR
            }
        }
        return false;
    }
    
    // Handle NOT conditions
    if (condition.contains("not")) {
        return !evaluate_condition(condition["not"], context);
    }
    
    return true;
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

// Workflow persistence implementation
void WorkflowOrchestrator::ensure_workflows_directory() {
    std::filesystem::create_directories(workflows_dir_);
    std::filesystem::create_directories(templates_dir_);
}

bool WorkflowOrchestrator::save_workflow_definition(const WorkflowDefinition& workflow) {
    try {
        ensure_workflows_directory();
        
        // Convert WorkflowDefinition to JSON
        json workflow_json;
        workflow_json["id"] = workflow.id;
        workflow_json["name"] = workflow.name;
        workflow_json["description"] = workflow.description;
        workflow_json["type"] = static_cast<int>(workflow.type);
        workflow_json["version"] = workflow.version;
        workflow_json["created_at"] = workflow.created_at;
        workflow_json["max_execution_time_ms"] = workflow.max_execution_time_ms;
        workflow_json["allow_partial_failure"] = workflow.allow_partial_failure;
        workflow_json["global_context"] = workflow.global_context;
        
        if (workflow.retry_policy.has_value()) {
            workflow_json["retry_policy"] = {
                {"max_retries", workflow.retry_policy.value().max_retries},
                {"backoff_multiplier", workflow.retry_policy.value().backoff_multiplier},
                {"initial_delay_ms", workflow.retry_policy.value().initial_delay_ms},
                {"max_delay_ms", workflow.retry_policy.value().max_delay_ms}
            };
        } else {
            workflow_json["retry_policy"] = {
                {"max_retries", workflow.default_retry_policy.max_retries},
                {"backoff_multiplier", workflow.default_retry_policy.backoff_multiplier},
                {"initial_delay_ms", workflow.default_retry_policy.initial_delay_ms},
                {"max_delay_ms", workflow.default_retry_policy.max_delay_ms}
            };
        }
        
        // Convert steps
        workflow_json["steps"] = json::array();
        for (const auto& step : workflow.steps) {
            json step_json;
            step_json["id"] = step.id;
            step_json["agent_name"] = step.agent_name;
            step_json["function_name"] = step.function_name;
            step_json["parameters"] = step.parameters;
            step_json["dependencies"] = step.dependencies;
            step_json["condition"] = step.condition;
            step_json["llm_model"] = step.llm_model;
            step_json["timeout_ms"] = step.timeout_ms;
            step_json["optional"] = step.optional;
            
            if (step.retry_policy.max_retries > 0) {
                step_json["retry_policy"] = {
                    {"max_retries", step.retry_policy.max_retries},
                    {"backoff_multiplier", step.retry_policy.backoff_multiplier},
                    {"initial_delay_ms", step.retry_policy.initial_delay_ms},
                    {"max_delay_ms", step.retry_policy.max_delay_ms}
                };
            }
            
            workflow_json["steps"].push_back(step_json);
        }
        
        // Convert loop configuration if present
        if (workflow.loop_config.has_value()) {
            const auto& loop_cfg = workflow.loop_config.value();
            workflow_json["loop_config"] = {
                {"max_iterations", loop_cfg.max_iterations},
                {"break_condition", loop_cfg.break_condition},
                {"iteration_context_key", loop_cfg.iteration_context_key}
            };
        }
        
        // Save to file
        std::string file_path = workflows_dir_ + "/" + workflow.name + ".json";
        std::ofstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        file << workflow_json.dump(2);
        file.close();
        
        // Update in-memory cache
        workflow_definitions_[workflow.name] = workflow;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving workflow definition: " << e.what() << std::endl;
        return false;
    }
}

bool WorkflowOrchestrator::load_workflow_definition(const std::string& name, WorkflowDefinition& workflow) {
    try {
        // Check in-memory cache first
        auto it = workflow_definitions_.find(name);
        if (it != workflow_definitions_.end()) {
            workflow = it->second;
            return true;
        }
        
        // Try loading from disk
        std::string file_path = workflows_dir_ + "/" + name + ".json";
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        json workflow_json;
        file >> workflow_json;
        file.close();
        
        // Parse JSON back to WorkflowDefinition
        workflow.id = workflow_json["id"];
        workflow.name = workflow_json["name"];
        workflow.description = workflow_json.value("description", "");
        workflow.type = static_cast<WorkflowType>(workflow_json["type"]);
        workflow.version = workflow_json.value("version", "1.0");
        workflow.created_at = workflow_json.value("created_at", "");
        workflow.max_execution_time_ms = workflow_json.value("max_execution_time_ms", 300000);
        workflow.allow_partial_failure = workflow_json.value("allow_partial_failure", false);
        workflow.global_context = workflow_json.value("global_context", json{});
        
        // Parse retry policy
        if (workflow_json.contains("retry_policy")) {
            const auto& retry_json = workflow_json["retry_policy"];
            RetryPolicy retry_policy;
            retry_policy.max_retries = retry_json.value("max_retries", 3);
            retry_policy.backoff_multiplier = retry_json.value("backoff_multiplier", 2.0f);
            retry_policy.initial_delay_ms = retry_json.value("initial_delay_ms", 1000);
            retry_policy.max_delay_ms = retry_json.value("max_delay_ms", 30000);
            workflow.retry_policy = retry_policy;
        }
        
        // Parse steps
        workflow.steps.clear();
        if (workflow_json.contains("steps")) {
            for (const auto& step_json : workflow_json["steps"]) {
                WorkflowStep step;
                step.id = step_json["id"];
                step.agent_name = step_json["agent_name"];
                step.function_name = step_json["function_name"];
                step.parameters = step_json.value("parameters", json{});
                step.dependencies = step_json.value("dependencies", std::vector<std::string>{});
                step.condition = step_json.value("condition", json{});
                step.llm_model = step_json.value("llm_model", "");
                step.timeout_ms = step_json.value("timeout_ms", 30000);
                step.optional = step_json.value("optional", false);
                
                // Parse step retry policy if present
                if (step_json.contains("retry_policy")) {
                    const auto& retry_json = step_json["retry_policy"];
                    RetryPolicy step_retry;
                    step_retry.max_retries = retry_json.value("max_retries", 3);
                    step_retry.backoff_multiplier = retry_json.value("backoff_multiplier", 2.0);
                    step_retry.initial_delay_ms = retry_json.value("initial_delay_ms", 1000);
                    step_retry.max_delay_ms = retry_json.value("max_delay_ms", 30000);
                    step.retry_policy = step_retry;
                }
                
                workflow.steps.push_back(step);
            }
        }
        
        // Parse loop configuration if present
        if (workflow_json.contains("loop_config")) {
            const auto& loop_json = workflow_json["loop_config"];
            WorkflowDefinition::LoopConfiguration loop_cfg;
            loop_cfg.max_iterations = loop_json.value("max_iterations", 10);
            loop_cfg.break_condition = loop_json.value("break_condition", json{});
            loop_cfg.iteration_context_key = loop_json.value("iteration_context_key", "iteration");
            workflow.loop_config = loop_cfg;
        }
        
        // Cache in memory
        workflow_definitions_[name] = workflow;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading workflow definition: " << e.what() << std::endl;
        return false;
    }
}

bool WorkflowOrchestrator::delete_workflow_definition(const std::string& name) {
    try {
        // Remove from in-memory cache
        workflow_definitions_.erase(name);
        
        // Remove from disk
        std::string file_path = workflows_dir_ + "/" + name + ".json";
        return std::filesystem::remove(file_path);
    } catch (const std::exception& e) {
        std::cerr << "Error deleting workflow definition: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> WorkflowOrchestrator::list_workflow_definitions() {
    std::vector<std::string> workflow_names;
    
    try {
        // Scan workflows directory
        if (std::filesystem::exists(workflows_dir_)) {
            for (const auto& entry : std::filesystem::directory_iterator(workflows_dir_)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    std::string name = entry.path().stem().string();
                    workflow_names.push_back(name);
                }
            }
        }
        
        // Also include any workflows currently in memory but not on disk
        for (const auto& pair : workflow_definitions_) {
            if (std::find(workflow_names.begin(), workflow_names.end(), pair.first) == workflow_names.end()) {
                workflow_names.push_back(pair.first);
            }
        }
        
        std::sort(workflow_names.begin(), workflow_names.end());
    } catch (const std::exception& e) {
        std::cerr << "Error listing workflow definitions: " << e.what() << std::endl;
    }
    
    return workflow_names;
}
