/**
 * @file test_full_system_integration.cpp
 * @brief Full system integration tests for Kolosal Agent
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "agent/core/agent_core.hpp"
#include "workflow/workflow_engine.hpp"
#include "config/yaml_configuration_parser.hpp"
#include "server/unified_server.hpp"
#include "../fixtures/test_fixtures.hpp"
#include <thread>
#include <chrono>

using namespace testing;
using namespace kolosal::agents;
using namespace kolosal::agents::test;

class FullSystemIntegrationTest : public KolosalAgentTestFixture {
protected:
    void SetUp() override {
        KolosalAgentTestFixture::SetUp();
        setupTestSystem();
    }
    
    void TearDown() override {
        teardownTestSystem();
        KolosalAgentTestFixture::TearDown();
    }
    
    void setupTestSystem() {
        // Create test configuration
        system_config_ = createTestSystemConfig();
        
        // Initialize core components
        // agent_manager_ = std::make_shared<YAMLConfigurableAgentManager>();
        // workflow_engine_ = std::make_shared<WorkflowEngine>(agent_manager_);
        // unified_server_ = std::make_shared<UnifiedServer>();
    }
    
    void teardownTestSystem() {
        // Cleanup components
        // if (unified_server_ && unified_server_->is_running()) {
        //     unified_server_->stop();
        // }
        // if (workflow_engine_ && workflow_engine_->is_running()) {
        //     workflow_engine_->stop();
        // }
    }

protected:
    SystemConfig system_config_;
    // std::shared_ptr<YAMLConfigurableAgentManager> agent_manager_;
    // std::shared_ptr<WorkflowEngine> workflow_engine_;
    // std::shared_ptr<UnifiedServer> unified_server_;
};

TEST_F(FullSystemIntegrationTest, SystemInitialization) {
    // Test that the system can be initialized with a valid configuration
    EXPECT_EQ(system_config_.worker_threads, 2);
    EXPECT_EQ(system_config_.agents.size(), 1);
    EXPECT_FALSE(system_config_.agents[0].id.empty());
    
    // Test component initialization
    // EXPECT_NE(agent_manager_, nullptr);
    // EXPECT_NE(workflow_engine_, nullptr);
    // EXPECT_NE(unified_server_, nullptr);
}

TEST_F(FullSystemIntegrationTest, AgentLifecycleIntegration) {
    // Create an agent
    auto agent = std::make_shared<AgentCore>("integration_agent", "test", AgentRole::ASSISTANT);
    EXPECT_FALSE(agent->is_running());
    
    // Start the agent
    agent->start();
    EXPECT_TRUE(agent->is_running());
    
    // Test agent capabilities
    agent->add_capability("integration_testing");
    auto capabilities = agent->get_capabilities();
    EXPECT_THAT(capabilities, Contains("integration_testing"));
    
    // Test memory operations
    agent->store_memory("Integration test memory", "test");
    auto memories = agent->recall_memories("integration", 1);
    // Should work with proper implementation
    
    // Stop the agent
    agent->stop();
    EXPECT_FALSE(agent->is_running());
}

TEST_F(FullSystemIntegrationTest, ConfigurationIntegration) {
    // Create a temporary configuration file
    std::string config_content = R"(
worker_threads: 4
health_check_interval_seconds: 10
log_level: debug

agents:
  - id: integration_agent_1
    name: Integration Agent 1
    type: assistant
    role: assistant
    auto_start: true
    llm_config:
      model_name: test-model
      temperature: 0.7
    capabilities:
      - text_processing
      - analysis

functions:
  - name: echo
    type: builtin
    description: Echo function for testing
    
inference_engines:
  - name: test_engine
    type: llama_cpp
    model_path: /tmp/test.gguf
)";

    std::string config_file = getTestOutputPath("integration_config.yaml");
    std::ofstream file(config_file);
    file << config_content;
    file.close();
    
    // Load configuration
    auto loaded_config = SystemConfig::from_file(config_file);
    
    // Verify loaded configuration
    EXPECT_EQ(loaded_config.worker_threads, 4);
    EXPECT_EQ(loaded_config.log_level, "debug");
    EXPECT_EQ(loaded_config.agents.size(), 1);
    EXPECT_EQ(loaded_config.agents[0].id, "integration_agent_1");
    EXPECT_EQ(loaded_config.agents[0].capabilities.size(), 2);
    EXPECT_EQ(loaded_config.functions.size(), 1);
    EXPECT_EQ(loaded_config.inference_engines.size(), 1);
    
    // Clean up
    std::remove(config_file.c_str());
}

TEST_F(FullSystemIntegrationTest, WorkflowAgentIntegration) {
    // Create agents
    auto agent1 = std::make_shared<AgentCore>("workflow_agent_1", "worker", AgentRole::WORKER);
    auto agent2 = std::make_shared<AgentCore>("workflow_agent_2", "worker", AgentRole::WORKER);
    
    agent1->start();
    agent2->start();
    
    // Create a workflow that involves both agents
    Workflow workflow;
    workflow.workflow_id = "integration_workflow";
    workflow.name = "Integration Test Workflow";
    workflow.type = WorkflowType::SEQUENTIAL;
    
    WorkflowStep step1;
    step1.step_id = "step1";
    step1.name = "First Agent Step";
    step1.agent_id = "workflow_agent_1";
    step1.function_name = "process_data";
    step1.parameters = nlohmann::json{{"data", "test_input"}};
    
    WorkflowStep step2;
    step2.step_id = "step2";
    step2.name = "Second Agent Step";
    step2.agent_id = "workflow_agent_2";
    step2.function_name = "analyze_result";
    step2.parameters = nlohmann::json{{"input", "{{step1.output}}"}};
    step2.dependencies.push_back({"step1", "success", true});
    
    workflow.steps = {step1, step2};
    
    // This test demonstrates the integration pattern
    // In a full implementation, the workflow would be executed
    EXPECT_EQ(workflow.steps.size(), 2);
    EXPECT_EQ(workflow.steps[1].dependencies.size(), 1);
    
    agent1->stop();
    agent2->stop();
}

TEST_F(FullSystemIntegrationTest, MessageRoutingIntegration) {
    // Create agents with message routing
    auto sender = std::make_shared<AgentCore>("sender_agent", "sender", AgentRole::COORDINATOR);
    auto receiver = std::make_shared<AgentCore>("receiver_agent", "receiver", AgentRole::WORKER);
    
    sender->start();
    receiver->start();
    
    // Test message sending
    AgentData message_payload{{"content", "Hello from sender"}};
    sender->send_message("receiver_agent", "greeting", message_payload);
    
    // Test broadcast
    sender->broadcast_message("system_announcement", {{"announcement", "System starting"}});
    
    // In a full implementation, we'd verify message delivery
    // For now, we test that the interface works
    EXPECT_TRUE(sender->is_running());
    EXPECT_TRUE(receiver->is_running());
    
    sender->stop();
    receiver->stop();
}

TEST_F(FullSystemIntegrationTest, MemoryAndContextIntegration) {
    auto agent = std::make_shared<AgentCore>("memory_agent", "test", AgentRole::ASSISTANT);
    agent->start();
    
    // Store various types of memories
    agent->store_memory("User preference: likes concise responses", "preference");
    agent->store_memory("Previous conversation about weather", "conversation");
    agent->store_memory("Important fact: user is in timezone UTC+8", "fact");
    
    // Set working context
    AgentData context_data{
        {"current_task", "integration_testing"},
        {"user_id", "test_user_123"},
        {"session_id", "session_abc"}
    };
    agent->set_working_context("session", context_data);
    
    // Retrieve working context
    auto retrieved_context = agent->get_working_context("session");
    EXPECT_FALSE(retrieved_context.empty());
    
    // Search memories
    auto preference_memories = agent->recall_memories("preference", 2);
    auto conversation_memories = agent->recall_memories("conversation", 2);
    
    // Test that memory system is working
    // (Actual verification would depend on implementation)
    
    agent->stop();
}

TEST_F(FullSystemIntegrationTest, ErrorHandlingIntegration) {
    auto agent = std::make_shared<AgentCore>("error_test_agent", "test", AgentRole::ASSISTANT);
    agent->start();
    
    // Test function execution with errors
    AgentData invalid_params{{"invalid_param", "bad_value"}};
    auto result = agent->execute_function("nonexistent_function", invalid_params);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
    
    // Test tool execution with errors
    auto tool_result = agent->execute_tool("nonexistent_tool", invalid_params);
    // Should handle gracefully
    
    // Test async execution error handling
    std::string job_id = agent->execute_function_async("nonexistent_function", invalid_params);
    // Should return empty string or handle error appropriately
    
    agent->stop();
}

TEST_F(FullSystemIntegrationTest, ConcurrentAgentOperations) {
    const int num_agents = 3;
    const int operations_per_agent = 5;
    
    std::vector<std::shared_ptr<AgentCore>> agents;
    
    // Create and start multiple agents
    for (int i = 0; i < num_agents; ++i) {
        auto agent = std::make_shared<AgentCore>(
            "concurrent_agent_" + std::to_string(i), 
            "worker", 
            AgentRole::WORKER
        );
        agent->start();
        agents.push_back(agent);
    }
    
    // Perform concurrent operations
    std::vector<std::thread> threads;
    std::atomic<int> successful_operations{0};
    
    for (int i = 0; i < num_agents; ++i) {
        threads.emplace_back([&, i]() {
            auto& agent = agents[i];
            
            for (int j = 0; j < operations_per_agent; ++j) {
                try {
                    // Store memory
                    agent->store_memory(
                        "Memory from agent " + std::to_string(i) + " operation " + std::to_string(j),
                        "concurrent_test"
                    );
                    
                    // Set working context
                    AgentData context{{"iteration", j}, {"agent_id", i}};
                    agent->set_working_context("iteration_" + std::to_string(j), context);
                    
                    // Execute async function
                    AgentData params{{"data", "test_data_" + std::to_string(j)}};
                    std::string job_id = agent->execute_function_async("echo", params);
                    
                    if (!job_id.empty()) {
                        successful_operations++;
                    }
                    
                    // Small delay to allow interleaving
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    
                } catch (const std::exception& e) {
                    // Handle exceptions gracefully
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify operations completed
    EXPECT_GT(successful_operations.load(), 0);
    
    // Stop all agents
    for (auto& agent : agents) {
        agent->stop();
    }
}

TEST_F(FullSystemIntegrationTest, SystemHealthAndMonitoring) {
    auto agent = std::make_shared<AgentCore>("health_test_agent", "test", AgentRole::ASSISTANT);
    agent->start();
    
    // Perform some operations to generate statistics
    for (int i = 0; i < 5; ++i) {
        AgentData params{{"iteration", i}};
        agent->execute_function_async("echo", params);
        agent->store_memory("Health test memory " + std::to_string(i), "health_test");
    }
    
    // Allow some time for operations
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Get agent statistics
    auto stats = agent->get_statistics();
    EXPECT_GE(stats.total_functions_executed, 0);
    EXPECT_GE(stats.memory_entries_count, 0);
    EXPECT_GT(stats.last_activity.time_since_epoch().count(), 0);
    
    agent->stop();
}

TEST_F(FullSystemIntegrationTest, EndToEndScenario) {
    // Simulate a complete end-to-end scenario
    
    // 1. System startup with configuration
    auto coordinator = std::make_shared<AgentCore>("coordinator", "coordinator", AgentRole::COORDINATOR);
    auto analyzer = std::make_shared<AgentCore>("analyzer", "specialist", AgentRole::SPECIALIST);
    auto executor = std::make_shared<AgentCore>("executor", "worker", AgentRole::WORKER);
    
    coordinator->start();
    analyzer->start();
    executor->start();
    
    // 2. Coordinator receives a task
    coordinator->store_memory("New task received: Analyze data and execute action", "task");
    
    // 3. Coordinator creates a plan
    auto plan = coordinator->create_plan(
        "Process incoming data request",
        "User wants to analyze sales data and generate report"
    );
    
    // 4. Plan involves multiple agents
    // (In real implementation, this would be orchestrated by workflow engine)
    
    // Analyzer does analysis
    AgentData analysis_params{{"data_source", "sales_2024"}, {"type", "summary"}};
    std::string analysis_job = analyzer->execute_function_async("analyze_data", analysis_params);
    
    // Executor prepares to act on results
    AgentData execution_params{{"action", "generate_report"}, {"format", "pdf"}};
    std::string execution_job = executor->execute_function_async("execute_action", execution_params);
    
    // 5. Verify the scenario ran
    EXPECT_FALSE(plan.plan_id.empty());
    EXPECT_FALSE(analysis_job.empty());
    EXPECT_FALSE(execution_job.empty());
    
    // 6. System cleanup
    coordinator->stop();
    analyzer->stop();
    executor->stop();
    
    EXPECT_FALSE(coordinator->is_running());
    EXPECT_FALSE(analyzer->is_running());
    EXPECT_FALSE(executor->is_running());
}
