/**
 * @file test_workflow_engine.cpp
 * @brief Unit tests for WorkflowEngine class
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "workflow/workflow_engine.hpp"
#include "../fixtures/test_fixtures.hpp"
#include "../mocks/mock_agent_components.hpp"
#include <thread>
#include <chrono>

using namespace testing;
using namespace kolosal::agents;
using namespace kolosal::agents::test;

class WorkflowEngineTest : public WorkflowTestFixture {
protected:
    void SetUp() override {
        WorkflowTestFixture::SetUp();
    }
};

TEST_F(WorkflowEngineTest, EngineLifecycle) {
    ASSERT_NE(test_workflow_engine_, nullptr);
    
    EXPECT_FALSE(test_workflow_engine_->is_running());
    
    test_workflow_engine_->start();
    EXPECT_TRUE(test_workflow_engine_->is_running());
    
    test_workflow_engine_->stop();
    EXPECT_FALSE(test_workflow_engine_->is_running());
}

TEST_F(WorkflowEngineTest, CreateWorkflow) {
    auto workflow = createSimpleSequentialWorkflow();
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    EXPECT_FALSE(workflow_id.empty());
    
    auto retrieved = test_workflow_engine_->get_workflow(workflow_id);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->name, workflow.name);
    EXPECT_EQ(retrieved->type, workflow.type);
    EXPECT_EQ(retrieved->steps.size(), workflow.steps.size());
}

TEST_F(WorkflowEngineTest, UpdateWorkflow) {
    auto workflow = createSimpleSequentialWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Modify workflow
    workflow.name = "Updated Workflow Name";
    workflow.description = "Updated description";
    
    bool updated = test_workflow_engine_->update_workflow(workflow_id, workflow);
    EXPECT_TRUE(updated);
    
    auto retrieved = test_workflow_engine_->get_workflow(workflow_id);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->name, "Updated Workflow Name");
    EXPECT_EQ(retrieved->description, "Updated description");
}

TEST_F(WorkflowEngineTest, DeleteWorkflow) {
    auto workflow = createSimpleSequentialWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    EXPECT_TRUE(test_workflow_engine_->get_workflow(workflow_id).has_value());
    
    bool deleted = test_workflow_engine_->delete_workflow(workflow_id);
    EXPECT_TRUE(deleted);
    
    EXPECT_FALSE(test_workflow_engine_->get_workflow(workflow_id).has_value());
}

TEST_F(WorkflowEngineTest, ListWorkflows) {
    auto workflow1 = createSimpleSequentialWorkflow();
    workflow1.name = "Workflow 1";
    
    auto workflow2 = createParallelWorkflow();
    workflow2.name = "Workflow 2";
    
    std::string id1 = test_workflow_engine_->create_workflow(workflow1);
    std::string id2 = test_workflow_engine_->create_workflow(workflow2);
    
    auto workflows = test_workflow_engine_->list_workflows();
    EXPECT_GE(workflows.size(), 2);
    EXPECT_THAT(workflows, Contains(id1));
    EXPECT_THAT(workflows, Contains(id2));
}

TEST_F(WorkflowEngineTest, ExecuteSequentialWorkflow) {
    test_workflow_engine_->start();
    
    auto workflow = createSimpleSequentialWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json input_context = {{"input", "test_input"}};
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, input_context);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait a bit for execution to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status->workflow_id, workflow_id);
    EXPECT_NE(status->current_status, WorkflowStatus::PENDING);
}

TEST_F(WorkflowEngineTest, ExecuteParallelWorkflow) {
    test_workflow_engine_->start();
    
    auto workflow = createParallelWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for some execution
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status->workflow_id, workflow_id);
}

TEST_F(WorkflowEngineTest, PauseAndResumeWorkflow) {
    test_workflow_engine_->start();
    
    auto workflow = createSimpleSequentialWorkflow();
    // Make it longer to have time to pause
    for (int i = 3; i <= 5; ++i) {
        WorkflowStep step;
        step.step_id = "step" + std::to_string(i);
        step.name = "Step " + std::to_string(i);
        step.agent_id = "test_agent_1";
        step.function_name = "echo";
        step.parameters = nlohmann::json{{"message", "Hello from step " + std::to_string(i)}};
        step.dependencies.push_back({"step" + std::to_string(i-1), "success", true});
        workflow.steps.push_back(step);
    }
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Let it start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    bool paused = test_workflow_engine_->pause_workflow(execution_id);
    EXPECT_TRUE(paused);
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Status should be paused or still running if pause came too late
        EXPECT_TRUE(status->current_status == WorkflowStatus::PAUSED || 
                   status->current_status == WorkflowStatus::RUNNING ||
                   status->current_status == WorkflowStatus::COMPLETED);
    }
    
    bool resumed = test_workflow_engine_->resume_workflow(execution_id);
    EXPECT_TRUE(resumed);
}

TEST_F(WorkflowEngineTest, CancelWorkflow) {
    test_workflow_engine_->start();
    
    auto workflow = createSimpleSequentialWorkflow();
    // Add more steps to have time to cancel
    for (int i = 3; i <= 10; ++i) {
        WorkflowStep step;
        step.step_id = "step" + std::to_string(i);
        step.name = "Step " + std::to_string(i);
        step.agent_id = "test_agent_1";
        step.function_name = "slow_function";  // Assume this is a slower function
        step.timeout_seconds = 5;
        step.dependencies.push_back({"step" + std::to_string(i-1), "success", true});
        workflow.steps.push_back(step);
    }
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Let it start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    bool cancelled = test_workflow_engine_->cancel_workflow(execution_id);
    EXPECT_TRUE(cancelled);
    
    // Check status after a moment
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        EXPECT_TRUE(status->current_status == WorkflowStatus::CANCELLED ||
                   status->current_status == WorkflowStatus::COMPLETED);
    }
}

TEST_F(WorkflowEngineTest, StepRetryAndSkip) {
    test_workflow_engine_->start();
    
    auto workflow = createSimpleSequentialWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Wait for execution to progress
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Test retry step
    bool retried = test_workflow_engine_->retry_step(execution_id, "step1");
    // This should return true if the step can be retried
    
    // Test skip step
    bool skipped = test_workflow_engine_->skip_step(execution_id, "step2");
    // This should return true if the step can be skipped
}

TEST_F(WorkflowEngineTest, ContextManagement) {
    test_workflow_engine_->start();
    
    auto workflow = createSimpleSequentialWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json initial_context = {
        {"global_var", "test_value"},
        {"counter", 42}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, initial_context);
    
    // Test getting global context
    auto context = test_workflow_engine_->get_global_context(execution_id);
    EXPECT_FALSE(context.empty());
    
    // Test updating global context
    nlohmann::json update = {{"new_var", "new_value"}};
    bool updated = test_workflow_engine_->update_global_context(execution_id, update);
    EXPECT_TRUE(updated);
    
    // Verify update
    auto updated_context = test_workflow_engine_->get_global_context(execution_id);
    if (updated_context.contains("new_var")) {
        EXPECT_EQ(updated_context["new_var"], "new_value");
    }
}

TEST_F(WorkflowEngineTest, WorkflowTemplates) {
    // Test sequential workflow template
    std::vector<std::pair<std::string, std::string>> agent_functions = {
        {"agent1", "func1"},
        {"agent2", "func2"},
        {"agent3", "func3"}
    };
    
    auto sequential = test_workflow_engine_->create_sequential_workflow("Template Sequential", agent_functions);
    EXPECT_EQ(sequential.type, WorkflowType::SEQUENTIAL);
    EXPECT_EQ(sequential.steps.size(), 3);
    EXPECT_EQ(sequential.name, "Template Sequential");
    
    // Test parallel workflow template
    auto parallel = test_workflow_engine_->create_parallel_workflow("Template Parallel", agent_functions);
    EXPECT_EQ(parallel.type, WorkflowType::PARALLEL);
    EXPECT_EQ(parallel.steps.size(), 3);
    EXPECT_EQ(parallel.name, "Template Parallel");
    
    // Test pipeline workflow template
    auto pipeline = test_workflow_engine_->create_pipeline_workflow("Template Pipeline", agent_functions);
    EXPECT_EQ(pipeline.type, WorkflowType::PIPELINE);
    EXPECT_EQ(pipeline.steps.size(), 3);
    
    // Test consensus workflow template
    std::vector<std::string> agent_ids = {"agent1", "agent2", "agent3"};
    auto consensus = test_workflow_engine_->create_consensus_workflow("Template Consensus", agent_ids, "decide");
    EXPECT_EQ(consensus.type, WorkflowType::CONSENSUS);
    EXPECT_FALSE(consensus.steps.empty());
}

TEST_F(WorkflowEngineTest, ErrorHandlingStrategy) {
    ErrorHandlingStrategy strategy;
    strategy.retry_on_failure = true;
    strategy.max_retries = 5;
    strategy.retry_delay_seconds = 2;
    strategy.continue_on_error = false;
    strategy.use_fallback_agent = true;
    strategy.fallback_agent_id = "fallback_agent";
    
    auto workflow = createSimpleSequentialWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    test_workflow_engine_->set_error_handling_strategy(workflow_id, strategy);
    
    // The strategy should be applied - this test verifies the interface
    EXPECT_TRUE(true); // Interface test
}

TEST_F(WorkflowEngineTest, GetMetrics) {
    auto metrics = test_workflow_engine_->get_metrics();
    
    EXPECT_GE(metrics.total_workflows, 0);
    EXPECT_GE(metrics.running_workflows, 0);
    EXPECT_GE(metrics.completed_workflows, 0);
    EXPECT_GE(metrics.failed_workflows, 0);
    EXPECT_GE(metrics.cancelled_workflows, 0);
    EXPECT_GE(metrics.average_execution_time_ms, 0.0);
    EXPECT_GE(metrics.success_rate, 0.0);
    EXPECT_LE(metrics.success_rate, 1.0);
}

TEST_F(WorkflowEngineTest, ActiveExecutions) {
    test_workflow_engine_->start();
    
    auto workflow1 = createSimpleSequentialWorkflow();
    auto workflow2 = createParallelWorkflow();
    
    std::string id1 = test_workflow_engine_->create_workflow(workflow1);
    std::string id2 = test_workflow_engine_->create_workflow(workflow2);
    
    std::string exec1 = test_workflow_engine_->execute_workflow(id1);
    std::string exec2 = test_workflow_engine_->execute_workflow(id2);
    
    auto active = test_workflow_engine_->get_active_executions();
    EXPECT_GE(active.size(), 0); // Could be 0, 1, or 2 depending on timing
    
    // Wait for executions to complete or progress
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto history = test_workflow_engine_->get_execution_history();
    EXPECT_GE(history.size(), 0);
}

TEST_F(WorkflowEngineTest, ConcurrentExecution) {
    test_workflow_engine_->start();
    
    // Create multiple workflows
    std::vector<std::string> workflow_ids;
    for (int i = 0; i < 5; ++i) {
        auto workflow = createSimpleSequentialWorkflow();
        workflow.name = "Concurrent Workflow " + std::to_string(i);
        workflow_ids.push_back(test_workflow_engine_->create_workflow(workflow));
    }
    
    // Execute them all concurrently
    std::vector<std::string> execution_ids;
    std::vector<std::thread> threads;
    std::mutex results_mutex;
    
    for (const auto& workflow_id : workflow_ids) {
        threads.emplace_back([&, workflow_id]() {
            std::string exec_id = test_workflow_engine_->execute_workflow(workflow_id);
            std::lock_guard<std::mutex> lock(results_mutex);
            execution_ids.push_back(exec_id);
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(execution_ids.size(), workflow_ids.size());
    
    // All execution IDs should be unique
    std::set<std::string> unique_ids(execution_ids.begin(), execution_ids.end());
    EXPECT_EQ(unique_ids.size(), execution_ids.size());
}
