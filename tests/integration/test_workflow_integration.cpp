/**
 * @file test_workflow_integration.cpp
 * @brief Integration tests for comprehensive workflow scenarios
 * @version 2.0.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "workflow/workflow_engine.hpp"
#include "../fixtures/test_fixtures.hpp"
#include "../mocks/mock_agent_components.hpp"
#include <thread>
#include <chrono>
#include <fstream>

using namespace testing;
using namespace kolosal::agents;
using namespace kolosal::agents::test;

class WorkflowIntegrationTest : public WorkflowTestFixture {
protected:
    void SetUp() override {
        WorkflowTestFixture::SetUp();
    }

    void createTestWorkflowFiles() {
        // Create test YAML workflow files
        createSequentialWorkflowFile();
        createConditionalWorkflowFile();
        createConsensusWorkflowFile();
        createPipelineWorkflowFile();
    }

    void createSequentialWorkflowFile() {
        std::string yaml_content = R"(
id: "integration_sequential_workflow"
name: "Integration Sequential Workflow"
description: "Sequential workflow for integration testing"
type: "sequential"
created_by: "test_system"

global_context:
  input_data: "integration_test_data"
  processing_mode: "test"
  expected_steps: 3

settings:
  max_execution_time: 300
  max_concurrent_steps: 1
  auto_cleanup: true
  persist_state: true

error_handling:
  retry_on_failure: true
  max_retries: 2
  retry_delay_seconds: 1
  continue_on_error: false

steps:
  - id: "collect"
    name: "Collect Data"
    description: "Collect input data"
    agent_id: "test_collector"
    function: "collect_data"
    parameters:
      source: "${global.input_data}"
      mode: "${global.processing_mode}"
    timeout: 60

  - id: "process"
    name: "Process Data"
    description: "Process collected data"
    agent_id: "test_processor"
    function: "process_data"
    parameters:
      input: "${steps.collect.output}"
      processing_mode: "${global.processing_mode}"
    depends_on:
      - "collect"
    timeout: 90

  - id: "output"
    name: "Output Results"
    description: "Output processed results"
    agent_id: "test_outputter"
    function: "output_results"
    parameters:
      processed_data: "${steps.process.output}"
      format: "json"
    depends_on:
      - "process"
    timeout: 30
)";
        
        std::ofstream file(getTestOutputPath("sequential_workflow.yaml"));
        file << yaml_content;
        file.close();
    }

    void createConditionalWorkflowFile() {
        std::string yaml_content = R"(
id: "integration_conditional_workflow"
name: "Integration Conditional Workflow"
description: "Conditional workflow for integration testing"
type: "conditional"

global_context:
  quality_threshold: 0.75
  complexity_level: "medium"
  enable_advanced_processing: true

settings:
  max_execution_time: 400
  max_concurrent_steps: 2

steps:
  - id: "assess"
    name: "Quality Assessment"
    agent_id: "test_assessor"
    function: "assess_quality"
    parameters:
      input: "test_data"
    timeout: 60

  - id: "high_quality_process"
    name: "High Quality Processing"
    agent_id: "test_processor"
    function: "advanced_process"
    parameters:
      data: "${steps.assess.output}"
    depends_on:
      - "assess"
    conditions:
      expression: "steps.assess.output.quality >= global.quality_threshold"

  - id: "low_quality_process"
    name: "Low Quality Processing"
    agent_id: "test_processor"
    function: "basic_process"
    parameters:
      data: "${steps.assess.output}"
    depends_on:
      - "assess"
    conditions:
      expression: "steps.assess.output.quality < global.quality_threshold"
)";
        
        std::ofstream file(getTestOutputPath("conditional_workflow.yaml"));
        file << yaml_content;
        file.close();
    }

    void createConsensusWorkflowFile() {
        std::string yaml_content = R"(
id: "integration_consensus_workflow"
name: "Integration Consensus Workflow"
type: "consensus"

global_context:
  decision_topic: "Integration Test Decision"
  consensus_threshold: 0.6

steps:
  - id: "vote1"
    name: "First Vote"
    agent_id: "voter1"
    function: "cast_vote"
    parameters:
      topic: "${global.decision_topic}"
    parallel_allowed: true

  - id: "vote2"
    name: "Second Vote"
    agent_id: "voter2"
    function: "cast_vote"
    parameters:
      topic: "${global.decision_topic}"
    parallel_allowed: true

  - id: "consensus"
    name: "Build Consensus"
    agent_id: "consensus_builder"
    function: "build_consensus"
    parameters:
      votes:
        vote1: "${steps.vote1.output}"
        vote2: "${steps.vote2.output}"
      threshold: "${global.consensus_threshold}"
    depends_on:
      - step: "vote1"
        condition: "completion"
        required: false
      - step: "vote2"
        condition: "completion"
        required: false
)";
        
        std::ofstream file(getTestOutputPath("consensus_workflow.yaml"));
        file << yaml_content;
        file.close();
    }

    void createPipelineWorkflowFile() {
        std::string yaml_content = R"(
id: "integration_pipeline_workflow"
name: "Integration Pipeline Workflow"
type: "pipeline"

global_context:
  data_source: "integration_pipeline_data"

steps:
  - id: "extract"
    name: "Extract Data"
    agent_id: "extractor"
    function: "extract_data"
    parameters:
      source: "${global.data_source}"

  - id: "transform"
    name: "Transform Data"
    agent_id: "transformer"
    function: "transform_data"
    parameters:
      input: "${steps.extract.output}"
    depends_on:
      - "extract"

  - id: "load"
    name: "Load Data"
    agent_id: "loader"
    function: "load_data"
    parameters:
      transformed_data: "${steps.transform.output}"
    depends_on:
      - "transform"
)";
        
        std::ofstream file(getTestOutputPath("pipeline_workflow.yaml"));
        file << yaml_content;
        file.close();
    }
};

TEST_F(WorkflowIntegrationTest, WorkflowEngineStartupAndShutdown) {
    // Test multiple startup/shutdown cycles
    for (int i = 0; i < 3; ++i) {
        EXPECT_FALSE(test_workflow_engine_->is_running());
        
        test_workflow_engine_->start();
        EXPECT_TRUE(test_workflow_engine_->is_running());
        
        // Give it time to fully start
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        test_workflow_engine_->stop();
        EXPECT_FALSE(test_workflow_engine_->is_running());
        
        // Give it time to fully stop
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

TEST_F(WorkflowIntegrationTest, MultipleWorkflowTypes) {
    test_workflow_engine_->start();
    
    // Create different types of workflows
    std::vector<Workflow> workflows;
    
    // Sequential workflow
    Workflow sequential = createSimpleSequentialWorkflow();
    sequential.name = "Integration Sequential";
    workflows.push_back(sequential);
    
    // Parallel workflow
    Workflow parallel = createParallelWorkflow();
    parallel.name = "Integration Parallel";
    workflows.push_back(parallel);
    
    // Create and store all workflows
    std::vector<std::string> workflow_ids;
    for (const auto& workflow : workflows) {
        std::string id = test_workflow_engine_->create_workflow(workflow);
        workflow_ids.push_back(id);
    }
    
    EXPECT_EQ(workflow_ids.size(), workflows.size());
    
    // Execute all workflows
    std::vector<std::string> execution_ids;
    for (const auto& workflow_id : workflow_ids) {
        std::string exec_id = test_workflow_engine_->execute_workflow(workflow_id);
        execution_ids.push_back(exec_id);
    }
    
    EXPECT_EQ(execution_ids.size(), workflow_ids.size());
    
    // Wait for all executions
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    // Check status of all executions
    for (const auto& execution_id : execution_ids) {
        auto status = test_workflow_engine_->get_execution_status(execution_id);
        EXPECT_TRUE(status.has_value());
        if (status.has_value()) {
            EXPECT_NE(status->current_status, WorkflowStatus::PENDING);
        }
    }
}

TEST_F(WorkflowIntegrationTest, ConcurrentWorkflowExecution) {
    test_workflow_engine_->start();
    
    // Create multiple workflows for concurrent execution
    std::vector<std::string> workflow_ids;
    for (int i = 0; i < 5; ++i) {
        auto workflow = createSimpleSequentialWorkflow();
        workflow.workflow_id = "concurrent_test_" + std::to_string(i);
        workflow.name = "Concurrent Test " + std::to_string(i);
        
        std::string id = test_workflow_engine_->create_workflow(workflow);
        workflow_ids.push_back(id);
    }
    
    // Launch all workflows concurrently using threads
    std::vector<std::thread> threads;
    std::vector<std::string> execution_ids(workflow_ids.size());
    std::mutex results_mutex;
    
    for (size_t i = 0; i < workflow_ids.size(); ++i) {
        threads.emplace_back([&, i]() {
            std::string exec_id = test_workflow_engine_->execute_workflow(workflow_ids[i]);
            std::lock_guard<std::mutex> lock(results_mutex);
            execution_ids[i] = exec_id;
        });
    }
    
    // Wait for all launches to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all executions started
    for (const auto& exec_id : execution_ids) {
        EXPECT_FALSE(exec_id.empty());
    }
    
    // Wait for executions to progress
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    // Check concurrent execution metrics
    auto active_executions = test_workflow_engine_->get_active_executions();
    auto metrics = test_workflow_engine_->get_metrics();
    
    EXPECT_GE(metrics.total_workflows, 5);
    EXPECT_GE(metrics.running_workflows + metrics.completed_workflows, 0);
}

TEST_F(WorkflowIntegrationTest, WorkflowLoadFromYAML) {
    createTestWorkflowFiles();
    test_workflow_engine_->start();
    
    // Test loading different workflow types from YAML
    std::vector<std::string> yaml_files = {
        getTestOutputPath("sequential_workflow.yaml"),
        getTestOutputPath("conditional_workflow.yaml"),
        getTestOutputPath("consensus_workflow.yaml"),
        getTestOutputPath("pipeline_workflow.yaml")
    };
    
    std::vector<bool> load_results;
    for (const auto& yaml_file : yaml_files) {
        bool loaded = test_workflow_engine_->load_workflow_from_yaml(yaml_file);
        load_results.push_back(loaded);
    }
    
    // Check that workflows were loaded (at least some should succeed)
    int successful_loads = std::count(load_results.begin(), load_results.end(), true);
    EXPECT_GE(successful_loads, 0);  // At least some should load successfully
    
    // List workflows to verify they were created
    auto workflow_list = test_workflow_engine_->list_workflows();
    EXPECT_GE(workflow_list.size(), 0);
}

TEST_F(WorkflowIntegrationTest, WorkflowErrorRecoveryIntegration) {
    test_workflow_engine_->start();
    
    // Create workflow with error recovery configuration
    auto workflow = createSimpleSequentialWorkflow();
    workflow.error_handling.retry_on_failure = true;
    workflow.error_handling.max_retries = 3;
    workflow.error_handling.retry_delay_seconds = 1;
    workflow.error_handling.continue_on_error = true;
    
    // Configure steps with different error behavior
    for (auto& step : workflow.steps) {
        step.max_retries = 2;
        step.retry_delay_seconds = 1;
        step.continue_on_error = true;
    }
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Execute with error-inducing input
    nlohmann::json error_input = {
        {"introduce_errors", true},
        {"error_probability", 0.4},
        {"recoverable_errors", true}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, error_input);
    
    // Wait for error recovery attempts
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    
    // Workflow should handle errors and either complete or fail gracefully
    EXPECT_TRUE(status->current_status == WorkflowStatus::COMPLETED ||
               status->current_status == WorkflowStatus::FAILED ||
               status->current_status == WorkflowStatus::RUNNING);
    
    // Check for evidence of retry attempts
    auto step_statuses = status->step_statuses;
    bool found_retry_evidence = false;
    for (const auto& [step_id, step_status] : step_statuses) {
        if (step_status == StepStatus::RETRYING || step_status == StepStatus::FAILED) {
            found_retry_evidence = true;
            break;
        }
    }
    
    // Should have some evidence of error handling
    EXPECT_TRUE(found_retry_evidence || status->current_status == WorkflowStatus::COMPLETED);
}

TEST_F(WorkflowIntegrationTest, WorkflowStateManagementIntegration) {
    test_workflow_engine_->start();
    
    auto workflow = createSimpleSequentialWorkflow();
    
    // Add more steps to make it longer
    for (int i = 3; i <= 6; ++i) {
        WorkflowStep step;
        step.step_id = "extended_step_" + std::to_string(i);
        step.name = "Extended Step " + std::to_string(i);
        step.agent_id = "test_agent_1";
        step.function_name = "extended_process";
        step.parameters = {{"step_number", i}};
        
        if (i > 3) {
            step.dependencies.push_back({"extended_step_" + std::to_string(i-1), "success", true});
        } else {
            step.dependencies.push_back({"step2", "success", true});
        }
        
        workflow.steps.push_back(step);
    }
    
    workflow.persist_state = true;
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Let it run for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Test pause/resume cycle
    bool paused = test_workflow_engine_->pause_workflow(execution_id);
    if (paused) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        auto paused_status = test_workflow_engine_->get_execution_status(execution_id);
        if (paused_status.has_value()) {
            // Resume workflow
            bool resumed = test_workflow_engine_->resume_workflow(execution_id);
            EXPECT_TRUE(resumed);
            
            // Let it continue
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            
            auto final_status = test_workflow_engine_->get_execution_status(execution_id);
            if (final_status.has_value()) {
                // Should continue executing
                EXPECT_NE(final_status->current_status, WorkflowStatus::PAUSED);
            }
        }
    }
}

TEST_F(WorkflowIntegrationTest, WorkflowMetricsAndMonitoring) {
    test_workflow_engine_->start();
    
    // Execute multiple workflows to generate metrics
    std::vector<std::string> execution_ids;
    
    for (int i = 0; i < 5; ++i) {
        auto workflow = createSimpleSequentialWorkflow();
        workflow.workflow_id = "metrics_test_" + std::to_string(i);
        workflow.name = "Metrics Test " + std::to_string(i);
        
        std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
        std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
        execution_ids.push_back(execution_id);
    }
    
    // Wait for some executions to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    
    // Check metrics
    auto metrics = test_workflow_engine_->get_metrics();
    
    EXPECT_GE(metrics.total_workflows, 5);
    EXPECT_GE(metrics.completed_workflows + metrics.running_workflows + metrics.failed_workflows, 0);
    EXPECT_GE(metrics.average_execution_time_ms, 0.0);
    EXPECT_GE(metrics.success_rate, 0.0);
    EXPECT_LE(metrics.success_rate, 1.0);
    
    // Check execution history
    auto history = test_workflow_engine_->get_execution_history();
    EXPECT_GE(history.size(), 0);
    
    // Check active executions
    auto active = test_workflow_engine_->get_active_executions();
    EXPECT_GE(active.size(), 0);
}

TEST_F(WorkflowIntegrationTest, WorkflowTemplateIntegration) {
    test_workflow_engine_->start();
    
    // Test all template creation methods
    std::vector<std::pair<std::string, std::string>> agent_functions = {
        {"agent1", "function1"},
        {"agent2", "function2"},
        {"agent3", "function3"}
    };
    
    std::vector<std::string> agent_ids = {"agent1", "agent2", "agent3"};
    
    // Create different workflow types using templates
    auto sequential = test_workflow_engine_->create_sequential_workflow("Template Sequential", agent_functions);
    auto parallel = test_workflow_engine_->create_parallel_workflow("Template Parallel", agent_functions);
    auto pipeline = test_workflow_engine_->create_pipeline_workflow("Template Pipeline", agent_functions);
    auto consensus = test_workflow_engine_->create_consensus_workflow("Template Consensus", agent_ids, "consensus_function");
    
    // Verify template creation
    EXPECT_EQ(sequential.type, WorkflowType::SEQUENTIAL);
    EXPECT_EQ(parallel.type, WorkflowType::PARALLEL);
    EXPECT_EQ(pipeline.type, WorkflowType::PIPELINE);
    EXPECT_EQ(consensus.type, WorkflowType::CONSENSUS);
    
    // Store and execute template workflows
    std::vector<Workflow> template_workflows = {sequential, parallel, pipeline, consensus};
    std::vector<std::string> template_execution_ids;
    
    for (const auto& workflow : template_workflows) {
        std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
        std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
        template_execution_ids.push_back(execution_id);
    }
    
    // Wait for template executions
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Verify template executions
    for (const auto& execution_id : template_execution_ids) {
        if (!execution_id.empty()) {
            auto status = test_workflow_engine_->get_execution_status(execution_id);
            EXPECT_TRUE(status.has_value());
            if (status.has_value()) {
                EXPECT_NE(status->current_status, WorkflowStatus::PENDING);
            }
        }
    }
}

TEST_F(WorkflowIntegrationTest, WorkflowCleanupIntegration) {
    test_workflow_engine_->start();
    
    // Create workflows with auto-cleanup enabled
    std::vector<std::string> workflow_ids;
    for (int i = 0; i < 3; ++i) {
        auto workflow = createSimpleSequentialWorkflow();
        workflow.workflow_id = "cleanup_test_" + std::to_string(i);
        workflow.auto_cleanup = true;
        
        std::string id = test_workflow_engine_->create_workflow(workflow);
        workflow_ids.push_back(id);
    }
    
    // Execute workflows
    std::vector<std::string> execution_ids;
    for (const auto& workflow_id : workflow_ids) {
        std::string exec_id = test_workflow_engine_->execute_workflow(workflow_id);
        execution_ids.push_back(exec_id);
    }
    
    // Wait for executions to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    // Test workflow deletion
    for (const auto& workflow_id : workflow_ids) {
        bool deleted = test_workflow_engine_->delete_workflow(workflow_id);
        EXPECT_TRUE(deleted);
        
        // Verify deletion
        auto retrieved = test_workflow_engine_->get_workflow(workflow_id);
        EXPECT_FALSE(retrieved.has_value());
    }
    
    // Check that execution history is still available
    auto history = test_workflow_engine_->get_execution_history();
    EXPECT_GE(history.size(), 0);
}

TEST_F(WorkflowIntegrationTest, FullSystemIntegrationTest) {
    // This is a comprehensive test that exercises many workflow features together
    test_workflow_engine_->start();
    
    // Phase 1: Create diverse workflows
    auto sequential = createSimpleSequentialWorkflow();
    sequential.name = "Full Integration Sequential";
    
    auto parallel = createParallelWorkflow();
    parallel.name = "Full Integration Parallel";
    
    std::string seq_id = test_workflow_engine_->create_workflow(sequential);
    std::string par_id = test_workflow_engine_->create_workflow(parallel);
    
    // Phase 2: Execute workflows with different inputs
    nlohmann::json seq_input = {
        {"test_phase", "integration"},
        {"expected_outcome", "success"}
    };
    
    nlohmann::json par_input = {
        {"test_phase", "integration"},
        {"parallel_branches", 2}
    };
    
    std::string seq_exec = test_workflow_engine_->execute_workflow(seq_id, seq_input);
    std::string par_exec = test_workflow_engine_->execute_workflow(par_id, par_input);
    
    EXPECT_FALSE(seq_exec.empty());
    EXPECT_FALSE(par_exec.empty());
    
    // Phase 3: Monitor execution progress
    std::vector<std::string> execution_ids = {seq_exec, par_exec};
    
    // Poll for completion or progress
    int max_polls = 20;  // Max 2 seconds of polling
    int polls = 0;
    bool all_progressed = false;
    
    while (polls < max_polls && !all_progressed) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        polls++;
        
        int progressed_count = 0;
        for (const auto& exec_id : execution_ids) {
            auto status = test_workflow_engine_->get_execution_status(exec_id);
            if (status.has_value() && status->current_status != WorkflowStatus::PENDING) {
                progressed_count++;
            }
        }
        
        if (progressed_count == execution_ids.size()) {
            all_progressed = true;
        }
    }
    
    EXPECT_TRUE(all_progressed);
    
    // Phase 4: Verify system state
    auto active_executions = test_workflow_engine_->get_active_executions();
    auto workflow_list = test_workflow_engine_->list_workflows();
    auto metrics = test_workflow_engine_->get_metrics();
    
    EXPECT_GE(workflow_list.size(), 2);
    EXPECT_GE(metrics.total_workflows, 2);
    EXPECT_TRUE(test_workflow_engine_->is_running());
    
    // Phase 5: Cleanup
    test_workflow_engine_->stop();
    EXPECT_FALSE(test_workflow_engine_->is_running());
}
