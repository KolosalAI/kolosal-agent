/**
 * @file test_agent_interfaces.cpp
 * @brief Unit tests for agent interface implementations
 */

#include <gtest/gtest.h>
#include "agent/core/agent_core.hpp"

using namespace kolosal::agents;

class AgentInterfacesTest : public ::testing::Test {
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

TEST_F(AgentInterfacesTest, FunctionManagerInterface) {
    auto func_manager = test_agent->get__function_manager();
    EXPECT_NE(func_manager, nullptr);
}

TEST_F(AgentInterfacesTest, JobManagerInterface) {
    auto job_manager = test_agent->get__job_manager();
    EXPECT_NE(job_manager, nullptr);
}

TEST_F(AgentInterfacesTest, MemoryManagerInterface) {
    auto memory_manager = test_agent->get__memory_manager();
    EXPECT_NE(memory_manager, nullptr);
}

TEST_F(AgentInterfacesTest, PlanningCoordinatorInterface) {
    auto planning_coord = test_agent->get__planning_coordinator();
    EXPECT_NE(planning_coord, nullptr);
}

TEST_F(AgentInterfacesTest, ToolRegistryInterface) {
    auto tool_registry = test_agent->get__tool_registry();
    EXPECT_NE(tool_registry, nullptr);
}

TEST_F(AgentInterfacesTest, BasicAgentProperties) {
    EXPECT_FALSE(test_agent->get__agent_name().empty());
    EXPECT_FALSE(test_agent->get__agent_type().empty());
}
