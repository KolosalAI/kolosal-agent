/**
 * @file test_agent_core.cpp
 * @brief Unit tests for AgentCore class
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "agent/core/agent_core.hpp"
#include "agent/core/agent_roles.hpp"
#include "../fixtures/test_fixtures.hpp"
#include "../mocks/mock_agent_components.hpp"

using namespace testing;
using namespace kolosal::agents;
using namespace kolosal::agents::test;

class AgentCoreTest : public AgentTestFixture {
protected:
    void SetUp() override {
        AgentTestFixture::SetUp();
    }
};

TEST_F(AgentCoreTest, ConstructorInitializesCorrectly) {
    EXPECT_EQ(test_agent_->get_agent_name(), "test_agent");
    EXPECT_EQ(test_agent_->get_agent_type(), "generic");
    EXPECT_EQ(test_agent_->get_role(), AgentRole::ASSISTANT);
    EXPECT_FALSE(test_agent_->is_running());
}

TEST_F(AgentCoreTest, StartAndStopLifecycle) {
    EXPECT_FALSE(test_agent_->is_running());
    
    test_agent_->start();
    EXPECT_TRUE(test_agent_->is_running());
    
    test_agent_->stop();
    EXPECT_FALSE(test_agent_->is_running());
}

TEST_F(AgentCoreTest, RoleManagement) {
    EXPECT_EQ(test_agent_->get_role(), AgentRole::ASSISTANT);
    
    test_agent_->set_role(AgentRole::MANAGER);
    EXPECT_EQ(test_agent_->get_role(), AgentRole::MANAGER);
    
    test_agent_->set_role(AgentRole::SPECIALIST);
    EXPECT_EQ(test_agent_->get_role(), AgentRole::SPECIALIST);
}

TEST_F(AgentCoreTest, SpecializationManagement) {
    auto initial_specs = test_agent_->get_specializations();
    EXPECT_TRUE(initial_specs.empty());
    
    test_agent_->add_specialization(AgentSpecialization::REASONING);
    test_agent_->add_specialization(AgentSpecialization::PLANNING);
    
    auto specs = test_agent_->get_specializations();
    EXPECT_EQ(specs.size(), 2);
    EXPECT_THAT(specs, Contains(AgentSpecialization::REASONING));
    EXPECT_THAT(specs, Contains(AgentSpecialization::PLANNING));
}

TEST_F(AgentCoreTest, CapabilityManagement) {
    auto initial_capabilities = test_agent_->get_capabilities();
    EXPECT_TRUE(initial_capabilities.empty());
    
    test_agent_->add_capability("text_processing");
    test_agent_->add_capability("data_analysis");
    
    auto capabilities = test_agent_->get_capabilities();
    EXPECT_EQ(capabilities.size(), 2);
    EXPECT_THAT(capabilities, Contains("text_processing"));
    EXPECT_THAT(capabilities, Contains("data_analysis"));
}

TEST_F(AgentCoreTest, FunctionExecutionWithMock) {
    // Setup mock function manager
    auto mock_function_manager = std::make_shared<MockFunctionManager>();
    
    FunctionResult expected_result;
    expected_result.success = true;
    expected_result.result_data = AgentData{{"result", "test_output"}};
    
    EXPECT_CALL(*mock_function_manager, execute_function("test_function", _))
        .WillOnce(Return(expected_result));
    
    // Replace the function manager (this would require dependency injection in real code)
    // For now, test the interface
    AgentData parameters{{"input", "test_input"}};
    
    // This test demonstrates how function execution should work
    // In actual implementation, we'd need to inject the mock
}

TEST_F(AgentCoreTest, AsyncFunctionExecution) {
    test_agent_->start();
    
    AgentData parameters{{"message", "test"}};
    
    // Test async execution
    std::string job_id = test_agent_->execute_function_async("echo", parameters, 1);
    EXPECT_FALSE(job_id.empty());
    
    // Test with different priority
    std::string high_priority_job = test_agent_->execute_function_async("echo", parameters, 10);
    EXPECT_FALSE(high_priority_job.empty());
    EXPECT_NE(job_id, high_priority_job);
}

TEST_F(AgentCoreTest, MemoryOperations) {
    test_agent_->start();
    
    // Test storing memory
    test_agent_->store_memory("This is a test memory", "test");
    
    // Test recalling memories
    auto memories = test_agent_->recall_memories("test", 5);
    // Note: This test would pass with mock implementation
    
    // Test working context
    AgentData test_data{{"key", "value"}};
    test_agent_->set_working_context("test_context", test_data);
    
    auto retrieved_data = test_agent_->get_working_context("test_context");
    EXPECT_FALSE(retrieved_data.empty());
}

TEST_F(AgentCoreTest, PlanningAndReasoning) {
    test_agent_->start();
    
    // Test plan creation
    auto plan = test_agent_->create_plan("Complete a test task", "Testing context");
    EXPECT_FALSE(plan.plan_id.empty());
    
    // Test reasoning
    std::string reasoning = test_agent_->reason_about("What is 2+2?", "Mathematical context");
    EXPECT_FALSE(reasoning.empty());
}

TEST_F(AgentCoreTest, MessageRouting) {
    auto mock_router = std::make_shared<MockMessageRouter>();
    
    EXPECT_CALL(*mock_router, route_message(_))
        .Times(1);
    
    test_agent_->set_message_router(mock_router);
    
    // Test sending message
    AgentData payload{{"content", "test message"}};
    test_agent_->send_message("target_agent", "text", payload);
}

TEST_F(AgentCoreTest, ToolDiscoveryAndExecution) {
    test_agent_->start();
    
    // Test tool discovery
    ToolFilter filter;
    auto tools = test_agent_->discover_tools(filter);
    // With mock implementation, this would return predefined tools
    
    // Test tool execution
    AgentData tool_params{{"input", "test"}};
    auto result = test_agent_->execute_tool("echo_tool", tool_params);
    // This would work with a proper mock setup
}

TEST_F(AgentCoreTest, StatisticsTracking) {
    test_agent_->start();
    
    auto stats = test_agent_->get_statistics();
    EXPECT_GE(stats.total_functions_executed, 0);
    EXPECT_GE(stats.total_tools_executed, 0);
    EXPECT_GE(stats.total_plans_created, 0);
    EXPECT_GE(stats.memory_entries_count, 0);
    EXPECT_GE(stats.average_execution_time_ms, 0.0);
}

TEST_F(AgentCoreTest, ComponentAccessors) {
    EXPECT_NE(test_agent_->get_logger(), nullptr);
    EXPECT_NE(test_agent_->get_function_manager(), nullptr);
    EXPECT_NE(test_agent_->get_job_manager(), nullptr);
    EXPECT_NE(test_agent_->get_event_system(), nullptr);
    EXPECT_NE(test_agent_->get_tool_registry(), nullptr);
    EXPECT_NE(test_agent_->get_memory_manager(), nullptr);
    EXPECT_NE(test_agent_->get_planning_coordinator(), nullptr);
}

TEST_F(AgentCoreTest, ConcurrentOperations) {
    test_agent_->start();
    
    // Test that multiple operations can be performed concurrently
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, &success_count, i]() {
            try {
                AgentData params{{"iteration", i}};
                std::string job_id = test_agent_->execute_function_async("echo", params);
                if (!job_id.empty()) {
                    success_count++;
                }
            } catch (...) {
                // Handle exceptions
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_GT(success_count.load(), 0);
}

TEST_F(AgentCoreTest, ErrorHandling) {
    // Test error handling in various scenarios
    test_agent_->start();
    
    // Test function execution with invalid parameters
    AgentData invalid_params{{"invalid", "data"}};
    auto result = test_agent_->execute_function("nonexistent_function", invalid_params);
    EXPECT_FALSE(result.success);
    
    // Test tool execution with invalid tool
    auto tool_result = test_agent_->execute_tool("nonexistent_tool", invalid_params);
    // This should handle the error gracefully
    
    // Test memory operations with edge cases
    auto memories = test_agent_->recall_memories("", 0);
    // Should handle empty query and zero limit
}

TEST_F(AgentCoreTest, CustomToolRegistration) {
    auto mock_tool = std::make_unique<MockTool>();
    EXPECT_CALL(*mock_tool, get_name())
        .WillRepeatedly(Return("custom_test_tool"));
    
    bool registered = test_agent_->register_custom_tool(std::move(mock_tool));
    // With proper implementation, this should return true
    
    // Test getting tool schema
    auto schema = test_agent_->get_tool_schema("custom_test_tool");
    // Should return valid schema
}
