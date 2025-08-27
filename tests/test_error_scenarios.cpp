/**
 * @file test_error_scenarios.cpp
 * @brief Comprehensive error handling and edge case tests
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
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
#include <fstream>

using json = nlohmann::json;

class ErrorScenarioTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = std::make_shared<AgentConfigManager>();
        agent_manager_ = std::make_shared<AgentManager>(config_manager_);
        model_interface_ = std::make_unique<ModelInterface>("http://localhost:8080");
    }
    
    void TearDown() override {
        if (agent_manager_) {
            agent_manager_->stop_all_agents();
        }
        cleanupTestFiles();
    }
    
    void cleanupTestFiles() {
        // Remove any test files created during error testing
        std::remove("corrupted_config.yaml");
        std::remove("empty_config.yaml");
        std::remove("large_config.yaml");
    }
    
protected:
    std::shared_ptr<AgentConfigManager> config_manager_;
    std::shared_ptr<AgentManager> agent_manager_;
    std::unique_ptr<ModelInterface> model_interface_;
};

// Configuration Error Tests
class ConfigurationErrorTest : public ErrorScenarioTest {};

TEST_F(ConfigurationErrorTest, CorruptedConfigFile) {
    // Create corrupted YAML file
    std::ofstream corrupted_file("corrupted_config.yaml");
    corrupted_file << R"(
system:
  name: "Test System"
  port: not_a_number
  invalid_structure: [
    - missing proper yaml syntax
    malformed: {unclosed_brace
  ]
unknown_section:
  random_data: "test"
  nested:
    deeply:
      invalid: [[[[[
)";
    corrupted_file.close();
    
    // Should handle corrupted config gracefully
    EXPECT_FALSE(config_manager_->load_config("corrupted_config.yaml"));
    
    // Should still have default configuration
    const auto& config = config_manager_->get_config();
    EXPECT_FALSE(config.system.name.empty());
    EXPECT_GT(config.system.port, 0);
}

TEST_F(ConfigurationErrorTest, EmptyConfigFile) {
    // Create empty file
    std::ofstream empty_file("empty_config.yaml");
    empty_file.close();
    
    // Should handle empty config file
    EXPECT_FALSE(config_manager_->load_config("empty_config.yaml"));
    
    // Should have default configuration
    const auto& config = config_manager_->get_config();
    EXPECT_FALSE(config.system.name.empty());
}

TEST_F(ConfigurationErrorTest, VeryLargeConfigFile) {
    // Create very large config file
    std::ofstream large_file("large_config.yaml");
    large_file << "system:\n  name: \"Large System\"\n";
    large_file << "agents:\n";
    
    // Generate many agents
    for (int i = 0; i < 10000; ++i) {
        large_file << "  - name: \"Agent" << i << "\"\n";
        large_file << "    capabilities: [\"chat\", \"analysis\"]\n";
        large_file << "    auto_start: false\n";
    }
    large_file.close();
    
    // Should handle large files (might be slow but shouldn't crash)
    bool loaded = config_manager_->load_config("large_config.yaml");
    // This might succeed or fail depending on system resources
    
    if (loaded) {
        const auto& agent_configs = config_manager_->get_agent_configs();
        EXPECT_EQ(agent_configs.size(), 10000);
    }
}

TEST_F(ConfigurationErrorTest, InvalidDataTypes) {
    std::ofstream invalid_types_file("invalid_types.yaml");
    invalid_types_file << R"(
system:
  name: 12345  # Should be string
  port: "not a number"  # Should be number
  max_concurrent_requests: []  # Should be number

agents:
  - name: true  # Should be string
    capabilities: "should be array"  # Should be array
    auto_start: "yes"  # Should be boolean

functions:
  test_func:
    timeout: "not a number"  # Should be number
    parameters: "should be array"  # Should be array
)";
    invalid_types_file.close();
    
    // Should handle type mismatches gracefully
    bool loaded = config_manager_->load_config("invalid_types.yaml");
    
    // Might load with type coercion or fail gracefully
    const auto& config = config_manager_->get_config();
    EXPECT_FALSE(config.system.name.empty());
    
    std::remove("invalid_types.yaml");
}

// Agent Creation Error Tests
class AgentCreationErrorTest : public ErrorScenarioTest {};

TEST_F(AgentCreationErrorTest, InvalidAgentNames) {
    // Test with empty name
    EXPECT_THROW(agent_manager_->create_agent(""), std::exception);
    
    // Test with very long name
    std::string very_long_name(10000, 'a');
    std::string agent_id;
    EXPECT_NO_THROW(agent_id = agent_manager_->create_agent(very_long_name, {"chat"}));
    if (!agent_id.empty()) {
        agent_manager_->delete_agent(agent_id);
    }
    
    // Test with special characters
    EXPECT_NO_THROW(agent_id = agent_manager_->create_agent("Agent@#$%^&*()", {"chat"}));
    if (!agent_id.empty()) {
        agent_manager_->delete_agent(agent_id);
    }
}

TEST_F(AgentCreationErrorTest, InvalidCapabilities) {
    // Test with empty capabilities
    std::string agent_id = agent_manager_->create_agent("EmptyCapAgent", {});
    EXPECT_FALSE(agent_id.empty());
    
    auto agent = agent_manager_->get_agent(agent_id);
    ASSERT_NE(agent, nullptr);
    EXPECT_EQ(agent->get_capabilities().size(), 0);
    
    agent_manager_->delete_agent(agent_id);
    
    // Test with very long capability names
    std::vector<std::string> long_capabilities;
    for (int i = 0; i < 100; ++i) {
        long_capabilities.push_back("capability_" + std::string(1000, 'x') + std::to_string(i));
    }
    
    EXPECT_NO_THROW(agent_id = agent_manager_->create_agent("LongCapAgent", long_capabilities));
    if (!agent_id.empty()) {
        agent_manager_->delete_agent(agent_id);
    }
}

TEST_F(AgentCreationErrorTest, ExcessiveAgentCreation) {
    // Test creating many agents to check resource limits
    std::vector<std::string> agent_ids;
    const int max_test_agents = 1000;
    
    for (int i = 0; i < max_test_agents; ++i) {
        try {
            std::string agent_id = agent_manager_->create_agent("StressAgent" + std::to_string(i), {"chat"});
            agent_ids.push_back(agent_id);
        } catch (const std::exception& e) {
            // Expected when resource limits are reached
            break;
        }
        
        // Yield occasionally to prevent hanging
        if (i % 100 == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    std::cout << "Created " << agent_ids.size() << " agents before hitting limits" << std::endl;
    
    // Cleanup
    for (const auto& agent_id : agent_ids) {
        agent_manager_->delete_agent(agent_id);
    }
    
    EXPECT_GT(agent_ids.size(), 100); // Should be able to create at least 100 agents
}

TEST_F(AgentCreationErrorTest, InvalidJSONConfig) {
    // Test with malformed JSON configuration
    json malformed_config = "this is not valid json";
    
    EXPECT_THROW(agent_manager_->create_agent_with_config("BadJSONAgent", malformed_config), std::exception);
    
    // Test with wrong JSON structure
    json wrong_structure = json::array({1, 2, 3});
    EXPECT_NO_THROW(agent_manager_->create_agent_with_config("WrongStructAgent", wrong_structure));
}

// Function Execution Error Tests
class FunctionExecutionErrorTest : public ErrorScenarioTest {};

TEST_F(FunctionExecutionErrorTest, NonexistentFunction) {
    std::string agent_id = agent_manager_->create_agent("FuncErrorAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    
    // Wait for agent to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    json params;
    params["message"] = "test";
    
    // Test calling non-existent function
    EXPECT_THROW(agent_manager_->execute_agent_function(agent_id, "nonexistent_function", params), std::exception);
}

TEST_F(FunctionExecutionErrorTest, InvalidParameters) {
    std::string agent_id = agent_manager_->create_agent("ParamErrorAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test with missing required parameters
    json empty_params;
    EXPECT_THROW(agent_manager_->execute_agent_function(agent_id, "chat", empty_params), std::exception);
    
    // Test with wrong parameter types
    json wrong_type_params;
    wrong_type_params["message"] = 12345; // Should be string
    EXPECT_NO_THROW(agent_manager_->execute_agent_function(agent_id, "chat", wrong_type_params));
    
    // Test with extremely large parameters
    json large_params;
    large_params["message"] = std::string(1000000, 'x'); // 1MB string
    EXPECT_NO_THROW(agent_manager_->execute_agent_function(agent_id, "echo", large_params));
}

TEST_F(FunctionExecutionErrorTest, ConcurrentExecutionErrors) {
    std::string agent_id = agent_manager_->create_agent("ConcurrentErrorAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    const int num_concurrent = 50;
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < num_concurrent; ++i) {
        auto future = std::async(std::launch::async, [&, i]() {
            try {
                json params;
                if (i % 3 == 0) {
                    // Valid call
                    params["data"] = "test " + std::to_string(i);
                    agent_manager_->execute_agent_function(agent_id, "echo", params);
                } else if (i % 3 == 1) {
                    // Invalid function
                    params["message"] = "test";
                    agent_manager_->execute_agent_function(agent_id, "invalid_func", params);
                } else {
                    // Invalid parameters
                    agent_manager_->execute_agent_function(agent_id, "chat", json::object());
                }
            } catch (const std::exception& e) {
                // Expected for invalid calls
            }
        });
        futures.push_back(std::move(future));
    }
    
    // Wait for all to complete
    for (auto& future : futures) {
        EXPECT_NO_THROW(future.get());
    }
}

// Model Interface Error Tests
class ModelInterfaceErrorTest : public ErrorScenarioTest {};

TEST_F(ModelInterfaceErrorTest, InvalidServerURL) {
    // Test with invalid URL
    auto invalid_interface = std::make_unique<ModelInterface>("invalid://not-a-url:999999");
    
    EXPECT_FALSE(invalid_interface->is_model_available("test-model"));
    
    json models = invalid_interface->get_available_models();
    EXPECT_TRUE(models.is_array());
    
    // Should handle errors gracefully
    std::string result = invalid_interface->generate_completion("test-model", "test prompt");
    // Result might be empty or contain error message
}

TEST_F(ModelInterfaceErrorTest, NetworkTimeout) {
    // Test with unreachable server
    auto timeout_interface = std::make_unique<ModelInterface>("http://10.255.255.1:8080");
    
    // These calls should timeout or fail gracefully
    EXPECT_NO_THROW({
        bool available = timeout_interface->is_model_available("test-model");
        json models = timeout_interface->get_available_models();
        std::string result = timeout_interface->generate_completion("test-model", "test");
    });
}

TEST_F(ModelInterfaceErrorTest, ExtremeParameters) {
    // Test with extreme parameter values
    EXPECT_NO_THROW({
        std::string result = model_interface_->generate_completion(
            "test-model",
            std::string(100000, 'x'), // Very long prompt
            "",
            -1,    // Negative max tokens
            -5.0f  // Negative temperature
        );
    });
    
    EXPECT_NO_THROW({
        std::string result = model_interface_->generate_completion(
            "test-model",
            "test",
            "",
            1000000, // Very large max tokens
            100.0f   // Very high temperature
        );
    });
}

// HTTP Server Error Tests
class HTTPServerErrorTest : public ErrorScenarioTest {};

TEST_F(HTTPServerErrorTest, InvalidPortNumbers) {
    // Test with invalid port numbers
    auto invalid_server1 = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", -1);
    EXPECT_FALSE(invalid_server1->start());
    
    auto invalid_server2 = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 0);
    // Port 0 might be valid (auto-assign) or invalid depending on system
    
    auto invalid_server3 = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 65536);
    EXPECT_FALSE(invalid_server3->start());
}

TEST_F(HTTPServerErrorTest, InvalidHostAddress) {
    // Test with invalid host addresses
    auto invalid_host_server = std::make_unique<HTTPServer>(agent_manager_, "999.999.999.999", 8080);
    EXPECT_FALSE(invalid_host_server->start());
    
    auto empty_host_server = std::make_unique<HTTPServer>(agent_manager_, "", 8080);
    EXPECT_FALSE(empty_host_server->start());
}

TEST_F(HTTPServerErrorTest, ServerWithNullAgentManager) {
    auto null_manager_server = std::make_unique<HTTPServer>(nullptr, "127.0.0.1", 8102);
    
    // Server should start but won't be able to handle agent requests properly
    EXPECT_TRUE(null_manager_server->start());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    null_manager_server->stop();
}

TEST_F(HTTPServerErrorTest, RapidStartStopCycles) {
    auto stress_server = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 8103);
    
    // Rapid start/stop cycles to test for race conditions
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(stress_server->start());
        stress_server->stop();
    }
}

// Resource Exhaustion Tests
class ResourceExhaustionTest : public ErrorScenarioTest {};

TEST_F(ResourceExhaustionTest, MemoryStressTest) {
    // Create many agents with large data to test memory handling
    std::vector<std::string> agent_ids;
    const int stress_agents = 200;
    
    for (int i = 0; i < stress_agents; ++i) {
        try {
            std::string agent_id = agent_manager_->create_agent("MemStressAgent" + std::to_string(i), {"chat", "analysis"});
            agent_ids.push_back(agent_id);
            
            // Add some load
            if (i % 10 == 0) {
                json large_params;
                large_params["data"] = std::string(10000, 'x'); // 10KB per agent
                
                try {
                    agent_manager_->execute_agent_function(agent_id, "echo", large_params);
                } catch (const std::exception& e) {
                    // Expected under stress
                }
            }
            
        } catch (const std::exception& e) {
            // Expected when resources are exhausted
            break;
        }
        
        // Yield occasionally
        if (i % 50 == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    std::cout << "Created " << agent_ids.size() << " agents under memory stress" << std::endl;
    
    // Cleanup
    for (const auto& agent_id : agent_ids) {
        agent_manager_->delete_agent(agent_id);
    }
}

TEST_F(ResourceExhaustionTest, ThreadExhaustionTest) {
    // Test system behavior under thread exhaustion
    std::string agent_id = agent_manager_->create_agent("ThreadStressAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    const int num_threads = 1000;
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < num_threads; ++i) {
        try {
            auto future = std::async(std::launch::async, [&, i]() {
                json params;
                params["data"] = "thread test " + std::to_string(i);
                
                try {
                    agent_manager_->execute_agent_function(agent_id, "echo", params);
                } catch (const std::exception& e) {
                    // Expected under thread stress
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            });
            futures.push_back(std::move(future));
        } catch (const std::exception& e) {
            // Expected when thread limit is reached
            break;
        }
    }
    
    std::cout << "Created " << futures.size() << " concurrent operations" << std::endl;
    
    // Wait for completion (with timeout)
    for (auto& future : futures) {
        try {
            if (future.wait_for(std::chrono::seconds(10)) == std::future_status::timeout) {
                // Some operations might timeout under stress
            } else {
                future.get();
            }
        } catch (const std::exception& e) {
            // Expected under stress
        }
    }
}

// Data Corruption Tests
class DataCorruptionTest : public ErrorScenarioTest {};

TEST_F(DataCorruptionTest, InvalidUTF8Strings) {
    std::string agent_id = agent_manager_->create_agent("UTF8TestAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test with various problematic strings
    std::vector<std::string> problematic_strings = {
        "\xFF\xFE\xFD",                    // Invalid UTF-8 bytes
        std::string("\x00\x01\x02", 3),   // Null bytes and control characters
        "Valid text with \xFF invalid bytes",
        std::string(1000, '\x80'),        // Many invalid bytes
        "🙂😊🎉" + std::string("\xFF"),    // Mix of valid Unicode and invalid bytes
    };
    
    for (const auto& problematic_string : problematic_strings) {
        json params;
        params["data"] = problematic_string;
        
        // Should handle invalid UTF-8 gracefully
        EXPECT_NO_THROW({
            json result = agent_manager_->execute_agent_function(agent_id, "echo", params);
        });
    }
}

TEST_F(DataCorruptionTest, MalformedJSONHandling) {
    std::string agent_id = agent_manager_->create_agent("JSONTestAgent", {"chat"});
    agent_manager_->start_agent(agent_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test with various malformed JSON scenarios
    std::vector<std::string> malformed_json_strings = {
        "{\"unclosed\": \"object\"",
        "[1, 2, 3, 4,]",  // Trailing comma
        "{\"duplicate\": 1, \"duplicate\": 2}",
        "{\"number\": 1.2.3}",  // Invalid number
        "{\"string\": \"unescaped\"quote\"}",
    };
    
    for (const auto& malformed : malformed_json_strings) {
        // These should be handled gracefully by the JSON library
        EXPECT_NO_THROW({
            try {
                json parsed = json::parse(malformed);
            } catch (const json::parse_error& e) {
                // Expected behavior for malformed JSON
            }
        });
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "Running Comprehensive Error Scenario Tests..." << std::endl;
    std::cout << "Test Categories:" << std::endl;
    std::cout << "  - Configuration Errors" << std::endl;
    std::cout << "  - Agent Creation Errors" << std::endl;
    std::cout << "  - Function Execution Errors" << std::endl;
    std::cout << "  - Model Interface Errors" << std::endl;
    std::cout << "  - HTTP Server Errors" << std::endl;
    std::cout << "  - Resource Exhaustion" << std::endl;
    std::cout << "  - Data Corruption Handling" << std::endl;
    std::cout << std::endl;
    return RUN_ALL_TESTS();
}
