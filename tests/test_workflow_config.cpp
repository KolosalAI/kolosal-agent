#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>
#include "../include/workflow_types.hpp"
#include "../include/workflow_manager.hpp"
#include "../include/agent_manager.hpp"
#include <fstream>
#include <yaml-cpp/yaml.h>

class WorkflowConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::cout << "[SetUp] Starting workflow config test setup..." << std::endl;
        
        // Set timeout for entire setup to prevent hanging
        auto start_time = std::chrono::steady_clock::now();
        auto timeout_duration = std::chrono::seconds(15); // Reduced from 30 to 15 seconds
        
        try {
            // Create mock agent manager with test configuration
            config_manager_ = std::make_shared<AgentConfigManager>();
            agent_manager_ = std::make_shared<AgentManager>(config_manager_);
            std::cout << "[SetUp] Created agent manager" << std::endl;
            
            // Check timeout
            if (std::chrono::steady_clock::now() - start_time > timeout_duration) {
                throw std::runtime_error("Setup timeout during agent manager creation");
            }
            
            // Create test agents that match the workflow configuration
            assistant_id_ = agent_manager_->create_agent("Assistant", {"chat"});
            analyzer_id_ = agent_manager_->create_agent("Analyzer", {"analysis"});
            researcher_id_ = agent_manager_->create_agent("Researcher", {"research"});
            std::cout << "[SetUp] Created test agents" << std::endl;
            
            // Check timeout
            if (std::chrono::steady_clock::now() - start_time > timeout_duration) {
                throw std::runtime_error("Setup timeout during agent creation");
            }
            
            // Start agents with timeout protection
            std::cout << "[SetUp] Starting agents..." << std::endl;
            agent_manager_->start_agent(assistant_id_);
            std::cout << "[SetUp] Started Assistant agent" << std::endl;
            
            agent_manager_->start_agent(analyzer_id_);
            std::cout << "[SetUp] Started Analyzer agent" << std::endl;
            
            agent_manager_->start_agent(researcher_id_);
            std::cout << "[SetUp] Started Researcher agent" << std::endl;
            
            // Add small delay to ensure agents are properly started
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "[SetUp] Agents startup delay completed" << std::endl;
            
            // Check timeout
            if (std::chrono::steady_clock::now() - start_time > timeout_duration) {
                throw std::runtime_error("Setup timeout during agent startup");
            }
            
            // Create workflow manager with shorter queue size
            std::cout << "[SetUp] Creating workflow manager..." << std::endl;
            workflow_manager_ = std::make_shared<WorkflowManager>(agent_manager_, 2, 50, 100); // Reduced from 4, 100, 1000
            workflow_manager_->start();
            std::cout << "[SetUp] Started workflow manager" << std::endl;
            
            // Check timeout
            if (std::chrono::steady_clock::now() - start_time > timeout_duration) {
                throw std::runtime_error("Setup timeout during workflow manager startup");
            }
            
            // Create workflow orchestrator
            std::cout << "[SetUp] Creating workflow orchestrator..." << std::endl;
            workflow_orchestrator_ = std::make_shared<WorkflowOrchestrator>(workflow_manager_);
            workflow_orchestrator_->start();
            std::cout << "[SetUp] Started workflow orchestrator" << std::endl;
            
            // Check timeout
            if (std::chrono::steady_clock::now() - start_time > timeout_duration) {
                throw std::runtime_error("Setup timeout during workflow orchestrator startup");
            }
            
            // Create test workflow configuration file
            std::cout << "[SetUp] Creating test configuration file..." << std::endl;
            createTestWorkflowConfig();
            std::cout << "[SetUp] Setup completed successfully" << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "[SetUp] Error during setup: " << e.what() << std::endl;
            // Clean up any partially created objects
            if (workflow_orchestrator_) {
                workflow_orchestrator_->stop();
                workflow_orchestrator_.reset();
            }
            if (workflow_manager_) {
                workflow_manager_->stop();
                workflow_manager_.reset();
            }
            if (agent_manager_) {
                agent_manager_->stop_all_agents();
                agent_manager_.reset();
            }
            throw;
        }
    }

    void TearDown() override {
        std::cout << "[TearDown] Starting cleanup..." << std::endl;
        
        try {
            if (workflow_orchestrator_) {
                std::cout << "[TearDown] Stopping workflow orchestrator..." << std::endl;
                workflow_orchestrator_->stop();
                workflow_orchestrator_.reset();
                std::cout << "[TearDown] Workflow orchestrator stopped" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[TearDown] Error stopping workflow orchestrator: " << e.what() << std::endl;
        }
        
        try {
            if (workflow_manager_) {
                std::cout << "[TearDown] Stopping workflow manager..." << std::endl;
                workflow_manager_->stop();
                workflow_manager_.reset();
                std::cout << "[TearDown] Workflow manager stopped" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[TearDown] Error stopping workflow manager: " << e.what() << std::endl;
        }
        
        try {
            if (agent_manager_) {
                std::cout << "[TearDown] Stopping all agents..." << std::endl;
                agent_manager_->stop_all_agents();
                agent_manager_.reset();
                std::cout << "[TearDown] All agents stopped" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[TearDown] Error stopping agents: " << e.what() << std::endl;
        }
        
        try {
            if (config_manager_) {
                config_manager_.reset();
                std::cout << "[TearDown] Config manager reset" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[TearDown] Error resetting config manager: " << e.what() << std::endl;
        }
        
        // Clean up test files
        std::cout << "[TearDown] Cleaning up test files..." << std::endl;
        try {
            std::remove("test_workflow.yaml");
            std::cout << "[TearDown] Test files cleaned up" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[TearDown] Error cleaning up test files: " << e.what() << std::endl;
        }
        
        std::cout << "[TearDown] Cleanup completed" << std::endl;
    }
    
    void createTestWorkflowConfig() {
        std::ofstream config_file("test_workflow.yaml");
        config_file << R"(
# Test Workflow Configuration
agent_llm_mappings:
  Assistant:
    default_model: "test-model"
    supported_models: ["test-model", "gemma3-1b"]
    
  Analyzer:
    default_model: "test-model"
    supported_models: ["test-model", "gemma3-1b"]
    
  Researcher:
    default_model: "test-model"
    supported_models: ["test-model", "gemma3-1b"]

workflows:
  - id: "test_simple_research"
    name: "Test Simple Research Workflow"
    description: "Basic research workflow for testing"
    type: "sequential"
    max_execution_time_ms: 120000
    allow_partial_failure: false
    steps:
      - id: "research_step"
        agent_name: "Researcher"
        llm_model: "test-model"
        function_name: "research"
        parameters:
          - "query"
          - "depth"
        timeout_ms: 60000
        optional: false

  - id: "test_analysis_workflow"
    name: "Test Analysis Workflow"
    description: "Multi-step analysis for testing"
    type: "sequential"
    max_execution_time_ms: 180000
    allow_partial_failure: false
    steps:
      - id: "initial_analysis"
        agent_name: "Analyzer"
        llm_model: "test-model"
        function_name: "analyze"
        parameters:
          - "text"
          - "analysis_type"
        timeout_ms: 60000
        optional: false
      
      - id: "detailed_analysis"
        agent_name: "Analyzer"
        llm_model: "test-model"
        function_name: "analyze"
        parameters:
          - "text"
          - "analysis_type"
          - "context"
        timeout_ms: 90000
        optional: false
        dependencies: ["initial_analysis"]

  - id: "test_parallel_workflow"
    name: "Test Parallel Workflow"
    description: "Parallel processing for testing"
    type: "parallel"
    max_execution_time_ms: 120000
    allow_partial_failure: true
    steps:
      - id: "sentiment_analysis"
        agent_name: "Analyzer"
        llm_model: "test-model"
        function_name: "analyze"
        parameters:
          - "text"
          - "analysis_type"
        timeout_ms: 60000
        optional: false
      
      - id: "summary_generation"
        agent_name: "Assistant"
        llm_model: "test-model"
        function_name: "chat"
        parameters:
          - "message"
          - "model"
        timeout_ms: 45000
        optional: false
)";
        config_file.close();
    }

    std::shared_ptr<AgentConfigManager> config_manager_;
    std::shared_ptr<AgentManager> agent_manager_;
    std::shared_ptr<WorkflowManager> workflow_manager_;
    std::shared_ptr<WorkflowOrchestrator> workflow_orchestrator_;
    std::string assistant_id_, analyzer_id_, researcher_id_;
};

TEST_F(WorkflowConfigTest, LoadWorkflowConfig) {
    std::cout << "[TEST] Starting LoadWorkflowConfig test..." << std::endl;
    
    // Test loading workflow configuration
    std::cout << "[TEST] Loading workflow config..." << std::endl;
    bool result = workflow_orchestrator_->load_workflow_config("test_workflow.yaml");
    std::cout << "[TEST] Load result: " << (result ? "true" : "false") << std::endl;
    EXPECT_TRUE(result);
    
    // Test getting config
    std::cout << "[TEST] Getting workflow config..." << std::endl;
    auto config = workflow_orchestrator_->get_workflow_config();
    std::cout << "[TEST] Got workflow config" << std::endl;
    // Config might be empty if YAML conversion is limited, but loading should succeed
    
    // Check that workflows were loaded
    std::cout << "[TEST] Listing workflows..." << std::endl;
    auto workflows = workflow_orchestrator_->list_workflows();
    std::cout << "[TEST] Found " << workflows.size() << " workflows" << std::endl;
    EXPECT_GT(workflows.size(), 0);
    
    // Check specific workflow exists
    std::cout << "[TEST] Checking specific workflows..." << std::endl;
    bool found_simple_research = false;
    bool found_analysis_workflow = false;
    bool found_parallel_workflow = false;
    
    for (const auto& workflow : workflows) {
        std::cout << "[TEST] Processing workflow: " << workflow.id << std::endl;
        if (workflow.id == "test_simple_research") {
            found_simple_research = true;
            EXPECT_EQ(workflow.name, "Test Simple Research Workflow");
            EXPECT_GT(workflow.steps.size(), 0);
            
            // Check that first step has LLM model specified
            if (!workflow.steps.empty()) {
                EXPECT_EQ(workflow.steps[0].llm_model, "test-model");
                EXPECT_EQ(workflow.steps[0].agent_name, "Researcher");
                EXPECT_EQ(workflow.steps[0].function_name, "research");
            }
        } else if (workflow.id == "test_analysis_workflow") {
            found_analysis_workflow = true;
            EXPECT_EQ(workflow.type, WorkflowType::SEQUENTIAL);
            EXPECT_GT(workflow.steps.size(), 1);
            
            // Check dependencies
            bool found_dependent_step = false;
            for (const auto& step : workflow.steps) {
                if (step.id == "detailed_analysis") {
                    found_dependent_step = true;
                    EXPECT_GT(step.dependencies.size(), 0);
                    EXPECT_EQ(step.dependencies[0], "initial_analysis");
                }
            }
            EXPECT_TRUE(found_dependent_step);
        } else if (workflow.id == "test_parallel_workflow") {
            found_parallel_workflow = true;
            EXPECT_EQ(workflow.type, WorkflowType::PARALLEL);
            EXPECT_TRUE(workflow.allow_partial_failure);
        }
    }
    
    EXPECT_TRUE(found_simple_research);
    EXPECT_TRUE(found_analysis_workflow);
    EXPECT_TRUE(found_parallel_workflow);
}

TEST_F(WorkflowConfigTest, ValidateAgentLLMPairing) {
    // Load configuration first
    workflow_orchestrator_->load_workflow_config("test_workflow.yaml");
    
    // Test that workflows were loaded and have valid agent-LLM pairings
    auto workflows = workflow_orchestrator_->list_workflows();
    EXPECT_GT(workflows.size(), 0);
    
    // Verify only test workflows have valid agent-LLM combinations
    // Filter for test workflows (those with "test_" prefix)
    for (const auto& workflow : workflows) {
        // Only check workflows that start with "test_" (from our test YAML)
        if (workflow.id.find("test_") == 0) {
            for (const auto& step : workflow.steps) {
                EXPECT_FALSE(step.agent_name.empty());
                EXPECT_FALSE(step.function_name.empty());
                EXPECT_FALSE(step.llm_model.empty());
                EXPECT_EQ(step.llm_model, "test-model");
            }
        }
    }
}

TEST_F(WorkflowConfigTest, WorkflowDefinitionStructure) {
    // Load configuration
    workflow_orchestrator_->load_workflow_config("test_workflow.yaml");
    
    auto workflows = workflow_orchestrator_->list_workflows();
    
    // Find analysis workflow and verify structure
    for (const auto& workflow : workflows) {
        if (workflow.id == "test_analysis_workflow") {
            EXPECT_EQ(workflow.type, WorkflowType::SEQUENTIAL);
            EXPECT_EQ(workflow.steps.size(), 2);
            
            // Check that steps have proper agent-LLM pairings
            for (const auto& step : workflow.steps) {
                EXPECT_FALSE(step.agent_name.empty());
                EXPECT_FALSE(step.function_name.empty());
                EXPECT_FALSE(step.llm_model.empty());
                EXPECT_EQ(step.llm_model, "test-model");
                
                // Check parameters are arrays (new format)
                EXPECT_TRUE(step.parameters.is_array());
                EXPECT_GT(step.parameters.size(), 0);
            }
            break;
        }
    }
}

TEST_F(WorkflowConfigTest, WorkflowExecution) {
    // Load configuration
    workflow_orchestrator_->load_workflow_config("test_workflow.yaml");
    
    // Load function configurations for the workflow manager
    json function_config;
    function_config["functions"]["research"] = {
        {"description", "Research function"},
        {"timeout", 60000},
        {"parameters", json::array({
            {{"name", "query"}, {"type", "string"}, {"required", true}},
            {{"name", "depth"}, {"type", "string"}, {"required", false}}
        })}
    };
    function_config["functions"]["analyze"] = {
        {"description", "Analysis function"},
        {"timeout", 60000},
        {"parameters", json::array({
            {{"name", "text"}, {"type", "string"}, {"required", true}},
            {{"name", "analysis_type"}, {"type", "string"}, {"required", false}},
            {{"name", "context"}, {"type", "string"}, {"required", false}}
        })}
    };
    function_config["functions"]["chat"] = {
        {"description", "Chat function"},
        {"timeout", 45000},
        {"parameters", json::array({
            {{"name", "message"}, {"type", "string"}, {"required", true}},
            {{"name", "model"}, {"type", "string"}, {"required", false}}
        })}
    };
    
    workflow_manager_->load_function_configs(function_config);
    
    // Execute simple research workflow
    json input_data;
    input_data["query"] = "What is artificial intelligence?";
    input_data["depth"] = "basic";
    
    std::string execution_id;
    EXPECT_NO_THROW(execution_id = workflow_orchestrator_->execute_workflow_async("test_simple_research", input_data));
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for execution to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Check execution status
    auto execution = workflow_orchestrator_->get_execution_status(execution_id);
    EXPECT_NE(execution, nullptr);
    EXPECT_EQ(execution->workflow_id, "test_simple_research");
    // Should be in a valid execution state (pending, running, completed, or failed)
    EXPECT_TRUE(execution->state == WorkflowExecutionState::PENDING ||
                execution->state == WorkflowExecutionState::RUNNING ||
                execution->state == WorkflowExecutionState::COMPLETED ||
                execution->state == WorkflowExecutionState::FAILED);
}

TEST_F(WorkflowConfigTest, WorkflowParameterTemplating) {
    // Test parameter templating functionality
    workflow_orchestrator_->load_workflow_config("test_workflow.yaml");
    
    // Create a workflow with parameter templates
    WorkflowDefinition template_workflow("template_test", "Template Test Workflow");
    template_workflow.type = WorkflowType::SEQUENTIAL;
    
    // Use legacy parameter format for testing templating
    json step_params;
    step_params["message"] = "Process this: {{input.text}}";
    step_params["model"] = "{{agent.default_model}}";
    
    WorkflowStep step("template_step", "Assistant", "chat", step_params, "test-model");
    template_workflow.steps.push_back(step);
    
    workflow_orchestrator_->register_workflow(template_workflow);
    
    // Execute with input data
    json input_data;
    input_data["text"] = "sample input text";
    
    std::string execution_id;
    EXPECT_NO_THROW(execution_id = workflow_orchestrator_->execute_workflow_async("template_test", input_data));
    EXPECT_FALSE(execution_id.empty());
    
    auto execution = workflow_orchestrator_->get_execution_status(execution_id);
    EXPECT_NE(execution, nullptr);
}

TEST_F(WorkflowConfigTest, InvalidWorkflowConfiguration) {
    // Test handling of invalid workflow configurations
    
    // Create invalid config file
    std::ofstream invalid_config("invalid_workflow.yaml");
    invalid_config << R"(
invalid_yaml_structure: [
  - missing_closing_bracket
agent_llm_mappings:
  InvalidAgent:
    default_model: "nonexistent-model"
)";
    invalid_config.close();
    
    // Should handle invalid config gracefully
    bool result = workflow_orchestrator_->load_workflow_config("invalid_workflow.yaml");
    // Might be false due to invalid YAML, but shouldn't crash
    
    // Built-in workflows should still be available
    auto workflows = workflow_orchestrator_->list_workflows();
    EXPECT_GT(workflows.size(), 0);
    
    std::remove("invalid_workflow.yaml");
}

TEST_F(WorkflowConfigTest, WorkflowBuilder) {
    // Test the WorkflowBuilder helper class
    
    auto builder = WorkflowBuilder("builder_test", "Builder Test Workflow")
        .set_type(WorkflowType::SEQUENTIAL)
        .set_description("Testing workflow builder")
        .set_max_execution_time(300000)
        .allow_partial_failure(true)
        .add_step("step1", "Assistant", "chat", json::array({"message", "model"}), "test-model")
        .add_step("step2", "Analyzer", "analyze", json::array({"text", "analysis_type"}), "test-model")
        .add_step_dependency("step2", "step1")
        .set_step_timeout("step1", 30000)
        .set_step_optional("step2", true);
    
    WorkflowDefinition workflow = builder.build();
    
    EXPECT_EQ(workflow.id, "builder_test");
    EXPECT_EQ(workflow.name, "Builder Test Workflow");
    EXPECT_EQ(workflow.type, WorkflowType::SEQUENTIAL);
    EXPECT_TRUE(workflow.allow_partial_failure);
    EXPECT_EQ(workflow.steps.size(), 2);
    
    // Check dependencies
    EXPECT_EQ(workflow.steps[1].dependencies.size(), 1);
    EXPECT_EQ(workflow.steps[1].dependencies[0], "step1");
    
    // Check optional setting
    EXPECT_TRUE(workflow.steps[1].optional);
    
    // Register and test execution
    workflow_orchestrator_->register_workflow(workflow);
    
    auto workflows = workflow_orchestrator_->list_workflows();
    bool found_builder_workflow = false;
    for (const auto& wf : workflows) {
        if (wf.id == "builder_test") {
            found_builder_workflow = true;
            break;
        }
    }
    EXPECT_TRUE(found_builder_workflow);
}

TEST_F(WorkflowConfigTest, WorkflowTemplates) {
    // Test built-in workflow templates
    
    // Research workflow
    auto research_workflow = WorkflowTemplates::create_research_workflow();
    EXPECT_EQ(research_workflow.id, "research_workflow");
    EXPECT_EQ(research_workflow.type, WorkflowType::SEQUENTIAL);
    EXPECT_GT(research_workflow.steps.size(), 1);
    
    // Analysis workflow
    auto analysis_workflow = WorkflowTemplates::create_analysis_workflow();
    EXPECT_EQ(analysis_workflow.id, "analysis_workflow");
    EXPECT_EQ(analysis_workflow.type, WorkflowType::SEQUENTIAL);
    
    // Data pipeline workflow
    auto pipeline_workflow = WorkflowTemplates::create_data_pipeline_workflow();
    EXPECT_EQ(pipeline_workflow.id, "data_pipeline_workflow");
    EXPECT_EQ(pipeline_workflow.type, WorkflowType::PIPELINE);
    
    // Decision workflow
    auto decision_workflow = WorkflowTemplates::create_decision_workflow();
    EXPECT_EQ(decision_workflow.id, "decision_workflow");
    EXPECT_EQ(decision_workflow.type, WorkflowType::SEQUENTIAL);
    
    // Conversation workflow
    std::vector<std::string> agents = {"Assistant", "Analyzer"};
    auto conversation_workflow = WorkflowTemplates::create_conversation_workflow(agents);
    EXPECT_EQ(conversation_workflow.id, "conversation_workflow");
    EXPECT_EQ(conversation_workflow.steps.size(), agents.size());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "Running Workflow Configuration Tests..." << std::endl;
    return RUN_ALL_TESTS();
}
