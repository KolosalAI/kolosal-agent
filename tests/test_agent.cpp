#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "agent.hpp"
#include <json.hpp>
#include <thread>
#include <chrono>

using json = nlohmann::json;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

class AgentTest : public ::testing::Test {
protected:
    void SetUp() override {
        agent = std::make_unique<Agent>("TestAgent");
    }

    void TearDown() override {
        if (agent && agent->is_running()) {
            agent->stop();
        }
        agent.reset();
    }

    std::unique_ptr<Agent> agent;
};

TEST_F(AgentTest, ConstructorSetsNameCorrectly) {
    EXPECT_EQ(agent->get_name(), "TestAgent");
    EXPECT_FALSE(agent->get_id().empty());
    EXPECT_FALSE(agent->is_running());
}

TEST_F(AgentTest, StartAndStopAgent) {
    EXPECT_TRUE(agent->start());
    EXPECT_TRUE(agent->is_running());
    
    agent->stop();
    EXPECT_FALSE(agent->is_running());
}

TEST_F(AgentTest, SystemInstructionManagement) {
    const std::string instruction = "You are a helpful assistant.";
    agent->set_system_instruction(instruction);
    EXPECT_EQ(agent->get_system_instruction(), instruction);
}

TEST_F(AgentTest, AgentSpecificPromptManagement) {
    const std::string prompt = "You are specialized in testing.";
    agent->set_agent_specific_prompt(prompt);
    EXPECT_EQ(agent->get_agent_specific_prompt(), prompt);
}

TEST_F(AgentTest, CombinedPromptCreation) {
    const std::string system_instruction = "You are a helpful assistant.";
    const std::string agent_prompt = "You are specialized in testing.";
    
    agent->set_system_instruction(system_instruction);
    agent->set_agent_specific_prompt(agent_prompt);
    
    std::string combined = agent->get_combined_prompt();
    EXPECT_THAT(combined, ::testing::HasSubstr(system_instruction));
    EXPECT_THAT(combined, ::testing::HasSubstr(agent_prompt));
}

TEST_F(AgentTest, CapabilityManagement) {
    std::vector<std::string> capabilities = {"analysis", "reasoning", "chat"};
    
    for (const auto& capability : capabilities) {
        agent->add_capability(capability);
    }
    
    const auto& agent_capabilities = agent->get_capabilities();
    EXPECT_EQ(agent_capabilities.size(), capabilities.size());
    
    for (const auto& capability : capabilities) {
        EXPECT_THAT(agent_capabilities, ::testing::Contains(capability));
    }
}

TEST_F(AgentTest, FunctionRegistration) {
    // Register a simple test function
    agent->register_function("test_function", [](const json& params) -> json {
        json result;
        result["status"] = "success";
        result["input"] = params;
        return result;
    });
    
    // Test function execution
    json params;
    params["test_param"] = "test_value";
    
    json result = agent->execute_function("test_function", params);
    EXPECT_EQ(result["status"], "success");
    EXPECT_EQ(result["input"]["test_param"], "test_value");
}

TEST_F(AgentTest, NonExistentFunctionExecution) {
    json params;
    json result = agent->execute_function("non_existent_function", params);
    
    // Should return an error response
    EXPECT_TRUE(result.contains("error"));
}

TEST_F(AgentTest, BuiltinFunctionsSetup) {
    agent->setup_builtin_functions();
    
    // Test that some basic builtin functions are available
    json info_result = agent->execute_function("get_agent_info", json{});
    EXPECT_FALSE(info_result.contains("error"));
}

TEST_F(AgentTest, GetInfoReturnsCorrectStructure) {
    agent->add_capability("test_capability");
    json info = agent->get_info();
    
    EXPECT_TRUE(info.contains("id"));
    EXPECT_TRUE(info.contains("name"));
    EXPECT_TRUE(info.contains("capabilities"));
    EXPECT_TRUE(info.contains("running"));
    
    EXPECT_EQ(info["name"], "TestAgent");
    EXPECT_EQ(info["running"], agent->is_running());
}

TEST_F(AgentTest, ModelConfiguration) {
    json model_configs = json::array();
    json model_config;
    model_config["id"] = "test_model";
    model_config["type"] = "llama";
    model_config["description"] = "Test model";
    model_configs.push_back(model_config);
    
    // This should not throw
    EXPECT_NO_THROW(agent->configure_models(model_configs));
}

TEST_F(AgentTest, CreateResearchFunctionResponse) {
    json params;
    params["query"] = "test query";
    
    json response = agent->create_research_function_response(
        "test_function", params, "Test task description");
    
    EXPECT_TRUE(response.contains("function_name"));
    EXPECT_TRUE(response.contains("parameters"));
    EXPECT_TRUE(response.contains("task_description"));
    EXPECT_EQ(response["function_name"], "test_function");
    EXPECT_EQ(response["task_description"], "Test task description");
}

// Test concurrent access
TEST_F(AgentTest, ConcurrentStartStop) {
    const int num_threads = 5;
    std::vector<std::thread> threads;
    std::vector<bool> results(num_threads);
    
    // Start multiple threads trying to start/stop the agent
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, &results]() {
            if (i % 2 == 0) {
                results[i] = agent->start();
            } else {
                agent->stop();
                results[i] = true; // stop() is void, assume success
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // At least one start should have succeeded
    bool any_start_succeeded = false;
    for (int i = 0; i < num_threads; i += 2) {
        if (results[i]) {
            any_start_succeeded = true;
            break;
        }
    }
    EXPECT_TRUE(any_start_succeeded);
}

// Test function registration with invalid parameters
TEST_F(AgentTest, FunctionRegistrationWithNullFunction) {
    // This should handle null/empty function gracefully
    EXPECT_NO_THROW(agent->register_function("empty_function", nullptr));
}

TEST_F(AgentTest, FunctionExecutionWithInvalidJSON) {
    agent->register_function("json_function", [](const json& params) -> json {
        if (params.is_null()) {
            throw std::invalid_argument("Invalid JSON");
        }
        return json{{"status", "success"}};
    });
    
    // Test with null JSON
    json result = agent->execute_function("json_function", json{});
    EXPECT_TRUE(result.contains("status"));
}

// Performance test for function execution
TEST_F(AgentTest, FunctionExecutionPerformance) {
    agent->register_function("fast_function", [](const json& params) -> json {
        return json{{"result", "fast"}};
    });
    
    const int num_executions = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_executions; ++i) {
        json result = agent->execute_function("fast_function", json{});
        EXPECT_EQ(result["result"], "fast");
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete 1000 executions in reasonable time (< 1 second)
    EXPECT_LT(duration.count(), 1000);
}

#ifdef BUILD_WITH_RETRIEVAL
TEST_F(AgentTest, RetrievalFunctionsSetup) {
    EXPECT_NO_THROW(agent->setup_retrieval_functions());
}

TEST_F(AgentTest, DeepResearchFunctionsSetup) {
    EXPECT_NO_THROW(agent->setup_deep_research_functions());
}

TEST_F(AgentTest, RetrievalConfiguration) {
    json config;
    config["retrieval_enabled"] = true;
    config["embedding_model"] = "test_model";
    
    EXPECT_NO_THROW(agent->configure_retrieval(config));
}
#endif
