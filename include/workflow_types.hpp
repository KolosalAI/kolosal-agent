#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <json.hpp>
#include <yaml-cpp/yaml.h>

// Forward declaration to avoid circular dependency
class WorkflowManager;

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
 * @brief Retry policy configuration
 */
struct RetryPolicy {
    int max_retries;
    float backoff_multiplier;
    int initial_delay_ms;
    int max_delay_ms;
    
    RetryPolicy() : max_retries(0), backoff_multiplier(1.5f), initial_delay_ms(1000), max_delay_ms(30000) {}
    
    RetryPolicy(int retries, float backoff = 1.5f, int initial = 1000, int max_delay = 30000)
        : max_retries(retries), backoff_multiplier(backoff), initial_delay_ms(initial), max_delay_ms(max_delay) {}
};

/**
 * @brief Step execution statistics
 */
struct StepExecutionStats {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    int retry_count;
    std::string error_message;
    bool completed_successfully;
    
    StepExecutionStats() : retry_count(0), completed_successfully(false) {}
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
    json condition;         // Alternative condition field (for compatibility)
    std::vector<std::string> dependencies; // Dependencies on other steps
    int timeout_ms;
    bool optional;          // Whether step failure should stop workflow
    RetryPolicy retry_policy; // Retry configuration for this step
    json context_injection;   // Additional context for this step
    
    // Default constructor
    WorkflowStep() : timeout_ms(30000), optional(false) {}
    
    WorkflowStep(const std::string& step_id, 
                const std::string& agent, 
                const std::string& function,
                const json& params = json::array(),  // Default to empty array for new format
                const std::string& model = "")
        : id(step_id), agent_name(agent), llm_model(model), function_name(function), 
          parameters(params), timeout_ms(30000), optional(false) {}
};

/**
 * @brief Workflow definition
 */
struct WorkflowDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string version;         // Version information
    std::string created_at;      // Creation timestamp
    WorkflowType type;
    std::vector<WorkflowStep> steps;
    json global_context;    // Shared context across all steps
    int max_execution_time_ms;
    bool allow_partial_failure;
    RetryPolicy default_retry_policy; // Default retry policy for all steps
    std::optional<RetryPolicy> retry_policy; // Optional retry policy override
    bool fail_fast;         // Stop on first error
    
    // Loop-specific configuration
    struct LoopConfiguration {
        int max_iterations;
        json termination_condition;
        json break_condition;  // Alternative name for compatibility
        std::string iteration_context_key;
        int iteration_delay_ms;
    };
    std::optional<LoopConfiguration> loop_config;
    
    // Pipeline-specific configuration  
    struct {
        bool pass_through_on_error;
        bool merge_outputs;
        std::string output_format; // "last_step", "all_steps", "merged"
    } pipeline_config;
    
    // Default constructor
    WorkflowDefinition() 
        : type(WorkflowType::SEQUENTIAL), max_execution_time_ms(300000), 
          allow_partial_failure(false), fail_fast(true) {
        pipeline_config.pass_through_on_error = false;
        pipeline_config.merge_outputs = false;
        pipeline_config.output_format = "last_step";
    }
    
    WorkflowDefinition(const std::string& workflow_id, 
                      const std::string& workflow_name,
                      WorkflowType workflow_type = WorkflowType::SEQUENTIAL)
        : id(workflow_id), name(workflow_name), type(workflow_type),
          max_execution_time_ms(300000), allow_partial_failure(false), fail_fast(true) {
        pipeline_config.pass_through_on_error = false;
        pipeline_config.merge_outputs = false;
        pipeline_config.output_format = "last_step";
    }
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
    std::map<std::string, StepExecutionStats> step_stats; // Step ID -> Execution statistics
    std::string error_message;
    double progress_percentage;
    std::string current_step_id; // Currently executing step
    int failed_step_count;      // Number of failed steps
    std::vector<std::string> execution_log; // Execution log messages
    
    WorkflowExecution(const std::string& exec_id, const std::string& wf_id)
        : execution_id(exec_id), workflow_id(wf_id), 
          state(WorkflowExecutionState::PENDING),
          start_time(std::chrono::system_clock::now()),
          progress_percentage(0.0), failed_step_count(0) {}
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
    std::string workflows_dir_;
    std::string templates_dir_;
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
    bool load_workflow_config(const std::string& config_file_path = "./configs/workflow.yaml");
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
    void update_execution_progress(std::shared_ptr<WorkflowExecution> execution);
    void move_to_completed(std::shared_ptr<WorkflowExecution> execution);
    json resolve_parameters(const json& parameters, const json& context);
    
    // Configuration helpers
    void load_agent_llm_mappings(const json& config);
    bool validate_agent_llm_pairing(const std::string& agent_name, const std::string& llm_model);
    bool validate_workflow_definition(const WorkflowDefinition& workflow);
    WorkflowDefinition parse_workflow_from_config(const json& workflow_config);
    WorkflowDefinition parse_workflow_from_yaml(const YAML::Node& workflow_config);
    
    // Workflow persistence
    bool save_workflow_definition(const WorkflowDefinition& workflow);
    bool load_workflow_definition(const std::string& name, WorkflowDefinition& workflow);
    bool delete_workflow_definition(const std::string& name);
    std::vector<std::string> list_workflow_definitions();
    void ensure_workflows_directory();
    
    // Step execution with retry support
    bool execute_step_with_retry(const WorkflowStep& step, 
                                std::shared_ptr<WorkflowExecution> execution);
    bool execute_step(const WorkflowStep& step, 
                     std::shared_ptr<WorkflowExecution> execution);
    void wait_for_step_completion(const std::string& request_id,
                                 std::shared_ptr<WorkflowExecution> execution,
                                 const WorkflowStep& step);
    
    // Enhanced condition evaluation
    bool evaluate_condition(const json& condition, const json& context);
    bool evaluate_complex_condition(const json& condition, const json& context);
    
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
