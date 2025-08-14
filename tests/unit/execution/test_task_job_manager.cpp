/**
 * @file test_task_job_manager.cpp  
 * @brief Tests for task and job management functionality
 */

#include <gtest/gtest.h>
#include "agent/core/agent_core.hpp"

using namespace kolosal::agents;

class TaskJobManagerTest : public ::testing::Test {
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

TEST_F(TaskJobManagerTest, BasicJobManagerAccess) {
    // Test access to job manager
    auto job_manager = agent->get__job_manager();
    EXPECT_NE(job_manager, nullptr);
}

TEST_F(TaskJobManagerTest, AgentWithJobManager) {
    // Test that agent has job management capabilities
    EXPECT_NE(agent, nullptr);
    EXPECT_NE(agent->get__job_manager(), nullptr);
}

TEST_F(TaskJobManagerTest, JobManagerLifecycle) {
    // Test job manager works with agent lifecycle
    agent->start();
    auto job_manager = agent->get__job_manager();
    EXPECT_NE(job_manager, nullptr);
    agent->stop();
}
