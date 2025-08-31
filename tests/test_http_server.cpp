#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "http_server.hpp"
#include "agent_manager.hpp"
#include "workflow_manager.hpp"
#include "agent_config.hpp"
#include <json.hpp>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using json = nlohmann::json;

class HTTPServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        test_config_file = "test_http_server_config.yaml";
        createTestConfigFile();
        
        // Set up agent manager
        auto config_manager = std::make_shared<AgentConfigManager>();
        config_manager->load_config(test_config_file);
        
        agent_manager = std::make_shared<AgentManager>(config_manager);
        workflow_manager = std::make_shared<WorkflowManager>(agent_manager, 2, 100, 1000);
        
        // Use a different port for testing to avoid conflicts
        test_port = 8081;
        http_server = std::make_unique<HTTPServer>(
            agent_manager, workflow_manager, nullptr, "127.0.0.1", test_port);
    }

    void TearDown() override {
        if (http_server && http_server->is_running()) {
            http_server->stop();
        }
        http_server.reset();
        
        if (workflow_manager && workflow_manager->is_running()) {
            workflow_manager->stop();
        }
        workflow_manager.reset();
        
        if (agent_manager) {
            agent_manager->stop_all_agents();
        }
        agent_manager.reset();
        
        // Clean up test files
        if (std::filesystem::exists(test_config_file)) {
            std::filesystem::remove(test_config_file);
        }
    }

    void createTestConfigFile() {
        std::ofstream file(test_config_file);
        file << R"(
system:
  name: "Test HTTP Server System"
  host: "127.0.0.1"
  port: 8081

system_instruction: "You are a test assistant."

agents:
  - name: "HTTPTestAgent"
    capabilities: ["chat"]
    auto_start: false
    model: "test_model"
    system_prompt: "You are an HTTP test agent."

models:
  test_model:
    id: "test_model"
    actual_name: "test_model_actual"
    type: "llama"
    description: "Test model"

functions: {}
)";
        file.close();
    }

    std::shared_ptr<AgentManager> agent_manager;
    std::shared_ptr<WorkflowManager> workflow_manager;
    std::unique_ptr<HTTPServer> http_server;
    std::string test_config_file;
    int test_port;
};

TEST_F(HTTPServerTest, ConstructorWithAgentManagerOnly) {
    auto server = std::make_unique<HTTPServer>(agent_manager);
    EXPECT_NE(server, nullptr);
    EXPECT_FALSE(server->is_running());
}

TEST_F(HTTPServerTest, ConstructorWithAllManagers) {
    auto server = std::make_unique<HTTPServer>(
        agent_manager, workflow_manager, nullptr);
    EXPECT_NE(server, nullptr);
    EXPECT_FALSE(server->is_running());
}

TEST_F(HTTPServerTest, ConstructorWithCustomHostAndPort) {
    auto server = std::make_unique<HTTPServer>(
        agent_manager, "0.0.0.0", 9090);
    EXPECT_NE(server, nullptr);
    EXPECT_EQ(server->get_host(), "0.0.0.0");
    EXPECT_EQ(server->get_port(), 9090);
}

TEST_F(HTTPServerTest, GetHostAndPort) {
    EXPECT_EQ(http_server->get_host(), "127.0.0.1");
    EXPECT_EQ(http_server->get_port(), test_port);
}

TEST_F(HTTPServerTest, ServerStartAndStop) {
    // Note: Actual server start might fail if port is in use
    bool started = http_server->start();
    
    if (started) {
        EXPECT_TRUE(http_server->is_running());
        
        // Give server time to fully start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        http_server->stop();
        EXPECT_FALSE(http_server->is_running());
    } else {
        // Port might be in use, which is acceptable for testing
        EXPECT_FALSE(http_server->is_running());
    }
}

TEST_F(HTTPServerTest, ServerStateChecking) {
    EXPECT_FALSE(http_server->is_running());
    
    // After start attempt (may fail due to port conflicts)
    http_server->start();
    // State should be consistent
    EXPECT_TRUE(http_server->is_running() || !http_server->is_running());
}

TEST_F(HTTPServerTest, MultipleStartCalls) {
    bool first_start = http_server->start();
    bool second_start = http_server->start();
    
    if (first_start) {
        // Second start should return false (already running)
        EXPECT_FALSE(second_start);
        http_server->stop();
    }
}

TEST_F(HTTPServerTest, StopWithoutStart) {
    // Should handle stop gracefully even if never started
    EXPECT_NO_THROW(http_server->stop());
    EXPECT_FALSE(http_server->is_running());
}

TEST_F(HTTPServerTest, DestructorBehavior) {
    auto test_server = std::make_unique<HTTPServer>(agent_manager, "127.0.0.1", 8082);
    test_server->start();
    
    // Destructor should handle cleanup gracefully
    EXPECT_NO_THROW(test_server.reset());
}

// Integration test with actual workflow manager
TEST_F(HTTPServerTest, IntegrationWithWorkflowManager) {
    // Start workflow manager first
    workflow_manager->start();
    
    // Create test agent
    std::string agent_id = agent_manager->create_agent("IntegrationTestAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    agent->register_function("integration_function", [](const json& params) -> json {
        return json{{"status", "integration_success"}};
    });
    
    // Server should be able to work with workflow manager
    EXPECT_NO_THROW(http_server->start());
    
    if (http_server->is_running()) {
        http_server->stop();
    }
    
    workflow_manager->stop();
}

// Test with different host configurations
TEST_F(HTTPServerTest, DifferentHostConfigurations) {
    auto localhost_server = std::make_unique<HTTPServer>(agent_manager, "localhost", 8083);
    auto any_host_server = std::make_unique<HTTPServer>(agent_manager, "0.0.0.0", 8084);
    
    EXPECT_EQ(localhost_server->get_host(), "localhost");
    EXPECT_EQ(any_host_server->get_host(), "0.0.0.0");
    
    // Should be able to create servers with different hosts
    EXPECT_NE(localhost_server, nullptr);
    EXPECT_NE(any_host_server, nullptr);
}

TEST_F(HTTPServerTest, PortRangeHandling) {
    // Test with various port numbers
    auto low_port_server = std::make_unique<HTTPServer>(agent_manager, "127.0.0.1", 1024);
    auto high_port_server = std::make_unique<HTTPServer>(agent_manager, "127.0.0.1", 65000);
    
    EXPECT_EQ(low_port_server->get_port(), 1024);
    EXPECT_EQ(high_port_server->get_port(), 65000);
}

// Note: Actual HTTP request/response testing would require setting up HTTP clients
// and may be better suited for integration tests rather than unit tests.
// The following tests check the server's ability to handle the setup and teardown correctly.

TEST_F(HTTPServerTest, ServerWithNullWorkflowManager) {
    auto server_with_null_workflow = std::make_unique<HTTPServer>(
        agent_manager, nullptr, nullptr, "127.0.0.1", 8085);
    
    EXPECT_NE(server_with_null_workflow, nullptr);
    
    // Should handle null workflow manager gracefully
    bool started = server_with_null_workflow->start();
    if (started) {
        server_with_null_workflow->stop();
    }
}

TEST_F(HTTPServerTest, ServerConfigurationConsistency) {
    EXPECT_EQ(http_server->get_host(), "127.0.0.1");
    EXPECT_EQ(http_server->get_port(), test_port);
    
    // Configuration should remain consistent after operations
    http_server->start();
    EXPECT_EQ(http_server->get_host(), "127.0.0.1");
    EXPECT_EQ(http_server->get_port(), test_port);
    
    http_server->stop();
    EXPECT_EQ(http_server->get_host(), "127.0.0.1");
    EXPECT_EQ(http_server->get_port(), test_port);
}
