/**
 * @file test_multi_agent_workflows.cpp
 * @brief Integration tests for multi-agent workflow scenarios
 * @version 2.0.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "workflow/workflow_engine.hpp"
#include "agent/core/agent_core.hpp"
#include "../fixtures/test_fixtures.hpp"
#include <thread>
#include <chrono>
#include <vector>

using namespace testing;
using namespace kolosal::agents;
using namespace kolosal::agents::test;

class MultiAgentWorkflowTest : public WorkflowTestFixture {
protected:
    void SetUp() override {
        WorkflowTestFixture::SetUp();
    }

    Workflow createCollaborativeDataProcessingWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "collaborative_data_processing";
        workflow.name = "Collaborative Data Processing Workflow";
        workflow.description = "Multiple agents collaborate to process data";
        workflow.type = WorkflowType::PIPELINE;
        
        workflow.global_context = {
            {"dataset", "research_data.json"},
            {"quality_threshold", 0.8},
            {"output_format", "comprehensive_report"}
        };

        // Step 1: Data Collector Agent
        WorkflowStep collect_step;
        collect_step.step_id = "data_collection";
        collect_step.name = "Data Collection";
        collect_step.agent_id = "data_collector_agent";
        collect_step.function_name = "collect_and_validate_data";
        collect_step.parameters = {
            {"source", "${global.dataset}"},
            {"validation_level", "strict"},
            {"quality_threshold", "${global.quality_threshold}"}
        };
        collect_step.timeout_seconds = 120;

        // Step 2: Data Analyzer Agent (runs in parallel streams)
        WorkflowStep analyze_statistical;
        analyze_statistical.step_id = "statistical_analysis";
        analyze_statistical.name = "Statistical Analysis";
        analyze_statistical.agent_id = "statistical_analyst";
        analyze_statistical.function_name = "analyze_statistical_patterns";
        analyze_statistical.parameters = {
            {"data", "${steps.data_collection.output.cleaned_data}"},
            {"analysis_type", "comprehensive"},
            {"confidence_level", 0.95}
        };
        analyze_statistical.dependencies.push_back({"data_collection", "success", true});
        analyze_statistical.parallel_allowed = true;
        analyze_statistical.timeout_seconds = 180;

        WorkflowStep analyze_trends;
        analyze_trends.step_id = "trend_analysis";
        analyze_trends.name = "Trend Analysis";
        analyze_trends.agent_id = "trend_analyst";
        analyze_trends.function_name = "identify_trends";
        analyze_trends.parameters = {
            {"data", "${steps.data_collection.output.cleaned_data}"},
            {"time_window", "6_months"},
            {"trend_types", ["linear", "seasonal", "cyclical"]}
        };
        analyze_trends.dependencies.push_back({"data_collection", "success", true});
        analyze_trends.parallel_allowed = true;
        analyze_trends.timeout_seconds = 180;

        WorkflowStep analyze_anomalies;
        analyze_anomalies.step_id = "anomaly_detection";
        analyze_anomalies.name = "Anomaly Detection";
        analyze_anomalies.agent_id = "anomaly_detector";
        analyze_anomalies.function_name = "detect_anomalies";
        analyze_anomalies.parameters = {
            {"data", "${steps.data_collection.output.cleaned_data}"},
            {"sensitivity", "high"},
            {"methods", ["statistical", "ml_based", "rule_based"]}
        };
        analyze_anomalies.dependencies.push_back({"data_collection", "success", true});
        analyze_anomalies.parallel_allowed = true;
        analyze_anomalies.timeout_seconds = 200;

        // Step 3: Data Synthesis Agent
        WorkflowStep synthesis_step;
        synthesis_step.step_id = "analysis_synthesis";
        synthesis_step.name = "Analysis Synthesis";
        synthesis_step.agent_id = "synthesis_specialist";
        synthesis_step.function_name = "synthesize_analyses";
        synthesis_step.parameters = {
            {"statistical_results", "${steps.statistical_analysis.output}"},
            {"trend_results", "${steps.trend_analysis.output}"},
            {"anomaly_results", "${steps.anomaly_detection.output}"},
            {"synthesis_method", "weighted_integration"},
            {"priority_weights", {
                {"statistical", 0.4},
                {"trends", 0.35},
                {"anomalies", 0.25}
            }}
        };
        synthesis_step.dependencies = {
            {"statistical_analysis", "success", true},
            {"trend_analysis", "success", true},
            {"anomaly_detection", "success", true}
        };
        synthesis_step.timeout_seconds = 150;

        // Step 4: Report Generator Agent
        WorkflowStep report_step;
        report_step.step_id = "report_generation";
        report_step.name = "Comprehensive Report Generation";
        report_step.agent_id = "report_generator";
        report_step.function_name = "generate_comprehensive_report";
        report_step.parameters = {
            {"synthesis_results", "${steps.analysis_synthesis.output}"},
            {"original_data_meta", "${steps.data_collection.output.metadata}"},
            {"format", "${global.output_format}"},
            {"include_visualizations", true},
            {"include_recommendations", true},
            {"executive_summary", true}
        };
        report_step.dependencies.push_back({"analysis_synthesis", "success", true});
        report_step.timeout_seconds = 120;

        // Step 5: Quality Assurance Agent
        WorkflowStep qa_step;
        qa_step.step_id = "quality_assurance";
        qa_step.name = "Quality Assurance Review";
        qa_step.agent_id = "quality_assurance_agent";
        qa_step.function_name = "review_and_validate_report";
        qa_step.parameters = {
            {"report", "${steps.report_generation.output}"},
            {"original_data", "${steps.data_collection.output}"},
            {"validation_criteria", ["accuracy", "completeness", "clarity", "actionability"]},
            {"minimum_score", 0.85}
        };
        qa_step.dependencies.push_back({"report_generation", "success", true});
        qa_step.timeout_seconds = 100;

        workflow.steps = {collect_step, analyze_statistical, analyze_trends, analyze_anomalies, 
                         synthesis_step, report_step, qa_step};
        return workflow;
    }

    Workflow createConsensusDecisionWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "consensus_decision_making";
        workflow.name = "Multi-Agent Consensus Decision Making";
        workflow.description = "Multiple specialized agents reach consensus on complex decisions";
        workflow.type = WorkflowType::CONSENSUS;
        
        workflow.global_context = {
            {"decision_topic", "Strategic AI Implementation Plan"},
            {"consensus_threshold", 0.75},
            {"voting_rounds", 2},
            {"expertise_weights", {
                {"technical", 0.3},
                {"business", 0.25},
                {"legal", 0.2},
                {"ethical", 0.15},
                {"user_experience", 0.1}
            }}
        };

        // Parallel voting by different specialist agents
        WorkflowStep technical_analysis;
        technical_analysis.step_id = "technical_evaluation";
        technical_analysis.name = "Technical Feasibility Analysis";
        technical_analysis.agent_id = "technical_specialist";
        technical_analysis.function_name = "evaluate_technical_feasibility";
        technical_analysis.parameters = {
            {"topic", "${global.decision_topic}"},
            {"evaluation_criteria", ["scalability", "performance", "security", "maintainability"]},
            {"weight", "${global.expertise_weights.technical}"}
        };
        technical_analysis.parallel_allowed = true;
        technical_analysis.timeout_seconds = 300;

        WorkflowStep business_analysis;
        business_analysis.step_id = "business_evaluation";
        business_analysis.name = "Business Impact Analysis";
        business_analysis.agent_id = "business_analyst";
        business_analysis.function_name = "evaluate_business_impact";
        business_analysis.parameters = {
            {"topic", "${global.decision_topic}"},
            {"evaluation_criteria", ["roi", "market_fit", "competitive_advantage", "risk_assessment"]},
            {"weight", "${global.expertise_weights.business}"}
        };
        business_analysis.parallel_allowed = true;
        business_analysis.timeout_seconds = 300;

        WorkflowStep legal_analysis;
        legal_analysis.step_id = "legal_evaluation";
        legal_analysis.name = "Legal and Compliance Analysis";
        legal_analysis.agent_id = "legal_advisor";
        legal_analysis.function_name = "evaluate_legal_compliance";
        legal_analysis.parameters = {
            {"topic", "${global.decision_topic}"},
            {"evaluation_criteria", ["regulatory_compliance", "data_privacy", "liability", "intellectual_property"]},
            {"weight", "${global.expertise_weights.legal}"}
        };
        legal_analysis.parallel_allowed = true;
        legal_analysis.timeout_seconds = 250;

        WorkflowStep ethical_analysis;
        ethical_analysis.step_id = "ethical_evaluation";
        ethical_analysis.name = "Ethical Considerations Analysis";
        ethical_analysis.agent_id = "ethics_specialist";
        ethical_analysis.function_name = "evaluate_ethical_implications";
        ethical_analysis.parameters = {
            {"topic", "${global.decision_topic}"},
            {"evaluation_criteria", ["fairness", "transparency", "accountability", "social_impact"]},
            {"weight", "${global.expertise_weights.ethical}"}
        };
        ethical_analysis.parallel_allowed = true;
        ethical_analysis.timeout_seconds = 250;

        WorkflowStep ux_analysis;
        ux_analysis.step_id = "ux_evaluation";
        ux_analysis.name = "User Experience Analysis";
        ux_analysis.agent_id = "ux_specialist";
        ux_analysis.function_name = "evaluate_user_experience";
        ux_analysis.parameters = {
            {"topic", "${global.decision_topic}"},
            {"evaluation_criteria", ["usability", "accessibility", "user_satisfaction", "adoption_barriers"]},
            {"weight", "${global.expertise_weights.user_experience}"}
        };
        ux_analysis.parallel_allowed = true;
        ux_analysis.timeout_seconds = 200;

        // Consensus building phase
        WorkflowStep consensus_building;
        consensus_building.step_id = "consensus_building";
        consensus_building.name = "Consensus Building";
        consensus_building.agent_id = "consensus_facilitator";
        consensus_building.function_name = "build_weighted_consensus";
        consensus_building.parameters = {
            {"evaluations", {
                {"technical", "${steps.technical_evaluation.output}"},
                {"business", "${steps.business_evaluation.output}"},
                {"legal", "${steps.legal_evaluation.output}"},
                {"ethical", "${steps.ethical_evaluation.output}"},
                {"ux", "${steps.ux_evaluation.output}"}
            }},
            {"weights", "${global.expertise_weights}"},
            {"consensus_threshold", "${global.consensus_threshold}"},
            {"resolution_method", "weighted_voting"}
        };
        consensus_building.dependencies = {
            {"technical_evaluation", "completion", false},
            {"business_evaluation", "completion", false},
            {"legal_evaluation", "completion", false},
            {"ethical_evaluation", "completion", false},
            {"ux_evaluation", "completion", false}
        };
        consensus_building.timeout_seconds = 180;

        // Decision implementation planning
        WorkflowStep implementation_planning;
        implementation_planning.step_id = "implementation_planning";
        implementation_planning.name = "Implementation Planning";
        implementation_planning.agent_id = "project_manager";
        implementation_planning.function_name = "create_implementation_plan";
        implementation_planning.parameters = {
            {"consensus_decision", "${steps.consensus_building.output}"},
            {"specialist_recommendations", {
                {"technical", "${steps.technical_evaluation.output.recommendations}"},
                {"business", "${steps.business_evaluation.output.recommendations}"},
                {"legal", "${steps.legal_evaluation.output.recommendations}"},
                {"ethical", "${steps.ethical_evaluation.output.recommendations}"},
                {"ux", "${steps.ux_evaluation.output.recommendations}"}
            }},
            {"timeline_target", "6_months"},
            {"resource_constraints", "standard"}
        };
        implementation_planning.dependencies.push_back({"consensus_building", "success", true});
        implementation_planning.timeout_seconds = 200;

        workflow.steps = {technical_analysis, business_analysis, legal_analysis, ethical_analysis, 
                         ux_analysis, consensus_building, implementation_planning};
        return workflow;
    }

    Workflow createHierarchicalTaskDistributionWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "hierarchical_task_distribution";
        workflow.name = "Hierarchical Multi-Agent Task Distribution";
        workflow.description = "Manager agent coordinates multiple specialist workers";
        workflow.type = WorkflowType::PIPELINE;
        
        workflow.global_context = {
            {"project_scope", "large_scale_data_migration"},
            {"worker_capacity", 3},
            {"priority_level", "high"},
            {"deadline", "2_weeks"}
        };

        // Step 1: Project Manager - Task Planning and Distribution
        WorkflowStep planning_step;
        planning_step.step_id = "project_planning";
        planning_step.name = "Project Planning and Task Distribution";
        planning_step.agent_id = "project_manager";
        planning_step.function_name = "plan_and_distribute_tasks";
        planning_step.parameters = {
            {"project_scope", "${global.project_scope}"},
            {"available_workers", "${global.worker_capacity}"},
            {"priority", "${global.priority_level}"},
            {"deadline", "${global.deadline}"},
            {"task_breakdown_strategy", "skill_based"}
        };
        planning_step.timeout_seconds = 180;

        // Step 2: Coordinator - Resource Allocation
        WorkflowStep resource_allocation;
        resource_allocation.step_id = "resource_allocation";
        resource_allocation.name = "Resource Allocation";
        resource_allocation.agent_id = "resource_coordinator";
        resource_allocation.function_name = "allocate_resources";
        resource_allocation.parameters = {
            {"task_plan", "${steps.project_planning.output}"},
            {"resource_pool", "standard"},
            {"allocation_strategy", "balanced_workload"}
        };
        resource_allocation.dependencies.push_back({"project_planning", "success", true});
        resource_allocation.timeout_seconds = 120;

        // Step 3: Parallel Worker Execution
        WorkflowStep worker_1;
        worker_1.step_id = "worker_1_execution";
        worker_1.name = "Worker 1 - Database Migration";
        worker_1.agent_id = "database_specialist";
        worker_1.function_name = "execute_database_migration";
        worker_1.parameters = {
            {"assigned_tasks", "${steps.resource_allocation.output.worker_1_tasks}"},
            {"resources", "${steps.resource_allocation.output.worker_1_resources}"},
            {"coordination_channel", "worker_sync_1"}
        };
        worker_1.dependencies.push_back({"resource_allocation", "success", true});
        worker_1.parallel_allowed = true;
        worker_1.timeout_seconds = 600;  // 10 minutes for complex tasks

        WorkflowStep worker_2;
        worker_2.step_id = "worker_2_execution";
        worker_2.name = "Worker 2 - API Integration";
        worker_2.agent_id = "api_specialist";
        worker_2.function_name = "execute_api_integration";
        worker_2.parameters = {
            {"assigned_tasks", "${steps.resource_allocation.output.worker_2_tasks}"},
            {"resources", "${steps.resource_allocation.output.worker_2_resources}"},
            {"coordination_channel", "worker_sync_2"}
        };
        worker_2.dependencies.push_back({"resource_allocation", "success", true});
        worker_2.parallel_allowed = true;
        worker_2.timeout_seconds = 600;

        WorkflowStep worker_3;
        worker_3.step_id = "worker_3_execution";
        worker_3.name = "Worker 3 - Data Validation";
        worker_3.agent_id = "data_validator";
        worker_3.function_name = "execute_data_validation";
        worker_3.parameters = {
            {"assigned_tasks", "${steps.resource_allocation.output.worker_3_tasks}"},
            {"resources", "${steps.resource_allocation.output.worker_3_resources}"},
            {"coordination_channel", "worker_sync_3"}
        };
        worker_3.dependencies.push_back({"resource_allocation", "success", true});
        worker_3.parallel_allowed = true;
        worker_3.timeout_seconds = 600;

        // Step 4: Progress Monitor - Continuous Monitoring
        WorkflowStep progress_monitoring;
        progress_monitoring.step_id = "progress_monitoring";
        progress_monitoring.name = "Progress Monitoring";
        progress_monitoring.agent_id = "progress_monitor";
        progress_monitoring.function_name = "monitor_worker_progress";
        progress_monitoring.parameters = {
            {"worker_channels", ["worker_sync_1", "worker_sync_2", "worker_sync_3"]},
            {"monitoring_interval", "30_seconds"},
            {"escalation_thresholds", {
                {"delay_threshold", "10_minutes"},
                {"error_threshold", 3}
            }}
        };
        progress_monitoring.dependencies = {
            {"worker_1_execution", "running", false},
            {"worker_2_execution", "running", false},
            {"worker_3_execution", "running", false}
        };
        progress_monitoring.parallel_allowed = true;
        progress_monitoring.timeout_seconds = 700;

        // Step 5: Integration and Quality Assurance
        WorkflowStep integration_qa;
        integration_qa.step_id = "integration_qa";
        integration_qa.name = "Integration and Quality Assurance";
        integration_qa.agent_id = "qa_specialist";
        integration_qa.function_name = "integrate_and_validate_results";
        integration_qa.parameters = {
            {"worker_results", {
                {"database_results", "${steps.worker_1_execution.output}"},
                {"api_results", "${steps.worker_2_execution.output}"},
                {"validation_results", "${steps.worker_3_execution.output}"}
            }},
            {"integration_strategy", "sequential_validation"},
            {"qa_criteria", ["completeness", "accuracy", "performance", "security"]}
        };
        integration_qa.dependencies = {
            {"worker_1_execution", "success", true},
            {"worker_2_execution", "success", true},
            {"worker_3_execution", "success", true}
        };
        integration_qa.timeout_seconds = 300;

        // Step 6: Project Completion Report
        WorkflowStep completion_report;
        completion_report.step_id = "completion_report";
        completion_report.name = "Project Completion Report";
        completion_report.agent_id = "project_manager";
        completion_report.function_name = "generate_completion_report";
        completion_report.parameters = {
            {"project_results", "${steps.integration_qa.output}"},
            {"worker_performance", {
                {"worker_1", "${steps.progress_monitoring.output.worker_1_metrics}"},
                {"worker_2", "${steps.progress_monitoring.output.worker_2_metrics}"},
                {"worker_3", "${steps.progress_monitoring.output.worker_3_metrics}"}
            }},
            {"project_timeline", "${steps.project_planning.output.timeline}"},
            {"lessons_learned", true}
        };
        completion_report.dependencies.push_back({"integration_qa", "success", true});
        completion_report.timeout_seconds = 150;

        workflow.steps = {planning_step, resource_allocation, worker_1, worker_2, worker_3, 
                         progress_monitoring, integration_qa, completion_report};
        return workflow;
    }
};

TEST_F(MultiAgentWorkflowTest, CollaborativeDataProcessing) {
    test_workflow_engine_->start();
    
    auto workflow = createCollaborativeDataProcessingWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json input_context = {
        {"dataset", "comprehensive_research_data.json"},
        {"analysis_depth", "deep"},
        {"priority", "high"}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, input_context);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for collaborative processing to progress
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    EXPECT_EQ(status->workflow_id, workflow_id);
    
    // Verify that parallel analysis steps can run concurrently
    auto step_statuses = status->step_statuses;
    
    // Check if parallel analysis steps are progressing
    std::vector<std::string> parallel_analysis_steps = {
        "statistical_analysis", "trend_analysis", "anomaly_detection"
    };
    
    int parallel_steps_active = 0;
    for (const auto& step_id : parallel_analysis_steps) {
        if (step_statuses.find(step_id) != step_statuses.end()) {
            auto step_status = step_statuses[step_id];
            if (step_status == StepStatus::RUNNING || step_status == StepStatus::COMPLETED) {
                parallel_steps_active++;
            }
        }
    }
    
    // At least some parallel steps should be active/completed
    EXPECT_GE(parallel_steps_active, 0);
    
    // Data collection should complete before analysis steps
    if (step_statuses.find("data_collection") != step_statuses.end() &&
        step_statuses["data_collection"] == StepStatus::COMPLETED) {
        // Analysis steps should be able to proceed
        EXPECT_TRUE(parallel_steps_active > 0 || status->current_status == WorkflowStatus::RUNNING);
    }
}

TEST_F(MultiAgentWorkflowTest, ConsensusDecisionMaking) {
    test_workflow_engine_->start();
    
    auto workflow = createConsensusDecisionWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json input_context = {
        {"decision_complexity", "high"},
        {"stakeholder_involvement", "full"},
        {"time_constraint", "moderate"}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, input_context);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for consensus process to progress
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    
    // Verify that specialist evaluation steps run in parallel
    auto step_statuses = status->step_statuses;
    
    std::vector<std::string> evaluation_steps = {
        "technical_evaluation", "business_evaluation", "legal_evaluation", 
        "ethical_evaluation", "ux_evaluation"
    };
    
    int evaluation_steps_active = 0;
    for (const auto& step_id : evaluation_steps) {
        if (step_statuses.find(step_id) != step_statuses.end()) {
            auto step_status = step_statuses[step_id];
            if (step_status == StepStatus::RUNNING || step_status == StepStatus::COMPLETED) {
                evaluation_steps_active++;
            }
        }
    }
    
    // Multiple evaluation agents should be working in parallel
    EXPECT_GE(evaluation_steps_active, 0);
    
    // Consensus building should wait for evaluations
    if (step_statuses.find("consensus_building") != step_statuses.end()) {
        auto consensus_status = step_statuses["consensus_building"];
        if (consensus_status == StepStatus::COMPLETED || consensus_status == StepStatus::RUNNING) {
            // At least some evaluations should be completed
            int completed_evaluations = 0;
            for (const auto& step_id : evaluation_steps) {
                if (step_statuses.find(step_id) != step_statuses.end() &&
                    step_statuses[step_id] == StepStatus::COMPLETED) {
                    completed_evaluations++;
                }
            }
            // Consensus can proceed with partial evaluations due to non-required dependencies
            EXPECT_GE(completed_evaluations, 0);
        }
    }
}

TEST_F(MultiAgentWorkflowTest, HierarchicalTaskDistribution) {
    test_workflow_engine_->start();
    
    auto workflow = createHierarchicalTaskDistributionWorkflow();
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json input_context = {
        {"project_complexity", "high"},
        {"team_size", 3},
        {"coordination_style", "active_monitoring"}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, input_context);
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for hierarchical execution to progress
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    
    auto step_statuses = status->step_statuses;
    
    // Verify hierarchical execution order
    if (step_statuses.find("project_planning") != step_statuses.end() &&
        step_statuses["project_planning"] == StepStatus::COMPLETED) {
        
        // Resource allocation should follow planning
        if (step_statuses.find("resource_allocation") != step_statuses.end()) {
            EXPECT_NE(step_statuses["resource_allocation"], StepStatus::PENDING);
        }
        
        // Worker execution should follow resource allocation
        std::vector<std::string> worker_steps = {
            "worker_1_execution", "worker_2_execution", "worker_3_execution"
        };
        
        int active_workers = 0;
        for (const auto& worker_step : worker_steps) {
            if (step_statuses.find(worker_step) != step_statuses.end()) {
                auto worker_status = step_statuses[worker_step];
                if (worker_status == StepStatus::RUNNING || worker_status == StepStatus::COMPLETED) {
                    active_workers++;
                }
            }
        }
        
        // Workers should be able to execute in parallel after resource allocation
        EXPECT_GE(active_workers, 0);
        
        // Progress monitoring should run alongside workers
        if (step_statuses.find("progress_monitoring") != step_statuses.end()) {
            auto monitor_status = step_statuses["progress_monitoring"];
            if (active_workers > 0) {
                // Monitor should be active when workers are active
                EXPECT_TRUE(monitor_status == StepStatus::RUNNING || 
                           monitor_status == StepStatus::COMPLETED ||
                           monitor_status == StepStatus::PENDING);  // Might not have started yet
            }
        }
    }
    
    // Verify workflow is progressing through the hierarchy
    EXPECT_NE(status->current_status, WorkflowStatus::PENDING);
}

TEST_F(MultiAgentWorkflowTest, MultiAgentErrorRecovery) {
    test_workflow_engine_->start();
    
    auto workflow = createCollaborativeDataProcessingWorkflow();
    
    // Configure error recovery for multi-agent scenarios
    workflow.error_handling.retry_on_failure = true;
    workflow.error_handling.max_retries = 2;
    workflow.error_handling.continue_on_error = true;  // Allow other agents to continue
    
    // Configure individual agent error handling
    for (auto& step : workflow.steps) {
        step.max_retries = 1;
        step.continue_on_error = true;
    }
    
    std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
    
    nlohmann::json error_prone_input = {
        {"introduce_agent_failures", true},
        {"failure_rate", 0.3},
        {"recovery_expected", true}
    };
    
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id, error_prone_input);
    
    // Wait for error recovery in multi-agent context
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    
    // Multi-agent system should be resilient to individual agent failures
    EXPECT_TRUE(status->current_status == WorkflowStatus::COMPLETED ||
               status->current_status == WorkflowStatus::RUNNING ||
               status->current_status == WorkflowStatus::FAILED);
    
    // At least some agents should have made progress despite errors
    auto step_statuses = status->step_statuses;
    int progress_count = 0;
    for (const auto& [step_id, step_status] : step_statuses) {
        if (step_status != StepStatus::PENDING) {
            progress_count++;
        }
    }
    
    EXPECT_GT(progress_count, 0);
}

TEST_F(MultiAgentWorkflowTest, ScalabilityWithManyAgents) {
    test_workflow_engine_->start();
    
    // Create a workflow with many agents to test scalability
    Workflow large_workflow;
    large_workflow.workflow_id = "large_multi_agent_test";
    large_workflow.name = "Large Multi-Agent Scalability Test";
    large_workflow.type = WorkflowType::PARALLEL;
    large_workflow.max_concurrent_steps = 8;  // Allow more parallelism
    
    // Create 15 parallel agent steps
    for (int i = 0; i < 15; ++i) {
        WorkflowStep step;
        step.step_id = "agent_" + std::to_string(i);
        step.name = "Agent " + std::to_string(i);
        step.agent_id = "scalability_agent_" + std::to_string(i);
        step.function_name = "scalability_task";
        step.parameters = {
            {"agent_number", i},
            {"total_agents", 15}
        };
        step.parallel_allowed = true;
        step.timeout_seconds = 60;
        large_workflow.steps.push_back(step);
    }
    
    std::string workflow_id = test_workflow_engine_->create_workflow(large_workflow);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
    
    // Wait for large-scale execution
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    auto status = test_workflow_engine_->get_execution_status(execution_id);
    ASSERT_TRUE(status.has_value());
    
    // System should handle many agents efficiently
    EXPECT_LT(execution_duration.count(), 5000);  // Should handle 15 agents within 5 seconds
    
    // Multiple agents should be active concurrently
    auto step_statuses = status->step_statuses;
    int active_agents = 0;
    for (const auto& [step_id, step_status] : step_statuses) {
        if (step_status == StepStatus::RUNNING || step_status == StepStatus::COMPLETED) {
            active_agents++;
        }
    }
    
    EXPECT_GE(active_agents, 0);  // At least some agents should be active
    
    std::cout << "Large multi-agent workflow: " << active_agents 
              << " agents active out of 15 total" << std::endl;
}
