/**
 * @file test_yaml_configuration_parser.cpp
 * @brief Unit tests for YAML configuration parser
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "config/yaml_configuration_parser.hpp"
#include "../fixtures/test_fixtures.hpp"
#include <yaml-cpp/yaml.h>
#include <fstream>

using namespace testing;
using namespace kolosal::agents;
using namespace kolosal::agents::test;

class YAMLConfigurationParserTest : public ConfigurationTestFixture {
protected:
    void SetUp() override {
        ConfigurationTestFixture::SetUp();
    }
};

TEST_F(YAMLConfigurationParserTest, ParseLLMConfig) {
    YAML::Node config;
    config["model_name"] = "test-model";
    config["api_endpoint"] = "http://localhost:8080";
    config["api_key"] = "test-key";
    config["instruction"] = "You are a helpful assistant";
    config["temperature"] = 0.8;
    config["max_tokens"] = 2048;
    config["timeout_seconds"] = 45;
    config["max_retries"] = 5;
    config["stop_sequences"] = std::vector<std::string>{"<|end|>", "STOP"};
    
    auto llm_config = LLMConfig::from_yaml(config);
    
    EXPECT_EQ(llm_config.model_name, "test-model");
    EXPECT_EQ(llm_config.api_endpoint, "http://localhost:8080");
    EXPECT_EQ(llm_config.api_key, "test-key");
    EXPECT_EQ(llm_config.instruction, "You are a helpful assistant");
    EXPECT_DOUBLE_EQ(llm_config.temperature, 0.8);
    EXPECT_EQ(llm_config.max_tokens, 2048);
    EXPECT_EQ(llm_config.timeout_seconds, 45);
    EXPECT_EQ(llm_config.max_retries, 5);
    EXPECT_EQ(llm_config.stop_sequences.size(), 2);
    EXPECT_EQ(llm_config.stop_sequences[0], "<|end|>");
    EXPECT_EQ(llm_config.stop_sequences[1], "STOP");
}

TEST_F(YAMLConfigurationParserTest, ParseLLMConfigDefaults) {
    YAML::Node minimal_config;
    minimal_config["model_name"] = "minimal-model";
    
    auto llm_config = LLMConfig::from_yaml(minimal_config);
    
    EXPECT_EQ(llm_config.model_name, "minimal-model");
    EXPECT_EQ(llm_config.api_endpoint, "");
    EXPECT_EQ(llm_config.api_key, "");
    EXPECT_EQ(llm_config.instruction, "");
    EXPECT_DOUBLE_EQ(llm_config.temperature, 0.7);
    EXPECT_EQ(llm_config.max_tokens, 2048);
    EXPECT_EQ(llm_config.timeout_seconds, 30);
    EXPECT_EQ(llm_config.max_retries, 3);
}

TEST_F(YAMLConfigurationParserTest, LLMConfigToYAML) {
    LLMConfig config;
    config.model_name = "test-model";
    config.api_endpoint = "http://test:8080";
    config.temperature = 0.9;
    config.max_tokens = 4096;
    config.stop_sequences = {"STOP", "END"};
    
    auto yaml_node = config.to_yaml();
    
    EXPECT_EQ(yaml_node["model_name"].as<std::string>(), "test-model");
    EXPECT_EQ(yaml_node["api_endpoint"].as<std::string>(), "http://test:8080");
    EXPECT_DOUBLE_EQ(yaml_node["temperature"].as<double>(), 0.9);
    EXPECT_EQ(yaml_node["max_tokens"].as<int>(), 4096);
    
    auto stop_seqs = yaml_node["stop_sequences"].as<std::vector<std::string>>();
    EXPECT_EQ(stop_seqs.size(), 2);
    EXPECT_EQ(stop_seqs[0], "STOP");
    EXPECT_EQ(stop_seqs[1], "END");
}

TEST_F(YAMLConfigurationParserTest, ParseFunctionConfig) {
    YAML::Node config;
    config["name"] = "test_function";
    config["type"] = "llm";
    config["description"] = "A test function";
    config["async_capable"] = true;
    config["timeout_ms"] = 10000;
    config["parameters"]["param1"] = "value1";
    config["parameters"]["param2"] = "value2";
    config["implementation"] = "custom implementation";
    config["endpoint"] = "http://api.example.com/function";
    
    auto func_config = FunctionConfig::from_yaml(config);
    
    EXPECT_EQ(func_config.name, "test_function");
    EXPECT_EQ(func_config.type, "llm");
    EXPECT_EQ(func_config.description, "A test function");
    EXPECT_TRUE(func_config.async_capable);
    EXPECT_EQ(func_config.timeout_ms, 10000);
    EXPECT_EQ(func_config.parameters.size(), 2);
    EXPECT_EQ(func_config.parameters["param1"], "value1");
    EXPECT_EQ(func_config.parameters["param2"], "value2");
    EXPECT_EQ(func_config.implementation, "custom implementation");
    EXPECT_EQ(func_config.endpoint, "http://api.example.com/function");
}

TEST_F(YAMLConfigurationParserTest, ParseAgentConfig) {
    YAML::Node config;
    config["id"] = "test_agent_1";
    config["name"] = "Test Agent";
    config["type"] = "specialist";
    config["description"] = "A test agent";
    config["role"] = "assistant";
    config["system_prompt"] = "You are a test assistant";
    config["capabilities"] = std::vector<std::string>{"text_processing", "analysis"};
    config["functions"] = std::vector<std::string>{"func1", "func2"};
    config["auto_start"] = false;
    config["max_concurrent_jobs"] = 10;
    config["heartbeat_interval_seconds"] = 15;
    
    // LLM config
    config["llm_config"]["model_name"] = "agent-model";
    config["llm_config"]["temperature"] = 0.5;
    
    // Custom settings
    config["custom_settings"]["setting1"] = "value1";
    
    auto agent_config = AgentConfig::from_yaml(config);
    
    EXPECT_EQ(agent_config.id, "test_agent_1");
    EXPECT_EQ(agent_config.name, "Test Agent");
    EXPECT_EQ(agent_config.type, "specialist");
    EXPECT_EQ(agent_config.description, "A test agent");
    EXPECT_EQ(agent_config.role, "assistant");
    EXPECT_EQ(agent_config.system_prompt, "You are a test assistant");
    EXPECT_EQ(agent_config.capabilities.size(), 2);
    EXPECT_EQ(agent_config.functions.size(), 2);
    EXPECT_FALSE(agent_config.auto_start);
    EXPECT_EQ(agent_config.max_concurrent_jobs, 10);
    EXPECT_EQ(agent_config.heartbeat_interval_seconds, 15);
    EXPECT_EQ(agent_config.llm_config.model_name, "agent-model");
    EXPECT_DOUBLE_EQ(agent_config.llm_config.temperature, 0.5);
    EXPECT_EQ(agent_config.custom_settings["setting1"], "value1");
}

TEST_F(YAMLConfigurationParserTest, ParseInferenceEngineConfig) {
    YAML::Node config;
    config["name"] = "test_engine";
    config["type"] = "llama_cpp";
    config["model_path"] = "/path/to/model.gguf";
    config["auto_load"] = false;
    config["context_size"] = 8192;
    config["batch_size"] = 1024;
    config["threads"] = 8;
    config["gpu_layers"] = 32;
    config["settings"]["setting1"] = "value1";
    config["settings"]["setting2"] = "value2";
    
    auto engine_config = InferenceEngineConfig::from_yaml(config);
    
    EXPECT_EQ(engine_config.name, "test_engine");
    EXPECT_EQ(engine_config.type, "llama_cpp");
    EXPECT_EQ(engine_config.model_path, "/path/to/model.gguf");
    EXPECT_FALSE(engine_config.auto_load);
    EXPECT_EQ(engine_config.context_size, 8192);
    EXPECT_EQ(engine_config.batch_size, 1024);
    EXPECT_EQ(engine_config.threads, 8);
    EXPECT_EQ(engine_config.gpu_layers, 32);
    EXPECT_EQ(engine_config.settings.size(), 2);
}

TEST_F(YAMLConfigurationParserTest, ParseSystemConfig) {
    YAML::Node root;
    root["worker_threads"] = 8;
    root["health_check_interval_seconds"] = 30;
    root["log_level"] = "debug";
    root["global_settings"]["global1"] = "value1";
    
    // Add agent
    YAML::Node agent;
    agent["id"] = "system_agent";
    agent["name"] = "System Agent";
    agent["type"] = "system";
    root["agents"].push_back(agent);
    
    // Add function
    YAML::Node function;
    function["name"] = "system_function";
    function["type"] = "builtin";
    root["functions"].push_back(function);
    
    // Add inference engine
    YAML::Node engine;
    engine["name"] = "main_engine";
    engine["type"] = "llama_cpp";
    root["inference_engines"].push_back(engine);
    
    auto system_config = SystemConfig::from_yaml(root);
    
    EXPECT_EQ(system_config.worker_threads, 8);
    EXPECT_EQ(system_config.health_check_interval_seconds, 30);
    EXPECT_EQ(system_config.log_level, "debug");
    EXPECT_EQ(system_config.global_settings["global1"], "value1");
    EXPECT_EQ(system_config.agents.size(), 1);
    EXPECT_EQ(system_config.functions.size(), 1);
    EXPECT_EQ(system_config.inference_engines.size(), 1);
}

TEST_F(YAMLConfigurationParserTest, LoadFromFile) {
    // Create a test configuration file
    YAML::Node test_config;
    test_config["worker_threads"] = 4;
    test_config["log_level"] = "info";
    
    YAML::Node agent;
    agent["id"] = "file_agent";
    agent["name"] = "File Agent";
    test_config["agents"].push_back(agent);
    
    std::string config_file = createTempConfigFile(test_config);
    
    auto system_config = SystemConfig::from_file(config_file);
    
    EXPECT_EQ(system_config.worker_threads, 4);
    EXPECT_EQ(system_config.log_level, "info");
    EXPECT_EQ(system_config.agents.size(), 1);
    EXPECT_EQ(system_config.agents[0].id, "file_agent");
}

TEST_F(YAMLConfigurationParserTest, SaveToFile) {
    SystemConfig config;
    config.worker_threads = 6;
    config.log_level = "warning";
    
    AgentConfig agent;
    agent.id = "save_test_agent";
    agent.name = "Save Test Agent";
    config.agents.push_back(agent);
    
    std::string output_file = getTestOutputPath("saved_config.yaml");
    bool saved = config.save_to_file(output_file);
    EXPECT_TRUE(saved);
    
    // Load it back and verify
    auto loaded_config = SystemConfig::from_file(output_file);
    EXPECT_EQ(loaded_config.worker_threads, 6);
    EXPECT_EQ(loaded_config.log_level, "warning");
    EXPECT_EQ(loaded_config.agents.size(), 1);
    EXPECT_EQ(loaded_config.agents[0].id, "save_test_agent");
}

TEST_F(YAMLConfigurationParserTest, HandleInvalidYAML) {
    std::string invalid_yaml_file = getTestOutputPath("invalid.yaml");
    std::ofstream file(invalid_yaml_file);
    file << "invalid: yaml: content:\n  - malformed\n    broken";
    file.close();
    temp_files_.push_back(invalid_yaml_file);
    
    EXPECT_THROW(SystemConfig::from_file(invalid_yaml_file), std::exception);
}

TEST_F(YAMLConfigurationParserTest, HandleMissingFile) {
    std::string missing_file = getTestOutputPath("nonexistent.yaml");
    EXPECT_THROW(SystemConfig::from_file(missing_file), std::exception);
}

TEST_F(YAMLConfigurationParserTest, RoundTripSerialization) {
    // Create a complex configuration
    SystemConfig original;
    original.worker_threads = 12;
    original.health_check_interval_seconds = 60;
    original.log_level = "error";
    original.global_settings["test"] = "value";
    
    AgentConfig agent;
    agent.id = "roundtrip_agent";
    agent.name = "RoundTrip Agent";
    agent.llm_config.model_name = "roundtrip-model";
    agent.llm_config.temperature = 0.123456;
    agent.capabilities = {"cap1", "cap2", "cap3"};
    original.agents.push_back(agent);
    
    FunctionConfig function;
    function.name = "roundtrip_function";
    function.type = "external_api";
    function.parameters["key1"] = "val1";
    function.parameters["key2"] = "val2";
    original.functions.push_back(function);
    
    // Save to file
    std::string temp_file = getTestOutputPath("roundtrip.yaml");
    EXPECT_TRUE(original.save_to_file(temp_file));
    
    // Load from file
    auto loaded = SystemConfig::from_file(temp_file);
    
    // Verify everything matches
    EXPECT_EQ(loaded.worker_threads, original.worker_threads);
    EXPECT_EQ(loaded.health_check_interval_seconds, original.health_check_interval_seconds);
    EXPECT_EQ(loaded.log_level, original.log_level);
    EXPECT_EQ(loaded.global_settings, original.global_settings);
    
    ASSERT_EQ(loaded.agents.size(), 1);
    EXPECT_EQ(loaded.agents[0].id, original.agents[0].id);
    EXPECT_EQ(loaded.agents[0].name, original.agents[0].name);
    EXPECT_EQ(loaded.agents[0].llm_config.model_name, original.agents[0].llm_config.model_name);
    EXPECT_DOUBLE_EQ(loaded.agents[0].llm_config.temperature, original.agents[0].llm_config.temperature);
    EXPECT_EQ(loaded.agents[0].capabilities, original.agents[0].capabilities);
    
    ASSERT_EQ(loaded.functions.size(), 1);
    EXPECT_EQ(loaded.functions[0].name, original.functions[0].name);
    EXPECT_EQ(loaded.functions[0].type, original.functions[0].type);
    EXPECT_EQ(loaded.functions[0].parameters, original.functions[0].parameters);
}
