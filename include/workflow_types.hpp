#pragma once

#include <string>
#include <memory>
#include <json.hpp>
#include <yaml-cpp/yaml.h>
#include "workflow_manager.hpp"

using json = nlohmann::json;

/**
 * @brief Workflow types and execution patterns
 */
enum class WorkflowType {
    SEQUENTIAL,      // Execute steps one by one
    PARALLEL,        // Execute steps in parallel
    CONDITIONAL,     // Execute based on conditions
    LOOP,           // Execute steps in a loop
    PIPELINE        // Execute as a data pipeline
};

/**
 * @brief Workflow step definition
 */
struct WorkflowStep {
    std::string id;
    std::string agent_name;
    std::string llm_model;    // Specific LLM model for this step
    std::string function_name;
    json parameters;          // Can be either:
                             // - Array of parameter names (new format): ["query", "depth"]
                             // - Object with key-value pairs (legacy format): {"query": "value", "depth": "detailed"}
    json conditions;        // Conditions for conditional execution
    std::vector<std::string> dependencies; // Dependencies on other steps
    int timeout_ms;
    bool optional;          // Whether step failure should stop workflow
    
    WorkflowStep(const std::string& step_id, 
                const std::string& agent, 
                const std::string& function,
                const json& params = json::array(),  // Default to empty array for new format
                const std::string& model = "")
        : id(step_id), agent_name(agent), function_name(function), 
          parameters(params), llm_model(model), timeout_ms(30000), optional(false) {}
};

/**
 * @brief Workflow definition
 */
struct WorkflowDefinition {
    std::string id;
    std::string name;
    std::string description;
    WorkflowType type;
    std::vector<WorkflowStep> steps;
    json global_context;    // Shared context across all steps
    int max_execution_time_ms;
    bool allow_partial_failure;
    
    // Default constructor
    WorkflowDefinition() 
        : type(WorkflowType::SEQUENTIAL), max_execution_time_ms(300000), allow_partial_failure(false) {}
    
    WorkflowDefinition(const std::string& workflow_id, 
                      const std::string& workflow_name,
                      WorkflowType workflow_type = WorkflowType::SEQUENTIAL)
        : id(workflow_id), name(workflow_name), type(workflow_type),
          max_execution_time_ms(300000), allow_partial_failure(false) {}
};

/**
 * @brief Workflow execution state
 */
enum class WorkflowExecutionState {
    PENDING,
    RUNNING,
    PAUSED,
    COMPLETED,
    FAILED,
    CANCELLED,
    TIMEOUT
};

/**
 * @brief Workflow execution context
 */
struct WorkflowExecution {
    std::string execution_id;
    std::string workflow_id;
    WorkflowExecutionState state;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    json input_data;
    json output_data;
    json context;           // Runtime context
    std::map<std::string, std::string> step_results; // Step ID -> Request ID mapping
    std::map<std::string, json> step_outputs;        // Step ID -> Output data
    std::string error_message;
    double progress_percentage;
    
    WorkflowExecution(const std::string& exec_id, const std::string& wf_id)
        : execution_id(exec_id), workflow_id(wf_id), 
          state(WorkflowExecutionState::PENDING),
          start_time(std::chrono::system_clock::now()),
          progress_percentage(0.0) {}
};

/**
 * @brief Workflow orchestrator for complex multi-agent workflows
 */
class WorkflowOrchestrator {
private:
    std::shared_ptr<WorkflowManager> workflow_manager_;
    std::map<std::string, WorkflowDefinition> workflow_definitions_;
    std::map<std::string, std::shared_ptr<WorkflowExecution>> active_executions_;
    std::map<std::string, std::shared_ptr<WorkflowExecution>> completed_executions_;
    
    // Configuration management
    json workflow_config_;
    std::string config_file_path_;
    std::map<std::string, std::map<std::string, std::vector<std::string>>> agent_llm_mappings_;
    
    mutable std::mutex orchestrator_mutex_;
    std::condition_variable execution_condition_;
    std::vector<std::thread> orchestrator_threads_;
    std::atomic<bool> running_{false};
    
public:
    explicit WorkflowOrchestrator(std::shared_ptr<WorkflowManager> workflow_manager);
    ~WorkflowOrchestrator();
    
    // Lifecycle
    bool start();
    void stop();
    bool is_running() const { return running_.load(); }
    
    // Configuration management
    bool load_workflow_config(const std::string& config_file_path);
    void reload_workflow_config();
    json get_workflow_config() const;
    
    // Workflow definition management
    void register_workflow(const WorkflowDefinition& workflow);
    bool remove_workflow(const std::string& workflow_id);
    std::vector<WorkflowDefinition> list_workflows() const;
    WorkflowDefinition* get_workflow(const std::string& workflow_id);
    
    // Workflow execution
    std::string execute_workflow(const std::string& workflow_id, const json& input_data = json{});
    std::string execute_workflow_async(const std::string& workflow_id, const json& input_data = json{});
    
    // Execution control
    bool pause_execution(const std::string& execution_id);
    bool resume_execution(const std::string& execution_id);
    bool cancel_execution(const std::string& execution_id);
    
    // Execution monitoring
    std::shared_ptr<WorkflowExecution> get_execution_status(const std::string& execution_id);
    json get_execution_progress(const std::string& execution_id);
    std::vector<std::shared_ptr<WorkflowExecution>> list_active_executions();
    
    // Built-in workflow templates
    void register_builtin_workflows();
    
private:
    // Execution engines for different workflow types
    void execute_sequential_workflow(std::shared_ptr<WorkflowExecution> execution);
    void execute_parallel_workflow(std::shared_ptr<WorkflowExecution> execution);
    void execute_conditional_workflow(std::shared_ptr<WorkflowExecution> execution);
    void execute_loop_workflow(std::shared_ptr<WorkflowExecution> execution);
    void execute_pipeline_workflow(std::shared_ptr<WorkflowExecution> execution);
    
    // Helper functions
    std::string generate_execution_id();
    bool evaluate_condition(const json& condition, const json& context);
    void update_execution_progress(std::shared_ptr<WorkflowExecution> execution);
    void move_to_completed(std::shared_ptr<WorkflowExecution> execution);
    json resolve_parameters(const json& parameters, const json& context);
    
    // Configuration helpers
    void load_agent_llm_mappings(const json& config);
    bool validate_agent_llm_pairing(const std::string& agent_name, const std::string& llm_model);
    bool validate_workflow_definition(const WorkflowDefinition& workflow);
    WorkflowDefinition parse_workflow_from_config(const json& workflow_config);
    WorkflowDefinition parse_workflow_from_yaml(const YAML::Node& workflow_config);
    
    // Step execution
    bool execute_step(const WorkflowStep& step, 
                     std::shared_ptr<WorkflowExecution> execution);
    void wait_for_step_completion(const std::string& request_id,
                                 std::shared_ptr<WorkflowExecution> execution,
                                 const WorkflowStep& step);
    
    // Orchestrator thread functions
    void orchestrator_thread();
    void process_execution(std::shared_ptr<WorkflowExecution> execution);
};

/**
 * @brief Workflow builder helper class
 */
class WorkflowBuilder {
private:
    WorkflowDefinition workflow_;
    
public:
    explicit WorkflowBuilder(const std::string& workflow_id, const std::string& name);
    
    // Workflow configuration
    WorkflowBuilder& set_type(WorkflowType type);
    WorkflowBuilder& set_description(const std::string& description);
    WorkflowBuilder& set_max_execution_time(int timeout_ms);
    WorkflowBuilder& allow_partial_failure(bool allow = true);
    WorkflowBuilder& set_global_context(const json& context);
    
    // Step building
    WorkflowBuilder& add_step(const std::string& id, 
                             const std::string& agent_name,
                             const std::string& function_name,
                             const json& parameters = json{},
                             const std::string& llm_model = "");
    WorkflowBuilder& add_conditional_step(const std::string& id,
                                         const std::string& agent_name,
                                         const std::string& function_name,
                                         const json& condition,
                                         const json& parameters = json{},
                                         const std::string& llm_model = "");
    WorkflowBuilder& add_step_dependency(const std::string& step_id, 
                                        const std::string& depends_on);
    WorkflowBuilder& set_step_timeout(const std::string& step_id, int timeout_ms);
    WorkflowBuilder& set_step_optional(const std::string& step_id, bool optional = true);
    
    // Build the workflow
    WorkflowDefinition build();
};

/**
 * @brief Predefined workflow templates
 */
namespace WorkflowTemplates {
    // Research workflow: question -> research -> analyze -> summarize
    WorkflowDefinition create_research_workflow();
    
    // Analysis workflow: input -> preprocess -> analyze -> report
    WorkflowDefinition create_analysis_workflow();
    
    // Multi-agent conversation: agents take turns responding
    WorkflowDefinition create_conversation_workflow(const std::vector<std::string>& agent_names);
    
    // Data processing pipeline: extract -> transform -> validate -> load
    WorkflowDefinition create_data_pipeline_workflow();
    
    // Decision making workflow: gather info -> analyze options -> decide -> execute
    WorkflowDefinition create_decision_workflow();
}
