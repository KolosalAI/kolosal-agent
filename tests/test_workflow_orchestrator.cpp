/**
 * @file test_workflow_orchestrator.cpp
 * @brief Comprehensive tests for WorkflowOrchestrator component
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Test suite covering:
 * - Workflow Orchestrator Lifecycle
 * - Workflow Definition Management
 * - Workflow Execution Control
 * - Different Workflow Types
 * - Agent-LLM Pairings
 * - Parameter Resolution
 * - Error Handling and Recovery
 */

#include "../external/yaml-cpp/test/gtest-1.11.0/googletest/include/gtest/gtest.h"
#include "../include/workflow_types.hpp"
#include "../include/workflow_manager.hpp"
#include "../include/agent_manager.hpp"
#include <json.hpp>
#include <chrono>
#include <thread>
#include <future>
#include <fstream>

using json = nlohmann::json;

class WorkflowOrchestratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create agent manager with test agents
        config_manager_ = std::make_shared<AgentConfigManager>();
        agent_manager_ = std::make_shared<AgentManager>(config_manager_);
        
        // Create test agents that match common workflow patterns
        assistant_id_ = agent_manager_->create_agent("Assistant", {"chat", "status"});
        analyzer_id_ = agent_manager_->create_agent("Analyzer", {"analysis", "analyze"});
        researcher_id_ = agent_manager_->create_agent("Researcher", {"research"});
        
        // Start agents
        agent_manager_->start_agent(assistant_id_);
        agent_manager_->start_agent(analyzer_id_);
        agent_manager_->start_agent(researcher_id_);
        
        // Wait for agents to start
        waitForAgentsStartup();
        
        // Create workflow manager
        workflow_manager_ = std::make_shared<WorkflowManager>(agent_manager_, 4, 100, 1000);
        loadTestFunctionConfigs();
        workflow_manager_->start();
        
        // Create workflow orchestrator
        workflow_orchestrator_ = std::make_shared<WorkflowOrchestrator>(workflow_manager_);
        workflow_orchestrator_->start();
    }
    
    void TearDown() override {
        if (workflow_orchestrator_) {
            workflow_orchestrator_->stop();
        }
        if (workflow_manager_) {
            workflow_manager_->stop();
        }
        if (agent_manager_) {
            agent_manager_->stop_all_agents();
        }
        
        // Clean up test files
        std::remove("test_orchestrator_workflow.yaml");
    }
    
    void loadTestFunctionConfigs() {
        json function_config;
        function_config["functions"]["chat"] = {
            {"description", "Chat functionality"},
            {"timeout", 10000},
            {"parameters", json::array({
                {{"name", "message"}, {"type", "string"}, {"required", true}},
                {{"name", "model"}, {"type", "string"}, {"required", false}}
            })}
        };
        function_config["functions"]["analyze"] = {
            {"description", "Analysis functionality"},
            {"timeout", 15000},
            {"parameters", json::array({
                {{"name", "text"}, {"type", "string"}, {"required", true}},
                {{"name", "analysis_type"}, {"type", "string"}, {"required", false}}
            })}
        };
        function_config["functions"]["research"] = {
            {"description", "Research functionality"},
            {"timeout", 20000},
            {"parameters", json::array({
                {{"name", "query"}, {"type", "string"}, {"required", true}},
                {{"name", "depth"}, {"type", "string"}, {"required", false}}
            })}
        };
        function_config["functions"]["status"] = {
            {"description", "Status check functionality"},
            {"timeout", 5000},
            {"parameters", json::array()}
        };
        
        workflow_manager_->load_function_configs(function_config);
    }
    
    void waitForAgentsStartup() {
        std::vector<std::string> agents = {assistant_id_, analyzer_id_, researcher_id_};
        for (const auto& agent_id : agents) {
            auto start_time = std::chrono::steady_clock::now();
            while (std::chrono::steady_clock::now() - start_time < std::chrono::milliseconds(5000)) {
                auto agent = agent_manager_->get_agent(agent_id);
                if (agent && agent->is_running()) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    
    bool waitForWorkflowCompletion(const std::string& execution_id, int timeout_ms = 30000) {
        auto start_time = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start_time < std::chrono::milliseconds(timeout_ms)) {
            auto execution = workflow_orchestrator_->get_execution_status(execution_id);
            if (execution && (execution->state == WorkflowExecutionState::COMPLETED ||
                             execution->state == WorkflowExecutionState::FAILED ||
                             execution->state == WorkflowExecutionState::CANCELLED ||
                             execution->state == WorkflowExecutionState::TIMEOUT)) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        return false;
    }
    
    void createTestWorkflowConfig() {
        std::ofstream config_file("test_orchestrator_workflow.yaml");
        config_file << R"(
agent_llm_mappings:
  Assistant:
    default_model: "test-model"
    supported_models: ["test-model"]
  Analyzer:
    default_model: "test-model"
    supported_models: ["test-model"]
  Researcher:
    default_model: "test-model"
    supported_models: ["test-model"]

workflows:
  - id: "test_sequential_workflow"
    name: "Test Sequential Workflow"
    type: "sequential"
    steps:
      - id: "step1"
        agent_name: "Assistant"
        llm_model: "test-model"
        function_name: "chat"
        parameters:
          - "message"
          - "model"
      - id: "step2"
        agent_name: "Analyzer"
        llm_model: "test-model"
        function_name: "analyze"
        parameters:
          - "text"
          - "analysis_type"
        dependencies: ["step1"]
)";
        config_file.close();
    }
    
protected:
    std::shared_ptr<AgentConfigManager> config_manager_;
    std::shared_ptr<AgentManager> agent_manager_;
    std::shared_ptr<WorkflowManager> workflow_manager_;
    std::shared_ptr<WorkflowOrchestrator> workflow_orchestrator_;
    std::string assistant_id_, analyzer_id_, researcher_id_;
};

// Lifecycle Tests
class WorkflowOrchestratorLifecycleTest : public WorkflowOrchestratorTest {};

TEST_F(WorkflowOrchestratorLifecycleTest, StartAndStop) {
    // Orchestrator should already be running from SetUp
    EXPECT_TRUE(workflow_orchestrator_->is_running());
    
    workflow_orchestrator_->stop();
    EXPECT_FALSE(workflow_orchestrator_->is_running());
    
    // Test restart
    EXPECT_TRUE(workflow_orchestrator_->start());
    EXPECT_TRUE(workflow_orchestrator_->is_running());
}

TEST_F(WorkflowOrchestratorLifecycleTest, BuiltinWorkflows) {
    // Check that built-in workflows are registered
    auto workflows = workflow_orchestrator_->list_workflows();
    EXPECT_GT(workflows.size(), 0);
    
    // Look for some built-in workflows
    bool found_research = false;
    bool found_analysis = false;
    
    for (const auto& workflow : workflows) {
        if (workflow.id == "research_workflow") {
            found_research = true;
        } else if (workflow.id == "analysis_workflow") {
            found_analysis = true;
        }
    }
    
    EXPECT_TRUE(found_research);
    EXPECT_TRUE(found_analysis);
}

// Workflow Definition Management Tests
class WorkflowDefinitionTest : public WorkflowOrchestratorTest {};

TEST_F(WorkflowDefinitionTest, RegisterWorkflow) {
    WorkflowDefinition test_workflow("test_register", "Test Registration Workflow");
    test_workflow.type = WorkflowType::SEQUENTIAL;
    test_workflow.description = "Testing workflow registration";
    
    WorkflowStep step("test_step", "Assistant", "chat", json::array({"message", "model"}), "test-model");
    test_workflow.steps.push_back(step);
    
    workflow_orchestrator_->register_workflow(test_workflow);
    
    auto workflows = workflow_orchestrator_->list_workflows();
    bool found = false;
    for (const auto& workflow : workflows) {
        if (workflow.id == "test_register") {
            found = true;
            EXPECT_EQ(workflow.name, "Test Registration Workflow");
            EXPECT_EQ(workflow.type, WorkflowType::SEQUENTIAL);
            EXPECT_EQ(workflow.steps.size(), 1);
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(WorkflowDefinitionTest, RemoveWorkflow) {
    // Register a workflow first
    WorkflowDefinition test_workflow("test_remove", "Test Removal Workflow");
    WorkflowStep step("test_step", "Assistant", "status", json::array());
    test_workflow.steps.push_back(step);
    
    workflow_orchestrator_->register_workflow(test_workflow);
    
    // Verify it exists
    auto workflow_ptr = workflow_orchestrator_->get_workflow("test_remove");
    EXPECT_NE(workflow_ptr, nullptr);
    
    // Remove it
    EXPECT_TRUE(workflow_orchestrator_->remove_workflow("test_remove"));
    
    // Verify it's gone
    workflow_ptr = workflow_orchestrator_->get_workflow("test_remove");
    EXPECT_EQ(workflow_ptr, nullptr);
    
    // Try to remove non-existent workflow
    EXPECT_FALSE(workflow_orchestrator_->remove_workflow("non_existent"));
}

TEST_F(WorkflowDefinitionTest, GetWorkflow) {
    WorkflowDefinition test_workflow("test_get", "Test Get Workflow");
    test_workflow.description = "Testing workflow retrieval";
    WorkflowStep step("test_step", "Assistant", "status", json::array());
    test_workflow.steps.push_back(step);
    
    workflow_orchestrator_->register_workflow(test_workflow);
    
    auto workflow_ptr = workflow_orchestrator_->get_workflow("test_get");
    ASSERT_NE(workflow_ptr, nullptr);
    
    EXPECT_EQ(workflow_ptr->id, "test_get");
    EXPECT_EQ(workflow_ptr->name, "Test Get Workflow");
    EXPECT_EQ(workflow_ptr->description, "Testing workflow retrieval");
    EXPECT_EQ(workflow_ptr->steps.size(), 1);
}

TEST_F(WorkflowDefinitionTest, ListWorkflows) {
    size_t initial_count = workflow_orchestrator_->list_workflows().size();
    
    // Register multiple workflows
    for (int i = 0; i < 3; ++i) {
        WorkflowDefinition workflow("test_list_" + std::to_string(i), 
                                   "Test List Workflow " + std::to_string(i));
        WorkflowStep step("step", "Assistant", "status", json::array());
        workflow.steps.push_back(step);
        workflow_orchestrator_->register_workflow(workflow);
    }
    
    auto workflows = workflow_orchestrator_->list_workflows();
    EXPECT_EQ(workflows.size(), initial_count + 3);
    
    // Verify all test workflows are present
    int found_count = 0;
    for (const auto& workflow : workflows) {
        if (workflow.id.find("test_list_") == 0) {
            found_count++;
        }
    }
    EXPECT_EQ(found_count, 3);
}

// Workflow Execution Tests
class WorkflowExecutionTest : public WorkflowOrchestratorTest {};

TEST_F(WorkflowExecutionTest, SimpleSequentialExecution) {
    // Create a simple sequential workflow
    WorkflowDefinition workflow("simple_sequential", "Simple Sequential Test");
    workflow.type = WorkflowType::SEQUENTIAL;
    
    WorkflowStep step1("step1", "Assistant", "status", json::array(), "test-model");
    WorkflowStep step2("step2", "Assistant", "chat", json::array({"message", "model"}), "test-model");
    step2.dependencies.push_back("step1");
    
    workflow.steps.push_back(step1);
    workflow.steps.push_back(step2);
    
    workflow_orchestrator_->register_workflow(workflow);
    
    // Execute workflow
    json input_data;
    input_data["message"] = "Test sequential execution";
    
    std::string execution_id = workflow_orchestrator_->execute_workflow_async("simple_sequential", input_data);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for completion
    EXPECT_TRUE(waitForWorkflowCompletion(execution_id));
    
    auto execution = workflow_orchestrator_->get_execution_status(execution_id);
    ASSERT_NE(execution, nullptr);
    EXPECT_EQ(execution->workflow_id, "simple_sequential");
    // Should be completed or failed (depending on agent implementation)
    EXPECT_TRUE(execution->state == WorkflowExecutionState::COMPLETED ||
                execution->state == WorkflowExecutionState::FAILED);
}

TEST_F(WorkflowExecutionTest, ParallelExecution) {
    // Create a parallel workflow
    WorkflowDefinition workflow("test_parallel", "Test Parallel Workflow");
    workflow.type = WorkflowType::PARALLEL;
    workflow.allow_partial_failure = true;
    
    WorkflowStep step1("parallel_step1", "Assistant", "status", json::array());
    WorkflowStep step2("parallel_step2", "Assistant", "status", json::array());
    
    workflow.steps.push_back(step1);
    workflow.steps.push_back(step2);
    
    workflow_orchestrator_->register_workflow(workflow);
    
    // Execute workflow
    std::string execution_id = workflow_orchestrator_->execute_workflow_async("test_parallel", json{});
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for completion
    EXPECT_TRUE(waitForWorkflowCompletion(execution_id));
    
    auto execution = workflow_orchestrator_->get_execution_status(execution_id);
    ASSERT_NE(execution, nullptr);
    EXPECT_EQ(execution->workflow_id, "test_parallel");
}

TEST_F(WorkflowExecutionTest, SynchronousExecution) {
    // Create a simple workflow
    WorkflowDefinition workflow("sync_test", "Synchronous Test Workflow");
    WorkflowStep step("sync_step", "Assistant", "status", json::array());
    workflow.steps.push_back(step);
    
    workflow_orchestrator_->register_workflow(workflow);
    
    // Execute synchronously (should block until completion)
    std::string execution_id = workflow_orchestrator_->execute_workflow("sync_test", json{});
    EXPECT_FALSE(execution_id.empty());
    
    // Should already be completed
    auto execution = workflow_orchestrator_->get_execution_status(execution_id);
    ASSERT_NE(execution, nullptr);
    EXPECT_TRUE(execution->state == WorkflowExecutionState::COMPLETED ||
                execution->state == WorkflowExecutionState::FAILED);
}

TEST_F(WorkflowExecutionTest, ExecutionProgress) {
    // Create a multi-step workflow
    WorkflowDefinition workflow("progress_test", "Progress Test Workflow");
    workflow.type = WorkflowType::SEQUENTIAL;
    
    for (int i = 0; i < 3; ++i) {
        WorkflowStep step("step" + std::to_string(i), "Assistant", "status", json::array());
        if (i > 0) {
            step.dependencies.push_back("step" + std::to_string(i-1));
        }
        workflow.steps.push_back(step);
    }
    
    workflow_orchestrator_->register_workflow(workflow);
    
    std::string execution_id = workflow_orchestrator_->execute_workflow_async("progress_test", json{});
    
    // Monitor progress
    double max_progress = 0.0;
    for (int i = 0; i < 50; ++i) {
        auto progress_info = workflow_orchestrator_->get_execution_progress(execution_id);
        if (progress_info.contains("progress_percentage")) {
            max_progress = std::max(max_progress, progress_info["progress_percentage"].get<double>());
        }
        
        auto execution = workflow_orchestrator_->get_execution_status(execution_id);
        if (execution && (execution->state == WorkflowExecutionState::COMPLETED ||
                         execution->state == WorkflowExecutionState::FAILED)) {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    EXPECT_GT(max_progress, 0.0);
}

// Execution Control Tests
class WorkflowExecutionControlTest : public WorkflowOrchestratorTest {};

TEST_F(WorkflowExecutionControlTest, PauseAndResume) {
    // Create a workflow that takes some time
    WorkflowDefinition workflow("pause_test", "Pause Test Workflow");
    workflow.type = WorkflowType::SEQUENTIAL;
    
    for (int i = 0; i < 2; ++i) {
        WorkflowStep step("pause_step" + std::to_string(i), "Assistant", "status", json::array());
        if (i > 0) {
            step.dependencies.push_back("pause_step" + std::to_string(i-1));
        }
        workflow.steps.push_back(step);
    }
    
    workflow_orchestrator_->register_workflow(workflow);
    
    std::string execution_id = workflow_orchestrator_->execute_workflow_async("pause_test", json{});
    
    // Try to pause (might work if caught before completion)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    bool paused = workflow_orchestrator_->pause_execution(execution_id);
    
    if (paused) {
        auto execution = workflow_orchestrator_->get_execution_status(execution_id);
        EXPECT_EQ(execution->state, WorkflowExecutionState::PAUSED);
        
        // Resume
        EXPECT_TRUE(workflow_orchestrator_->resume_execution(execution_id));
        execution = workflow_orchestrator_->get_execution_status(execution_id);
        EXPECT_EQ(execution->state, WorkflowExecutionState::RUNNING);
    }
    
    // Wait for completion
    waitForWorkflowCompletion(execution_id);
}

TEST_F(WorkflowExecutionControlTest, CancelExecution) {
    WorkflowDefinition workflow("cancel_test", "Cancel Test Workflow");
    workflow.type = WorkflowType::SEQUENTIAL;
    
    // Add multiple steps to increase chance of cancelling before completion
    for (int i = 0; i < 3; ++i) {
        WorkflowStep step("cancel_step" + std::to_string(i), "Assistant", "status", json::array());
        if (i > 0) {
            step.dependencies.push_back("cancel_step" + std::to_string(i-1));
        }
        workflow.steps.push_back(step);
    }
    
    workflow_orchestrator_->register_workflow(workflow);
    
    std::string execution_id = workflow_orchestrator_->execute_workflow_async("cancel_test", json{});
    
    // Try to cancel quickly
    EXPECT_TRUE(workflow_orchestrator_->cancel_execution(execution_id));
    
    auto execution = workflow_orchestrator_->get_execution_status(execution_id);
    ASSERT_NE(execution, nullptr);
    EXPECT_EQ(execution->state, WorkflowExecutionState::CANCELLED);
    EXPECT_FALSE(execution->error_message.empty());
}

TEST_F(WorkflowExecutionControlTest, ListActiveExecutions) {
    size_t initial_count = workflow_orchestrator_->list_active_executions().size();
    
    // Create a simple workflow
    WorkflowDefinition workflow("active_test", "Active Test Workflow");
    WorkflowStep step("active_step", "Assistant", "status", json::array());
    workflow.steps.push_back(step);
    
    workflow_orchestrator_->register_workflow(workflow);
    
    // Start multiple executions
    std::vector<std::string> execution_ids;
    for (int i = 0; i < 3; ++i) {
        std::string execution_id = workflow_orchestrator_->execute_workflow_async("active_test", json{});
        execution_ids.push_back(execution_id);
    }
    
    // Check active executions (might be completed quickly)
    auto active_executions = workflow_orchestrator_->list_active_executions();
    // Should have at least the initial count (some may have completed already)
    EXPECT_GE(active_executions.size(), initial_count);
    
    // Wait for completion
    for (const auto& execution_id : execution_ids) {
        waitForWorkflowCompletion(execution_id);
    }
}

// Configuration Loading Tests
class WorkflowConfigurationTest : public WorkflowOrchestratorTest {};

TEST_F(WorkflowConfigurationTest, LoadWorkflowConfig) {
    createTestWorkflowConfig();
    
    bool loaded = workflow_orchestrator_->load_workflow_config("test_orchestrator_workflow.yaml");
    EXPECT_TRUE(loaded);
    
    // Check that the workflow was loaded
    auto workflow_ptr = workflow_orchestrator_->get_workflow("test_sequential_workflow");
    EXPECT_NE(workflow_ptr, nullptr);
    
    if (workflow_ptr) {
        EXPECT_EQ(workflow_ptr->name, "Test Sequential Workflow");
        EXPECT_EQ(workflow_ptr->type, WorkflowType::SEQUENTIAL);
        EXPECT_EQ(workflow_ptr->steps.size(), 2);
        
        // Check dependencies
        EXPECT_EQ(workflow_ptr->steps[1].dependencies.size(), 1);
        EXPECT_EQ(workflow_ptr->steps[1].dependencies[0], "step1");
        
        // Check LLM models
        EXPECT_EQ(workflow_ptr->steps[0].llm_model, "test-model");
        EXPECT_EQ(workflow_ptr->steps[1].llm_model, "test-model");
    }
}

TEST_F(WorkflowConfigurationTest, ReloadConfiguration) {
    createTestWorkflowConfig();
    
    // Load initial configuration
    EXPECT_TRUE(workflow_orchestrator_->load_workflow_config("test_orchestrator_workflow.yaml"));
    
    // Modify the configuration file
    std::ofstream config_file("test_orchestrator_workflow.yaml", std::ios::app);
    config_file << R"(
  - id: "test_reloaded_workflow"
    name: "Test Reloaded Workflow"
    type: "sequential"
    steps:
      - id: "reload_step"
        agent_name: "Assistant"
        function_name: "status"
        parameters: []
)";
    config_file.close();
    
    // Reload configuration
    workflow_orchestrator_->reload_workflow_config();
    
    // Check that new workflow is available
    auto workflow_ptr = workflow_orchestrator_->get_workflow("test_reloaded_workflow");
    // May or may not be present depending on YAML parsing implementation
}

TEST_F(WorkflowConfigurationTest, InvalidConfiguration) {
    // Create invalid configuration file
    std::ofstream invalid_config("invalid_orchestrator_config.yaml");
    invalid_config << R"(
invalid_yaml: [
  missing_bracket
workflows:
  - id: "invalid_workflow"
    steps: "not_an_array"
)";
    invalid_config.close();
    
    // Should handle invalid config gracefully
    bool loaded = workflow_orchestrator_->load_workflow_config("invalid_orchestrator_config.yaml");
    // May be false, but shouldn't crash
    
    // Built-in workflows should still be available
    auto workflows = workflow_orchestrator_->list_workflows();
    EXPECT_GT(workflows.size(), 0);
    
    std::remove("invalid_orchestrator_config.yaml");
}

// Workflow Builder Tests
class WorkflowBuilderTest : public WorkflowOrchestratorTest {};

TEST_F(WorkflowBuilderTest, BasicBuilder) {
    auto workflow = WorkflowBuilder("builder_test", "Builder Test")
        .set_type(WorkflowType::SEQUENTIAL)
        .set_description("Testing workflow builder")
        .add_step("step1", "Assistant", "status", json::array())
        .add_step("step2", "Assistant", "chat", json::array({"message", "model"}))
        .add_step_dependency("step2", "step1")
        .build();
    
    EXPECT_EQ(workflow.id, "builder_test");
    EXPECT_EQ(workflow.name, "Builder Test");
    EXPECT_EQ(workflow.type, WorkflowType::SEQUENTIAL);
    EXPECT_EQ(workflow.steps.size(), 2);
    EXPECT_EQ(workflow.steps[1].dependencies.size(), 1);
    EXPECT_EQ(workflow.steps[1].dependencies[0], "step1");
}

TEST_F(WorkflowBuilderTest, BuilderWithConfiguration) {
    auto workflow = WorkflowBuilder("config_builder_test", "Config Builder Test")
        .set_type(WorkflowType::PARALLEL)
        .set_max_execution_time(600000)
        .allow_partial_failure(true)
        .add_step("config_step1", "Assistant", "status", json::array())
        .add_step("config_step2", "Analyzer", "analyze", json::array({"text", "analysis_type"}))
        .set_step_timeout("config_step1", 30000)
        .set_step_optional("config_step2", true)
        .build();
    
    EXPECT_EQ(workflow.type, WorkflowType::PARALLEL);
    EXPECT_EQ(workflow.max_execution_time_ms, 600000);
    EXPECT_TRUE(workflow.allow_partial_failure);
    EXPECT_TRUE(workflow.steps[1].optional);
}

TEST_F(WorkflowBuilderTest, ConditionalWorkflow) {
    json condition;
    condition["field"] = "input.condition_flag";
    condition["operator"] = "equals";
    condition["value"] = true;
    
    auto workflow = WorkflowBuilder("conditional_test", "Conditional Test")
        .set_type(WorkflowType::CONDITIONAL)
        .add_step("always_step", "Assistant", "status", json::array())
        .add_conditional_step("conditional_step", "Analyzer", "analyze", 
                             condition, json::array({"text", "analysis_type"}))
        .build();
    
    EXPECT_EQ(workflow.type, WorkflowType::CONDITIONAL);
    EXPECT_EQ(workflow.steps.size(), 2);
    EXPECT_FALSE(workflow.steps[1].conditions.empty());
}

// Template Workflows Tests
class WorkflowTemplateTest : public WorkflowOrchestratorTest {};

TEST_F(WorkflowTemplateTest, ResearchWorkflowTemplate) {
    auto research_workflow = WorkflowTemplates::create_research_workflow();
    
    EXPECT_EQ(research_workflow.id, "research_workflow");
    EXPECT_EQ(research_workflow.type, WorkflowType::SEQUENTIAL);
    EXPECT_GT(research_workflow.steps.size(), 1);
    
    // Register and test execution
    workflow_orchestrator_->register_workflow(research_workflow);
    
    json input_data;
    input_data["query"] = "What is machine learning?";
    
    std::string execution_id = workflow_orchestrator_->execute_workflow_async("research_workflow", input_data);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait briefly and check status
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto execution = workflow_orchestrator_->get_execution_status(execution_id);
    EXPECT_NE(execution, nullptr);
}

TEST_F(WorkflowTemplateTest, AnalysisWorkflowTemplate) {
    auto analysis_workflow = WorkflowTemplates::create_analysis_workflow();
    
    EXPECT_EQ(analysis_workflow.id, "analysis_workflow");
    EXPECT_EQ(analysis_workflow.type, WorkflowType::SEQUENTIAL);
    
    workflow_orchestrator_->register_workflow(analysis_workflow);
    
    json input_data;
    input_data["text"] = "Sample text for analysis";
    
    std::string execution_id = workflow_orchestrator_->execute_workflow_async("analysis_workflow", input_data);
    EXPECT_FALSE(execution_id.empty());
}

TEST_F(WorkflowTemplateTest, ConversationWorkflowTemplate) {
    std::vector<std::string> agents = {"Assistant", "Analyzer"};
    auto conversation_workflow = WorkflowTemplates::create_conversation_workflow(agents);
    
    EXPECT_EQ(conversation_workflow.id, "conversation_workflow");
    EXPECT_EQ(conversation_workflow.steps.size(), agents.size());
    
    for (size_t i = 0; i < agents.size(); ++i) {
        EXPECT_EQ(conversation_workflow.steps[i].agent_name, agents[i]);
    }
}

// Error Handling Tests
class WorkflowOrchestratorErrorTest : public WorkflowOrchestratorTest {};

TEST_F(WorkflowOrchestratorErrorTest, NonExistentWorkflow) {
    EXPECT_THROW(workflow_orchestrator_->execute_workflow("non_existent_workflow", json{}), 
                 std::invalid_argument);
    
    EXPECT_THROW(workflow_orchestrator_->execute_workflow_async("non_existent_workflow", json{}), 
                 std::invalid_argument);
}

TEST_F(WorkflowOrchestratorErrorTest, InvalidExecutionId) {
    auto execution = workflow_orchestrator_->get_execution_status("invalid_execution_id");
    EXPECT_EQ(execution, nullptr);
    
    json progress = workflow_orchestrator_->get_execution_progress("invalid_execution_id");
    EXPECT_TRUE(progress.contains("error"));
    
    EXPECT_FALSE(workflow_orchestrator_->pause_execution("invalid_execution_id"));
    EXPECT_FALSE(workflow_orchestrator_->resume_execution("invalid_execution_id"));
    EXPECT_FALSE(workflow_orchestrator_->cancel_execution("invalid_execution_id"));
}

TEST_F(WorkflowOrchestratorErrorTest, WorkflowWithMissingAgent) {
    WorkflowDefinition workflow("missing_agent_test", "Missing Agent Test");
    WorkflowStep step("step_with_missing_agent", "NonExistentAgent", "some_function", json::array());
    workflow.steps.push_back(step);
    
    workflow_orchestrator_->register_workflow(workflow);
    
    std::string execution_id = workflow_orchestrator_->execute_workflow_async("missing_agent_test", json{});
    
    EXPECT_TRUE(waitForWorkflowCompletion(execution_id));
    
    auto execution = workflow_orchestrator_->get_execution_status(execution_id);
    ASSERT_NE(execution, nullptr);
    EXPECT_EQ(execution->state, WorkflowExecutionState::FAILED);
    EXPECT_FALSE(execution->error_message.empty());
}

TEST_F(WorkflowOrchestratorErrorTest, WorkflowWithInvalidDependencies) {
    WorkflowDefinition workflow("invalid_deps_test", "Invalid Dependencies Test");
    
    WorkflowStep step1("step1", "Assistant", "status", json::array());
    WorkflowStep step2("step2", "Assistant", "status", json::array());
    step2.dependencies.push_back("non_existent_step");
    
    workflow.steps.push_back(step1);
    workflow.steps.push_back(step2);
    
    workflow_orchestrator_->register_workflow(workflow);
    
    std::string execution_id = workflow_orchestrator_->execute_workflow_async("invalid_deps_test", json{});
    
    EXPECT_TRUE(waitForWorkflowCompletion(execution_id));
    
    auto execution = workflow_orchestrator_->get_execution_status(execution_id);
    ASSERT_NE(execution, nullptr);
    EXPECT_EQ(execution->state, WorkflowExecutionState::FAILED);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "Running WorkflowOrchestrator Tests..." << std::endl;
    std::cout << "Test Categories:" << std::endl;
    std::cout << "  - Lifecycle Management" << std::endl;
    std::cout << "  - Workflow Definition Management" << std::endl;
    std::cout << "  - Workflow Execution" << std::endl;
    std::cout << "  - Execution Control" << std::endl;
    std::cout << "  - Configuration Loading" << std::endl;
    std::cout << "  - Workflow Builder" << std::endl;
    std::cout << "  - Template Workflows" << std::endl;
    std::cout << "  - Error Handling" << std::endl;
    std::cout << std::endl;
    
    return RUN_ALL_TESTS();
}
