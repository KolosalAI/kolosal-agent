/**
 * @file test_config_manager.cpp
 * @brief Tests for Agent Configuration Manager
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 */

#include "../external/yaml-cpp/test/gtest-1.11.0/googletest/include/gtest/gtest.h"
#include "../include/agent_config.hpp"
#include <json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

class AgentConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = std::make_unique<AgentConfigManager>();
        createTestConfigFile();
    }
    
    void TearDown() override {
        cleanupTestFiles();
    }
    
    void createTestConfigFile() {
        test_config_path_ = "test_config.yaml";
        std::ofstream config_file(test_config_path_);
        config_file << R"(
system:
  name: "Test System"
  version: "1.0.0"
  host: "localhost"
  port: 8080
  log_level: "debug"
  max_concurrent_requests: 50

system_instruction: |
  Test system instruction for agents.
  This is a multi-line instruction.

agents:
  - name: "TestAgent1"
    capabilities: ["chat", "analysis"]
    auto_start: true
    model: "test-model-1"
    system_prompt: "You are TestAgent1"
  
  - name: "TestAgent2"
    capabilities: ["research", "summarization"]
    auto_start: false
    model: "test-model-2"
    system_prompt: "You are TestAgent2"

functions:
  test_function:
    description: "A test function"
    timeout: 15000
    parameters:
      - name: "input"
        type: "string"
        required: true
        description: "Input parameter"
      - name: "optional_param"
        type: "number"
        required: false
        description: "Optional parameter"
  
  another_function:
    description: "Another test function"
    timeout: 30000
    parameters: []

performance:
  max_memory_usage: "2GB"
  cache_size: "512MB"
  worker_threads: 4
  request_timeout: 25000
  max_request_size: "20MB"

logging:
  level: "info"
  file: "test.log"
  max_file_size: "50MB"
  max_files: 5
  console_output: true

security:
  enable_cors: true
  allowed_origins: ["http://localhost:3000"]
  max_request_rate: 200
  enable_auth: true
  api_key: "test-api-key"
)";
        config_file.close();
        
        // Create invalid config file for error testing
        invalid_config_path_ = "invalid_config.yaml";
        std::ofstream invalid_file(invalid_config_path_);
        invalid_file << R"(
invalid_yaml_structure:
  - this is not valid
    - missing proper indentation
  malformed: [unclosed array
)";
        invalid_file.close();
    }
    
    void cleanupTestFiles() {
        std::remove(test_config_path_.c_str());
        std::remove(invalid_config_path_.c_str());
        std::remove("test.log");
    }
    
protected:
    std::unique_ptr<AgentConfigManager> config_manager_;
    std::string test_config_path_;
    std::string invalid_config_path_;
};

TEST_F(AgentConfigManagerTest, DefaultConstruction) {
    auto default_manager = std::make_unique<AgentConfigManager>();
    EXPECT_NE(default_manager, nullptr);
    
    // Should have default configuration
    const auto& config = default_manager->get_config();
    EXPECT_FALSE(config.system.name.empty());
}

TEST_F(AgentConfigManagerTest, LoadValidConfiguration) {
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    const auto& config = config_manager_->get_config();
    
    // Verify system configuration
    EXPECT_EQ(config.system.name, "Test System");
    EXPECT_EQ(config.system.version, "1.0.0");
    EXPECT_EQ(config.system.host, "localhost");
    EXPECT_EQ(config.system.port, 8080);
    EXPECT_EQ(config.system.log_level, "debug");
    EXPECT_EQ(config.system.max_concurrent_requests, 50);
    
    // Verify system instruction
    EXPECT_FALSE(config_manager_->get_system_instruction().empty());
    EXPECT_TRUE(config_manager_->get_system_instruction().find("Test system instruction") != std::string::npos);
}

TEST_F(AgentConfigManagerTest, LoadAgentConfigurations) {
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    const auto& agent_configs = config_manager_->get_agent_configs();
    EXPECT_EQ(agent_configs.size(), 2);
    
    // Check first agent
    const auto& agent1 = agent_configs[0];
    EXPECT_EQ(agent1.name, "TestAgent1");
    EXPECT_TRUE(agent1.auto_start);
    EXPECT_EQ(agent1.model, "test-model-1");
    EXPECT_EQ(agent1.system_prompt, "You are TestAgent1");
    EXPECT_EQ(agent1.capabilities.size(), 2);
    EXPECT_EQ(agent1.capabilities[0], "chat");
    EXPECT_EQ(agent1.capabilities[1], "analysis");
    
    // Check second agent
    const auto& agent2 = agent_configs[1];
    EXPECT_EQ(agent2.name, "TestAgent2");
    EXPECT_FALSE(agent2.auto_start);
    EXPECT_EQ(agent2.model, "test-model-2");
    EXPECT_EQ(agent2.system_prompt, "You are TestAgent2");
    EXPECT_EQ(agent2.capabilities.size(), 2);
    EXPECT_EQ(agent2.capabilities[0], "research");
    EXPECT_EQ(agent2.capabilities[1], "summarization");
}

TEST_F(AgentConfigManagerTest, LoadFunctionConfigurations) {
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    const auto& function_configs = config_manager_->get_function_configs();
    EXPECT_EQ(function_configs.size(), 2);
    
    // Check first function
    auto it = function_configs.find("test_function");
    ASSERT_NE(it, function_configs.end());
    
    const auto& func1 = it->second;
    EXPECT_EQ(func1.description, "A test function");
    EXPECT_EQ(func1.timeout, 15000);
    EXPECT_EQ(func1.parameters.size(), 2);
    
    // Check parameters
    EXPECT_EQ(func1.parameters[0]["name"], "input");
    EXPECT_EQ(func1.parameters[0]["type"], "string");
    EXPECT_EQ(func1.parameters[0]["required"], true);
    
    EXPECT_EQ(func1.parameters[1]["name"], "optional_param");
    EXPECT_EQ(func1.parameters[1]["type"], "number");
    EXPECT_EQ(func1.parameters[1]["required"], false);
    
    // Check second function
    it = function_configs.find("another_function");
    ASSERT_NE(it, function_configs.end());
    
    const auto& func2 = it->second;
    EXPECT_EQ(func2.description, "Another test function");
    EXPECT_EQ(func2.timeout, 30000);
    EXPECT_EQ(func2.parameters.size(), 0);
}

TEST_F(AgentConfigManagerTest, LoadPerformanceConfiguration) {
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    const auto& config = config_manager_->get_config();
    
    EXPECT_EQ(config.performance.max_memory_usage, "2GB");
    EXPECT_EQ(config.performance.cache_size, "512MB");
    EXPECT_EQ(config.performance.worker_threads, 4);
    EXPECT_EQ(config.performance.request_timeout, 25000);
    EXPECT_EQ(config.performance.max_request_size, "20MB");
}

TEST_F(AgentConfigManagerTest, LoadLoggingConfiguration) {
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    const auto& config = config_manager_->get_config();
    
    EXPECT_EQ(config.logging.level, "info");
    EXPECT_EQ(config.logging.file, "test.log");
    EXPECT_EQ(config.logging.max_file_size, "50MB");
    EXPECT_EQ(config.logging.max_files, 5);
    EXPECT_TRUE(config.logging.console_output);
}

TEST_F(AgentConfigManagerTest, LoadSecurityConfiguration) {
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    const auto& config = config_manager_->get_config();
    
    EXPECT_TRUE(config.security.enable_cors);
    EXPECT_EQ(config.security.allowed_origins.size(), 1);
    EXPECT_EQ(config.security.allowed_origins[0], "http://localhost:3000");
    EXPECT_EQ(config.security.max_request_rate, 200);
    EXPECT_TRUE(config.security.enable_auth);
    EXPECT_EQ(config.security.api_key, "test-api-key");
}

TEST_F(AgentConfigManagerTest, LoadNonexistentFile) {
    EXPECT_FALSE(config_manager_->load_config("nonexistent.yaml"));
    
    // Should still have default configuration
    const auto& config = config_manager_->get_config();
    EXPECT_FALSE(config.system.name.empty());
}

TEST_F(AgentConfigManagerTest, LoadInvalidYAML) {
    EXPECT_FALSE(config_manager_->load_config(invalid_config_path_));
    
    // Should still have default configuration
    const auto& config = config_manager_->get_config();
    EXPECT_FALSE(config.system.name.empty());
}

TEST_F(AgentConfigManagerTest, ReloadConfiguration) {
    // Load initial configuration
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    const auto& initial_config = config_manager_->get_config();
    EXPECT_EQ(initial_config.system.name, "Test System");
    
    // Modify the configuration file
    std::ofstream modified_file(test_config_path_);
    modified_file << R"(
system:
  name: "Modified Test System"
  version: "2.0.0"
  host: "127.0.0.1"
  port: 9090
  log_level: "error"
  max_concurrent_requests: 100
)";
    modified_file.close();
    
    // Reload configuration
    EXPECT_TRUE(config_manager_->reload_config());
    
    const auto& reloaded_config = config_manager_->get_config();
    EXPECT_EQ(reloaded_config.system.name, "Modified Test System");
    EXPECT_EQ(reloaded_config.system.version, "2.0.0");
    EXPECT_EQ(reloaded_config.system.port, 9090);
}

TEST_F(AgentConfigManagerTest, ConfigValidation) {
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    // Should validate successfully
    EXPECT_TRUE(config_manager_->validate_config());
}

TEST_F(AgentConfigManagerTest, ToJSONConversion) {
    EXPECT_TRUE(config_manager_->load_config(test_config_path_));
    
    json config_json = config_manager_->to_json();
    
    EXPECT_TRUE(config_json.contains("system"));
    EXPECT_TRUE(config_json.contains("agents"));
    EXPECT_TRUE(config_json.contains("functions"));
    EXPECT_TRUE(config_json.contains("performance"));
    EXPECT_TRUE(config_json.contains("logging"));
    EXPECT_TRUE(config_json.contains("security"));
    
    // Verify system data
    EXPECT_EQ(config_json["system"]["name"], "Test System");
    EXPECT_EQ(config_json["system"]["port"], 8080);
    
    // Verify agents array
    EXPECT_TRUE(config_json["agents"].is_array());
    EXPECT_EQ(config_json["agents"].size(), 2);
    EXPECT_EQ(config_json["agents"][0]["name"], "TestAgent1");
    
    // Verify functions object
    EXPECT_TRUE(config_json["functions"].is_object());
    EXPECT_TRUE(config_json["functions"].contains("test_function"));
}

TEST_F(AgentConfigManagerTest, DefaultConfigFallback) {
    auto fallback_manager = std::make_unique<AgentConfigManager>();
    
    // Don't load any config file - should use defaults
    const auto& config = fallback_manager->get_config();
    
    // Should have reasonable defaults
    EXPECT_FALSE(config.system.name.empty());
    EXPECT_GT(config.system.port, 0);
    EXPECT_FALSE(config.system.log_level.empty());
    EXPECT_GT(config.system.max_concurrent_requests, 0);
    
    // Should have default performance settings
    EXPECT_FALSE(config.performance.max_memory_usage.empty());
    EXPECT_GT(config.performance.worker_threads, 0);
    EXPECT_GT(config.performance.request_timeout, 0);
}

TEST_F(AgentConfigManagerTest, PartialConfigurationHandling) {
    // Create config with only partial data
    std::string partial_config_path = "partial_config.yaml";
    std::ofstream partial_file(partial_config_path);
    partial_file << R"(
system:
  name: "Partial System"
  port: 7070

agents:
  - name: "PartialAgent"
    capabilities: ["chat"]

# Missing functions, performance, logging, security sections
)";
    partial_file.close();
    
    EXPECT_TRUE(config_manager_->load_config(partial_config_path));
    
    const auto& config = config_manager_->get_config();
    
    // Should have loaded partial data
    EXPECT_EQ(config.system.name, "Partial System");
    EXPECT_EQ(config.system.port, 7070);
    
    // Should have defaults for missing sections
    EXPECT_GT(config.performance.worker_threads, 0);
    EXPECT_FALSE(config.logging.level.empty());
    
    const auto& agent_configs = config_manager_->get_agent_configs();
    EXPECT_EQ(agent_configs.size(), 1);
    EXPECT_EQ(agent_configs[0].name, "PartialAgent");
    
    std::remove(partial_config_path.c_str());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "Running Agent Configuration Manager Tests..." << std::endl;
    return RUN_ALL_TESTS();
}
