#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "agent_manager.hpp"
#include "agent_config.hpp"
#include <json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
using ::testing::_;
using ::testing::Return;
using ::testing::HasSubstr;

class AgentManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_config_file = "test_agent_manager_config.yaml";
        createTestConfigFile();
        
        auto config_manager = std::make_shared<AgentConfigManager>();
        config_manager->load_config(test_config_file);
        
        agent_manager = std::make_unique<AgentManager>(config_manager);
    }

    void TearDown() override {
        if (agent_manager) {
            agent_manager->stop_all_agents();
            agent_manager->stop_kolosal_server();
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
  name: "Test Kolosal Agent System"
  version: "1.0.0"
  host: "127.0.0.1"
  port: 8080
  log_level: "info"
  max_concurrent_requests: 100

system_instruction: |
  You are a test AI assistant.

agents:
  - name: "TestAgent1"
    capabilities: ["chat", "analysis"]
    auto_start: false
    model: "test_model"
    system_prompt: "You are test agent 1."
  - name: "TestAgent2"
    capabilities: ["research"]
    auto_start: false
    model: "test_model"
    system_prompt: "You are test agent 2."

models:
  test_model:
    id: "test_model"
    actual_name: "test_model_actual"
    type: "llama"
    description: "Test model"

functions:
  test_function:
    description: "Test function"
    timeout: 30000
    parameters: []

performance:
  max_memory_usage: "1GB"
  cache_size: "100MB"
  worker_threads: 4
  request_timeout: 30
  max_request_size: "10MB"

logging:
  level: "info"
  file: "test.log"
  max_file_size: "10MB"
  max_files: 5
  console_output: true

security:
  enable_cors: true
  allowed_origins: ["*"]
  max_request_rate: 100
  enable_auth: false
  api_key: ""
)";
        file.close();
    }

    std::unique_ptr<AgentManager> agent_manager;
    std::string test_config_file;
};

TEST_F(AgentManagerTest, ConstructorWithoutConfigManager) {
    auto manager = std::make_unique<AgentManager>();
    EXPECT_TRUE(manager->get_config_manager() != nullptr);
}

TEST_F(AgentManagerTest, ConstructorWithConfigManager) {
    auto config_manager = std::make_shared<AgentConfigManager>();
    auto manager = std::make_unique<AgentManager>(config_manager);
    EXPECT_EQ(manager->get_config_manager(), config_manager);
}

TEST_F(AgentManagerTest, LoadConfiguration) {
    EXPECT_TRUE(agent_manager->load_configuration(test_config_file));
}

TEST_F(AgentManagerTest, CreateAgent) {
    std::string agent_id = agent_manager->create_agent("NewTestAgent");
    EXPECT_FALSE(agent_id.empty());
    EXPECT_TRUE(agent_manager->agent_exists(agent_id));
    
    Agent* agent = agent_manager->get_agent(agent_id);
    EXPECT_NE(agent, nullptr);
    EXPECT_EQ(agent->get_name(), "NewTestAgent");
}

TEST_F(AgentManagerTest, CreateAgentWithCapabilities) {
    std::vector<std::string> capabilities = {"analysis", "reasoning", "chat"};
    std::string agent_id = agent_manager->create_agent("CapableAgent", capabilities);
    
    EXPECT_FALSE(agent_id.empty());
    Agent* agent = agent_manager->get_agent(agent_id);
    EXPECT_NE(agent, nullptr);
    
    const auto& agent_capabilities = agent->get_capabilities();
    for (const auto& capability : capabilities) {
        EXPECT_THAT(agent_capabilities, ::testing::Contains(capability));
    }
}

TEST_F(AgentManagerTest, CreateAgentWithConfig) {
    json config;
    config["capabilities"] = json::array({"custom_capability"});
    config["model"] = "test_model";
    config["system_prompt"] = "Custom system prompt";
    
    std::string agent_id = agent_manager->create_agent_with_config("ConfigAgent", config);
    
    EXPECT_FALSE(agent_id.empty());
    Agent* agent = agent_manager->get_agent(agent_id);
    EXPECT_NE(agent, nullptr);
    EXPECT_EQ(agent->get_name(), "ConfigAgent");
}

TEST_F(AgentManagerTest, StartAndStopAgent) {
    std::string agent_id = agent_manager->create_agent("StartStopAgent");
    
    EXPECT_TRUE(agent_manager->start_agent(agent_id));
    
    Agent* agent = agent_manager->get_agent(agent_id);
    EXPECT_TRUE(agent->is_running());
    
    agent_manager->stop_agent(agent_id);
    EXPECT_FALSE(agent->is_running());
}

TEST_F(AgentManagerTest, StartNonExistentAgent) {
    EXPECT_FALSE(agent_manager->start_agent("non_existent_id"));
}

TEST_F(AgentManagerTest, DeleteAgent) {
    std::string agent_id = agent_manager->create_agent("DeleteAgent");
    EXPECT_TRUE(agent_manager->agent_exists(agent_id));
    
    EXPECT_TRUE(agent_manager->delete_agent(agent_id));
    EXPECT_FALSE(agent_manager->agent_exists(agent_id));
    EXPECT_EQ(agent_manager->get_agent(agent_id), nullptr);
}

TEST_F(AgentManagerTest, DeleteNonExistentAgent) {
    EXPECT_FALSE(agent_manager->delete_agent("non_existent_id"));
}

TEST_F(AgentManagerTest, GetAgentIdByName) {
    std::string agent_id = agent_manager->create_agent("NamedAgent");
    
    std::string found_id = agent_manager->get_agent_id_by_name("NamedAgent");
    EXPECT_EQ(found_id, agent_id);
    
    // Test non-existent agent
    std::string not_found = agent_manager->get_agent_id_by_name("NonExistentAgent");
    EXPECT_TRUE(not_found.empty());
}

TEST_F(AgentManagerTest, GetAgentNameById) {
    std::string agent_id = agent_manager->create_agent("NamedAgent");
    
    std::string found_name = agent_manager->get_agent_name_by_id(agent_id);
    EXPECT_EQ(found_name, "NamedAgent");
    
    // Test non-existent agent
    std::string not_found = agent_manager->get_agent_name_by_id("non_existent_id");
    EXPECT_TRUE(not_found.empty());
}

TEST_F(AgentManagerTest, ListAgents) {
    // Create multiple agents
    std::string agent1_id = agent_manager->create_agent("Agent1");
    std::string agent2_id = agent_manager->create_agent("Agent2");
    
    json agents_list = agent_manager->list_agents();
    EXPECT_TRUE(agents_list.is_array());
    EXPECT_EQ(agents_list.size(), 2);
    
    // Verify agent information in list
    bool found_agent1 = false, found_agent2 = false;
    for (const auto& agent_info : agents_list) {
        if (agent_info["name"] == "Agent1") found_agent1 = true;
        if (agent_info["name"] == "Agent2") found_agent2 = true;
    }
    EXPECT_TRUE(found_agent1);
    EXPECT_TRUE(found_agent2);
}

TEST_F(AgentManagerTest, StopAllAgents) {
    // Create and start multiple agents
    std::string agent1_id = agent_manager->create_agent("Agent1");
    std::string agent2_id = agent_manager->create_agent("Agent2");
    
    agent_manager->start_agent(agent1_id);
    agent_manager->start_agent(agent2_id);
    
    EXPECT_TRUE(agent_manager->get_agent(agent1_id)->is_running());
    EXPECT_TRUE(agent_manager->get_agent(agent2_id)->is_running());
    
    agent_manager->stop_all_agents();
    
    EXPECT_FALSE(agent_manager->get_agent(agent1_id)->is_running());
    EXPECT_FALSE(agent_manager->get_agent(agent2_id)->is_running());
}

TEST_F(AgentManagerTest, ExecuteAgentFunction) {
    std::string agent_id = agent_manager->create_agent("FunctionAgent");
    Agent* agent = agent_manager->get_agent(agent_id);
    
    // Register a test function
    agent->register_function("test_function", [](const json& params) -> json {
        json result;
        result["status"] = "success";
        result["echo"] = params;
        return result;
    });
    
    json params;
    params["test_param"] = "test_value";
    
    json result = agent_manager->execute_agent_function(agent_id, "test_function", params);
    EXPECT_EQ(result["status"], "success");
    EXPECT_EQ(result["echo"]["test_param"], "test_value");
}

TEST_F(AgentManagerTest, ExecuteFunctionOnNonExistentAgent) {
    json params;
    json result = agent_manager->execute_agent_function("non_existent_id", "test_function", params);
    
    EXPECT_TRUE(result.contains("error"));
}

TEST_F(AgentManagerTest, InitializeDefaultAgents) {
    agent_manager->load_configuration(test_config_file);
    agent_manager->initialize_default_agents();
    
    // Should have created agents from configuration
    std::string agent1_id = agent_manager->get_agent_id_by_name("TestAgent1");
    std::string agent2_id = agent_manager->get_agent_id_by_name("TestAgent2");
    
    EXPECT_FALSE(agent1_id.empty());
    EXPECT_FALSE(agent2_id.empty());
    
    Agent* agent1 = agent_manager->get_agent(agent1_id);
    Agent* agent2 = agent_manager->get_agent(agent2_id);
    
    EXPECT_NE(agent1, nullptr);
    EXPECT_NE(agent2, nullptr);
    EXPECT_EQ(agent1->get_name(), "TestAgent1");
    EXPECT_EQ(agent2->get_name(), "TestAgent2");
}

TEST_F(AgentManagerTest, CreateDuplicateAgentNames) {
    std::string agent1_id = agent_manager->create_agent("DuplicateName");
    std::string agent2_id = agent_manager->create_agent("DuplicateName");
    
    // Both should be created with different IDs
    EXPECT_FALSE(agent1_id.empty());
    EXPECT_FALSE(agent2_id.empty());
    EXPECT_NE(agent1_id, agent2_id);
}

// Note: Kolosal Server tests are commented out as they require actual server setup
/*
TEST_F(AgentManagerTest, KolosalServerManagement) {
    // These tests require actual server setup and may not work in CI environment
    EXPECT_FALSE(agent_manager->is_kolosal_server_running());
    
    // Note: Actual server start might fail without proper setup
    // EXPECT_TRUE(agent_manager->start_kolosal_server());
    // EXPECT_TRUE(agent_manager->is_kolosal_server_running());
    
    std::string server_url = agent_manager->get_kolosal_server_url();
    EXPECT_FALSE(server_url.empty());
    
    // auto status = agent_manager->get_kolosal_server_status();
    // EXPECT_NO_THROW(agent_manager->stop_kolosal_server());
}
*/

TEST_F(AgentManagerTest, GetNonExistentAgent) {
    Agent* agent = agent_manager->get_agent("non_existent_id");
    EXPECT_EQ(agent, nullptr);
}

TEST_F(AgentManagerTest, AgentExistsCheck) {
    std::string agent_id = agent_manager->create_agent("ExistenceTestAgent");
    
    EXPECT_TRUE(agent_manager->agent_exists(agent_id));
    EXPECT_FALSE(agent_manager->agent_exists("non_existent_id"));
}

TEST_F(AgentManagerTest, CreateAgentFromConfigStruct) {
    AgentSystemConfig::AgentConfig agent_config;
    agent_config.name = "StructAgent";
    agent_config.capabilities = {"capability1", "capability2"};
    agent_config.auto_start = false;
    agent_config.model = "test_model";
    agent_config.system_prompt = "Test system prompt";
    
    std::string agent_id = agent_manager->create_agent_from_config(agent_config);
    
    EXPECT_FALSE(agent_id.empty());
    Agent* agent = agent_manager->get_agent(agent_id);
    EXPECT_NE(agent, nullptr);
    EXPECT_EQ(agent->get_name(), "StructAgent");
    
    const auto& capabilities = agent->get_capabilities();
    EXPECT_THAT(capabilities, ::testing::Contains("capability1"));
    EXPECT_THAT(capabilities, ::testing::Contains("capability2"));
}
