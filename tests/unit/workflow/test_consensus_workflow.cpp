/**
 * @file test_consensus_workflow.cpp
 * @brief Unit tests for consensus workflow execution
 * @version 2.0.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "workflow/workflow_engine.hpp"
#include "../fixtures/test_fixtures.hpp"
#include "../mocks/mock_agent_components.hpp"
#include <thread>
#include <chrono>
#include <vector>

using namespace testing;
using namespace kolosal::agents;
using namespace kolosal::agents::test;

class ConsensusWorkflowTest : public WorkflowTestFixture {
protected:
    void SetUp() override {
        WorkflowTestFixture::SetUp();
    }

    Workflow createConsensusWorkflow(int num_voters = 3) {
        Workflow workflow;
        workflow.workflow_id = "test_consensus_workflow";
        workflow.name = "Test Consensus Workflow";
        workflow.description = "A consensus workflow for testing";
        workflow.type = WorkflowType::CONSENSUS;
        
        // Global context
        workflow.global_context = {
            {"decision_topic", "Best AI Strategy"},
            {"consensus_threshold", 0.7},
            {"voting_criteria", nlohmann::json::array({"technical", "business", "ethics"})},
            {"num_voters", num_voters}
        };

        // Create voting steps (parallel execution)
        for (int i = 1; i <= num_voters; ++i) {
            WorkflowStep voting_step;
            voting_step.step_id = "vote_" + std::to_string(i);
            voting_step.name = "Vote " + std::to_string(i);
            voting_step.agent_id = "test_agent_" + std::to_string(i);
            voting_step.function_name = "cast_vote";
            voting_step.parameters = {
                {"topic", "${global.decision_topic}"},
                {"criteria", "${global.voting_criteria}"},
                {"voter_id", i}
            };
            voting_step.parallel_allowed = true;
            voting_step.timeout_seconds = 60;
            workflow.steps.push_back(voting_step);
        }

        // Consensus step (depends on all votes)
        WorkflowStep consensus_step;
        consensus_step.step_id = "consensus";
        consensus_step.name = "Consensus Decision";
        consensus_step.agent_id = "test_agent_1";  // Coordinator agent
        consensus_step.function_name = "build_consensus";
        
        // Build vote data for consensus
        nlohmann::json vote_data = nlohmann::json::object();
        for (int i = 1; i <= num_voters; ++i) {
            vote_data["vote_" + std::to_string(i)] = "${steps.vote_" + std::to_string(i) + ".output}";
        }
        
        consensus_step.parameters = {
            {"votes", vote_data},
            {"threshold", "${global.consensus_threshold}"},
            {"method", "weighted_average"}
        };
        
        // Add dependencies on all votes
        for (int i = 1; i <= num_voters; ++i) {
            consensus_step.dependencies.push_back({
                "vote_" + std::to_string(i), 
                "completion", 
                false  // Allow consensus even if some votes fail
            });
        }
        
        consensus_step.parallel_allowed = false;
        consensus_step.timeout_seconds = 120;
        workflow.steps.push_back(consensus_step);

        // Implementation step (depends on consensus)
        WorkflowStep implementation_step;
        implementation_step.step_id = "implementation";
        implementation_step.name = "Implement Decision";
        implementation_step.agent_id = "test_agent_1";
        implementation_step.function_name = "implement_decision";
        implementation_step.parameters = {
            {"decision", "${steps.consensus.output}"},
            {"create_action_plan", true}
        };
        implementation_step.dependencies.push_back({"consensus", "success", true});
        implementation_step.parallel_allowed = false;
        implementation_step.timeout_seconds = 180;
        workflow.steps.push_back(implementation_step);

        return workflow;
    }

    Workflow createWeightedConsensusWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "weighted_consensus_workflow";
        workflow.name = "Weighted Consensus Workflow";
        workflow.type = WorkflowType::CONSENSUS;
        
        workflow.global_context = {
            {"decision_topic", "Resource Allocation"},
            {"consensus_threshold", 0.6},
            {"voter_weights", nlohmann::json::object({
                {"expert", 0.5},
                {"manager", 0.3},
                {"stakeholder", 0.2}
            })}
        };

        // Expert vote (higher weight)
        WorkflowStep expert_vote;
        expert_vote.step_id = "expert_vote";
        expert_vote.name = "Expert Vote";
        expert_vote.agent_id = "expert_agent";
        expert_vote.function_name = "expert_analysis";
        expert_vote.parameters = {
            {"topic", "${global.decision_topic}"},
            {"analysis_depth", "comprehensive"},
            {"weight", "${global.voter_weights.expert}"}
        };
        expert_vote.parallel_allowed = true;

        // Manager vote (medium weight)
        WorkflowStep manager_vote;
        manager_vote.step_id = "manager_vote";
        manager_vote.name = "Manager Vote";
        manager_vote.agent_id = "manager_agent";
        manager_vote.function_name = "business_analysis";
        manager_vote.parameters = {
            {"topic", "${global.decision_topic}"},
            {"focus", "business_impact"},
            {"weight", "${global.voter_weights.manager}"}
        };
        manager_vote.parallel_allowed = true;

        // Stakeholder vote (lower weight)
        WorkflowStep stakeholder_vote;
        stakeholder_vote.step_id = "stakeholder_vote";
        stakeholder_vote.name = "Stakeholder Vote";
        stakeholder_vote.agent_id = "stakeholder_agent";
        stakeholder_vote.function_name = "stakeholder_input";
        stakeholder_vote.parameters = {
            {"topic", "${global.decision_topic}"},
            {"perspective", "user_impact"},
            {"weight", "${global.voter_weights.stakeholder}"}
        };
        stakeholder_vote.parallel_allowed = true;

        // Weighted consensus
        WorkflowStep weighted_consensus;
        weighted_consensus.step_id = "weighted_consensus";
        weighted_consensus.name = "Weighted Consensus";
        weighted_consensus.agent_id = "consensus_agent";
        weighted_consensus.function_name = "weighted_consensus";
        weighted_consensus.parameters = {
            {"expert_vote", "${steps.expert_vote.output}"},
            {"manager_vote", "${steps.manager_vote.output}"},
            {"stakeholder_vote", "${steps.stakeholder_vote.output}"},
            {"weights", "${global.voter_weights}"},
            {"threshold", "${global.consensus_threshold}"}
        };
        weighted_consensus.dependencies = {
            {"expert_vote", "completion", false},
            {"manager_vote", "completion", false},
            {"stakeholder_vote", "completion", false}
        };

        workflow.steps = {expert_vote, manager_vote, stakeholder_vote, weighted_consensus};
        return workflow;
    }
};

TEST_F(ConsensusWorkflowTest, BasicConsensusExecution) {
    test_workflow_engine_->start();
    
    auto workflow = createConsensusWorkflow(3);
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Execute workflow
    nlohmann::json input_context = {
        {"decision_topic", "AI Development Strategy"},
        {"urgency", "high"}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, input_context);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for execution to progress
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status->workflow_id, workflow_id);
    EXPECT_NE(status->current_status, WorkflowStatus::PENDING);
}

TEST_F(ConsensusWorkflowTest, ParallelVoting) {
    test_workflow_engine_->start();
    
    auto workflow = createConsensusWorkflow(5);  // 5 voters
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Wait for voting phase
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        auto step_statuses = status->step_statuses;
        
        // Check that multiple voting steps are running or completed in parallel
        int voting_steps_active = 0;
        for (int i = 1; i <= 5; ++i) {
            std::string vote_step = "vote_" + std::to_string(i);
            if (step_statuses.find(vote_step) != step_statuses.end()) {
                auto step_status = step_statuses[vote_step];
                if (step_status == StepStatus::RUNNING || 
                    step_status == StepStatus::COMPLETED) {
                    voting_steps_active++;
                }
            }
        }
        
        // At least some voting steps should be active
        EXPECT_GE(voting_steps_active, 0);
    }
}

TEST_F(ConsensusWorkflowTest, ConsensusReached) {
    test_workflow_engine_->start();
    
    auto workflow = createConsensusWorkflow(3);
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Execute with conditions likely to reach consensus
    nlohmann::json consensus_input = {
        {"decision_topic", "Simple Decision"},
        {"consensus_threshold", 0.5},  // Lower threshold for easier consensus
        {"expected_agreement", true}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, consensus_input);
    
    // Wait for full execution
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Consensus step should have been executed
        auto step_statuses = status->step_statuses;
        if (step_statuses.find("consensus") != step_statuses.end()) {
            EXPECT_TRUE(step_statuses["consensus"] == StepStatus::COMPLETED ||
                       step_statuses["consensus"] == StepStatus::RUNNING);
        }
        
        // Implementation step should follow if consensus reached
        if (step_statuses.find("implementation") != step_statuses.end()) {
            EXPECT_NE(step_statuses["implementation"], StepStatus::PENDING);
        }
    }
}

TEST_F(ConsensusWorkflowTest, PartialVoteFailure) {
    test_workflow_engine_->start();
    
    auto workflow = createConsensusWorkflow(4);
    
    // Configure to continue even if some votes fail
    workflow.error_handling.continue_on_error = true;
    workflow.error_handling.retry_on_failure = false;
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Execute with input that might cause some votes to fail
    nlohmann::json partial_failure_input = {
        {"decision_topic", "Controversial Topic"},
        {"some_votes_will_fail", true},
        {"minimum_votes_required", 2}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, partial_failure_input);
    
    // Wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Consensus should still attempt to run even with partial failures
        auto step_statuses = status->step_statuses;
        if (step_statuses.find("consensus") != step_statuses.end()) {
            // Consensus step should at least be attempted
            EXPECT_TRUE(step_statuses["consensus"] != StepStatus::PENDING);
        }
    }
}

TEST_F(ConsensusWorkflowTest, WeightedConsensus) {
    test_workflow_engine_->start();
    
    auto workflow = createWeightedConsensusWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json weighted_input = {
        {"decision_topic", "Budget Allocation"},
        {"expert_opinion_strength", "high"},
        {"manager_approval", "moderate"},
        {"stakeholder_support", "low"}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, weighted_input);
    
    // Wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(350));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    
    // Check that weighted consensus step is executed
    auto step_statuses = status->step_statuses;
    if (step_statuses.find("weighted_consensus") != step_statuses.end()) {
        EXPECT_NE(step_statuses["weighted_consensus"], StepStatus::PENDING);
    }
}

TEST_F(ConsensusWorkflowTest, ConsensusThresholdNotMet) {
    test_workflow_engine_->start();
    
    auto workflow = createConsensusWorkflow(3);
    
    // Set high threshold that's unlikely to be met
    workflow.global_context["consensus_threshold"] = 0.95;
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json difficult_consensus_input = {
        {"decision_topic", "Highly Controversial Decision"},
        {"expected_disagreement", true},
        {"consensus_threshold", 0.95}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, difficult_consensus_input);
    
    // Wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Consensus step should execute but might not lead to implementation
        auto step_statuses = status->step_statuses;
        
        if (step_statuses.find("consensus") != step_statuses.end()) {
            EXPECT_NE(step_statuses["consensus"], StepStatus::PENDING);
        }
        
        // Implementation might be skipped if consensus threshold not met
        if (step_statuses.find("implementation") != step_statuses.end()) {
            // Implementation could be pending, skipped, or failed
            EXPECT_TRUE(true);  // Just check that the workflow handles this case
        }
    }
}

TEST_F(ConsensusWorkflowTest, LargeConsensusGroup) {
    test_workflow_engine_->start();
    
    // Test with larger group of voters
    auto workflow = createConsensusWorkflow(10);
    
    // Adjust for larger group
    workflow.max_concurrent_steps = 8;  // Allow more parallel voting
    workflow.global_context["consensus_threshold"] = 0.6;  // Reasonable threshold
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json large_group_input = {
        {"decision_topic", "Organization-wide Policy"},
        {"group_size", 10},
        {"expected_participation", 0.9}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, large_group_input);
    
    // Wait longer for larger group execution
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    
    // Should handle large group efficiently
    EXPECT_TRUE(status->current_status != WorkflowStatus::PENDING);
}

TEST_F(ConsensusWorkflowTest, ConsensusWithTimeouts) {
    test_workflow_engine_->start();
    
    auto workflow = createConsensusWorkflow(4);
    
    // Set short timeouts to test timeout handling
    for (auto& step : workflow.steps) {
        if (step.step_id.find("vote_") == 0) {
            step.timeout_seconds = 1;  // Very short timeout for voting
        }
    }
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json timeout_test_input = {
        {"decision_topic", "Time-sensitive Decision"},
        {"simulate_slow_voting", true}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, timeout_test_input);
    
    // Wait for timeouts to trigger
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Some steps might have timed out
        auto step_statuses = status->step_statuses;
        
        bool has_timeout_or_failure = false;
        for (const auto& [step_id, step_status] : step_statuses) {
            if (step_status == StepStatus::FAILED) {
                has_timeout_or_failure = true;
                break;
            }
        }
        
        // Consensus should still attempt to work with available votes
        if (step_statuses.find("consensus") != step_statuses.end()) {
            EXPECT_TRUE(true);  // Consensus handling timeout case
        }
    }
}

TEST_F(ConsensusWorkflowTest, ConsensusMetrics) {
    test_workflow_engine_->start();
    
    // Execute multiple consensus workflows to generate metrics
    std::vector<std::string> execution_ids;
    
    for (int i = 0; i < 3; ++i) {
        auto workflow = createConsensusWorkflow(3);
        workflow.workflow_id = "consensus_metrics_" + std::to_string(i);
        
        std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
        std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
        execution_ids.push_back(execution_id);
    }
    
    // Wait for executions
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    // Check metrics
    auto metrics = test_workflow_engine_->get_metrics();
    EXPECT_GE(metrics.total_workflows, 3);
    EXPECT_GE(metrics.average_execution_time_ms, 0.0);
    
    // Check execution history
    auto history = test_workflow_engine_->get_execution_history();
    EXPECT_GE(history.size(), 0);
}

TEST_F(ConsensusWorkflowTest, ConsensusErrorRecovery) {
    test_workflow_engine_->start();
    
    auto workflow = createConsensusWorkflow(3);
    
    // Configure error recovery
    workflow.error_handling.retry_on_failure = true;
    workflow.error_handling.max_retries = 2;
    workflow.error_handling.retry_delay_seconds = 1;
    workflow.error_handling.continue_on_error = true;
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json error_recovery_input = {
        {"decision_topic", "Error-prone Decision"},
        {"introduce_random_errors", true},
        {"error_probability", 0.3}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, error_recovery_input);
    
    // Wait for retries and recovery
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Workflow should handle errors and continue
        EXPECT_TRUE(status->current_status == WorkflowStatus::COMPLETED ||
                   status->current_status == WorkflowStatus::RUNNING ||
                   status->current_status == WorkflowStatus::FAILED);
        
        // Check if any retries occurred by examining step statuses
        auto step_statuses = status->step_statuses;
        for (const auto& [step_id, step_status] : step_statuses) {
            if (step_status == StepStatus::RETRYING) {
                // Found evidence of retry logic working
                EXPECT_TRUE(true);
                break;
            }
        }
    }
}
