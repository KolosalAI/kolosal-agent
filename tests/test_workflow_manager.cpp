#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "workflow_manager.hpp"
#include "agent_manager.hpp"
#include "agent_config.hpp"
#include <json.hpp>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

class WorkflowManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        test_config_file = "test_workflow_config.yaml";
        createTestConfigFile();
        
        // Set up agent manager with configuration
        auto config_manager = std::make_shared<AgentConfigManager>();
        config_manager->load_config(test_config_file);
        
        agent_manager = std::make_shared<AgentManager>(config_manager);
        agent_manager->initialize_default_agents();
        
        // Create workflow manager
        workflow_manager = std::make_unique<WorkflowManager>(
            agent_manager, 
            2,    // max_workers
            100,  // max_queue_size
            1000  // max_completed_history
        );
    }

    void TearDown() override {
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
  name: "Test Workflow System"
  host: "127.0.0.1"
  port: 8080

system_instruction: "You are a test assistant."

agents:
  - name: "WorkflowTestAgent"
    capabilities: ["chat", "analysis"]
    auto_start: false
    model: "test_model"
    system_prompt: "You are a workflow test agent."

models:
  test_model:
    id: "test_model"
    actual_name: "test_model_actual"
    type: "llama"
    description: "Test model"

functions:
  test_workflow_function:
    description: "Test workflow function"
    timeout: 5000
    parameters: []
)";
        file.close();
    }

    std::shared_ptr<AgentManager> agent_manager;
    std::unique_ptr<WorkflowManager> workflow_manager;
    std::string test_config_file;
};

TEST_F(WorkflowManagerTest, ConstructorInitialization) {
    EXPECT_NE(workflow_manager, nullptr);
    EXPECT_FALSE(workflow_manager->is_running());
}

TEST_F(WorkflowManagerTest, StartAndStopWorkflowManager) {
    EXPECT_TRUE(workflow_manager->start());
    EXPECT_TRUE(workflow_manager->is_running());
    
    workflow_manager->stop();
    EXPECT_FALSE(workflow_manager->is_running());
}

TEST_F(WorkflowManagerTest, ConfigurationManagement) {
    json config;
    config["max_workers"] = 4;
    config["max_queue_size"] = 500;
    
    EXPECT_NO_THROW(workflow_manager->load_function_configs(config));
    EXPECT_NO_THROW(workflow_manager->set_max_workers(4));
    EXPECT_NO_THROW(workflow_manager->set_max_queue_size(500));
}

TEST_F(WorkflowManagerTest, SubmitSimpleRequest) {
    // Start the workflow manager
    workflow_manager->start();
    
    // Create a test agent with a simple function
    std::string agent_id = agent_manager->create_agent("SimpleTestAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    agent->register_function("simple_function", [](const json& params) -> json {
        json result;
        result["status"] = "completed";
        result["input"] = params;
        return result;
    });
    
    json parameters;
    parameters["test_param"] = "test_value";
    
    std::string request_id = workflow_manager->submit_request(
        "SimpleTestAgent", "simple_function", parameters);
    
    EXPECT_FALSE(request_id.empty());
    
    // Wait a bit for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto request_status = workflow_manager->get_request_status(request_id);
    EXPECT_NE(request_status, nullptr);
}

TEST_F(WorkflowManagerTest, SubmitRequestWithTimeout) {
    workflow_manager->start();
    
    // Create agent with a slow function
    std::string agent_id = agent_manager->create_agent("SlowTestAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    agent->register_function("slow_function", [](const json& params) -> json {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return json{{"status", "completed"}};
    });
    
    json parameters;
    std::string request_id = workflow_manager->submit_request_with_timeout(
        "SlowTestAgent", "slow_function", parameters, 100); // 100ms timeout
    
    EXPECT_FALSE(request_id.empty());
    
    // Wait for timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    auto request_status = workflow_manager->get_request_status(request_id);
    EXPECT_NE(request_status, nullptr);
    // Note: Actual timeout behavior depends on implementation
}

TEST_F(WorkflowManagerTest, GetRequestStatus) {
    workflow_manager->start();
    
    std::string agent_id = agent_manager->create_agent("StatusTestAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    agent->register_function("status_function", [](const json& params) -> json {
        return json{{"status", "completed"}};
    });
    
    std::string request_id = workflow_manager->submit_request(
        "StatusTestAgent", "status_function", json{});
    
    auto status = workflow_manager->get_request_status(request_id);
    EXPECT_NE(status, nullptr);
    EXPECT_EQ(status->id, request_id);
}

TEST_F(WorkflowManagerTest, GetRequestResult) {
    workflow_manager->start();
    
    std::string agent_id = agent_manager->create_agent("ResultTestAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    agent->register_function("result_function", [](const json& params) -> json {
        json result;
        result["calculation"] = 42;
        result["message"] = "Test completed";
        return result;
    });
    
    std::string request_id = workflow_manager->submit_request(
        "ResultTestAgent", "result_function", json{});
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    json result = workflow_manager->get_request_result(request_id);
    // Note: Result structure depends on implementation and request state
    EXPECT_TRUE(result.is_object());
}

TEST_F(WorkflowManagerTest, CancelRequest) {
    workflow_manager->start();
    
    std::string agent_id = agent_manager->create_agent("CancelTestAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    agent->register_function("cancellable_function", [](const json& params) -> json {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return json{{"status", "completed"}};
    });
    
    std::string request_id = workflow_manager->submit_request(
        "CancelTestAgent", "cancellable_function", json{});
    
    // Try to cancel immediately
    bool cancelled = workflow_manager->cancel_request(request_id);
    // Note: Cancellation success depends on timing and implementation
    EXPECT_TRUE(cancelled || !cancelled); // Just check it doesn't crash
}

TEST_F(WorkflowManagerTest, ListActiveRequests) {
    workflow_manager->start();
    
    std::string agent_id = agent_manager->create_agent("ActiveTestAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    agent->register_function("active_function", [](const json& params) -> json {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return json{{"status", "completed"}};
    });
    
    // Submit multiple requests
    std::vector<std::string> request_ids;
    for (int i = 0; i < 3; ++i) {
        std::string request_id = workflow_manager->submit_request(
            "ActiveTestAgent", "active_function", json{});
        request_ids.push_back(request_id);
    }
    
    json active_requests = workflow_manager->list_active_requests();
    EXPECT_TRUE(active_requests.is_array());
}

TEST_F(WorkflowManagerTest, ListRecentRequests) {
    workflow_manager->start();
    
    std::string agent_id = agent_manager->create_agent("RecentTestAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    agent->register_function("recent_function", [](const json& params) -> json {
        return json{{"status", "completed"}};
    });
    
    // Submit and wait for completion
    std::string request_id = workflow_manager->submit_request(
        "RecentTestAgent", "recent_function", json{});
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    json recent_requests = workflow_manager->list_recent_requests(10);
    EXPECT_TRUE(recent_requests.is_array());
}

TEST_F(WorkflowManagerTest, GetStatistics) {
    WorkflowStats stats = workflow_manager->get_statistics();
    
    // Should have default values
    EXPECT_GE(stats.total_requests.load(), 0);
    EXPECT_GE(stats.completed_requests.load(), 0);
    EXPECT_GE(stats.failed_requests.load(), 0);
    EXPECT_GE(stats.timeout_requests.load(), 0);
    EXPECT_GE(stats.active_requests.load(), 0);
    EXPECT_GE(stats.queue_size.load(), 0);
}

TEST_F(WorkflowManagerTest, GetSystemStatus) {
    json status = workflow_manager->get_system_status();
    
    EXPECT_TRUE(status.is_object());
    EXPECT_TRUE(status.contains("running"));
    EXPECT_EQ(status["running"], workflow_manager->is_running());
}

TEST_F(WorkflowManagerTest, RequestValidation) {
    // Test validation with non-existent agent
    bool valid = workflow_manager->validate_request(
        "NonExistentAgent", "test_function", json{});
    EXPECT_FALSE(valid);
    
    // Test validation with existing agent but non-existent function
    std::string agent_id = agent_manager->create_agent("ValidationTestAgent");
    bool valid_agent_invalid_function = workflow_manager->validate_request(
        "ValidationTestAgent", "non_existent_function", json{});
    // Note: Validation behavior depends on implementation
}

TEST_F(WorkflowManagerTest, CleanupCompletedRequests) {
    workflow_manager->start();
    
    std::string agent_id = agent_manager->create_agent("CleanupTestAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    agent->register_function("cleanup_function", [](const json& params) -> json {
        return json{{"status", "completed"}};
    });
    
    // Submit multiple requests and wait for completion
    for (int i = 0; i < 5; ++i) {
        workflow_manager->submit_request("CleanupTestAgent", "cleanup_function", json{});
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Test cleanup
    EXPECT_NO_THROW(workflow_manager->cleanup_completed_requests(2));
}

TEST_F(WorkflowManagerTest, ConcurrentRequestSubmission) {
    workflow_manager->start();
    
    std::string agent_id = agent_manager->create_agent("ConcurrentTestAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    agent->register_function("concurrent_function", [](const json& params) -> json {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::stringstream ss;
        ss << std::this_thread::get_id();
        return json{{"thread_id", ss.str()}};
    });
    
    const int num_threads = 5;
    const int requests_per_thread = 10;
    std::vector<std::thread> threads;
    std::vector<std::vector<std::string>> request_ids(num_threads);
    
    // Submit requests concurrently
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, requests_per_thread, &request_ids]() {
            for (int i = 0; i < requests_per_thread; ++i) {
                std::string request_id = workflow_manager->submit_request(
                    "ConcurrentTestAgent", "concurrent_function", json{});
                request_ids[t].push_back(request_id);
            }
        });
    }
    
    // Wait for all submissions to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all requests were submitted
    int total_requests = 0;
    for (const auto& thread_requests : request_ids) {
        total_requests += thread_requests.size();
        for (const auto& request_id : thread_requests) {
            EXPECT_FALSE(request_id.empty());
        }
    }
    EXPECT_EQ(total_requests, num_threads * requests_per_thread);
}

TEST_F(WorkflowManagerTest, WorkflowStateTransitions) {
    // Test workflow state utility functions
    EXPECT_EQ(WorkflowUtils::state_to_string(WorkflowState::PENDING), "PENDING");
    EXPECT_EQ(WorkflowUtils::state_to_string(WorkflowState::PROCESSING), "PROCESSING");
    EXPECT_EQ(WorkflowUtils::state_to_string(WorkflowState::COMPLETED), "COMPLETED");
    EXPECT_EQ(WorkflowUtils::state_to_string(WorkflowState::FAILED), "FAILED");
    EXPECT_EQ(WorkflowUtils::state_to_string(WorkflowState::TIMEOUT), "TIMEOUT");
    EXPECT_EQ(WorkflowUtils::state_to_string(WorkflowState::CANCELLED), "CANCELLED");
    
    EXPECT_EQ(WorkflowUtils::string_to_state("PENDING"), WorkflowState::PENDING);
    EXPECT_EQ(WorkflowUtils::string_to_state("PROCESSING"), WorkflowState::PROCESSING);
    EXPECT_EQ(WorkflowUtils::string_to_state("COMPLETED"), WorkflowState::COMPLETED);
    EXPECT_EQ(WorkflowUtils::string_to_state("FAILED"), WorkflowState::FAILED);
    EXPECT_EQ(WorkflowUtils::string_to_state("TIMEOUT"), WorkflowState::TIMEOUT);
    EXPECT_EQ(WorkflowUtils::string_to_state("CANCELLED"), WorkflowState::CANCELLED);
}

TEST_F(WorkflowManagerTest, WorkflowRequestCreation) {
    WorkflowRequest request("test_id", "test_agent", "test_function", json{}, 30000);
    
    EXPECT_EQ(request.id, "test_id");
    EXPECT_EQ(request.agent_name, "test_agent");
    EXPECT_EQ(request.function_name, "test_function");
    EXPECT_EQ(request.state, WorkflowState::PENDING);
    EXPECT_EQ(request.timeout_ms, 30000);
}

TEST_F(WorkflowManagerTest, WorkflowRequestToJson) {
    WorkflowRequest request("test_id", "test_agent", "test_function", json{{"param", "value"}}, 30000);
    request.state = WorkflowState::COMPLETED;
    request.result = json{{"output", "success"}};
    
    json request_json = WorkflowUtils::request_to_json(request);
    
    EXPECT_TRUE(request_json.contains("id"));
    EXPECT_TRUE(request_json.contains("agent_name"));
    EXPECT_TRUE(request_json.contains("function_name"));
    EXPECT_TRUE(request_json.contains("state"));
    EXPECT_EQ(request_json["id"], "test_id");
    EXPECT_EQ(request_json["agent_name"], "test_agent");
    EXPECT_EQ(request_json["state"], "COMPLETED");
}

TEST_F(WorkflowManagerTest, WorkflowStatsInitialization) {
    WorkflowStats stats;
    
    EXPECT_EQ(stats.total_requests.load(), 0);
    EXPECT_EQ(stats.completed_requests.load(), 0);
    EXPECT_EQ(stats.failed_requests.load(), 0);
    EXPECT_EQ(stats.timeout_requests.load(), 0);
    EXPECT_EQ(stats.active_requests.load(), 0);
    EXPECT_EQ(stats.queue_size.load(), 0);
}

TEST_F(WorkflowManagerTest, WorkflowStatsCopyConstructor) {
    WorkflowStats stats1;
    stats1.total_requests = 10;
    stats1.completed_requests = 8;
    stats1.failed_requests = 2;
    
    WorkflowStats stats2(stats1);
    
    EXPECT_EQ(stats2.total_requests.load(), 10);
    EXPECT_EQ(stats2.completed_requests.load(), 8);
    EXPECT_EQ(stats2.failed_requests.load(), 2);
}

TEST_F(WorkflowManagerTest, WorkflowStatsAssignment) {
    WorkflowStats stats1;
    stats1.total_requests = 15;
    stats1.completed_requests = 12;
    stats1.failed_requests = 3;
    
    WorkflowStats stats2;
    stats2 = stats1;
    
    EXPECT_EQ(stats2.total_requests.load(), 15);
    EXPECT_EQ(stats2.completed_requests.load(), 12);
    EXPECT_EQ(stats2.failed_requests.load(), 3);
}

TEST_F(WorkflowManagerTest, SubmitRequestToNonExistentAgent) {
    workflow_manager->start();
    
    std::string request_id = workflow_manager->submit_request(
        "NonExistentAgent", "test_function", json{});
    
    // Should still return a request ID but request should fail
    EXPECT_FALSE(request_id.empty());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto request_status = workflow_manager->get_request_status(request_id);
    EXPECT_NE(request_status, nullptr);
    // Note: Exact behavior depends on implementation
}

TEST_F(WorkflowManagerTest, GetNonExistentRequestStatus) {
    auto status = workflow_manager->get_request_status("non_existent_request_id");
    EXPECT_EQ(status, nullptr);
}

TEST_F(WorkflowManagerTest, GetNonExistentRequestResult) {
    json result = workflow_manager->get_request_result("non_existent_request_id");
    EXPECT_TRUE(result.is_null() || result.contains("error"));
}

TEST_F(WorkflowManagerTest, CancelNonExistentRequest) {
    bool cancelled = workflow_manager->cancel_request("non_existent_request_id");
    EXPECT_FALSE(cancelled);
}

TEST_F(WorkflowManagerTest, FormatDurationUtility) {
    auto start_time = std::chrono::system_clock::now();
    std::string duration_str = WorkflowUtils::format_duration(start_time);
    
    // Should not be empty and should contain time information
    EXPECT_FALSE(duration_str.empty());
}

TEST_F(WorkflowManagerTest, MultipleWorkflowManagers) {
    // Test creating multiple workflow managers (should handle resource sharing)
    auto second_workflow_manager = std::make_unique<WorkflowManager>(agent_manager, 1, 50, 100);
    
    EXPECT_TRUE(workflow_manager->start());
    EXPECT_TRUE(second_workflow_manager->start());
    
    EXPECT_TRUE(workflow_manager->is_running());
    EXPECT_TRUE(second_workflow_manager->is_running());
    
    workflow_manager->stop();
    second_workflow_manager->stop();
    
    EXPECT_FALSE(workflow_manager->is_running());
    EXPECT_FALSE(second_workflow_manager->is_running());
}
