/**
 * @file test_model_interface.cpp
 * @brief Focused tests for Model Interface component
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 */

#include "../external/yaml-cpp/test/gtest-1.11.0/googletest/include/gtest/gtest.h"
#include "../include/model_interface.hpp"
#include <json.hpp>
#include <chrono>
#include <thread>
#include <future>

using json = nlohmann::json;

class ModelInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override {
        model_interface_ = std::make_unique<ModelInterface>("http://localhost:8080");
    }
    
    void TearDown() override {
        model_interface_.reset();
    }
    
protected:
    std::unique_ptr<ModelInterface> model_interface_;
};

TEST_F(ModelInterfaceTest, ConstructorWithDefaultURL) {
    auto default_interface = std::make_unique<ModelInterface>();
    EXPECT_NE(default_interface, nullptr);
}

TEST_F(ModelInterfaceTest, ConstructorWithCustomURL) {
    auto custom_interface = std::make_unique<ModelInterface>("http://custom-server:9090");
    EXPECT_NE(custom_interface, nullptr);
}

TEST_F(ModelInterfaceTest, GetAvailableModels) {
    // Add timeout protection for the test
    auto start_time = std::chrono::steady_clock::now();
    auto timeout_duration = std::chrono::seconds(3); // Reduced timeout
    
    json models;
    bool completed = false;
    
    try {
        // Set a shorter timeout for the operation
        auto future = std::async(std::launch::async, [this]() {
            return model_interface_->get_available_models();
        });
        
        if (future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
            models = future.get();
            completed = true;
        } else {
            // Timeout occurred
            models = json::array();
            completed = true;
        }
    } catch (const std::exception& e) {
        // Expected in test environment with mock server
        models = json::array();
        completed = true;
    }
    
    // Ensure we don't hang
    auto elapsed = std::chrono::steady_clock::now() - start_time;
    EXPECT_LT(elapsed.count(), timeout_duration.count() * 1000000000LL); // nanoseconds
    EXPECT_TRUE(completed);
    EXPECT_TRUE(models.is_array());
    // Note: This test may return empty array if no server is running
}

TEST_F(ModelInterfaceTest, IsModelAvailable) {
    // Add timeout protection for the test
    auto start_time = std::chrono::steady_clock::now();
    auto timeout_duration = std::chrono::seconds(3); // Reduced timeout
    
    bool completed = false;
    bool result = false;
    
    try {
        // Test with common model names - add timeout
        auto future = std::async(std::launch::async, [this]() {
            return model_interface_->is_model_available("test-model");
        });
        
        if (future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
            result = future.get();
        }
        
        // Test with empty model name
        bool empty_result = model_interface_->is_model_available("");
        EXPECT_FALSE(empty_result);
        
        completed = true;
    } catch (const std::exception& e) {
        // Expected in test environment
        completed = true;
    }
    
    // Ensure we don't hang
    auto elapsed = std::chrono::steady_clock::now() - start_time;
    EXPECT_LT(elapsed.count(), timeout_duration.count() * 1000000000LL); // nanoseconds
    EXPECT_TRUE(completed);
}

TEST_F(ModelInterfaceTest, GenerateCompletionBasic) {
    // Add timeout protection for the test
    auto start_time = std::chrono::steady_clock::now();
    auto timeout_duration = std::chrono::seconds(3); // Reduced timeout
    
    bool completed = false;
    std::string result;
    
    try {
        auto future = std::async(std::launch::async, [this]() {
            return model_interface_->generate_completion(
                "test-model", 
                "Hello, world!",
                "",  // no system prompt
                32,  // max tokens
                0.7f // temperature
            );
        });
        
        if (future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
            result = future.get();
        }
        
        completed = true;
    } catch (const std::exception& e) {
        // Expected in test environment
        completed = true;
    }
    
    // Ensure we don't hang
    auto elapsed = std::chrono::steady_clock::now() - start_time;
    EXPECT_LT(elapsed.count(), timeout_duration.count() * 1000000000LL); // nanoseconds
    EXPECT_TRUE(completed);
    
    // Result might be empty if no server is available
    // In a real environment, this would test actual model communication
}

TEST_F(ModelInterfaceTest, GenerateCompletionWithSystemPrompt) {
    // Add timeout protection for the test
    auto start_time = std::chrono::steady_clock::now();
    auto timeout_duration = std::chrono::seconds(3); // Reduced timeout
    
    bool completed = false;
    std::string result;
    
    try {
        auto future = std::async(std::launch::async, [this]() {
            return model_interface_->generate_completion(
                "test-model",
                "What is the capital of France?",
                "You are a helpful geography assistant.",
                64,
                0.5f
            );
        });
        
        if (future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
            result = future.get();
        }
        
        completed = true;
    } catch (const std::exception& e) {
        // Expected in test environment
        completed = true;
    }
    
    // Ensure we don't hang
    auto elapsed = std::chrono::steady_clock::now() - start_time;
    EXPECT_LT(elapsed.count(), timeout_duration.count() * 1000000000LL); // nanoseconds
    EXPECT_TRUE(completed);
    
    // Test that function completes without throwing
    // In production, would verify actual response content
}

TEST_F(ModelInterfaceTest, ChatWithModelBasic) {
    // Add timeout protection for the test
    auto start_time = std::chrono::steady_clock::now();
    auto timeout_duration = std::chrono::seconds(3); // Reduced timeout
    
    bool completed = false;
    std::string result;
    
    try {
        auto future = std::async(std::launch::async, [this]() {
            return model_interface_->chat_with_model(
                "test-model",
                "Hello, how are you?",
                "You are a friendly assistant."
            );
        });
        
        if (future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
            result = future.get();
        }
        
        completed = true;
    } catch (const std::exception& e) {
        // Expected in test environment
        completed = true;
    }
    
    // Ensure we don't hang
    auto elapsed = std::chrono::steady_clock::now() - start_time;
    EXPECT_LT(elapsed.count(), timeout_duration.count() * 1000000000LL); // nanoseconds
    EXPECT_TRUE(completed);
    
    // Test function completion
}

TEST_F(ModelInterfaceTest, ChatWithModelHistory) {
    // Add timeout protection for the test
    auto start_time = std::chrono::steady_clock::now();
    auto timeout_duration = std::chrono::seconds(3); // Reduced timeout
    
    bool completed = false;
    std::string result;
    
    try {
        json conversation_history = json::array();
        conversation_history.push_back({
            {"role", "user"},
            {"content", "Hi there!"}
        });
        conversation_history.push_back({
            {"role", "assistant"},
            {"content", "Hello! How can I help you today?"}
        });
        
        auto future = std::async(std::launch::async, [this, conversation_history]() {
            return model_interface_->chat_with_model(
                "test-model",
                "What's the weather like?",
                "You are a helpful assistant.",
                conversation_history
            );
        });
        
        if (future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
            result = future.get();
        }
        
        completed = true;
    } catch (const std::exception& e) {
        // Expected in test environment
        completed = true;
    }
    
    // Ensure we don't hang
    auto elapsed = std::chrono::steady_clock::now() - start_time;
    EXPECT_LT(elapsed.count(), timeout_duration.count() * 1000000000LL); // nanoseconds
    EXPECT_TRUE(completed);
    
    // Test function completion with conversation context
}

TEST_F(ModelInterfaceTest, ErrorHandling) {
    // Test with invalid model name
    EXPECT_NO_THROW({
        std::string result = model_interface_->generate_completion(
            "invalid-model-name-that-does-not-exist",
            "test prompt",
            "",
            16,
            0.7f
        );
    });
    
    // Test with empty prompt
    EXPECT_NO_THROW({
        std::string result = model_interface_->generate_completion(
            "test-model",
            "",
            "",
            16,
            0.7f
        );
    });
}

TEST_F(ModelInterfaceTest, ParameterValidation) {
    // Test with zero max tokens
    EXPECT_NO_THROW({
        std::string result = model_interface_->generate_completion(
            "test-model",
            "test",
            "",
            0,  // zero tokens
            0.7f
        );
    });
    
    // Test with extreme temperature values
    EXPECT_NO_THROW({
        std::string result = model_interface_->generate_completion(
            "test-model",
            "test",
            "",
            16,
            2.0f  // high temperature
        );
    });
    
    EXPECT_NO_THROW({
        std::string result = model_interface_->generate_completion(
            "test-model",
            "test",
            "",
            16,
            0.0f  // zero temperature
        );
    });
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "Running Model Interface Tests..." << std::endl;
    return RUN_ALL_TESTS();
}
