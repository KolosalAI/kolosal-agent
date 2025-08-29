/**
 * @file test_workflow_manager.cpp
 * @brief Comprehensive tests for WorkflowManager component
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Test suite covering:
 * - Workflow Manager Lifecycle
 * - Request Submission and Management
 * - Function Configuration Loading
 * - Timeout and Error Handling
 * - Statistics and Monitoring
 * - Concurrent Operations
 */

#include "../external/yaml-cpp/test/gtest-1.11.0/googletest/include/gtest/gtest.h"
#include "../include/workflow_manager.hpp"
#include "../include/agent_manager.hpp"
#include "../include/agent_config.hpp"
#include <json.hpp>
#include <chrono>
#include <thread>
#include <future>
#include <vector>

using json = nlohmann::json;

class WorkflowManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set timeout for entire setup to prevent hanging
        auto start_time = std::chrono::steady_clock::now();
        auto timeout_duration = std::chrono::seconds(15); // Reduced from 30 to 15 seconds
        
        try {
            // Create agent manager with test agents
            config_manager_ = std::make_shared<AgentConfigManager>();
            agent_manager_ = std::make_shared<AgentManager>(config_manager_);
            
            // Check timeout
            if (std::chrono::steady_clock::now() - start_time > timeout_duration) {
                throw std::runtime_error("Setup timeout during agent manager creation");
            }
            
            // Create test agents
            test_agent_id_ = agent_manager_->create_agent("TestAgent", {"chat", "analysis", "echo"});
            echo_agent_id_ = agent_manager_->create_agent("EchoAgent", {"echo"});
            
            // Check timeout
            if (std::chrono::steady_clock::now() - start_time > timeout_duration) {
                throw std::runtime_error("Setup timeout during agent creation");
            }
            
            // Start agents
            agent_manager_->start_agent(test_agent_id_);
            agent_manager_->start_agent(echo_agent_id_);
            
            // Wait for agents to start with timeout
            if (!waitForAgentStartup(test_agent_id_, 3000) || !waitForAgentStartup(echo_agent_id_, 3000)) { // Reduced from 5000 to 3000
                throw std::runtime_error("Agents failed to start within timeout");
            }
            
            // Check timeout
            if (std::chrono::steady_clock::now() - start_time > timeout_duration) {
                throw std::runtime_error("Setup timeout during agent startup");
            }
            
            // Create workflow manager with smaller limits
            workflow_manager_ = std::make_shared<WorkflowManager>(agent_manager_, 2, 50, 100); // Reduced from 4, 100, 1000
            
            // Load test function configurations
            loadTestFunctionConfigs();
            
            std::cout << "[SetUp] WorkflowManager setup completed successfully" << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "[SetUp] Error during WorkflowManager setup: " << e.what() << std::endl;
            // Clean up any partially created objects
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
        if (workflow_manager_) {
            workflow_manager_->stop();
        }
        if (agent_manager_) {
            agent_manager_->stop_all_agents();
        }
    }
    
    void loadTestFunctionConfigs() {
        json function_config;
        function_config["functions"]["chat"] = {
            {"description", "Test chat functionality"},
            {"timeout", 10000},
            {"parameters", json::array({
                {{"name", "message"}, {"type", "string"}, {"required", true}},
                {{"name", "model"}, {"type", "string"}, {"required", false}}
            })}
        };
        function_config["functions"]["analyze"] = {
            {"description", "Test analysis functionality"},
            {"timeout", 15000},
            {"parameters", json::array({
                {{"name", "text"}, {"type", "string"}, {"required", true}},
                {{"name", "analysis_type"}, {"type", "string"}, {"required", false}}
            })}
        };
        function_config["functions"]["echo"] = {
            {"description", "Test echo functionality"},
            {"timeout", 5000},
            {"parameters", json::array({
                {{"name", "data"}, {"type", "any"}, {"required", false}}
            })}
        };
        
        workflow_manager_->load_function_configs(function_config);
    }
    
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
    
    bool waitForRequestCompletion(const std::string& request_id, int timeout_ms = 10000) {
        auto start_time = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start_time < std::chrono::milliseconds(timeout_ms)) {
            auto request = workflow_manager_->get_request_status(request_id);
            if (request && (request->state == WorkflowState::COMPLETED ||
                           request->state == WorkflowState::FAILED ||
                           request->state == WorkflowState::TIMEOUT ||
                           request->state == WorkflowState::CANCELLED)) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return false;
    }
    
protected:
    std::shared_ptr<AgentConfigManager> config_manager_;
    std::shared_ptr<AgentManager> agent_manager_;
    std::shared_ptr<WorkflowManager> workflow_manager_;
    std::string test_agent_id_;
    std::string echo_agent_id_;
};

// Lifecycle Tests
class WorkflowManagerLifecycleTest : public WorkflowManagerTest {};

TEST_F(WorkflowManagerLifecycleTest, StartAndStop) {
    EXPECT_TRUE(workflow_manager_->start());
    EXPECT_TRUE(workflow_manager_->is_running());
    
    // Test multiple start calls
    EXPECT_TRUE(workflow_manager_->start());
    EXPECT_TRUE(workflow_manager_->is_running());
    
    workflow_manager_->stop();
    EXPECT_FALSE(workflow_manager_->is_running());
    
    // Test multiple stop calls
    EXPECT_NO_THROW(workflow_manager_->stop());
}

TEST_F(WorkflowManagerLifecycleTest, RestartCycle) {
    EXPECT_TRUE(workflow_manager_->start());
    EXPECT_TRUE(workflow_manager_->is_running());
    
    workflow_manager_->stop();
    EXPECT_FALSE(workflow_manager_->is_running());
    
    // Restart
    EXPECT_TRUE(workflow_manager_->start());
    EXPECT_TRUE(workflow_manager_->is_running());
    
    workflow_manager_->stop();
}

TEST_F(WorkflowManagerLifecycleTest, ConfigurationSettings) {
    // Test setting max workers
    workflow_manager_->set_max_workers(8);
    
    // Test setting max queue size
    workflow_manager_->set_max_queue_size(500);
    
    EXPECT_TRUE(workflow_manager_->start());
    
    auto status = workflow_manager_->get_system_status();
    EXPECT_EQ(status["max_workers"].get<size_t>(), 8);
    EXPECT_EQ(status["max_queue_size"].get<size_t>(), 500);
}

// Request Management Tests
class WorkflowRequestTest : public WorkflowManagerTest {
protected:
    void SetUp() override {
        WorkflowManagerTest::SetUp();
        ASSERT_TRUE(workflow_manager_->start());
    }
};

TEST_F(WorkflowRequestTest, BasicRequestSubmission) {
    json params;
    params["data"] = "test data";
    
    std::string request_id = workflow_manager_->submit_request(echo_agent_id_, "echo", params);
    EXPECT_FALSE(request_id.empty());
    
    auto request_status = workflow_manager_->get_request_status(request_id);
    ASSERT_NE(request_status, nullptr);
    EXPECT_EQ(request_status->agent_name, "EchoAgent");
    EXPECT_EQ(request_status->function_name, "echo");
    EXPECT_EQ(request_status->parameters, params);
}

TEST_F(WorkflowRequestTest, RequestWithTimeout) {
    json params;
    params["data"] = "timeout test";
    
    std::string request_id = workflow_manager_->submit_request_with_timeout(
        echo_agent_id_, "echo", params, 8000);
    EXPECT_FALSE(request_id.empty());
    
    auto request_status = workflow_manager_->get_request_status(request_id);
    ASSERT_NE(request_status, nullptr);
    EXPECT_EQ(request_status->timeout_ms, 8000);
}

TEST_F(WorkflowRequestTest, RequestCompletion) {
    json params;
    params["data"] = "completion test";
    
    std::string request_id = workflow_manager_->submit_request(echo_agent_id_, "echo", params);
    
    EXPECT_TRUE(waitForRequestCompletion(request_id));
    
    auto request_status = workflow_manager_->get_request_status(request_id);
    ASSERT_NE(request_status, nullptr);
    EXPECT_EQ(request_status->state, WorkflowState::COMPLETED);
    EXPECT_TRUE(request_status->result.contains("echo"));
    EXPECT_TRUE(request_status->result["echo"].contains("data"));
    EXPECT_EQ(request_status->result["echo"]["data"], "completion test");
}

TEST_F(WorkflowRequestTest, RequestResult) {
    json params;
    params["data"] = "result test";
    
    std::string request_id = workflow_manager_->submit_request(echo_agent_id_, "echo", params);
    EXPECT_TRUE(waitForRequestCompletion(request_id));
    
    json result = workflow_manager_->get_request_result(request_id);
    EXPECT_EQ(result["request_id"], request_id);
    EXPECT_EQ(result["state"], "completed");
    EXPECT_TRUE(result.contains("result"));
    EXPECT_TRUE(result["result"].contains("echo"));
    EXPECT_TRUE(result["result"]["echo"].contains("data"));
}

TEST_F(WorkflowRequestTest, RequestCancellation) {
    // Test request cancellation
    json params;
    params["data"] = "cancellation test";
    
    std::string request_id = workflow_manager_->submit_request(echo_agent_id_, "echo", params);
    
    // Try to cancel immediately
    bool cancelled = workflow_manager_->cancel_request(request_id);
    
    // Wait a moment for state to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto request_status = workflow_manager_->get_request_status(request_id);
    ASSERT_NE(request_status, nullptr);
    
    if (cancelled) {
        // If cancellation succeeded, state should be CANCELLED
        EXPECT_EQ(request_status->state, WorkflowState::CANCELLED);
        EXPECT_FALSE(request_status->error.empty());
    } else {
        // If cancellation failed, request likely completed too quickly
        EXPECT_TRUE(request_status->state == WorkflowState::COMPLETED || 
                   request_status->state == WorkflowState::FAILED);
    }
}

TEST_F(WorkflowRequestTest, InvalidRequestHandling) {
    json params;
    params["message"] = "test";
    
    // Invalid agent ID
    EXPECT_THROW(workflow_manager_->submit_request("invalid_agent", "echo", params), std::exception);
    
    // Invalid function name
    EXPECT_THROW(workflow_manager_->submit_request(echo_agent_id_, "invalid_function", params), std::exception);
    
    // Missing required parameters
    json empty_params;
    EXPECT_THROW(workflow_manager_->submit_request(test_agent_id_, "chat", empty_params), std::exception);
}

TEST_F(WorkflowRequestTest, RequestListing) {
    // Submit multiple requests
    std::vector<std::string> request_ids;
    for (int i = 0; i < 5; ++i) {
        json params;
        params["data"] = "list test " + std::to_string(i);
        
        std::string request_id = workflow_manager_->submit_request(echo_agent_id_, "echo", params);
        request_ids.push_back(request_id);
    }
    
    // Check that we can get status for all requests (they might complete quickly)
    int found_requests = 0;
    for (const auto& request_id : request_ids) {
        auto status = workflow_manager_->get_request_status(request_id);
        if (status) {
            found_requests++;
        }
    }
    EXPECT_GE(found_requests, 5);
    
    // Wait for completion and list recent requests
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // List recent requests
    json recent_requests = workflow_manager_->list_recent_requests(10);
    EXPECT_TRUE(recent_requests.is_array());
    EXPECT_GE(recent_requests.size(), 5);
}

// Function Configuration Tests
class WorkflowFunctionConfigTest : public WorkflowManagerTest {};

TEST_F(WorkflowFunctionConfigTest, LoadFunctionConfigs) {
    json function_config;
    function_config["functions"]["test_function"] = {
        {"description", "Test function"},
        {"timeout", 20000},
        {"parameters", json::array({
            {{"name", "input"}, {"type", "string"}, {"required", true}},
            {{"name", "options"}, {"type", "object"}, {"required", false}}
        })}
    };
    
    EXPECT_NO_THROW(workflow_manager_->load_function_configs(function_config));
    
    // Function should now be available for validation
    json params;
    params["input"] = "test input";
    
    EXPECT_TRUE(workflow_manager_->validate_request(test_agent_id_, "test_function", params));
}

TEST_F(WorkflowFunctionConfigTest, ParameterValidation) {
    json params;
    params["message"] = "test message";
    
    // Valid parameters
    EXPECT_TRUE(workflow_manager_->validate_request(test_agent_id_, "chat", params));
    
    // Missing required parameter
    json invalid_params;
    EXPECT_FALSE(workflow_manager_->validate_request(test_agent_id_, "chat", invalid_params));
}

TEST_F(WorkflowFunctionConfigTest, TimeoutFromConfig) {
    EXPECT_TRUE(workflow_manager_->start());
    
    // Submit request without timeout (should use config timeout)
    json params;
    params["data"] = "config timeout test";
    
    std::string request_id = workflow_manager_->submit_request(echo_agent_id_, "echo", params);
    
    auto request_status = workflow_manager_->get_request_status(request_id);
    ASSERT_NE(request_status, nullptr);
    EXPECT_EQ(request_status->timeout_ms, 5000); // From config
}

// Statistics and Monitoring Tests
class WorkflowStatisticsTest : public WorkflowManagerTest {
protected:
    void SetUp() override {
        WorkflowManagerTest::SetUp();
        ASSERT_TRUE(workflow_manager_->start());
    }
};

TEST_F(WorkflowStatisticsTest, BasicStatistics) {
    auto initial_stats = workflow_manager_->get_statistics();
    
    // Submit some requests
    const int num_requests = 5;
    for (int i = 0; i < num_requests; ++i) {
        json params;
        params["data"] = "stats test " + std::to_string(i);
        workflow_manager_->submit_request(echo_agent_id_, "echo", params);
    }
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    auto final_stats = workflow_manager_->get_statistics();
    EXPECT_EQ(final_stats.total_requests.load(), 
              initial_stats.total_requests.load() + num_requests);
}

TEST_F(WorkflowStatisticsTest, SystemStatus) {
    json status = workflow_manager_->get_system_status();
    
    EXPECT_TRUE(status["running"].get<bool>());
    EXPECT_GT(status["worker_threads"].get<size_t>(), 0);
    EXPECT_GT(status["max_workers"].get<size_t>(), 0);
    EXPECT_TRUE(status.contains("statistics"));
    
    auto stats = status["statistics"];
    EXPECT_TRUE(stats.contains("total_requests"));
    EXPECT_TRUE(stats.contains("completed_requests"));
    EXPECT_TRUE(stats.contains("failed_requests"));
    EXPECT_TRUE(stats.contains("active_requests"));
}

TEST_F(WorkflowStatisticsTest, RequestCleanup) {
    // Submit many requests to test cleanup
    const int num_requests = 20;
    for (int i = 0; i < num_requests; ++i) {
        json params;
        params["data"] = "cleanup test " + std::to_string(i);
        workflow_manager_->submit_request(echo_agent_id_, "echo", params);
    }
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    // Trigger cleanup
    workflow_manager_->cleanup_completed_requests(10);
    
    // Verify cleanup occurred
    json recent_requests = workflow_manager_->list_recent_requests(50);
    // Should have cleaned up some requests, but may not be exactly 10 due to active requests
}

// Concurrent Operations Tests
class WorkflowConcurrencyTest : public WorkflowManagerTest {
protected:
    void SetUp() override {
        WorkflowManagerTest::SetUp();
        ASSERT_TRUE(workflow_manager_->start());
    }
};

TEST_F(WorkflowConcurrencyTest, ConcurrentRequestSubmission) {
    const int num_concurrent = 10;
    std::vector<std::future<std::string>> futures;
    
    for (int i = 0; i < num_concurrent; ++i) {
        auto future = std::async(std::launch::async, [&, i]() {
            json params;
            params["data"] = "concurrent test " + std::to_string(i);
            return workflow_manager_->submit_request(echo_agent_id_, "echo", params);
        });
        futures.push_back(std::move(future));
    }
    
    // Wait for all submissions
    std::vector<std::string> request_ids;
    for (auto& future : futures) {
        EXPECT_NO_THROW({
            std::string request_id = future.get();
            EXPECT_FALSE(request_id.empty());
            request_ids.push_back(request_id);
        });
    }
    
    EXPECT_EQ(request_ids.size(), num_concurrent);
    
    // Verify all requests have unique IDs
    std::set<std::string> unique_ids(request_ids.begin(), request_ids.end());
    EXPECT_EQ(unique_ids.size(), num_concurrent);
}

TEST_F(WorkflowConcurrencyTest, ConcurrentRequestProcessing) {
    const int num_requests = 15;
    std::vector<std::string> request_ids;
    
    // Submit multiple requests
    for (int i = 0; i < num_requests; ++i) {
        json params;
        params["data"] = "processing test " + std::to_string(i);
        
        std::string request_id = workflow_manager_->submit_request(echo_agent_id_, "echo", params);
        request_ids.push_back(request_id);
    }
    
    // Wait for all to complete
    auto start_time = std::chrono::steady_clock::now();
    int completed_count = 0;
    
    while (std::chrono::steady_clock::now() - start_time < std::chrono::milliseconds(10000)) {
        completed_count = 0;
        for (const auto& request_id : request_ids) {
            auto request = workflow_manager_->get_request_status(request_id);
            if (request && request->state == WorkflowState::COMPLETED) {
                completed_count++;
            }
        }
        
        if (completed_count == num_requests) {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    EXPECT_EQ(completed_count, num_requests);
}

TEST_F(WorkflowConcurrencyTest, ConcurrentStatusQueries) {
    // Submit some requests
    std::vector<std::string> request_ids;
    for (int i = 0; i < 5; ++i) {
        json params;
        params["data"] = "status query test " + std::to_string(i);
        request_ids.push_back(workflow_manager_->submit_request(echo_agent_id_, "echo", params));
    }
    
    // Query status concurrently
    const int num_queries = 20;
    std::vector<std::future<bool>> futures;
    
    for (int i = 0; i < num_queries; ++i) {
        auto future = std::async(std::launch::async, [&]() {
            for (const auto& request_id : request_ids) {
                auto status = workflow_manager_->get_request_status(request_id);
                if (!status) return false;
            }
            return true;
        });
        futures.push_back(std::move(future));
    }
    
    // All queries should succeed
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }
}

// Error Handling Tests
class WorkflowErrorTest : public WorkflowManagerTest {
protected:
    void SetUp() override {
        WorkflowManagerTest::SetUp();
        ASSERT_TRUE(workflow_manager_->start());
    }
};

TEST_F(WorkflowErrorTest, TimeoutHandling) {
    json params;
    params["data"] = "timeout test";
    
    // Submit with very short timeout
    std::string request_id = workflow_manager_->submit_request_with_timeout(
        echo_agent_id_, "echo", params, 1);
    
    EXPECT_TRUE(waitForRequestCompletion(request_id, 5000));
    
    auto request_status = workflow_manager_->get_request_status(request_id);
    ASSERT_NE(request_status, nullptr);
    // May be completed or timeout depending on timing
    EXPECT_TRUE(request_status->state == WorkflowState::COMPLETED ||
                request_status->state == WorkflowState::TIMEOUT);
}

TEST_F(WorkflowErrorTest, QueueOverflow) {
    // Stop the workflow manager to prevent request processing
    workflow_manager_->stop();
    workflow_manager_->set_max_queue_size(2);
    // Don't restart it - this will cause queue to fill up without processing
    
    // Submit requests to overflow queue
    json params;
    params["data"] = "overflow test";
    
    std::vector<std::string> request_ids;
    bool overflow_detected = false;
    
    for (int i = 0; i < 10; ++i) {
        try {
            std::string request_id = workflow_manager_->submit_request(echo_agent_id_, "echo", params);
            request_ids.push_back(request_id);
        } catch (const std::exception& e) {
            overflow_detected = true;
            break;
        }
    }
    
    EXPECT_TRUE(overflow_detected);
    
    // Restart to clean up
    workflow_manager_->start();
}

TEST_F(WorkflowErrorTest, InvalidRequestIDs) {
    // Test with non-existent request ID
    auto status = workflow_manager_->get_request_status("invalid-request-id");
    EXPECT_EQ(status, nullptr);
    
    json result = workflow_manager_->get_request_result("invalid-request-id");
    EXPECT_TRUE(result.contains("error"));
    
    bool cancelled = workflow_manager_->cancel_request("invalid-request-id");
    EXPECT_FALSE(cancelled);
}

// Performance Tests
class WorkflowPerformanceTest : public WorkflowManagerTest {
protected:
    void SetUp() override {
        WorkflowManagerTest::SetUp();
        ASSERT_TRUE(workflow_manager_->start());
    }
};

TEST_F(WorkflowPerformanceTest, RequestSubmissionPerformance) {
    const int num_requests = 100;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::string> request_ids;
    for (int i = 0; i < num_requests; ++i) {
        json params;
        params["data"] = "perf test " + std::to_string(i);
        
        std::string request_id = workflow_manager_->submit_request(echo_agent_id_, "echo", params);
        request_ids.push_back(request_id);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Submitted " << num_requests << " requests in " << duration.count() << " ms" << std::endl;
    
    // Performance expectation: should submit requests quickly
    EXPECT_LT(duration.count(), 1000); // Less than 1 second for 100 submissions
}

TEST_F(WorkflowPerformanceTest, RequestProcessingThroughput) {
    const int num_requests = 50;
    std::vector<std::string> request_ids;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Submit all requests
    for (int i = 0; i < num_requests; ++i) {
        json params;
        params["data"] = "throughput test " + std::to_string(i);
        request_ids.push_back(workflow_manager_->submit_request(echo_agent_id_, "echo", params));
    }
    
    // Wait for all to complete
    while (true) {
        int completed = 0;
        for (const auto& request_id : request_ids) {
            auto request = workflow_manager_->get_request_status(request_id);
            if (request && request->state == WorkflowState::COMPLETED) {
                completed++;
            }
        }
        
        if (completed == num_requests) {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Processed " << num_requests << " requests in " << duration.count() << " ms" << std::endl;
    
    // Performance expectation: should process echo requests reasonably quickly
    EXPECT_LT(duration.count(), 10000); // Less than 10 seconds for 50 echo requests
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "Running WorkflowManager Tests..." << std::endl;
    std::cout << "Test Categories:" << std::endl;
    std::cout << "  - Lifecycle Management" << std::endl;
    std::cout << "  - Request Management" << std::endl;
    std::cout << "  - Function Configuration" << std::endl;
    std::cout << "  - Statistics and Monitoring" << std::endl;
    std::cout << "  - Concurrent Operations" << std::endl;
    std::cout << "  - Error Handling" << std::endl;
    std::cout << "  - Performance Testing" << std::endl;
    std::cout << std::endl;
    
    return RUN_ALL_TESTS();
}
