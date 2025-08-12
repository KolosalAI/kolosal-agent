/**
 * @file test_fixtures.hpp
 * @brief Common test fixtures for Kolosal Agent tests
 */

#pragma once

// Check if GoogleTest is available
#ifdef GTEST_INCLUDE_GTEST_GTEST_H_
    #include <gtest/gtest.h>
    #include <gmock/gmock.h>
    #define GTEST_AVAILABLE 1
#else
    // Define minimal test infrastructure if GoogleTest is not available
    #define GTEST_AVAILABLE 0
    namespace testing {
        class Test {
        public:
            virtual ~Test() = default;
            virtual void SetUp() {}
            virtual void TearDown() {}
        };
    }
#endif

#include <memory>
#include <string>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include "json.hpp" // Use direct path to nlohmann json
#include "agent/core/agent_core.hpp"
#include "agent/core/agent_roles.hpp"
#include "config/yaml_configuration_parser.hpp"
#include "workflow/workflow_engine.hpp"

// Test directory definitions (these should be defined by CMake)
#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "./tests/fixtures"
#endif

#ifndef TEST_OUTPUT_DIR
#define TEST_OUTPUT_DIR "./build/test_output"
#endif

namespace kolosal::agents::test {

using namespace kolosal::agents;

/**
 * @brief Base test fixture for all Kolosal Agent tests
 */
class KolosalAgentTestFixture : public testing::Test {
protected:
    void SetUp() {
        // Setup test environment
        test_data_dir_ = TEST_DATA_DIR;
        test_output_dir_ = TEST_OUTPUT_DIR;
        
        // Create test configuration
        setupTestConfig();
        
        // Initialize test logger
        initializeTestLogger();
    }
    
    void TearDown() {
        // Cleanup test environment
        cleanupTestFiles();
    }
    
    std::string getTestDataPath(const std::string& filename) {
        return test_data_dir_ + "/" + filename;
    }
    
    std::string getTestOutputPath(const std::string& filename) {
        return test_output_dir_ + "/" + filename;
    }
    
    YAML::Node createBasicAgentConfig(const std::string& id = "test_agent",
                                     const std::string& name = "Test Agent") {
        YAML::Node config;
        config["id"] = id;
        config["name"] = name;
        config["type"] = "generic";
        config["role"] = "assistant";
        config["llm_config"]["model_name"] = "test-model";
        config["llm_config"]["api_endpoint"] = "http://localhost:8080";
        config["llm_config"]["temperature"] = 0.7;
        config["llm_config"]["max_tokens"] = 1024;
        return config;
    }
    
    AgentConfig createTestSystemConfig() {
        AgentConfig config;
        
        // Set up basic agent configuration
        config.id = "test_agent_1";
        config.name = "Test Agent 1";
        config.type = "generic";
        config.role = "assistant";
        config.llm_config.model_name = "test-model";
        config.llm_config.api_endpoint = "http://localhost:8080";
        config.llm_config.temperature = 0.7;
        config.llm_config.max_tokens = 1024;
        
        return config;
    }

protected:
    std::string test_data_dir_;
    std::string test_output_dir_;
    
private:
    void setupTestConfig() {
        // Setup test-specific configuration
    }
    
    void initializeTestLogger() {
        // Initialize test logger
    }
    
    void cleanupTestFiles() {
        // Cleanup test files
    }
};

/**
 * @brief Test fixture for agent-related tests
 */
class AgentTestFixture : public KolosalAgentTestFixture {
protected:
    void SetUp() {
        KolosalAgentTestFixture::SetUp();
        
        // Create test agent
        test_agent_ = std::make_shared<AgentCore>("test_agent", "generic", AgentRole::ASSISTANT);
        
        // Setup test functions
        setupTestFunctions();
    }
    
    void TearDown() {
        if (test_agent_) {
            test_agent_->stop();
            test_agent_.reset();
        }
        KolosalAgentTestFixture::TearDown();
    }

protected:
    std::shared_ptr<AgentCore> test_agent_;
    
private:
    void setupTestFunctions() {
        // Setup test functions for agent
    }
};

/**
 * @brief Test fixture for workflow-related tests
 */
class WorkflowTestFixture : public KolosalAgentTestFixture {
protected:
    void SetUp() {
        KolosalAgentTestFixture::SetUp();
        
        // Create test agent manager
        // test_agent_manager_ = std::make_shared<YAMLConfigurableAgentManager>();
        
        // Create workflow engine
        // test_workflow_engine_ = std::make_shared<WorkflowEngine>(test_agent_manager_);
        
        // Setup test workflows
        setupTestWorkflows();
    }
    
    void TearDown() {
        if (test_workflow_engine_) {
            test_workflow_engine_->stop();
            test_workflow_engine_.reset();
        }
        KolosalAgentTestFixture::TearDown();
    }
    
    Workflow createSimpleSequentialWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "test_sequential_workflow";
        workflow.name = "Test Sequential Workflow";
        workflow.type = WorkflowType::SEQUENTIAL;
        
        WorkflowStep step1;
        step1.step_id = "step1";
        step1.name = "First Step";
        step1.agent_id = "test_agent_1";
        step1.function_name = "echo";
        step1.parameters = nlohmann::json{{"message", "Hello from step 1"}};
        
        WorkflowStep step2;
        step2.step_id = "step2";
        step2.name = "Second Step";
        step2.agent_id = "test_agent_1";
        step2.function_name = "echo";
        step2.parameters = nlohmann::json{{"message", "Hello from step 2"}};
        
        // Create dependency
        StepDependency dep;
        dep.step_id = "step1";
        dep.condition = "success";
        dep.required = true;
        step2.dependencies.push_back(dep);
        
        workflow.steps = {step1, step2};
        return workflow;
    }
    
    Workflow createParallelWorkflow() {
        Workflow workflow;
        workflow.workflow_id = "test_parallel_workflow";
        workflow.name = "Test Parallel Workflow";
        workflow.type = WorkflowType::PARALLEL;
        
        WorkflowStep step1;
        step1.step_id = "step1";
        step1.name = "Parallel Step 1";
        step1.agent_id = "test_agent_1";
        step1.function_name = "echo";
        step1.parameters = nlohmann::json{{"message", "Parallel 1"}};
        step1.parallel_allowed = true;
        
        WorkflowStep step2;
        step2.step_id = "step2";
        step2.name = "Parallel Step 2";
        step2.agent_id = "test_agent_1";
        step2.function_name = "echo";
        step2.parameters = nlohmann::json{{"message", "Parallel 2"}};
        step2.parallel_allowed = true;
        
        workflow.steps = {step1, step2};
        return workflow;
    }

protected:
    // std::shared_ptr<YAMLConfigurableAgentManager> test_agent_manager_;
    std::shared_ptr<WorkflowEngine> test_workflow_engine_;
    
private:
    void setupTestWorkflows() {
        // Setup test workflows
    }
};

/**
 * @brief Test fixture for configuration tests
 */
class ConfigurationTestFixture : public KolosalAgentTestFixture {
protected:
    void SetUp() {
        KolosalAgentTestFixture::SetUp();
        createTestConfigFiles();
    }
    
    void TearDown() {
        cleanupTestConfigFiles();
        KolosalAgentTestFixture::TearDown();
    }
    
    std::string createTempConfigFile(const YAML::Node& config) {
        std::string filename = getTestOutputPath("temp_config_" + std::to_string(temp_file_counter_++) + ".yaml");
        std::ofstream file(filename);
        file << config;
        file.close();
        temp_files_.push_back(filename);
        return filename;
    }

protected:
    std::vector<std::string> temp_files_;
    int temp_file_counter_ = 0;
    
private:
    void createTestConfigFiles() {
        // Create test configuration files
    }
    
    void cleanupTestConfigFiles() {
        for (const auto& file : temp_files_) {
            std::remove(file.c_str());
        }
        temp_files_.clear();
    }
};

} // namespace kolosal::agents::test
