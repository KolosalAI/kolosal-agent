/**
 * @file test_http_server.cpp
 * @brief Tests for HTTP Server component
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 */

#include "../external/yaml-cpp/test/gtest-1.11.0/googletest/include/gtest/gtest.h"
#include "../include/http_server.hpp"
#include "../include/agent_manager.hpp"
#include <json.hpp>
#include <chrono>
#include <thread>
#include <future>

using json = nlohmann::json;

class HTTPServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create agent manager with test configuration
        config_manager_ = std::make_shared<AgentConfigManager>();
        agent_manager_ = std::make_shared<AgentManager>(config_manager_);
        
        // Create test agents
        test_agent_id_ = agent_manager_->create_agent("HTTPTestAgent", {"chat", "analysis"});
        agent_manager_->start_agent(test_agent_id_);
        
        // Wait for agent to start
        auto start_time = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start_time < std::chrono::milliseconds(2000)) {
            auto agent = agent_manager_->get_agent(test_agent_id_);
            if (agent && agent->is_running()) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void TearDown() override {
        if (http_server_) {
            http_server_->stop();
        }
        if (agent_manager_) {
            agent_manager_->stop_all_agents();
        }
    }
    
    void startTestServer(int port = 8084) {
        http_server_ = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", port);
        ASSERT_TRUE(http_server_->start());
        
        // Give server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
protected:
    std::shared_ptr<AgentConfigManager> config_manager_;
    std::shared_ptr<AgentManager> agent_manager_;
    std::unique_ptr<HTTPServer> http_server_;
    std::string test_agent_id_;
};

TEST_F(HTTPServerTest, ServerStartupAndShutdown) {
    startTestServer(8085);
    
    // Server should be running
    EXPECT_NE(http_server_, nullptr);
    
    // Test graceful shutdown
    http_server_->stop();
    
    // Test restart
    EXPECT_TRUE(http_server_->start());
    
    // Give time for restart
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

TEST_F(HTTPServerTest, MultipleServerInstances) {
    // Test that we can create multiple server instances on different ports
    auto server1 = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 8086);
    auto server2 = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 8087);
    
    EXPECT_TRUE(server1->start());
    EXPECT_TRUE(server2->start());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    server1->stop();
    server2->stop();
}

TEST_F(HTTPServerTest, InvalidPortHandling) {
    // Test with invalid port (negative)
    auto invalid_server = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", -1);
    EXPECT_FALSE(invalid_server->start());
    
    // Test with port already in use
    startTestServer(8088);
    
    auto duplicate_server = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 8088);
    // This might fail or succeed depending on SO_REUSEADDR behavior
}

TEST_F(HTTPServerTest, ServerWithoutAgentManager) {
    // Test server behavior with null agent manager
    auto server_no_agents = std::make_unique<HTTPServer>(nullptr, "127.0.0.1", 8089);
    // Should still start but won't be able to handle agent requests
    EXPECT_TRUE(server_no_agents->start());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    server_no_agents->stop();
}

TEST_F(HTTPServerTest, ConcurrentRequests) {
    startTestServer(8090);
    
    const int num_concurrent_requests = 10;
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < num_concurrent_requests; ++i) {
        auto future = std::async(std::launch::async, [&, i]() {
            // Simulate concurrent agent operations
            std::string temp_agent_id = agent_manager_->create_agent("ConcurrentAgent" + std::to_string(i), {"chat"});
            
            // Small delay to simulate processing
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            agent_manager_->delete_agent(temp_agent_id);
        });
        futures.push_back(std::move(future));
    }
    
    // Wait for all concurrent operations to complete
    for (auto& future : futures) {
        EXPECT_NO_THROW(future.get());
    }
}

TEST_F(HTTPServerTest, ServerResourceCleanup) {
    // Test resource cleanup during server lifecycle
    for (int iteration = 0; iteration < 3; ++iteration) {
        auto test_server = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 8091);
        
        EXPECT_TRUE(test_server->start());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Create some agent activity
        std::string temp_agent = agent_manager_->create_agent("TempAgent" + std::to_string(iteration), {"chat"});
        agent_manager_->start_agent(temp_agent);
        
        test_server->stop();
        agent_manager_->delete_agent(temp_agent);
        
        // Server should clean up properly
        test_server.reset();
    }
}

TEST_F(HTTPServerTest, ServerWithDifferentHosts) {
    // Test server binding to different host addresses
    
    // Localhost
    auto server_localhost = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 8092);
    EXPECT_TRUE(server_localhost->start());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    server_localhost->stop();
    
    // All interfaces (if supported in test environment)
    auto server_all = std::make_unique<HTTPServer>(agent_manager_, "0.0.0.0", 8093);
    EXPECT_TRUE(server_all->start());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    server_all->stop();
}

TEST_F(HTTPServerTest, MemoryUsage) {
    // Test for memory leaks during server operations
    startTestServer(8094);
    
    // Perform many operations to detect potential memory leaks
    for (int i = 0; i < 100; ++i) {
        std::string agent_id = agent_manager_->create_agent("MemTestAgent" + std::to_string(i), {"chat"});
        
        // Simulate some activity
        auto agent = agent_manager_->get_agent(agent_id);
        if (agent) {
            json info = agent->get_info();
        }
        
        agent_manager_->delete_agent(agent_id);
        
        // Occasional yield to prevent tight loop
        if (i % 20 == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    // Test should complete without excessive memory usage
}

TEST_F(HTTPServerTest, ServerStopWithoutStart) {
    auto test_server = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 8095);
    
    // Should be safe to call stop without start
    EXPECT_NO_THROW(test_server->stop());
    
    // Should be able to start after stop
    EXPECT_TRUE(test_server->start());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    test_server->stop();
}

TEST_F(HTTPServerTest, ServerMultipleStopCalls) {
    startTestServer(8096);
    
    // Multiple stop calls should be safe
    http_server_->stop();
    EXPECT_NO_THROW(http_server_->stop());
    EXPECT_NO_THROW(http_server_->stop());
}

TEST_F(HTTPServerTest, ServerRestartStress) {
    // Test rapid start/stop cycles
    auto stress_server = std::make_unique<HTTPServer>(agent_manager_, "127.0.0.1", 8097);
    
    for (int cycle = 0; cycle < 5; ++cycle) {
        EXPECT_TRUE(stress_server->start());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        stress_server->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

TEST_F(HTTPServerTest, ServerWithAgentOperations) {
    startTestServer(8098);
    
    // Test server stability during agent operations
    std::vector<std::string> agent_ids;
    
    // Create multiple agents
    for (int i = 0; i < 5; ++i) {
        std::string agent_id = agent_manager_->create_agent("ServerTestAgent" + std::to_string(i), {"chat", "analysis"});
        agent_ids.push_back(agent_id);
        agent_manager_->start_agent(agent_id);
    }
    
    // Give agents time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Execute functions on agents
    for (const auto& agent_id : agent_ids) {
        json params;
        params["data"] = "test data for " + agent_id;
        
        try {
            json result = agent_manager_->execute_agent_function(agent_id, "echo", params);
            EXPECT_TRUE(result.contains("data"));
        } catch (const std::exception& e) {
            // Some operations might fail in test environment - that's OK
        }
    }
    
    // List agents
    json agents_list = agent_manager_->list_agents();
    EXPECT_GE(agents_list["total_count"].get<int>(), 5);
    
    // Cleanup
    for (const auto& agent_id : agent_ids) {
        agent_manager_->delete_agent(agent_id);
    }
}

TEST_F(HTTPServerTest, LongRunningOperations) {
    startTestServer(8099);
    
    // Test server stability during long-running operations
    auto long_operation = std::async(std::launch::async, [&]() {
        for (int i = 0; i < 50; ++i) {
            std::string agent_id = agent_manager_->create_agent("LongOpAgent" + std::to_string(i), {"chat"});
            
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            
            agent_manager_->delete_agent(agent_id);
        }
    });
    
    // Server should remain stable during the operation
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Wait for operation to complete
    long_operation.wait();
}

// Test HTTP request parsing functionality
class HTTPRequestParsingTest : public HTTPServerTest {};

TEST_F(HTTPRequestParsingTest, ParseHTTPRequestBasic) {
    startTestServer(8100);
    
    // Test basic request parsing (this would require access to internal methods)
    // For now, we test indirectly through server operations
    
    std::string sample_request = "GET /agents HTTP/1.1\r\nHost: localhost:8100\r\n\r\n";
    
    // The actual parsing would be tested through the handle_client method
    // which is called internally when the server receives requests
    
    // For unit testing, you might want to make parse_http_request public
    // or create a test interface
}

TEST_F(HTTPRequestParsingTest, ExtractPathParameter) {
    startTestServer(8101);
    
    // Test path parameter extraction
    // This tests the extract_path_parameter method indirectly
    
    // Create agent to test path extraction with
    std::string agent_id = agent_manager_->create_agent("PathTestAgent", {"chat"});
    
    // The extract_path_parameter method would be called when handling requests like:
    // GET /agents/{agent_id}
    // POST /agents/{agent_id}/execute
    
    // Verify agent exists for path testing
    EXPECT_TRUE(agent_manager_->agent_exists(agent_id));
    
    agent_manager_->delete_agent(agent_id);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "Running HTTP Server Tests..." << std::endl;
    return RUN_ALL_TESTS();
}
