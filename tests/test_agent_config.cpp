#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "agent_config.hpp"
#include <fstream>
#include <filesystem>
#include <yaml-cpp/yaml.h>

class AgentConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager = std::make_unique<AgentConfigManager>();
        test_config_file = "test_agent_config.yaml";
        
        // Create a test configuration file
        createTestConfigFile();
    }

    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(test_config_file)) {
            std::filesystem::remove(test_config_file);
        }
        config_manager.reset();
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
  max_concurrent_requests: 50

system_instruction: |
  You are a test AI assistant.

agents:
  - name: "TestAgent"
    capabilities: ["chat", "analysis"]
    auto_start: true
    model: "test_model"
    system_prompt: "You are a test agent."

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

    std::unique_ptr<AgentConfigManager> config_manager;
    std::string test_config_file;
};

TEST_F(AgentConfigTest, DefaultConfigurationIsValid) {
    EXPECT_TRUE(config_manager->validate_config());
}

TEST_F(AgentConfigTest, LoadValidConfigFile) {
    EXPECT_TRUE(config_manager->load_config(test_config_file));
    
    const auto& config = config_manager->get_config();
    EXPECT_EQ(config.system.name, "Test Kolosal Agent System");
    EXPECT_EQ(config.system.host, "127.0.0.1");
    EXPECT_EQ(config.system.port, 8080);
}

TEST_F(AgentConfigTest, LoadNonExistentConfigFile) {
    EXPECT_FALSE(config_manager->load_config("non_existent_file.yaml"));
}

TEST_F(AgentConfigTest, ReloadConfiguration) {
    // First load
    EXPECT_TRUE(config_manager->load_config(test_config_file));
    
    // Modify the file
    std::ofstream file(test_config_file);
    file << R"(
system:
  name: "Modified Test System"
  host: "0.0.0.0"
  port: 9090
agents: []
models: {}
functions: {}
)";
    file.close();
    
    // Reload
    EXPECT_TRUE(config_manager->reload_config());
    
    const auto& config = config_manager->get_config();
    EXPECT_EQ(config.system.name, "Modified Test System");
    EXPECT_EQ(config.system.host, "0.0.0.0");
    EXPECT_EQ(config.system.port, 9090);
}

TEST_F(AgentConfigTest, GetSystemInstruction) {
    config_manager->load_config(test_config_file);
    
    std::string instruction = config_manager->get_system_instruction();
    EXPECT_THAT(instruction, ::testing::HasSubstr("test AI assistant"));
}

TEST_F(AgentConfigTest, GetHostAndPort) {
    config_manager->load_config(test_config_file);
    
    EXPECT_EQ(config_manager->get_host(), "127.0.0.1");
    EXPECT_EQ(config_manager->get_port(), 8080);
}

TEST_F(AgentConfigTest, GetAgentConfigs) {
    config_manager->load_config(test_config_file);
    
    const auto& agent_configs = config_manager->get_agent_configs();
    EXPECT_EQ(agent_configs.size(), 1);
    EXPECT_EQ(agent_configs[0].name, "TestAgent");
    EXPECT_THAT(agent_configs[0].capabilities, ::testing::Contains("chat"));
    EXPECT_THAT(agent_configs[0].capabilities, ::testing::Contains("analysis"));
}

TEST_F(AgentConfigTest, GetFunctionConfigs) {
    config_manager->load_config(test_config_file);
    
    const auto& function_configs = config_manager->get_function_configs();
    EXPECT_EQ(function_configs.size(), 1);
    EXPECT_TRUE(function_configs.find("test_function") != function_configs.end());
    EXPECT_EQ(function_configs.at("test_function").description, "Test function");
    EXPECT_EQ(function_configs.at("test_function").timeout, 30000);
}

TEST_F(AgentConfigTest, ConfigurationValidation) {
    config_manager->load_config(test_config_file);
    EXPECT_TRUE(config_manager->validate_config());
}

TEST_F(AgentConfigTest, ToJsonConversion) {
    config_manager->load_config(test_config_file);
    
    json config_json = config_manager->to_json();
    EXPECT_TRUE(config_json.contains("system"));
    EXPECT_TRUE(config_json.contains("agents"));
    EXPECT_TRUE(config_json.contains("models"));
    EXPECT_EQ(config_json["system"]["name"], "Test Kolosal Agent System");
}

TEST_F(AgentConfigTest, PrintConfigSummary) {
    config_manager->load_config(test_config_file);
    
    // This should not throw
    EXPECT_NO_THROW(config_manager->print_config_summary());
}

TEST_F(AgentConfigTest, InvalidYamlFile) {
    // Create invalid YAML file
    std::string invalid_config_file = "invalid_config.yaml";
    std::ofstream file(invalid_config_file);
    file << "invalid: yaml: content: [\n";
    file.close();
    
    EXPECT_FALSE(config_manager->load_config(invalid_config_file));
    
    // Clean up
    std::filesystem::remove(invalid_config_file);
}

TEST_F(AgentConfigTest, EmptyConfigFile) {
    // Create empty config file
    std::string empty_config_file = "empty_config.yaml";
    std::ofstream file(empty_config_file);
    file << "";
    file.close();
    
    EXPECT_FALSE(config_manager->load_config(empty_config_file));
    
    // Clean up
    std::filesystem::remove(empty_config_file);
}

TEST_F(AgentConfigTest, PartialConfigFile) {
    // Create partial config file with only system section
    std::string partial_config_file = "partial_config.yaml";
    std::ofstream file(partial_config_file);
    file << R"(
system:
  name: "Partial System"
  host: "localhost"
  port: 8080
)";
    file.close();
    
    // Should load successfully with defaults for missing sections
    EXPECT_TRUE(config_manager->load_config(partial_config_file));
    
    const auto& config = config_manager->get_config();
    EXPECT_EQ(config.system.name, "Partial System");
    
    // Clean up
    std::filesystem::remove(partial_config_file);
}

TEST_F(AgentConfigTest, ConfigFilePathTracking) {
    config_manager->load_config(test_config_file);
    
    std::string config_path = config_manager->get_config_file_path();
    EXPECT_FALSE(config_path.empty());
    EXPECT_THAT(config_path, ::testing::HasSubstr(test_config_file));
}

TEST_F(AgentConfigTest, MultipleAgentsConfiguration) {
    // Create config with multiple agents
    std::string multi_agent_config = "multi_agent_config.yaml";
    std::ofstream file(multi_agent_config);
    file << R"(
system:
  name: "Multi Agent System"
  host: "127.0.0.1"
  port: 8080

agents:
  - name: "Agent1"
    capabilities: ["chat"]
    auto_start: true
    model: "model1"
    system_prompt: "Agent 1 prompt"
  - name: "Agent2"
    capabilities: ["analysis", "reasoning"]
    auto_start: false
    model: "model2"
    system_prompt: "Agent 2 prompt"
  - name: "Agent3"
    capabilities: ["research"]
    auto_start: true
    model: "model3"
    system_prompt: "Agent 3 prompt"

models: {}
functions: {}
)";
    file.close();
    
    EXPECT_TRUE(config_manager->load_config(multi_agent_config));
    
    const auto& agent_configs = config_manager->get_agent_configs();
    EXPECT_EQ(agent_configs.size(), 3);
    
    // Verify specific agent configurations
    EXPECT_EQ(agent_configs[0].name, "Agent1");
    EXPECT_TRUE(agent_configs[0].auto_start);
    EXPECT_EQ(agent_configs[1].name, "Agent2");
    EXPECT_FALSE(agent_configs[1].auto_start);
    EXPECT_EQ(agent_configs[2].name, "Agent3");
    EXPECT_TRUE(agent_configs[2].auto_start);
    
    // Clean up
    std::filesystem::remove(multi_agent_config);
}
