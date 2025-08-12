/**
 * @file mock_llm_service.hpp
 * @brief Mock LLM service for testing
 */

#pragma once

#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <map>

namespace kolosal::agents::test {

struct MockLLMRequest {
    std::string prompt;
    std::string model;
    double temperature = 0.7;
    int max_tokens = 1024;
    std::vector<std::string> stop_sequences;
    std::map<std::string, std::string> metadata;
};

struct MockLLMResponse {
    std::string text;
    int tokens_used = 0;
    bool success = true;
    std::string error_message;
    double response_time_ms = 100.0;
};

/**
 * @brief Mock LLM Service for testing agent interactions
 */
class MockLLMService {
public:
    MOCK_METHOD(MockLLMResponse, generate_response, (const MockLLMRequest& request), ());
    MOCK_METHOD(bool, is_model_available, (const std::string& model), (const));
    MOCK_METHOD(std::vector<std::string>, list_available_models, (), (const));
    MOCK_METHOD(bool, load_model, (const std::string& model_path), ());
    MOCK_METHOD(void, unload_model, (const std::string& model), ());
    
    // Helper methods for setting up expectations
    void setDefaultResponse(const std::string& response) {
        default_response_ = response;
        ON_CALL(*this, generate_response)
            .WillByDefault([this](const MockLLMRequest&) {
                MockLLMResponse resp;
                resp.text = default_response_;
                resp.success = true;
                resp.tokens_used = default_response_.length() / 4; // Rough estimate
                return resp;
            });
    }
    
    void setModelAvailable(const std::string& model, bool available = true) {
        ON_CALL(*this, is_model_available(model))
            .WillByDefault(testing::Return(available));
    }
    
    void setAvailableModels(const std::vector<std::string>& models) {
        available_models_ = models;
        ON_CALL(*this, list_available_models)
            .WillByDefault(testing::Return(available_models_));
    }

private:
    std::string default_response_ = "Mock LLM response";
    std::vector<std::string> available_models_ = {"test-model", "mock-model"};
};

} // namespace kolosal::agents::test
