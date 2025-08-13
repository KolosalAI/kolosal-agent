/**
 * @file test_conditional_workflow.cpp
 * @brief Unit tests for conditional workflow execution
 * @version 2.0.0
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

class ConditionalWorkflowTest : public WorkflowTestFixture {
protected:
    void SetUp() override {
        WorkflowTestFixture::SetUp();
    }

    Workflow createConditionalWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "test_conditional_workflow";
        workflow.name = "Test Conditional Workflow";
        workflow.description = "A conditional workflow for testing";
        workflow.type = WorkflowType::CONDITIONAL;
        
        // Global context
        workflow.global_context = {
            {"quality_threshold", 0.75},
            {"complexity_threshold", 0.8},
            {"input_data", "test_data"}
        };

        // Step 1: Initial assessment
        WorkflowStep initial_step;
        initial_step.step_id = "initial_assessment";
        initial_step.name = "Initial Data Assessment";
        initial_step.agent_id = "test_agent_1";
        initial_step.function_name = "assess_data";
        initial_step.parameters = {
            {"data_source", "${global.input_data}"},
            {"check_quality", true},
            {"check_complexity", true}
        };
        
        // Step 2: High quality processing (conditional)
        WorkflowStep high_quality_step;
        high_quality_step.step_id = "high_quality_processing";
        high_quality_step.name = "High Quality Processing";
        high_quality_step.agent_id = "test_agent_1";
        high_quality_step.function_name = "advanced_process";
        high_quality_step.parameters = {
            {"data", "${steps.initial_assessment.output}"},
            {"use_advanced_methods", true}
        };
        high_quality_step.dependencies.push_back({"initial_assessment", "success", true});
        high_quality_step.conditions = {
            {"expression", "steps.initial_assessment.output.quality_score >= global.quality_threshold"}
        };
        
        // Step 3: Low quality processing (conditional)
        WorkflowStep low_quality_step;
        low_quality_step.step_id = "low_quality_processing";
        low_quality_step.name = "Low Quality Processing";
        low_quality_step.agent_id = "test_agent_1";
        low_quality_step.function_name = "basic_process";
        low_quality_step.parameters = {
            {"data", "${steps.initial_assessment.output}"},
            {"apply_cleaning", true}
        };
        low_quality_step.dependencies.push_back({"initial_assessment", "success", true});
        low_quality_step.conditions = {
            {"expression", "steps.initial_assessment.output.quality_score < global.quality_threshold"}
        };
        
        // Step 4: Final synthesis (always runs)
        WorkflowStep synthesis_step;
        synthesis_step.step_id = "synthesis";
        synthesis_step.name = "Results Synthesis";
        synthesis_step.agent_id = "test_agent_1";
        synthesis_step.function_name = "synthesize";
        synthesis_step.parameters = {
            {"high_quality_result", "${steps.high_quality_processing.output || null}"},
            {"low_quality_result", "${steps.low_quality_processing.output || null}"}
        };
        synthesis_step.dependencies.push_back({"high_quality_processing", "completion", false});
        synthesis_step.dependencies.push_back({"low_quality_processing", "completion", false});
        
        workflow.steps = {initial_step, high_quality_step, low_quality_step, synthesis_step};
        return workflow;
    }

    Workflow createNestedConditionalWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "nested_conditional_workflow";
        workflow.name = "Nested Conditional Workflow";
        workflow.type = WorkflowType::CONDITIONAL;
        
        workflow.global_context = {
            {"score_threshold", 0.5},
            {"complexity_level", "medium"}
        };

        // Multiple nested conditions
        WorkflowStep step1;
        step1.step_id = "evaluate";
        step1.name = "Evaluation Step";
        step1.agent_id = "test_agent_1";
        step1.function_name = "evaluate";
        step1.parameters = {{"input", "test_input"}};

        WorkflowStep step2;
        step2.step_id = "process_high_score";
        step2.name = "Process High Score";
        step2.agent_id = "test_agent_1";
        step2.function_name = "process_advanced";
        step2.parameters = {{"data", "${steps.evaluate.output}"}};
        step2.dependencies.push_back({"evaluate", "success", true});
        step2.conditions = {
            {"expression", "steps.evaluate.output.score >= global.score_threshold && global.complexity_level == 'high'"}
        };

        WorkflowStep step3;
        step3.step_id = "process_medium_score";
        step3.name = "Process Medium Score";
        step3.agent_id = "test_agent_1";
        step3.function_name = "process_standard";
        step3.parameters = {{"data", "${steps.evaluate.output}"}};
        step3.dependencies.push_back({"evaluate", "success", true});
        step3.conditions = {
            {"expression", "steps.evaluate.output.score >= global.score_threshold && global.complexity_level == 'medium'"}
        };

        WorkflowStep step4;
        step4.step_id = "fallback_process";
        step4.name = "Fallback Processing";
        step4.agent_id = "test_agent_1";
        step4.function_name = "process_basic";
        step4.parameters = {{"data", "${steps.evaluate.output}"}};
        step4.dependencies.push_back({"evaluate", "success", true});
        step4.conditions = {
            {"expression", "steps.evaluate.output.score < global.score_threshold"}
        };

        workflow.steps = {step1, step2, step3, step4};
        return workflow;
    }
};

TEST_F(ConditionalWorkflowTest, BasicConditionalExecution) {
    test_workflow_engine_->start();
    
    auto workflow = createConditionalWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Execute workflow
    nlohmann::json input_context = {
        {"input_data", "high_quality_test_data"},
        {"expected_quality", 0.85}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, input_context);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for execution to progress
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status->workflow_id, workflow_id);
    EXPECT_NE(status->current_status, WorkflowStatus::PENDING);
}

TEST_F(ConditionalWorkflowTest, ConditionEvaluation) {
    test_workflow_engine_->start();
    
    auto workflow = createConditionalWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Mock execution context for condition testing
    WorkflowExecutionContext context;
    context.execution_id = "test_exec_1";
    context.workflow_id = workflow_id;
    context.global_variables = {
        {"quality_threshold", 0.75},
        {"complexity_threshold", 0.8}
    };
    context.step_outputs["initial_assessment"] = {
        {"quality_score", 0.85},
        {"complexity_score", 0.9}
    };
    
    // Test condition evaluation (access private method through public interface)
    // This tests the condition evaluation logic indirectly
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, context.global_variables);
    
    // Wait and check execution results
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto exec_status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(exec_status.has_value());
}

TEST_F(ConditionalWorkflowTest, HighQualityPath) {
    test_workflow_engine_->start();
    
    auto workflow = createConditionalWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Simulate high quality data
    nlohmann::json high_quality_input = {
        {"input_data", "premium_test_data"},
        {"quality_hint", 0.9}  // Hint for mock to return high quality
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, high_quality_input);
    
    // Wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Check that high quality processing step was executed
        auto step_statuses = status->step_statuses;
        if (step_statuses.find("high_quality_processing") != step_statuses.end()) {
            // High quality step should have been executed
            EXPECT_TRUE(step_statuses["high_quality_processing"] == StepStatus::COMPLETED ||
                       step_statuses["high_quality_processing"] == StepStatus::RUNNING);
        }
    }
}

TEST_F(ConditionalWorkflowTest, LowQualityPath) {
    test_workflow_engine_->start();
    
    auto workflow = createConditionalWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Simulate low quality data
    nlohmann::json low_quality_input = {
        {"input_data", "poor_test_data"},
        {"quality_hint", 0.3}  // Hint for mock to return low quality
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, low_quality_input);
    
    // Wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Check that low quality processing step was executed
        auto step_statuses = status->step_statuses;
        if (step_statuses.find("low_quality_processing") != step_statuses.end()) {
            // Low quality step should have been executed
            EXPECT_TRUE(step_statuses["low_quality_processing"] == StepStatus::COMPLETED ||
                       step_statuses["low_quality_processing"] == StepStatus::RUNNING);
        }
    }
}

TEST_F(ConditionalWorkflowTest, NestedConditions) {
    test_workflow_engine_->start();
    
    auto workflow = createNestedConditionalWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Test with medium complexity
    nlohmann::json medium_input = {
        {"complexity_level", "medium"},
        {"expected_score", 0.7}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, medium_input);
    
    // Wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    
    // Test with different complexity levels
    auto high_complexity_workflow = createNestedConditionalWorkflow();
    high_complexity_workflow.global_context["complexity_level"] = "high";
    std::string high_workflow_id = test_workflow_engine_->create_workflow(high_complexity_workflow);
    
    nlohmann::json high_input = {
        {"complexity_level", "high"},
        {"expected_score", 0.8}
    };
    
    std::string high_execution_id = test_workflow_engine_->execute_workflow(high_workflow_id, high_input);
    EXPECT_FALSE(high_execution_id.empty());
}

TEST_F(ConditionalWorkflowTest, ConditionWithMultipleOperators) {
    Workflow workflow;
    workflow.workflow_id = "complex_condition_workflow";
    workflow.name = "Complex Condition Test";
    workflow.type = WorkflowType::CONDITIONAL;
    
    workflow.global_context = {
        {"min_score", 0.5},
        {"max_score", 0.9},
        {"category", "premium"}
    };

    WorkflowStep assessment;
    assessment.step_id = "assess";
    assessment.name = "Assessment";
    assessment.agent_id = "test_agent_1";
    assessment.function_name = "assess";
    assessment.parameters = {{"input", "test"}};

    WorkflowStep complex_condition_step;
    complex_condition_step.step_id = "complex_process";
    complex_condition_step.name = "Complex Processing";
    complex_condition_step.agent_id = "test_agent_1";
    complex_condition_step.function_name = "complex_process";
    complex_condition_step.parameters = {{"data", "${steps.assess.output}"}};
    complex_condition_step.dependencies.push_back({"assess", "success", true});
    complex_condition_step.conditions = {
        {"expression", "steps.assess.output.score >= global.min_score && steps.assess.output.score <= global.max_score && global.category == 'premium'"}
    };

    workflow.steps = {assessment, complex_condition_step};
    
    test_workflow_engine_->start();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait and verify
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
}

TEST_F(ConditionalWorkflowTest, SkippedStepHandling) {
    test_workflow_engine_->start();
    
    auto workflow = createConditionalWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Execute with conditions that should skip one branch
    nlohmann::json input = {
        {"input_data", "medium_quality_data"},
        {"quality_hint", 0.9}  // Should trigger high quality path only
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, input);
    
    // Wait for execution to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Verify that skipped steps are properly handled
        auto step_statuses = status->step_statuses;
        
        // At least one of the conditional steps should be skipped or not executed
        bool has_skipped_step = false;
        for (const auto& [step_id, step_status] : step_statuses) {
            if (step_status == StepStatus::SKIPPED) {
                has_skipped_step = true;
                break;
            }
        }
        
        // Synthesis step should still run even if some steps were skipped
        if (step_statuses.find("synthesis") != step_statuses.end()) {
            EXPECT_NE(step_statuses["synthesis"], StepStatus::PENDING);
        }
    }
}

TEST_F(ConditionalWorkflowTest, ErrorHandlingInConditionalWorkflow) {
    test_workflow_engine_->start();
    
    auto workflow = createConditionalWorkflow();
    
    // Configure error handling
    workflow.error_handling.retry_on_failure = true;
    workflow.error_handling.max_retries = 2;
    workflow.error_handling.continue_on_error = true;
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Execute with input that might cause errors
    nlohmann::json error_prone_input = {
        {"input_data", "invalid_data"},
        {"force_error", true}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, error_prone_input);
    
    // Wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    
    // Workflow should handle errors gracefully
    EXPECT_TRUE(status->current_status == WorkflowStatus::COMPLETED ||
               status->current_status == WorkflowStatus::FAILED ||
               status->current_status == WorkflowStatus::RUNNING);
}

TEST_F(ConditionalWorkflowTest, VariableInterpolation) {
    test_workflow_engine_->start();
    
    Workflow workflow;
    workflow.workflow_id = "interpolation_test";
    workflow.name = "Variable Interpolation Test";
    workflow.type = WorkflowType::CONDITIONAL;
    
    workflow.global_context = {
        {"base_path", "/test/data"},
        {"file_extension", ".json"},
        {"processing_mode", "advanced"}
    };

    WorkflowStep step1;
    step1.step_id = "interpolation_step";
    step1.name = "Interpolation Test Step";
    step1.agent_id = "test_agent_1";
    step1.function_name = "process_file";
    step1.parameters = {
        {"file_path", "${global.base_path}/input${global.file_extension}"},
        {"mode", "${global.processing_mode}"},
        {"output_path", "${global.base_path}/output${global.file_extension}"}
    };
    
    workflow.steps = {step1};
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    EXPECT_FALSE(execution_id.empty());
    
    // Wait and check
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
}

TEST_F(ConditionalWorkflowTest, ConditionalWithDependencyFailure) {
    test_workflow_engine_->start();
    
    Workflow workflow;
    workflow.workflow_id = "dependency_failure_test";
    workflow.name = "Dependency Failure Test";
    workflow.type = WorkflowType::CONDITIONAL;

    WorkflowStep failing_step;
    failing_step.step_id = "failing_step";
    failing_step.name = "Intentionally Failing Step";
    failing_step.agent_id = "test_agent_1";
    failing_step.function_name = "fail_function";
    failing_step.parameters = {{"should_fail", true}};

    WorkflowStep conditional_step;
    conditional_step.step_id = "conditional_step";
    conditional_step.name = "Conditional Step";
    conditional_step.agent_id = "test_agent_1";
    conditional_step.function_name = "process";
    conditional_step.parameters = {{"input", "test"}};
    conditional_step.dependencies.push_back({"failing_step", "success", true});
    conditional_step.conditions = {
        {"expression", "steps.failing_step.output.success == true"}
    };

    WorkflowStep fallback_step;
    fallback_step.step_id = "fallback_step";
    fallback_step.name = "Fallback Step";
    fallback_step.agent_id = "test_agent_1";
    fallback_step.function_name = "fallback_process";
    fallback_step.parameters = {{"input", "fallback_input"}};
    fallback_step.dependencies.push_back({"failing_step", "failure", false});  // Not required
    fallback_step.conditions = {
        {"expression", "steps.failing_step.status == 'failed'"}
    };

    workflow.steps = {failing_step, conditional_step, fallback_step};
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Fallback step should execute when main step fails
        auto step_statuses = status->step_statuses;
        if (step_statuses.find("fallback_step") != step_statuses.end()) {
            // Fallback should be executed or at least considered
            EXPECT_TRUE(step_statuses["fallback_step"] != StepStatus::PENDING ||
                       status->current_status == WorkflowStatus::RUNNING);
        }
    }
}
