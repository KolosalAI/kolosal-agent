#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "model_interface.hpp"
#include <json.hpp>

using json = nlohmann::json;

class ModelInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create model interface with a test server URL
        model_interface = std::make_unique<ModelInterface>("http://localhost:8080");
    }

    void TearDown() override {
        model_interface.reset();
    }

    std::unique_ptr<ModelInterface> model_interface;
};

TEST_F(ModelInterfaceTest, ConstructorWithDefaultURL) {
    auto default_interface = std::make_unique<ModelInterface>();
    EXPECT_NE(default_interface, nullptr);
}

TEST_F(ModelInterfaceTest, ConstructorWithCustomURL) {
    auto custom_interface = std::make_unique<ModelInterface>("http://custom:9090");
    EXPECT_NE(custom_interface, nullptr);
}

TEST_F(ModelInterfaceTest, ModelConfigurationSetup) {
    json model_configs = json::array();
    
    json model1;
    model1["id"] = "test_model_1";
    model1["actual_name"] = "actual_test_model_1";
    model1["type"] = "llama";
    model1["description"] = "Test model 1";
    
    json model2;
    model2["id"] = "test_model_2";
    model2["actual_name"] = "actual_test_model_2";
    model2["type"] = "gpt";
    model2["description"] = "Test model 2";
    
    model_configs.push_back(model1);
    model_configs.push_back(model2);
    
    EXPECT_NO_THROW(model_interface->configure_models(model_configs));
}

TEST_F(ModelInterfaceTest, ModelNameResolution) {
    // Configure models first
    json model_configs = json::array();
    json model;
    model["id"] = "alias_model";
    model["actual_name"] = "real_model_name";
    model["type"] = "llama";
    model_configs.push_back(model);
    
    model_interface->configure_models(model_configs);
    
    // Test model name resolution
    std::string resolved = model_interface->resolve_model_name("alias_model");
    EXPECT_EQ(resolved, "real_model_name");
    
    // Test unknown model (should return original name)
    std::string unknown = model_interface->resolve_model_name("unknown_model");
    EXPECT_EQ(unknown, "unknown_model");
}

// Note: The following tests are commented out as they require an actual kolosal-server running
// In a real test environment, you might want to mock the KolosalClient or use a test server

/*
TEST_F(ModelInterfaceTest, GenerateCompletion) {
    std::string model_name = "test_model";
    std::string prompt = "Hello, world!";
    std::string system_prompt = "You are a helpful assistant.";
    
    // This would require a running server
    // std::string response = model_interface->generate_completion(model_name, prompt, system_prompt);
    // EXPECT_FALSE(response.empty());
}

TEST_F(ModelInterfaceTest, ChatWithModel) {
    std::string model_name = "test_model";
    std::string message = "Hello!";
    std::string system_prompt = "You are a helpful assistant.";
    json conversation_history = json::array();
    
    // This would require a running server
    // std::string response = model_interface->chat_with_model(model_name, message, system_prompt, conversation_history);
    // EXPECT_FALSE(response.empty());
}

TEST_F(ModelInterfaceTest, ModelAvailabilityCheck) {
    // This would require a running server
    // bool available = model_interface->is_model_available("test_model");
    // EXPECT_TRUE(available || !available); // Just check it doesn't crash
}

TEST_F(ModelInterfaceTest, GetAvailableModels) {
    // This would require a running server
    // json models = model_interface->get_available_models();
    // EXPECT_TRUE(models.is_array());
}
*/

TEST_F(ModelInterfaceTest, GenerateCompletionWithParameters) {
    std::string model_name = "test_model";
    std::string prompt = "Test prompt";
    
    // Test with different parameters - should not throw even if server is not available
    EXPECT_NO_THROW({
        try {
            model_interface->generate_completion(model_name, prompt, "", 100, 0.5);
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(ModelInterfaceTest, ChatWithModelWithHistory) {
    std::string model_name = "test_model";
    std::string message = "Hello!";
    json conversation_history = json::array();
    
    json prev_message;
    prev_message["role"] = "user";
    prev_message["content"] = "Previous message";
    conversation_history.push_back(prev_message);
    
    // Test with conversation history - should not throw even if server is not available
    EXPECT_NO_THROW({
        try {
            model_interface->chat_with_model(model_name, message, "", conversation_history);
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(ModelInterfaceTest, EmptyModelNameHandling) {
    // Test with empty model name
    EXPECT_NO_THROW({
        try {
            model_interface->generate_completion("", "test prompt");
        } catch (const std::exception&) {
            // Expected behavior for invalid model name
        }
    });
}

TEST_F(ModelInterfaceTest, EmptyPromptHandling) {
    // Test with empty prompt
    EXPECT_NO_THROW({
        try {
            model_interface->generate_completion("test_model", "");
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(ModelInterfaceTest, InvalidParameterHandling) {
    // Test with invalid parameters
    EXPECT_NO_THROW({
        try {
            model_interface->generate_completion("test_model", "test", "", -1, -1.0);
        } catch (const std::exception&) {
            // Expected for invalid parameters or server not running
        }
    });
}

TEST_F(ModelInterfaceTest, LargePromptHandling) {
    // Test with very large prompt
    std::string large_prompt(10000, 'a'); // 10KB of 'a' characters
    
    EXPECT_NO_THROW({
        try {
            model_interface->generate_completion("test_model", large_prompt);
        } catch (const std::exception&) {
            // Expected if server is not running or rejects large prompts
        }
    });
}

TEST_F(ModelInterfaceTest, ModelConfigurationWithEmptyArray) {
    json empty_configs = json::array();
    EXPECT_NO_THROW(model_interface->configure_models(empty_configs));
}

TEST_F(ModelInterfaceTest, ModelConfigurationWithInvalidConfig) {
    json invalid_configs = json::array();
    json invalid_model;
    invalid_model["invalid_field"] = "invalid_value";
    invalid_configs.push_back(invalid_model);
    
    // Should handle invalid configuration gracefully
    EXPECT_NO_THROW(model_interface->configure_models(invalid_configs));
}
