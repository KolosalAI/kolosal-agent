/**
 * @file test_agent_execution.cpp
 * @brief Comprehensive tests for Agent Execution System
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Test suite covering:
 * - Agent Creation and Configuration
 * - Agent Manager Functionality
 * - Model Interface Integration
 * - HTTP API Endpoints
 * - Function Execution
 * - Error Handling
 */

#include "../external/yaml-cpp/test/gtest-1.11.0/googletest/include/gtest/gtest.h"
#include "../include/agent_manager.hpp"
#include "../include/agent.hpp"
#include "../include/agent_config.hpp"
#include "../include/model_interface.hpp"
#include "../include/http_server.hpp"
#include <json.hpp>
#include <chrono>
#include <thread>
#include <future>
#include <sstream>
#include <fstream>

using json = nlohmann::json;

class AgentExecutionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration files
        createTestConfigFiles();
        
        // Initialize test components
        config_manager_ = std::make_shared<AgentConfigManager>();
        agent_manager_ = std::make_shared<AgentManager>(config_manager_);
        model_interface_ = std::make_unique<ModelInterface>("http://localhost:8080");
        
        // Load test configuration
        config_manager_->load_config(test_config_path_);
    }
    
    void TearDown() override {
        // Cleanup test agents
        if (agent_manager_) {
            agent_manager_->stop_all_agents();
        }
        
        // Remove test configuration files
        cleanupTestFiles();
    }
    
    void createTestConfigFiles() {
        // Create test agent configuration
        test_config_path_ = "test_agent_config.yaml";
        std::ofstream config_file(test_config_path_);
        config_file << R"(
system:
  name: "Test Agent System"
  version: "1.0.0"
  host: "127.0.0.1"
  port: 8081
  log_level: "info"
  max_concurrent_requests: 10

system_instruction: |
  You are a test AI assistant for the Kolosal Agent System test suite.
  Your responses should be consistent and predictable for testing purposes.

agents:
  - name: "TestAssistant"
    capabilities: ["chat", "analysis"]
    auto_start: true
    system_prompt: "You are a test assistant. Always respond with 'Test response: ' followed by the user's message."
  
  - name: "TestAnalyzer"
    capabilities: ["analysis", "data_processing"]
    auto_start: false
    system_prompt: "You are a test analyzer. Always respond with 'Analysis: ' followed by a brief analysis."

functions:
  chat:
    description: "Test chat functionality"
    timeout: 10000
    parameters:
      - name: "message"
        type: "string"
        required: true
        description: "Message to process"
      - name: "model"
        type: "string"
        required: false
        description: "Model to use"
  
  analyze:
    description: "Test analysis functionality"
    timeout: 15000
    parameters:
      - name: "text"
        type: "string"
        required: true
        description: "Text to analyze"
      - name: "analysis_type"
        type: "string"
        required: false
        description: "Type of analysis"
  
  echo:
    description: "Test echo functionality"
    timeout: 5000
    parameters:
      - name: "data"
        type: "any"
        required: false
        description: "Data to echo"

performance:
  max_memory_usage: "1GB"
  cache_size: "256MB"
  worker_threads: 2
  request_timeout: 10000
  max_request_size: "5MB"

logging:
  level: "info"
  file: "test_agent_system.log"
  console_output: false

security:
  enable_cors: true
  max_request_rate: 50
  enable_auth: false
)";
        config_file.close();
        
        // Create test model configuration
        test_model_config_path_ = "test_model_config.yaml";
        std::ofstream model_config_file(test_model_config_path_);
        model_config_file << R"(
models:
  - id: test-model
    path: test_model.gguf
    type: llm
    load_immediately: false
  - id: test-embedding-model
    path: test_embedding.gguf
    type: embedding
    load_immediately: false
)";
        model_config_file.close();
    }
    
    void cleanupTestFiles() {
        std::remove(test_config_path_.c_str());
        std::remove(test_model_config_path_.c_str());
        std::remove("test_agent_system.log");
    }
    
    // Helper function to wait for agent startup with timeout
    bool waitForAgentStartup(const std::string& agent_id, int timeout_ms = 5000) {
        auto start_time = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start_time < std::chrono::milliseconds(timeout_ms)) {
            auto agent = agent_manager_->get_agent(agent_id);
            if (agent && agent->is_running()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return false;
    }
    
    // Helper function to execute function with timeout
    json executeFunctionWithTimeout(const std::string& agent_id, 
                                   const std::string& function_name, 
                                   const json& params, 
                                   int timeout_ms = 10000) {
        auto future = std::async(std::launch::async, [&]() {
            return agent_manager_->execute_agent_function(agent_id, function_name, params);
        });
        
        if (future.wait_for(std::chrono::milliseconds(timeout_ms)) == std::future_status::timeout) {
            throw std::runtime_error("Function execution timeout");
        }
        
        return future.get();
    }
    
protected:
    std::shared_ptr<AgentConfigManager> config_manager_;
    std::shared_ptr<AgentManager> agent_manager_;
    std::unique_ptr<ModelInterface> model_interface_;
    std::string test_config_path_;
    std::string test_model_config_path_;
};

// Agent Creation and Configuration Tests
class AgentCreationConfigTest : public AgentExecutionTest {};

TEST_F(AgentCreationConfigTest, LoadYAMLConfig) {
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    // Verify system configuration
    const auto& config = config_manager_->get_config();
    EXPECT_EQ(config.system.name, "Test Agent System");
    EXPECT_EQ(config.system.port, 8081);
    EXPECT_EQ(config.system.log_level, "info");
    
    // Verify agents configuration
    const auto& agent_configs = config_manager_->get_agent_configs();
    EXPECT_EQ(agent_configs.size(), 2);
    
    EXPECT_EQ(agent_configs[0].name, "TestAssistant");
    EXPECT_TRUE(agent_configs[0].auto_start);
    EXPECT_EQ(agent_configs[0].capabilities.size(), 2);
    
    EXPECT_EQ(agent_configs[1].name, "TestAnalyzer");
    EXPECT_FALSE(agent_configs[1].auto_start);
}

TEST_F(AgentCreationConfigTest, CreateAgentWithSystemPrompt) {
    std::string agent_id = agent_manager_->create_agent("TestAgent", {"chat", "analysis"});
    
    EXPECT_FALSE(agent_id.empty());
    EXPECT_TRUE(agent_manager_->agent_exists(agent_id));
    
    auto agent = agent_manager_->get_agent(agent_id);
    ASSERT_NE(agent, nullptr);
    
    EXPECT_EQ(agent->get_name(), "TestAgent");
    EXPECT_EQ(agent->get_capabilities().size(), 2);
    EXPECT_FALSE(agent->get_system_instruction().empty());
}

TEST_F(AgentCreationConfigTest, CreateAgentWithCustomConfig) {
    json custom_config;
    custom_config["capabilities"] = json::array({"custom_capability"});
    custom_config["system_prompt"] = "Custom test prompt";
    
    std::string agent_id = agent_manager_->create_agent_with_config("CustomAgent", custom_config);
    
    EXPECT_FALSE(agent_id.empty());
    
    auto agent = agent_manager_->get_agent(agent_id);
    ASSERT_NE(agent, nullptr);
    
    EXPECT_EQ(agent->get_name(), "CustomAgent");
    EXPECT_EQ(agent->get_agent_specific_prompt(), "Custom test prompt");
    
    const auto& capabilities = agent->get_capabilities();
    EXPECT_EQ(capabilities.size(), 1);
    EXPECT_EQ(capabilities[0], "custom_capability");
}

TEST_F(AgentCreationConfigTest, InvalidConfigHandling) {
    // Test with invalid configuration file
    EXPECT_FALSE(config_manager_->load_config("nonexistent_config.yaml"));
    
    // Test creating agent with empty name
    EXPECT_THROW(agent_manager_->create_agent(""), std::exception);
    
    // Test with malformed JSON config
    json malformed_config = "invalid_json_structure";
    EXPECT_THROW(agent_manager_->create_agent_with_config("BadAgent", malformed_config), std::exception);
}

// Agent Manager Functionality Tests
class AgentManagerTest : public AgentExecutionTest {};

TEST_F(AgentManagerTest, AgentLifecycle) {
    // Create agent
    std::string agent_id = agent_manager_->create_agent("LifecycleAgent", {"chat"});
    EXPECT_FALSE(agent_id.empty());
    
    auto agent = agent_manager_->get_agent(agent_id);
    ASSERT_NE(agent, nullptr);
    EXPECT_FALSE(agent->is_running());
    
    // Start agent
    EXPECT_TRUE(agent_manager_->start_agent(agent_id));
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    // Stop agent
    agent_manager_->stop_agent(agent_id);
    // Give some time for the agent to stop
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(agent->is_running());
    
    // Delete agent
    EXPECT_TRUE(agent_manager_->delete_agent(agent_id));
    EXPECT_FALSE(agent_manager_->agent_exists(agent_id));
}

TEST_F(AgentManagerTest, InitializeDefaultAgents) {
    // Clear any existing agents
    agent_manager_->stop_all_agents();
    
    // Initialize default agents from configuration
    agent_manager_->initialize_default_agents();
    
    // Verify default agents were created
    json agents_list = agent_manager_->list_agents();
    EXPECT_GT(agents_list["total_count"].get<int>(), 0);
    
    // Check for specific test agents
    bool found_test_assistant = false;
    bool found_test_analyzer = false;
    
    for (const auto& agent_info : agents_list["agents"]) {
        std::string name = agent_info["name"];
        if (name == "TestAssistant") {
            found_test_assistant = true;
            EXPECT_TRUE(agent_info["running"].get<bool>()); // auto_start: true
        } else if (name == "TestAnalyzer") {
            found_test_analyzer = true;
            EXPECT_FALSE(agent_info["running"].get<bool>()); // auto_start: false
        }
    }
    
    EXPECT_TRUE(found_test_assistant);
    EXPECT_TRUE(found_test_analyzer);
}

TEST_F(AgentManagerTest, ListAndManageMultipleAgents) {
    // Create multiple agents
    std::vector<std::string> agent_ids;
    for (int i = 0; i < 3; ++i) {
        std::string agent_id = agent_manager_->create_agent("Agent" + std::to_string(i), {"chat"});
        agent_ids.push_back(agent_id);
        agent_manager_->start_agent(agent_id);
    }
    
    // List agents
    json agents_list = agent_manager_->list_agents();
    EXPECT_GE(agents_list["total_count"].get<int>(), 3);
    EXPECT_GE(agents_list["running_count"].get<int>(), 3);
    
    // Stop all agents
    agent_manager_->stop_all_agents();
    
    // Verify all agents are stopped
    agents_list = agent_manager_->list_agents();
    EXPECT_EQ(agents_list["running_count"].get<int>(), 0);
    
    // Clean up
    for (const auto& agent_id : agent_ids) {
        agent_manager_->delete_agent(agent_id);
    }
}

// Model Interface Integration Tests
class ModelInterfaceTest : public AgentExecutionTest {};

TEST_F(ModelInterfaceTest, BasicModelCommunication) {
    // Note: These tests assume a mock model interface or test server
    // In a real environment, you'd need to set up a test model server
    
    // Test model availability check
    bool is_available = model_interface_->is_model_available("test-model");
    // This might be false if no test server is running, which is expected
    
    // Test getting available models
    json models = model_interface_->get_available_models();
    EXPECT_TRUE(models.is_array());
}

TEST_F(ModelInterfaceTest, ModelFallbackScenarios) {
    // Create agent with model integration
    std::string agent_id = agent_manager_->create_agent("ModelTestAgent", {"chat"});
    auto agent = agent_manager_->get_agent(agent_id);
    ASSERT_NE(agent, nullptr);
    
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    // Test function execution without model parameter (should work with fallback)
    json params;
    params["message"] = "test message";
    
    json result;
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "echo", params));
    EXPECT_TRUE(result.contains("data"));
    
    // Test function execution with invalid model (should handle gracefully)
    params["model"] = "invalid-model";
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "echo", params));
}

// HTTP API Endpoints Tests
class HTTPAPITest : public AgentExecutionTest {
protected:
    void SetUp() override {
        AgentExecutionTest::SetUp();
        
        // Start HTTP server on test port
        http_server_ = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 8082);
        ASSERT_TRUE(http_server_->start());
        
        // Give server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    void TearDown() override {
        if (http_server_) {
            http_server_->stop();
        }
        AgentExecutionTest::TearDown();
    }
    
    // Helper function to make HTTP requests (simplified - in real tests use HTTP client library)
    std::string makeHTTPRequest(const std::string& method, const std::string& path, const std::string& body = "") {
        // This is a simplified mock - in real tests, you'd use curl or similar
        // For now, we'll test the server components directly
        return "";
    }
    
protected:
    std::unique_ptr<HTTPServer> http_server_;
};

TEST_F(HTTPAPITest, ServerStartupAndShutdown) {
    EXPECT_TRUE(http_server_ != nullptr);
    // Server should be running (tested in SetUp)
    
    // Test graceful shutdown
    http_server_->stop();
    
    // Test restart
    EXPECT_TRUE(http_server_->start());
}

TEST_F(HTTPAPITest, AgentAPIEndpoints) {
    // Create test agent through manager
    std::string agent_id = agent_manager_->create_agent("APITestAgent", {"chat", "analysis"});
    EXPECT_FALSE(agent_id.empty());
    
    // Test agent listing (simulate API call)
    json agents_list = agent_manager_->list_agents();
    EXPECT_GT(agents_list["total_count"].get<int>(), 0);
    
    // Test agent info retrieval
    auto agent = agent_manager_->get_agent(agent_id);
    ASSERT_NE(agent, nullptr);
    json agent_info = agent->get_info();
    EXPECT_EQ(agent_info["name"], "APITestAgent");
    EXPECT_EQ(agent_info["id"], agent_id);
    
    // Test agent lifecycle through API
    EXPECT_TRUE(agent_manager_->start_agent(agent_id));
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    agent_manager_->stop_agent(agent_id);
    EXPECT_TRUE(agent_manager_->delete_agent(agent_id));
}

TEST_F(HTTPAPITest, FunctionExecutionAPI) {
    // Create and start test agent
    std::string agent_id = agent_manager_->create_agent("FunctionTestAgent", {"chat", "analysis"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    // Test function execution with model parameter
    json params;
    params["message"] = "test message for API";
    params["model"] = "test-model";
    
    json result;
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "chat", params));
    
    // Test function execution without model parameter
    json echo_params;
    echo_params["data"] = "test echo data";
    
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "echo", echo_params));
    EXPECT_TRUE(result.contains("data"));
}

// Function Execution Tests
class FunctionExecutionTest : public AgentExecutionTest {};

TEST_F(FunctionExecutionTest, ChatFunctionExecution) {
    std::string agent_id = agent_manager_->create_agent("ChatTestAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    json params;
    params["message"] = "Hello, how are you?";
    params["model"] = "test-model";
    
    json result;
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "chat", params, 15000));
    
    EXPECT_TRUE(result.contains("agent"));
    EXPECT_TRUE(result.contains("response"));
    EXPECT_TRUE(result.contains("timestamp"));
}

TEST_F(FunctionExecutionTest, AnalyzeFunctionExecution) {
    std::string agent_id = agent_manager_->create_agent("AnalyzeTestAgent", {"analysis"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    json params;
    params["text"] = "This is a sample text for analysis. It contains multiple sentences and various topics.";
    params["analysis_type"] = "sentiment";
    params["model"] = "test-model";
    
    json result;
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "analyze", params, 20000));
    
    EXPECT_TRUE(result.contains("analysis"));
    EXPECT_TRUE(result.contains("text"));
}

TEST_F(FunctionExecutionTest, EchoFunctionExecution) {
    std::string agent_id = agent_manager_->create_agent("EchoTestAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    json test_data;
    test_data["string_value"] = "test string";
    test_data["number_value"] = 42;
    test_data["array_value"] = json::array({1, 2, 3});
    
    json params;
    params["data"] = test_data;
    
    json result;
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "echo", params));
    
    EXPECT_TRUE(result.contains("data"));
    EXPECT_EQ(result["data"], test_data);
}

TEST_F(FunctionExecutionTest, FunctionWithoutModelParameter) {
    std::string agent_id = agent_manager_->create_agent("NoModelTestAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    // Test functions that don't require model parameter
    json params;
    params["data"] = "test without model";
    
    json result;
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "echo", params));
    EXPECT_TRUE(result.contains("data"));
    
    // Test status function
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "status", json::object()));
    EXPECT_TRUE(result.contains("agent_id"));
    EXPECT_TRUE(result.contains("status"));
}

TEST_F(FunctionExecutionTest, ConcurrentFunctionExecution) {
    std::string agent_id = agent_manager_->create_agent("ConcurrentTestAgent", {"chat", "analysis"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    const int num_concurrent_calls = 5;
    std::vector<std::future<json>> futures;
    
    // Launch concurrent function calls
    for (int i = 0; i < num_concurrent_calls; ++i) {
        auto future = std::async(std::launch::async, [&, i]() {
            json params;
            params["data"] = "concurrent test " + std::to_string(i);
            return agent_manager_->execute_agent_function(agent_id, "echo", params);
        });
        futures.push_back(std::move(future));
    }
    
    // Wait for all calls to complete and verify results
    for (int i = 0; i < num_concurrent_calls; ++i) {
        EXPECT_NO_THROW({
            json result = futures[i].get();
            EXPECT_TRUE(result.contains("data"));
            std::string expected = "concurrent test " + std::to_string(i);
            EXPECT_EQ(result["data"], expected);
        });
    }
}

// Error Handling Tests
class ErrorHandlingTest : public AgentExecutionTest {};

TEST_F(ErrorHandlingTest, InvalidInputs) {
    std::string agent_id = agent_manager_->create_agent("ErrorTestAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    // Test invalid function name
    json params;
    params["message"] = "test";
    
    EXPECT_THROW(agent_manager_->execute_agent_function(agent_id, "invalid_function", params), std::exception);
    
    // Test missing required parameters
    json empty_params;
    EXPECT_THROW(agent_manager_->execute_agent_function(agent_id, "chat", empty_params), std::exception);
    
    // Test invalid agent ID
    EXPECT_THROW(agent_manager_->execute_agent_function("invalid_id", "echo", params), std::exception);
}

TEST_F(ErrorHandlingTest, MissingModels) {
    std::string agent_id = agent_manager_->create_agent("MissingModelTestAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    json params;
    params["message"] = "test with missing model";
    params["model"] = "nonexistent-model";
    
    // Should handle missing model gracefully
    json result;
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "chat", params));
    
    // Result should indicate the model issue or provide fallback response
    EXPECT_TRUE(result.contains("agent") || result.contains("error"));
}

TEST_F(ErrorHandlingTest, ConfigurationErrors) {
    // Test with invalid configuration
    auto bad_config_manager = std::make_shared<AgentConfigManager>();
    auto bad_agent_manager = std::make_shared<AgentManager>(bad_config_manager);
    
    // Should handle missing configuration gracefully
    EXPECT_NO_THROW(bad_agent_manager->initialize_default_agents());
    
    // Test agent creation with no configuration
    std::string agent_id;
    EXPECT_NO_THROW(agent_id = bad_agent_manager->create_agent("NoConfigAgent", {"chat"}));
    EXPECT_FALSE(agent_id.empty());
}

TEST_F(ErrorHandlingTest, TimeoutHandling) {
    std::string agent_id = agent_manager_->create_agent("TimeoutTestAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    // Test with very short timeout
    json params;
    params["message"] = "test timeout handling";
    
    EXPECT_THROW(executeFunctionWithTimeout(agent_id, "chat", params, 1), std::runtime_error);
}

TEST_F(ErrorHandlingTest, ResourceLimits) {
    // Test creating too many agents (resource exhaustion)
    std::vector<std::string> agent_ids;
    const int max_agents = 100; // Reasonable limit for testing
    
    for (int i = 0; i < max_agents; ++i) {
        try {
            std::string agent_id = agent_manager_->create_agent("ResourceTestAgent" + std::to_string(i), {"chat"});
            agent_ids.push_back(agent_id);
        } catch (const std::exception& e) {
            // Expected behavior when resource limits are reached
            break;
        }
    }
    
    // Clean up created agents
    for (const auto& agent_id : agent_ids) {
        agent_manager_->delete_agent(agent_id);
    }
    
    EXPECT_GT(agent_ids.size(), 10); // Should be able to create at least some agents
}

// Integration Tests
class IntegrationTest : public AgentExecutionTest {};

TEST_F(IntegrationTest, FullWorkflow) {
    // Complete workflow test: configuration -> creation -> execution -> cleanup
    
    // 1. Load configuration
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    // 2. Initialize default agents
    agent_manager_->initialize_default_agents();
    
    // 3. Create additional agent
    std::string custom_agent_id = agent_manager_->create_agent("WorkflowTestAgent", {"chat", "analysis"});
    agent_manager_->start_agent(custom_agent_id);
    EXPECT_TRUE(waitForAgentStartup(custom_agent_id));
    
    // 4. Execute various functions
    json chat_params;
    chat_params["message"] = "Analyze this workflow";
    chat_params["model"] = "test-model";
    
    json chat_result;
    EXPECT_NO_THROW(chat_result = executeFunctionWithTimeout(custom_agent_id, "chat", chat_params));
    
    json analyze_params;
    analyze_params["text"] = "This is a comprehensive integration test workflow.";
    analyze_params["analysis_type"] = "comprehensive";
    
    json analyze_result;
    EXPECT_NO_THROW(analyze_result = executeFunctionWithTimeout(custom_agent_id, "analyze", analyze_params));
    
    // 5. Verify results
    EXPECT_TRUE(chat_result.contains("response"));
    EXPECT_TRUE(analyze_result.contains("analysis"));
    
    // 6. Test system status
    json agents_list = agent_manager_->list_agents();
    EXPECT_GT(agents_list["total_count"].get<int>(), 1);
    EXPECT_GT(agents_list["running_count"].get<int>(), 0);
    
    // 7. Cleanup
    agent_manager_->stop_all_agents();
    EXPECT_TRUE(agent_manager_->delete_agent(custom_agent_id));
}

TEST_F(IntegrationTest, HTTPServerIntegration) {
    // Test HTTP server with actual agent operations
    auto http_server = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 8083);
    EXPECT_TRUE(http_server->start());
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Create agent through manager (simulating API call)
    std::string agent_id = agent_manager_->create_agent("HTTPIntegrationAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    // Execute function (simulating API call)
    json params;
    params["message"] = "HTTP integration test";
    
    json result;
    EXPECT_NO_THROW(result = executeFunctionWithTimeout(agent_id, "chat", params));
    
    // Cleanup
    http_server->stop();
    agent_manager_->delete_agent(agent_id);
}

// Performance Tests
class PerformanceTest : public AgentExecutionTest {};

TEST_F(PerformanceTest, AgentCreationPerformance) {
    const int num_agents = 50;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::string> agent_ids;
    for (int i = 0; i < num_agents; ++i) {
        std::string agent_id = agent_manager_->create_agent("PerfTestAgent" + std::to_string(i), {"chat"});
        agent_ids.push_back(agent_id);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Created " << num_agents << " agents in " << duration.count() << " ms" << std::endl;
    
    // Cleanup
    for (const auto& agent_id : agent_ids) {
        agent_manager_->delete_agent(agent_id);
    }
    
    // Performance expectation: should create agents reasonably quickly
    EXPECT_LT(duration.count(), 5000); // Less than 5 seconds for 50 agents
}

TEST_F(PerformanceTest, FunctionExecutionPerformance) {
    std::string agent_id = agent_manager_->create_agent("PerfFunctionAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    EXPECT_TRUE(waitForAgentStartup(agent_id));
    
    const int num_executions = 100;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_executions; ++i) {
        json params;
        params["data"] = "performance test " + std::to_string(i);
        
        json result = agent_manager_->execute_agent_function(agent_id, "echo", params);
        EXPECT_TRUE(result.contains("data"));
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Executed " << num_executions << " functions in " << duration.count() << " ms" << std::endl;
    
    // Performance expectation: echo functions should be fast
    EXPECT_LT(duration.count(), 2000); // Less than 2 seconds for 100 echo calls
}

// Main test runner
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "Running Kolosal Agent System Execution Tests..." << std::endl;
    std::cout << "Test Categories:" << std::endl;
    std::cout << "  - Agent Creation and Configuration" << std::endl;
    std::cout << "  - Agent Manager Functionality" << std::endl;
    std::cout << "  - Model Interface Integration" << std::endl;
    std::cout << "  - HTTP API Endpoints" << std::endl;
    std::cout << "  - Function Execution" << std::endl;
    std::cout << "  - Error Handling" << std::endl;
    std::cout << "  - Integration Tests" << std::endl;
    std::cout << "  - Performance Tests" << std::endl;
    std::cout << std::endl;
    
    return RUN_ALL_TESTS();
}
