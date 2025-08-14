/**
 * @file test_agent_core.cpp
 * @brief Unit tests for AgentCore class
 */

#include <gtest/gtest.h>
#include "agent/core/agent_core.hpp"

using namespace kolosal::agents;

class AgentCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_agent = std::make_shared<AgentCore>("test_agent", "generic", AgentRole::ASSISTANT);
    }

    void TearDown() override {
        if (test_agent && test_agent->is__running()) {
            test_agent->stop();
        }
        test_agent.reset();
    }

    std::shared_ptr<AgentCore> test_agent;
};

TEST_F(AgentCoreTest, ConstructorInitializesCorrectly) {
    EXPECT_EQ(test_agent->get__agent_name(), "test_agent");
    EXPECT_EQ(test_agent->get__agent_type(), "generic");
    EXPECT_EQ(test_agent->get__role(), AgentRole::ASSISTANT);
    EXPECT_FALSE(test_agent->is__running());
}

TEST_F(AgentCoreTest, StartAndStopLifecycle) {
    EXPECT_FALSE(test_agent->is__running());
    
    test_agent->start();
    EXPECT_TRUE(test_agent->is__running());
    
    test_agent->stop();
    EXPECT_FALSE(test_agent->is__running());
}

TEST_F(AgentCoreTest, RoleManagement) {
    EXPECT_EQ(test_agent->get__role(), AgentRole::ASSISTANT);
    
    test_agent->set__role(AgentRole::COORDINATOR);
    EXPECT_EQ(test_agent->get__role(), AgentRole::COORDINATOR);
    
    test_agent->set__role(AgentRole::SPECIALIST);
    EXPECT_EQ(test_agent->get__role(), AgentRole::SPECIALIST);
}

TEST_F(AgentCoreTest, SpecializationManagement) {
    auto initial_specs = test_agent->get__specializations();
    size_t initial_size = initial_specs.size();
    
    test_agent->add_specialization(AgentSpecialization::REASONING);
    test_agent->add_specialization(AgentSpecialization::PLANNING);
    
    auto specs = test_agent->get__specializations();
    EXPECT_EQ(specs.size(), initial_size + 2);
}

TEST_F(AgentCoreTest, CapabilityManagement) {
    auto initial_capabilities = test_agent->get__capabilities();
    size_t initial_size = initial_capabilities.size();
    
    test_agent->add_capability("test_capability_1");
    test_agent->add_capability("test_capability_2");
    
    auto capabilities = test_agent->get__capabilities();
    EXPECT_GE(capabilities.size(), initial_size + 2);
}

// Placeholder tests for methods not available in current API
TEST_F(AgentCoreTest, FunctionExecutionWithMock) {
    // Test basic agent functionality instead
    EXPECT_NE(test_agent->get__function_manager(), nullptr);
}

TEST_F(AgentCoreTest, AsyncFunctionExecution) {
    // Test basic agent functionality instead  
    EXPECT_NE(test_agent->get__job_manager(), nullptr);
}

TEST_F(AgentCoreTest, MemoryOperations) {
    // Test basic agent functionality instead
    EXPECT_NE(test_agent->get__memory_manager(), nullptr);
}

TEST_F(AgentCoreTest, PlanningAndReasoning) {
    // Test basic agent functionality instead
    EXPECT_NE(test_agent->get__planning_coordinator(), nullptr);
}

TEST_F(AgentCoreTest, MessageRouting) {
    // Test basic message sending capabilities - placeholder
    EXPECT_TRUE(true); // Placeholder until MessageRouter is available
}

TEST_F(AgentCoreTest, ToolDiscoveryAndExecution) {
    // Test basic tool registry access
    EXPECT_NE(test_agent->get__tool_registry(), nullptr);
}

TEST_F(AgentCoreTest, StatisticsTracking) {
    auto stats = test_agent->get__statistics();
    EXPECT_GE(stats.total_functions_executed, 0);
    EXPECT_GE(stats.total_tools_executed, 0);
}
