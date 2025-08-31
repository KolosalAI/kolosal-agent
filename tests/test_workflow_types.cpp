#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "workflow_types.hpp"
#include "workflow_manager.hpp"
#include "agent_manager.hpp"
#include <json.hpp>
#include <memory>

using json = nlohmann::json;

class WorkflowTypesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple workflow manager for testing
        agent_manager = std::make_shared<AgentManager>();
        workflow_manager = std::make_shared<WorkflowManager>(agent_manager);
        orchestrator = std::make_unique<WorkflowOrchestrator>(workflow_manager);
    }

    void TearDown() override {
        if (orchestrator && orchestrator->is_running()) {
            orchestrator->stop();
        }
        orchestrator.reset();
        workflow_manager.reset();
        agent_manager.reset();
    }

    std::shared_ptr<AgentManager> agent_manager;
    std::shared_ptr<WorkflowManager> workflow_manager;
    std::unique_ptr<WorkflowOrchestrator> orchestrator;
};

// Test WorkflowStep structure
TEST_F(WorkflowTypesTest, WorkflowStepCreation) {
    WorkflowStep step("test_step", "test_agent", "test_function");
    
    EXPECT_EQ(step.id, "test_step");
    EXPECT_EQ(step.agent_name, "test_agent");
    EXPECT_EQ(step.function_name, "test_function");
    EXPECT_EQ(step.timeout_ms, 30000); // Default timeout
    EXPECT_FALSE(step.optional); // Default not optional
}

TEST_F(WorkflowTypesTest, WorkflowStepWithParameters) {
    json params = json::array({"param1", "param2"});
    WorkflowStep step("test_step", "test_agent", "test_function", params, "test_model");
    
    EXPECT_EQ(step.id, "test_step");
    EXPECT_EQ(step.llm_model, "test_model");
    EXPECT_EQ(step.parameters, params);
}

// Test WorkflowDefinition structure
TEST_F(WorkflowTypesTest, WorkflowDefinitionCreation) {
    WorkflowDefinition workflow("test_workflow", "Test Workflow");
    
    EXPECT_EQ(workflow.id, "test_workflow");
    EXPECT_EQ(workflow.name, "Test Workflow");
    EXPECT_EQ(workflow.type, WorkflowType::SEQUENTIAL); // Default type
    EXPECT_EQ(workflow.max_execution_time_ms, 300000); // Default timeout
    EXPECT_FALSE(workflow.allow_partial_failure); // Default behavior
}

TEST_F(WorkflowTypesTest, WorkflowDefinitionWithType) {
    WorkflowDefinition workflow("parallel_workflow", "Parallel Test", WorkflowType::PARALLEL);
    
    EXPECT_EQ(workflow.type, WorkflowType::PARALLEL);
}

// Test WorkflowExecution structure
TEST_F(WorkflowTypesTest, WorkflowExecutionCreation) {
    WorkflowExecution execution("exec_123", "workflow_456");
    
    EXPECT_EQ(execution.execution_id, "exec_123");
    EXPECT_EQ(execution.workflow_id, "workflow_456");
    EXPECT_EQ(execution.state, WorkflowExecutionState::PENDING);
    EXPECT_EQ(execution.progress_percentage, 0.0);
}

// Test WorkflowOrchestrator
TEST_F(WorkflowTypesTest, OrchestratorConstruction) {
    EXPECT_NE(orchestrator, nullptr);
    EXPECT_FALSE(orchestrator->is_running());
}

TEST_F(WorkflowTypesTest, OrchestratorStartStop) {
    EXPECT_TRUE(orchestrator->start());
    EXPECT_TRUE(orchestrator->is_running());
    
    orchestrator->stop();
    EXPECT_FALSE(orchestrator->is_running());
}

TEST_F(WorkflowTypesTest, RegisterWorkflow) {
    WorkflowDefinition workflow("test_workflow", "Test Workflow");
    workflow.description = "A test workflow for validation";
    
    EXPECT_NO_THROW(orchestrator->register_workflow(workflow));
    
    auto retrieved_workflow = orchestrator->get_workflow("test_workflow");
    EXPECT_NE(retrieved_workflow, nullptr);
    EXPECT_EQ(retrieved_workflow->id, "test_workflow");
    EXPECT_EQ(retrieved_workflow->name, "Test Workflow");
}

TEST_F(WorkflowTypesTest, RemoveWorkflow) {
    WorkflowDefinition workflow("removable_workflow", "Removable Workflow");
    orchestrator->register_workflow(workflow);
    
    EXPECT_TRUE(orchestrator->remove_workflow("removable_workflow"));
    EXPECT_EQ(orchestrator->get_workflow("removable_workflow"), nullptr);
}

TEST_F(WorkflowTypesTest, RemoveNonExistentWorkflow) {
    EXPECT_FALSE(orchestrator->remove_workflow("non_existent_workflow"));
}

TEST_F(WorkflowTypesTest, ListWorkflows) {
    WorkflowDefinition workflow1("workflow_1", "Workflow 1");
    WorkflowDefinition workflow2("workflow_2", "Workflow 2");
    
    orchestrator->register_workflow(workflow1);
    orchestrator->register_workflow(workflow2);
    
    auto workflows = orchestrator->list_workflows();
    EXPECT_EQ(workflows.size(), 2);
    
    bool found_1 = false, found_2 = false;
    for (const auto& wf : workflows) {
        if (wf.id == "workflow_1") found_1 = true;
        if (wf.id == "workflow_2") found_2 = true;
    }
    EXPECT_TRUE(found_1);
    EXPECT_TRUE(found_2);
}

TEST_F(WorkflowTypesTest, GetNonExistentWorkflow) {
    auto workflow = orchestrator->get_workflow("non_existent");
    EXPECT_EQ(workflow, nullptr);
}

TEST_F(WorkflowTypesTest, WorkflowExecutionWithoutStart) {
    WorkflowDefinition workflow("exec_test_workflow", "Execution Test");
    orchestrator->register_workflow(workflow);
    
    // Try to execute without starting orchestrator
    std::string execution_id = orchestrator->execute_workflow("exec_test_workflow");
    EXPECT_TRUE(execution_id.empty()); // Should fail or return empty
}

TEST_F(WorkflowTypesTest, ExecuteNonExistentWorkflow) {
    orchestrator->start();
    
    std::string execution_id = orchestrator->execute_workflow("non_existent_workflow");
    EXPECT_TRUE(execution_id.empty());
}

TEST_F(WorkflowTypesTest, GetExecutionStatusNonExistent) {
    auto execution = orchestrator->get_execution_status("non_existent_execution");
    EXPECT_EQ(execution, nullptr);
}

TEST_F(WorkflowTypesTest, ListActiveExecutions) {
    auto executions = orchestrator->list_active_executions();
    EXPECT_TRUE(executions.empty()); // Should be empty initially
}

TEST_F(WorkflowTypesTest, BuiltinWorkflowsRegistration) {
    EXPECT_NO_THROW(orchestrator->register_builtin_workflows());
    
    auto workflows = orchestrator->list_workflows();
    EXPECT_FALSE(workflows.empty()); // Should have registered some builtin workflows
}

// Test WorkflowBuilder
TEST_F(WorkflowTypesTest, WorkflowBuilderBasic) {
    WorkflowBuilder builder("builder_test", "Builder Test Workflow");
    
    WorkflowDefinition workflow = builder
        .set_type(WorkflowType::SEQUENTIAL)
        .set_description("Test workflow built with builder")
        .set_max_execution_time(60000)
        .allow_partial_failure(true)
        .build();
    
    EXPECT_EQ(workflow.id, "builder_test");
    EXPECT_EQ(workflow.name, "Builder Test Workflow");
    EXPECT_EQ(workflow.type, WorkflowType::SEQUENTIAL);
    EXPECT_EQ(workflow.description, "Test workflow built with builder");
    EXPECT_EQ(workflow.max_execution_time_ms, 60000);
    EXPECT_TRUE(workflow.allow_partial_failure);
}

TEST_F(WorkflowTypesTest, WorkflowBuilderWithSteps) {
    WorkflowBuilder builder("steps_test", "Steps Test Workflow");
    
    json step1_params = json::array({"query"});
    json step2_params = json::array({"data", "format"});
    
    WorkflowDefinition workflow = builder
        .set_type(WorkflowType::SEQUENTIAL)
        .add_step("step1", "agent1", "function1", step1_params, "model1")
        .add_step("step2", "agent2", "function2", step2_params, "model2")
        .add_step_dependency("step2", "step1")
        .set_step_timeout("step1", 15000)
        .set_step_optional("step2", true)
        .build();
    
    EXPECT_EQ(workflow.steps.size(), 2);
    EXPECT_EQ(workflow.steps[0].id, "step1");
    EXPECT_EQ(workflow.steps[0].agent_name, "agent1");
    EXPECT_EQ(workflow.steps[0].function_name, "function1");
    EXPECT_EQ(workflow.steps[0].llm_model, "model1");
    EXPECT_EQ(workflow.steps[0].timeout_ms, 15000);
    
    EXPECT_EQ(workflow.steps[1].id, "step2");
    EXPECT_TRUE(workflow.steps[1].optional);
    EXPECT_THAT(workflow.steps[1].dependencies, ::testing::Contains("step1"));
}

TEST_F(WorkflowTypesTest, WorkflowBuilderConditionalStep) {
    WorkflowBuilder builder("conditional_test", "Conditional Test Workflow");
    
    json condition;
    condition["field"] = "success";
    condition["operator"] = "equals";
    condition["value"] = true;
    
    json params = json::array({"result"});
    
    WorkflowDefinition workflow = builder
        .add_conditional_step("conditional_step", "agent", "function", condition, params)
        .build();
    
    EXPECT_EQ(workflow.steps.size(), 1);
    EXPECT_EQ(workflow.steps[0].id, "conditional_step");
    EXPECT_FALSE(workflow.steps[0].conditions.empty());
}

TEST_F(WorkflowTypesTest, WorkflowBuilderGlobalContext) {
    WorkflowBuilder builder("context_test", "Context Test Workflow");
    
    json global_context;
    global_context["user_id"] = "test_user";
    global_context["session_id"] = "test_session";
    
    WorkflowDefinition workflow = builder
        .set_global_context(global_context)
        .build();
    
    EXPECT_EQ(workflow.global_context["user_id"], "test_user");
    EXPECT_EQ(workflow.global_context["session_id"], "test_session");
}

// Test WorkflowTemplates
TEST_F(WorkflowTypesTest, ResearchWorkflowTemplate) {
    WorkflowDefinition research_workflow = WorkflowTemplates::create_research_workflow();
    
    EXPECT_FALSE(research_workflow.id.empty());
    EXPECT_FALSE(research_workflow.name.empty());
    EXPECT_FALSE(research_workflow.steps.empty());
}

TEST_F(WorkflowTypesTest, AnalysisWorkflowTemplate) {
    WorkflowDefinition analysis_workflow = WorkflowTemplates::create_analysis_workflow();
    
    EXPECT_FALSE(analysis_workflow.id.empty());
    EXPECT_FALSE(analysis_workflow.name.empty());
    EXPECT_FALSE(analysis_workflow.steps.empty());
}

TEST_F(WorkflowTypesTest, ConversationWorkflowTemplate) {
    std::vector<std::string> agent_names = {"agent1", "agent2", "agent3"};
    WorkflowDefinition conversation_workflow = WorkflowTemplates::create_conversation_workflow(agent_names);
    
    EXPECT_FALSE(conversation_workflow.id.empty());
    EXPECT_FALSE(conversation_workflow.name.empty());
    EXPECT_GE(conversation_workflow.steps.size(), agent_names.size());
}

TEST_F(WorkflowTypesTest, DataPipelineWorkflowTemplate) {
    WorkflowDefinition pipeline_workflow = WorkflowTemplates::create_data_pipeline_workflow();
    
    EXPECT_FALSE(pipeline_workflow.id.empty());
    EXPECT_FALSE(pipeline_workflow.name.empty());
    EXPECT_FALSE(pipeline_workflow.steps.empty());
    EXPECT_EQ(pipeline_workflow.type, WorkflowType::PIPELINE);
}

TEST_F(WorkflowTypesTest, DecisionWorkflowTemplate) {
    WorkflowDefinition decision_workflow = WorkflowTemplates::create_decision_workflow();
    
    EXPECT_FALSE(decision_workflow.id.empty());
    EXPECT_FALSE(decision_workflow.name.empty());
    EXPECT_FALSE(decision_workflow.steps.empty());
}

// Test workflow execution states
TEST_F(WorkflowTypesTest, WorkflowExecutionStates) {
    // Test all execution states exist
    WorkflowExecutionState states[] = {
        WorkflowExecutionState::PENDING,
        WorkflowExecutionState::RUNNING,
        WorkflowExecutionState::PAUSED,
        WorkflowExecutionState::COMPLETED,
        WorkflowExecutionState::FAILED,
        WorkflowExecutionState::CANCELLED,
        WorkflowExecutionState::TIMEOUT
    };
    
    // Should be able to assign all states
    for (auto state : states) {
        WorkflowExecution execution("test", "workflow");
        execution.state = state;
        EXPECT_EQ(execution.state, state);
    }
}

// Test workflow types
TEST_F(WorkflowTypesTest, WorkflowTypes) {
    // Test all workflow types exist
    WorkflowType types[] = {
        WorkflowType::SEQUENTIAL,
        WorkflowType::PARALLEL,
        WorkflowType::CONDITIONAL,
        WorkflowType::LOOP,
        WorkflowType::PIPELINE
    };
    
    // Should be able to assign all types
    for (auto type : types) {
        WorkflowDefinition workflow("test", "test");
        workflow.type = type;
        EXPECT_EQ(workflow.type, type);
    }
}

TEST_F(WorkflowTypesTest, WorkflowStepDependencies) {
    WorkflowStep step("dependent_step", "agent", "function");
    step.dependencies.push_back("prerequisite_step");
    step.dependencies.push_back("another_prerequisite");
    
    EXPECT_EQ(step.dependencies.size(), 2);
    EXPECT_THAT(step.dependencies, ::testing::Contains("prerequisite_step"));
    EXPECT_THAT(step.dependencies, ::testing::Contains("another_prerequisite"));
}

TEST_F(WorkflowTypesTest, WorkflowStepConditions) {
    WorkflowStep step("conditional_step", "agent", "function");
    step.conditions["condition_type"] = "success_check";
    step.conditions["threshold"] = 0.8;
    
    EXPECT_TRUE(step.conditions.contains("condition_type"));
    EXPECT_EQ(step.conditions["threshold"], 0.8);
}

TEST_F(WorkflowTypesTest, WorkflowExecutionProgressTracking) {
    WorkflowExecution execution("progress_test", "workflow_id");
    
    execution.progress_percentage = 25.0;
    EXPECT_EQ(execution.progress_percentage, 25.0);
    
    execution.progress_percentage = 50.0;
    EXPECT_EQ(execution.progress_percentage, 50.0);
    
    execution.progress_percentage = 100.0;
    EXPECT_EQ(execution.progress_percentage, 100.0);
}

TEST_F(WorkflowTypesTest, WorkflowExecutionStepResults) {
    WorkflowExecution execution("results_test", "workflow_id");
    
    execution.step_results["step1"] = "request_123";
    execution.step_results["step2"] = "request_456";
    
    execution.step_outputs["step1"] = json{{"output", "result1"}};
    execution.step_outputs["step2"] = json{{"output", "result2"}};
    
    EXPECT_EQ(execution.step_results["step1"], "request_123");
    EXPECT_EQ(execution.step_outputs["step1"]["output"], "result1");
}

TEST_F(WorkflowTypesTest, WorkflowExecutionInputOutput) {
    WorkflowExecution execution("io_test", "workflow_id");
    
    execution.input_data = json{{"input_param", "input_value"}};
    execution.output_data = json{{"output_param", "output_value"}};
    execution.context = json{{"context_param", "context_value"}};
    
    EXPECT_EQ(execution.input_data["input_param"], "input_value");
    EXPECT_EQ(execution.output_data["output_param"], "output_value");
    EXPECT_EQ(execution.context["context_param"], "context_value");
}

TEST_F(WorkflowTypesTest, WorkflowConfigurationLoading) {
    // Test loading workflow configuration (should not throw even if file doesn't exist)
    EXPECT_NO_THROW(orchestrator->load_workflow_config("non_existent_workflow.yaml"));
}

TEST_F(WorkflowTypesTest, GetWorkflowConfig) {
    json config = orchestrator->get_workflow_config();
    EXPECT_TRUE(config.is_object() || config.is_null());
}

TEST_F(WorkflowTypesTest, WorkflowBuilderEmptyWorkflow) {
    WorkflowBuilder builder("empty_test", "Empty Test Workflow");
    
    WorkflowDefinition workflow = builder.build();
    
    EXPECT_EQ(workflow.id, "empty_test");
    EXPECT_EQ(workflow.name, "Empty Test Workflow");
    EXPECT_TRUE(workflow.steps.empty());
}

TEST_F(WorkflowTypesTest, WorkflowBuilderChaining) {
    WorkflowBuilder builder("chain_test", "Chain Test Workflow");
    
    // Test method chaining
    WorkflowDefinition workflow = builder
        .set_type(WorkflowType::PARALLEL)
        .set_description("Chained builder test")
        .set_max_execution_time(120000)
        .allow_partial_failure(true)
        .add_step("step1", "agent1", "func1")
        .add_step("step2", "agent2", "func2")
        .build();
    
    EXPECT_EQ(workflow.type, WorkflowType::PARALLEL);
    EXPECT_EQ(workflow.description, "Chained builder test");
    EXPECT_EQ(workflow.max_execution_time_ms, 120000);
    EXPECT_TRUE(workflow.allow_partial_failure);
    EXPECT_EQ(workflow.steps.size(), 2);
}

TEST_F(WorkflowTypesTest, WorkflowStepOptionalFlag) {
    WorkflowStep required_step("required", "agent", "function");
    WorkflowStep optional_step("optional", "agent", "function");
    optional_step.optional = true;
    
    EXPECT_FALSE(required_step.optional);
    EXPECT_TRUE(optional_step.optional);
}

TEST_F(WorkflowTypesTest, WorkflowStepTimeoutCustomization) {
    WorkflowStep step("timeout_test", "agent", "function");
    step.timeout_ms = 60000; // 1 minute
    
    EXPECT_EQ(step.timeout_ms, 60000);
}

TEST_F(WorkflowTypesTest, WorkflowExecutionTiming) {
    WorkflowExecution execution("timing_test", "workflow_id");
    
    auto start_time = execution.start_time;
    
    // Simulate execution completion
    execution.end_time = std::chrono::system_clock::now();
    
    EXPECT_GE(execution.end_time, start_time);
}

TEST_F(WorkflowTypesTest, WorkflowExecutionErrorHandling) {
    WorkflowExecution execution("error_test", "workflow_id");
    
    execution.state = WorkflowExecutionState::FAILED;
    execution.error_message = "Test error message";
    
    EXPECT_EQ(execution.state, WorkflowExecutionState::FAILED);
    EXPECT_EQ(execution.error_message, "Test error message");
}

TEST_F(WorkflowTypesTest, ConversationWorkflowWithEmptyAgents) {
    std::vector<std::string> empty_agents;
    WorkflowDefinition conversation = WorkflowTemplates::create_conversation_workflow(empty_agents);
    
    // Should handle empty agent list gracefully
    EXPECT_FALSE(conversation.id.empty());
}

TEST_F(WorkflowTypesTest, ConversationWorkflowWithSingleAgent) {
    std::vector<std::string> single_agent = {"lonely_agent"};
    WorkflowDefinition conversation = WorkflowTemplates::create_conversation_workflow(single_agent);
    
    EXPECT_FALSE(conversation.id.empty());
    EXPECT_FALSE(conversation.steps.empty());
}
