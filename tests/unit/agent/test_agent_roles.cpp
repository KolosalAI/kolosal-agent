/**
 * @file test_agent_roles.cpp
 * @brief Unit tests for agent roles and specializations
 */

#include <gtest/gtest.h>
#include "agent/core/agent_roles.hpp"

using namespace testing;
using namespace kolosal::agents;

TEST(AgentRolesTest, AgentRoleEnum) {
    EXPECT_NE(AgentRole::GENERIC, AgentRole::ASSISTANT);
    EXPECT_NE(AgentRole::ASSISTANT, AgentRole::MANAGER);
    EXPECT_NE(AgentRole::MANAGER, AgentRole::COORDINATOR);
    EXPECT_NE(AgentRole::COORDINATOR, AgentRole::SPECIALIST);
    EXPECT_NE(AgentRole::SPECIALIST, AgentRole::WORKER);
}

TEST(AgentRolesTest, AgentSpecializationEnum) {
    EXPECT_NE(AgentSpecialization::NONE, AgentSpecialization::REASONING);
    EXPECT_NE(AgentSpecialization::REASONING, AgentSpecialization::PLANNING);
    EXPECT_NE(AgentSpecialization::PLANNING, AgentSpecialization::ANALYSIS);
    EXPECT_NE(AgentSpecialization::ANALYSIS, AgentSpecialization::COMMUNICATION);
}
