/**
 * @file test_agent_core_simple.cpp
 * @brief Simple agent core tests that work with current API
 */

#include <gtest/gtest.h>
#include "agent/core/agent_core.hpp"

using namespace kolosal::agents;

class AgentCoreSimpleTest : public ::testing::Test {
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

TEST_F(AgentCoreSimpleTest, BasicConstruction) {
    EXPECT_NE(agent, nullptr);
    EXPECT_EQ(agent->get__agent_name(), "test_agent");
    EXPECT_EQ(agent->get__agent_type(), "test_type");
    EXPECT_EQ(agent->get__role(), AgentRole::GENERIC);
    EXPECT_FALSE(agent->is__running());
}

TEST_F(AgentCoreSimpleTest, LifecycleManagement) {
    EXPECT_FALSE(agent->is__running());
    
    // Start the agent
    agent->start();
    EXPECT_TRUE(agent->is__running());
    
    // Stop the agent
    agent->stop();
    EXPECT_FALSE(agent->is__running());
}

TEST_F(AgentCoreSimpleTest, RoleManagement) {
    EXPECT_EQ(agent->get__role(), AgentRole::GENERIC);
    
    agent->set__role(AgentRole::COORDINATOR);
    EXPECT_EQ(agent->get__role(), AgentRole::COORDINATOR);
    
    agent->set__role(AgentRole::SPECIALIST);
    EXPECT_EQ(agent->get__role(), AgentRole::SPECIALIST);
}

TEST_F(AgentCoreSimpleTest, CapabilityManagement) {
    auto initial_capabilities = agent->get__capabilities();
    size_t initial_size = initial_capabilities.size();
    
    // Add capabilities
    agent->add_capability("test_capability_1");
    agent->add_capability("test_capability_2");
    
    auto capabilities = agent->get__capabilities();
    EXPECT_GE(capabilities.size(), initial_size + 2);
}

TEST_F(AgentCoreSimpleTest, SpecializationManagement) {
    auto initial_specs = agent->get__specializations();
    size_t initial_size = initial_specs.size();
    
    // Add specializations
    agent->add_specialization(AgentSpecialization::REASONING);
    agent->add_specialization(AgentSpecialization::PLANNING);
    
    auto specs = agent->get__specializations();
    EXPECT_EQ(specs.size(), initial_size + 2);
}

TEST_F(AgentCoreSimpleTest, GetStatistics) {
    auto stats = agent->get__statistics();
    
    // Should have valid statistics
    EXPECT_GE(stats.total_functions_executed, 0);
    EXPECT_GE(stats.total_tools_executed, 0);
    EXPECT_GE(stats.total_plans_created, 0);
    EXPECT_GE(stats.memory_entries_count, 0);
    EXPECT_GE(stats.average_execution_time_ms, 0.0);
}
