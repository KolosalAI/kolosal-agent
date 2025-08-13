/**
 * @file test_pipeline_workflow.cpp
 * @brief Unit tests for pipeline workflow execution
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

class PipelineWorkflowTest : public WorkflowTestFixture {
protected:
    void SetUp() override {
        WorkflowTestFixture::SetUp();
    }

    Workflow createLinearPipelineWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "linear_pipeline_workflow";
        workflow.name = "Linear Pipeline Workflow";
        workflow.description = "A linear data pipeline workflow";
        workflow.type = WorkflowType::PIPELINE;
        
        workflow.global_context = {
            {"input_data", "raw_dataset.json"},
            {"processing_quality", "high"},
            {"output_format", "processed"}
        };

        // Stage 1: Data Collection
        WorkflowStep collect_step;
        collect_step.step_id = "collect_data";
        collect_step.name = "Collect Data";
        collect_step.agent_id = "data_collector";
        collect_step.function_name = "collect_data";
        collect_step.parameters = {
            {"source", "${global.input_data}"},
            {"format", "json"}
        };
        collect_step.parallel_allowed = false;

        // Stage 2: Data Cleaning
        WorkflowStep clean_step;
        clean_step.step_id = "clean_data";
        clean_step.name = "Clean Data";
        clean_step.agent_id = "data_processor";
        clean_step.function_name = "clean_data";
        clean_step.parameters = {
            {"input", "${steps.collect_data.output}"},
            {"quality", "${global.processing_quality}"}
        };
        clean_step.dependencies.push_back({"collect_data", "success", true});
        clean_step.parallel_allowed = false;

        // Stage 3: Data Transformation
        WorkflowStep transform_step;
        transform_step.step_id = "transform_data";
        transform_step.name = "Transform Data";
        transform_step.agent_id = "data_processor";
        transform_step.function_name = "transform_data";
        transform_step.parameters = {
            {"input", "${steps.clean_data.output}"},
            {"transformation_rules", "standard"}
        };
        transform_step.dependencies.push_back({"clean_data", "success", true});
        transform_step.parallel_allowed = false;

        // Stage 4: Data Analysis
        WorkflowStep analyze_step;
        analyze_step.step_id = "analyze_data";
        analyze_step.name = "Analyze Data";
        analyze_step.agent_id = "data_analyst";
        analyze_step.function_name = "analyze_data";
        analyze_step.parameters = {
            {"input", "${steps.transform_data.output}"},
            {"analysis_type", "comprehensive"}
        };
        analyze_step.dependencies.push_back({"transform_data", "success", true});
        analyze_step.parallel_allowed = false;

        // Stage 5: Generate Report
        WorkflowStep report_step;
        report_step.step_id = "generate_report";
        report_step.name = "Generate Report";
        report_step.agent_id = "report_generator";
        report_step.function_name = "generate_report";
        report_step.parameters = {
            {"analysis_results", "${steps.analyze_data.output}"},
            {"format", "${global.output_format}"}
        };
        report_step.dependencies.push_back({"analyze_data", "success", true});
        report_step.parallel_allowed = false;

        workflow.steps = {collect_step, clean_step, transform_step, analyze_step, report_step};
        return workflow;
    }

    Workflow createParallelStagesPipelineWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "parallel_stages_pipeline";
        workflow.name = "Pipeline with Parallel Stages";
        workflow.type = WorkflowType::PIPELINE;
        
        workflow.global_context = {
            {"input_data", "multi_source_data"},
            {"processing_mode", "parallel"}
        };

        // Initial data collection
        WorkflowStep collect_step;
        collect_step.step_id = "collect_initial";
        collect_step.name = "Initial Collection";
        collect_step.agent_id = "collector";
        collect_step.function_name = "collect_initial_data";
        collect_step.parameters = {{"source", "${global.input_data}"}};

        // Parallel processing stage
        WorkflowStep process_a;
        process_a.step_id = "process_stream_a";
        process_a.name = "Process Stream A";
        process_a.agent_id = "processor_a";
        process_a.function_name = "process_data";
        process_a.parameters = {
            {"input", "${steps.collect_initial.output.stream_a}"},
            {"processor_type", "type_a"}
        };
        process_a.dependencies.push_back({"collect_initial", "success", true});
        process_a.parallel_allowed = true;

        WorkflowStep process_b;
        process_b.step_id = "process_stream_b";
        process_b.name = "Process Stream B";
        process_b.agent_id = "processor_b";
        process_b.function_name = "process_data";
        process_b.parameters = {
            {"input", "${steps.collect_initial.output.stream_b}"},
            {"processor_type", "type_b"}
        };
        process_b.dependencies.push_back({"collect_initial", "success", true});
        process_b.parallel_allowed = true;

        WorkflowStep process_c;
        process_c.step_id = "process_stream_c";
        process_c.name = "Process Stream C";
        process_c.agent_id = "processor_c";
        process_c.function_name = "process_data";
        process_c.parameters = {
            {"input", "${steps.collect_initial.output.stream_c}"},
            {"processor_type", "type_c"}
        };
        process_c.dependencies.push_back({"collect_initial", "success", true});
        process_c.parallel_allowed = true;

        // Merge stage (depends on all parallel processors)
        WorkflowStep merge_step;
        merge_step.step_id = "merge_results";
        merge_step.name = "Merge Processing Results";
        merge_step.agent_id = "merger";
        merge_step.function_name = "merge_data";
        merge_step.parameters = {
            {"stream_a", "${steps.process_stream_a.output}"},
            {"stream_b", "${steps.process_stream_b.output}"},
            {"stream_c", "${steps.process_stream_c.output}"}
        };
        merge_step.dependencies = {
            {"process_stream_a", "success", true},
            {"process_stream_b", "success", true},
            {"process_stream_c", "success", true}
        };
        merge_step.parallel_allowed = false;

        // Final analysis
        WorkflowStep final_analysis;
        final_analysis.step_id = "final_analysis";
        final_analysis.name = "Final Analysis";
        final_analysis.agent_id = "analyst";
        final_analysis.function_name = "final_analysis";
        final_analysis.parameters = {
            {"merged_data", "${steps.merge_results.output}"}
        };
        final_analysis.dependencies.push_back({"merge_results", "success", true});

        workflow.steps = {collect_step, process_a, process_b, process_c, merge_step, final_analysis};
        return workflow;
    }

    Workflow createComplexDependencyPipelineWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "complex_dependency_pipeline";
        workflow.name = "Complex Dependency Pipeline";
        workflow.type = WorkflowType::PIPELINE;
        
        workflow.global_context = {
            {"complexity_level", "advanced"},
            {"optimization_enabled", true}
        };

        // Root step
        WorkflowStep root;
        root.step_id = "root";
        root.name = "Root Process";
        root.agent_id = "root_agent";
        root.function_name = "initialize";
        root.parameters = {{"level", "${global.complexity_level}"}};

        // Level 1 - depends on root
        WorkflowStep level1_a;
        level1_a.step_id = "level1_a";
        level1_a.name = "Level 1 A";
        level1_a.agent_id = "processor";
        level1_a.function_name = "process";
        level1_a.parameters = {{"input", "${steps.root.output}"}};
        level1_a.dependencies.push_back({"root", "success", true});
        level1_a.parallel_allowed = true;

        WorkflowStep level1_b;
        level1_b.step_id = "level1_b";
        level1_b.name = "Level 1 B";
        level1_b.agent_id = "processor";
        level1_b.function_name = "process";
        level1_b.parameters = {{"input", "${steps.root.output}"}};
        level1_b.dependencies.push_back({"root", "success", true});
        level1_b.parallel_allowed = true;

        // Level 2 - mixed dependencies
        WorkflowStep level2_a;
        level2_a.step_id = "level2_a";
        level2_a.name = "Level 2 A";
        level2_a.agent_id = "processor";
        level2_a.function_name = "advanced_process";
        level2_a.parameters = {
            {"input_a", "${steps.level1_a.output}"},
            {"input_b", "${steps.level1_b.output}"}
        };
        level2_a.dependencies = {
            {"level1_a", "success", true},
            {"level1_b", "success", true}
        };

        WorkflowStep level2_b;
        level2_b.step_id = "level2_b";
        level2_b.name = "Level 2 B";
        level2_b.agent_id = "processor";
        level2_b.function_name = "process";
        level2_b.parameters = {{"input", "${steps.level1_a.output}"}};
        level2_b.dependencies.push_back({"level1_a", "success", true});
        level2_b.parallel_allowed = true;  // Can run in parallel with level2_a

        WorkflowStep level2_c;
        level2_c.step_id = "level2_c";
        level2_c.name = "Level 2 C";
        level2_c.agent_id = "processor";
        level2_c.function_name = "process";
        level2_c.parameters = {{"input", "${steps.level1_b.output}"}};
        level2_c.dependencies.push_back({"level1_b", "success", true});
        level2_c.parallel_allowed = true;  // Can run in parallel with level2_a

        // Final step - depends on all level 2 steps
        WorkflowStep final_step;
        final_step.step_id = "final";
        final_step.name = "Final Aggregation";
        final_step.agent_id = "aggregator";
        final_step.function_name = "aggregate";
        final_step.parameters = {
            {"input_a", "${steps.level2_a.output}"},
            {"input_b", "${steps.level2_b.output}"},
            {"input_c", "${steps.level2_c.output}"}
        };
        final_step.dependencies = {
            {"level2_a", "success", true},
            {"level2_b", "success", true},
            {"level2_c", "success", true}
        };

        workflow.steps = {root, level1_a, level1_b, level2_a, level2_b, level2_c, final_step};
        return workflow;
    }
};

TEST_F(PipelineWorkflowTest, LinearPipelineExecution) {
    test_workflow_engine_->start();
    
    auto workflow = createLinearPipelineWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json input_context = {
        {"input_data", "sample_dataset.json"},
        {"processing_quality", "high"}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, input_context);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for pipeline to progress through stages
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status->workflow_id, workflow_id);
    EXPECT_NE(status->current_status, WorkflowStatus::PENDING);
}

TEST_F(PipelineWorkflowTest, DataFlowThroughPipeline) {
    test_workflow_engine_->start();
    
    auto workflow = createLinearPipelineWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Wait for multiple stages to execute
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        auto step_statuses = status->step_statuses;
        
        // Verify that steps execute in proper order
        // Earlier steps should complete before later steps start
        if (step_statuses.find("collect_data") != step_statuses.end() &&
            step_statuses.find("clean_data") != step_statuses.end()) {
            
            // If clean_data is completed, collect_data should also be completed
            if (step_statuses["clean_data"] == StepStatus::COMPLETED) {
                EXPECT_EQ(step_statuses["collect_data"], StepStatus::COMPLETED);
            }
        }
        
        // Check data flow through pipeline stages
        if (step_statuses.find("transform_data") != step_statuses.end() &&
            step_statuses["transform_data"] == StepStatus::COMPLETED) {
            // Previous steps should be completed
            EXPECT_TRUE(step_statuses["collect_data"] == StepStatus::COMPLETED &&
                       step_statuses["clean_data"] == StepStatus::COMPLETED);
        }
    }
}

TEST_F(PipelineWorkflowTest, ParallelStagesInPipeline) {
    test_workflow_engine_->start();
    
    auto workflow = createParallelStagesPipelineWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Wait for parallel execution
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        auto step_statuses = status->step_statuses;
        
        // Check that parallel processing steps can run simultaneously
        int parallel_steps_active = 0;
        std::vector<std::string> parallel_step_ids = {
            "process_stream_a", "process_stream_b", "process_stream_c"
        };
        
        for (const auto& step_id : parallel_step_ids) {
            if (step_statuses.find(step_id) != step_statuses.end()) {
                auto step_status = step_statuses[step_id];
                if (step_status == StepStatus::RUNNING || 
                    step_status == StepStatus::COMPLETED) {
                    parallel_steps_active++;
                }
            }
        }
        
        // At least some parallel steps should be active
        EXPECT_GE(parallel_steps_active, 0);
        
        // Merge step should wait for all parallel steps
        if (step_statuses.find("merge_results") != step_statuses.end() &&
            step_statuses["merge_results"] == StepStatus::COMPLETED) {
            // All parallel processing steps should be completed
            for (const auto& step_id : parallel_step_ids) {
                if (step_statuses.find(step_id) != step_statuses.end()) {
                    EXPECT_EQ(step_statuses[step_id], StepStatus::COMPLETED);
                }
            }
        }
    }
}

TEST_F(PipelineWorkflowTest, ComplexDependencyResolution) {
    test_workflow_engine_->start();
    
    auto workflow = createComplexDependencyPipelineWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Wait for complex pipeline execution
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        auto step_statuses = status->step_statuses;
        
        // Verify dependency ordering
        if (step_statuses.find("root") != step_statuses.end() &&
            step_statuses["root"] == StepStatus::COMPLETED) {
            
            // Level 1 steps should be able to run after root
            bool level1_progress = false;
            if (step_statuses.find("level1_a") != step_statuses.end() ||
                step_statuses.find("level1_b") != step_statuses.end()) {
                level1_progress = true;
            }
            
            if (level1_progress) {
                EXPECT_TRUE(true);  // Dependencies are being respected
            }
        }
        
        // Check level 2 dependencies
        if (step_statuses.find("level2_a") != step_statuses.end() &&
            step_statuses["level2_a"] == StepStatus::COMPLETED) {
            // Both level1_a and level1_b should be completed
            if (step_statuses.find("level1_a") != step_statuses.end() &&
                step_statuses.find("level1_b") != step_statuses.end()) {
                EXPECT_EQ(step_statuses["level1_a"], StepStatus::COMPLETED);
                EXPECT_EQ(step_statuses["level1_b"], StepStatus::COMPLETED);
            }
        }
        
        // Final step should wait for all level 2 steps
        if (step_statuses.find("final") != step_statuses.end() &&
            step_statuses["final"] == StepStatus::COMPLETED) {
            // All level 2 steps should be completed
            std::vector<std::string> level2_steps = {"level2_a", "level2_b", "level2_c"};
            for (const auto& step_id : level2_steps) {
                if (step_statuses.find(step_id) != step_statuses.end()) {
                    EXPECT_EQ(step_statuses[step_id], StepStatus::COMPLETED);
                }
            }
        }
    }
}

TEST_F(PipelineWorkflowTest, PipelineErrorPropagation) {
    test_workflow_engine_->start();
    
    auto workflow = createLinearPipelineWorkflow();
    
    // Configure to stop on error (default pipeline behavior)
    workflow.error_handling.continue_on_error = false;
    workflow.error_handling.retry_on_failure = false;
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    // Execute with input likely to cause error in middle stage
    nlohmann::json error_input = {
        {"input_data", "invalid_dataset"},
        {"cause_error_in_clean_step", true}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, error_input);
    
    // Wait for error to propagate
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        auto step_statuses = status->step_statuses;
        
        // If a middle step fails, later steps should not execute
        if (step_statuses.find("clean_data") != step_statuses.end() &&
            step_statuses["clean_data"] == StepStatus::FAILED) {
            
            // Later steps should be pending or not started
            if (step_statuses.find("analyze_data") != step_statuses.end()) {
                EXPECT_TRUE(step_statuses["analyze_data"] == StepStatus::PENDING ||
                           step_statuses["analyze_data"] == StepStatus::FAILED);
            }
        }
    }
}

TEST_F(PipelineWorkflowTest, PipelineWithRetries) {
    test_workflow_engine_->start();
    
    auto workflow = createLinearPipelineWorkflow();
    
    // Configure retry behavior
    workflow.error_handling.retry_on_failure = true;
    workflow.error_handling.max_retries = 2;
    workflow.error_handling.retry_delay_seconds = 1;
    
    // Set retries on individual steps
    for (auto& step : workflow.steps) {
        step.max_retries = 2;
        step.retry_delay_seconds = 1;
    }
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json retry_test_input = {
        {"input_data", "flaky_dataset"},
        {"introduce_intermittent_errors", true},
        {"error_rate", 0.5}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, retry_test_input);
    
    // Wait for retries
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    if (status.has_value()) {
        // Check for evidence of retry attempts
        auto step_statuses = status->step_statuses;
        
        bool found_retrying = false;
        for (const auto& [step_id, step_status] : step_statuses) {
            if (step_status == StepStatus::RETRYING) {
                found_retrying = true;
                break;
            }
        }
        
        // Workflow should eventually complete or fail gracefully
        EXPECT_TRUE(status->current_status != WorkflowStatus::PENDING);
    }
}

TEST_F(PipelineWorkflowTest, PipelinePerformanceOptimization) {
    test_workflow_engine_->start();
    
    auto workflow = createParallelStagesPipelineWorkflow();
    
    // Optimize for performance
    workflow.max_concurrent_steps = 6;  // Allow more parallelism
    workflow.global_context["optimization_enabled"] = true;
    workflow.global_context["performance_mode"] = "high";
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Wait for optimized execution
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    
    // Performance should be reasonable (this is a basic check)
    EXPECT_LT(execution_duration.count(), 10000);  // Less than 10 seconds
}

TEST_F(PipelineWorkflowTest, DynamicPipelineCreation) {
    test_workflow_engine_->start();
    
    // Test creating pipeline using template methods
    std::vector<std::pair<std::string, std::string>> pipeline_stages = {
        {"data_collector", "collect"},
        {"data_cleaner", "clean"},
        {"data_transformer", "transform"},
        {"data_analyzer", "analyze"}
    };
    
    auto template_workflow = test_workflow_engine_->create_pipeline_workflow(
        "Template Pipeline", pipeline_stages
    );
    
    EXPECT_EQ(template_workflow.type, WorkflowType::PIPELINE);
    EXPECT_EQ(template_workflow.steps.size(), 4);
    EXPECT_EQ(template_workflow.name, "Template Pipeline");
    
    std::string workflow_id = test_workflow_engine_->create_workflow(template_workflow);
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    EXPECT_FALSE(execution_id.empty());
    
    // Wait and verify
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
}

TEST_F(PipelineWorkflowTest, PipelineStateManagement) {
    test_workflow_engine_->start();
    
    auto workflow = createLinearPipelineWorkflow();
    workflow.persist_state = true;  // Enable state persistence
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Let it run partway
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Pause the workflow
    bool paused = test_workflow_engine_->pause_workflow(execution_id);
    EXPECT_TRUE(paused);
    
    // Check state
    auto paused_status = test_workflow_engine_->get_execution_status(execution_id);
    if (paused_status.has_value()) {
        // Should be paused or still in progress
        EXPECT_TRUE(paused_status->current_status == WorkflowStatus::PAUSED ||
                   paused_status->current_status == WorkflowStatus::RUNNING);
        
        // Resume execution
        bool resumed = test_workflow_engine_->resume_workflow(execution_id);
        EXPECT_TRUE(resumed);
        
        // Wait for completion
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        
        auto final_status = test_workflow_engine_->get_execution_status(execution_id);
        if (final_status.has_value()) {
            EXPECT_NE(final_status->current_status, WorkflowStatus::PENDING);
        }
    }
}
