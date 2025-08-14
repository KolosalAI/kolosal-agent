/**
 * @file test_function_execution_manager.cpp
 * @brief Tests for function execution manager functionality
 */

#include <gtest/gtest.h>
#include "agent/core/agent_core.hpp"

using namespace kolosal::agents;

class FunctionExecutionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        agent = std::make_shared<AgentCore>("test_agent", "test_type", AgentRole::GENERIC);
    }

    void TearDown() override {
        if (agent && agent->is__running()) {
            agent->stop();
        }
        agent.reset();
    }

    std::shared_ptr<AgentCore> agent;
};

TEST_F(FunctionExecutionManagerTest, BasicAgentFunctionality) {
    // Test that agent has basic functionality
    EXPECT_NE(agent, nullptr);
    EXPECT_NE(agent->get__function_manager(), nullptr);
}

TEST_F(FunctionExecutionManagerTest, AgentLifecycleForFunctions) {
    // Test agent lifecycle affects function execution
    EXPECT_FALSE(agent->is__running());
    agent->start();
    EXPECT_TRUE(agent->is__running());
    agent->stop();
    EXPECT_FALSE(agent->is__running());
}

TEST_F(FunctionExecutionManagerTest, BasicFunctionManagerAccess) {
    // Test access to function manager
    auto func_manager = agent->get__function_manager();
    EXPECT_NE(func_manager, nullptr);
}
