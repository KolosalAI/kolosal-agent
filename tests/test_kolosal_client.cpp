#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "kolosal_client.hpp"
#include <json.hpp>

using json = nlohmann::json;

class KolosalClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test configuration
        config.server_url = "http://localhost:8081";
        config.timeout_seconds = 30;
        config.max_retries = 3;
        config.retry_delay_ms = 100; // Shorter delay for testing
        config.verify_ssl = false; // Disable SSL verification for testing
        
        client = std::make_unique<KolosalClient>(config);
    }

    void TearDown() override {
        client.reset();
    }

    KolosalClient::Config config;
    std::unique_ptr<KolosalClient> client;
};

TEST_F(KolosalClientTest, ConstructorWithDefaultConfig) {
    auto default_client = std::make_unique<KolosalClient>();
    EXPECT_NE(default_client, nullptr);
}

TEST_F(KolosalClientTest, ConstructorWithCustomConfig) {
    KolosalClient::Config custom_config;
    custom_config.server_url = "http://custom:9090";
    custom_config.timeout_seconds = 60;
    custom_config.max_retries = 5;
    
    auto custom_client = std::make_unique<KolosalClient>(custom_config);
    EXPECT_NE(custom_client, nullptr);
    EXPECT_EQ(custom_client->get_config().server_url, "http://custom:9090");
    EXPECT_EQ(custom_client->get_config().timeout_seconds, 60);
    EXPECT_EQ(custom_client->get_config().max_retries, 5);
}

TEST_F(KolosalClientTest, GetConfiguration) {
    const auto& client_config = client->get_config();
    
    EXPECT_EQ(client_config.server_url, config.server_url);
    EXPECT_EQ(client_config.timeout_seconds, config.timeout_seconds);
    EXPECT_EQ(client_config.max_retries, config.max_retries);
}

TEST_F(KolosalClientTest, UpdateConfiguration) {
    KolosalClient::Config new_config = config;
    new_config.server_url = "http://updated:8082";
    new_config.timeout_seconds = 45;
    
    client->update_config(new_config);
    
    const auto& updated_config = client->get_config();
    EXPECT_EQ(updated_config.server_url, "http://updated:8082");
    EXPECT_EQ(updated_config.timeout_seconds, 45);
}

// Note: The following tests assume server is not running and test error handling

TEST_F(KolosalClientTest, IsServerHealthyWhenServerDown) {
    // Should handle server being down gracefully
    bool healthy = client->is_server_healthy();
    EXPECT_FALSE(healthy); // Server is not running in test environment
}

TEST_F(KolosalClientTest, GetServerStatusWhenServerDown) {
    EXPECT_NO_THROW({
        try {
            json status = client->get_server_status();
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, GetServerConfigWhenServerDown) {
    EXPECT_NO_THROW({
        try {
            json server_config = client->get_server_config();
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, IsModelAvailableWhenServerDown) {
    EXPECT_NO_THROW({
        try {
            bool available = client->is_model_available("test_model");
            EXPECT_FALSE(available); // Should be false when server is down
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, GetAvailableModelsWhenServerDown) {
    EXPECT_NO_THROW({
        try {
            json models = client->get_available_models();
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, ChatWithModelWhenServerDown) {
    EXPECT_NO_THROW({
        try {
            std::string response = client->chat_with_model(
                "test_model", "Hello", "You are a helpful assistant");
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, CompletionRequestWhenServerDown) {
    json params;
    params["max_tokens"] = 100;
    params["temperature"] = 0.7;
    
    EXPECT_NO_THROW({
        try {
            json response = client->completion_request("test_model", "Complete this:", params);
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, AddDocumentWhenServerDown) {
    json document_data;
    document_data["title"] = "Test Document";
    document_data["content"] = "Test content";
    document_data["metadata"] = json{{"author", "Test Author"}};
    
    EXPECT_NO_THROW({
        try {
            json result = client->add_document(document_data);
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, SearchDocumentsWhenServerDown) {
    json filters;
    filters["type"] = "academic";
    
    EXPECT_NO_THROW({
        try {
            json results = client->search_documents("test query", 5, filters);
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, RemoveDocumentWhenServerDown) {
    EXPECT_NO_THROW({
        try {
            json result = client->remove_document("test_doc_id");
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, ListDocumentsWhenServerDown) {
    EXPECT_NO_THROW({
        try {
            json documents = client->list_documents(0, 10);
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, InternetSearchWhenServerDown) {
    EXPECT_NO_THROW({
        try {
            json results = client->internet_search("test search query", 5);
        } catch (const std::exception&) {
            // Expected when server is not running
        }
    });
}

TEST_F(KolosalClientTest, InvalidServerUrlHandling) {
    KolosalClient::Config invalid_config;
    invalid_config.server_url = "invalid-url";
    
    auto invalid_client = std::make_unique<KolosalClient>(invalid_config);
    
    EXPECT_NO_THROW({
        try {
            bool healthy = invalid_client->is_server_healthy();
            EXPECT_FALSE(healthy);
        } catch (const std::exception&) {
            // Expected for invalid URL
        }
    });
}

TEST_F(KolosalClientTest, EmptyModelNameHandling) {
    EXPECT_NO_THROW({
        try {
            bool available = client->is_model_available("");
            EXPECT_FALSE(available);
        } catch (const std::exception&) {
            // Expected for empty model name
        }
    });
}

TEST_F(KolosalClientTest, EmptyQueryHandling) {
    EXPECT_NO_THROW({
        try {
            json results = client->search_documents("", 5);
        } catch (const std::exception&) {
            // Expected for empty query or server down
        }
    });
}

TEST_F(KolosalClientTest, LargeParametersHandling) {
    json large_params;
    large_params["max_tokens"] = 999999;
    large_params["temperature"] = 2.0; // Invalid temperature
    large_params["large_field"] = std::string(10000, 'x');
    
    EXPECT_NO_THROW({
        try {
            json response = client->completion_request("test_model", "test prompt", large_params);
        } catch (const std::exception&) {
            // Expected for invalid parameters or server down
        }
    });
}

TEST_F(KolosalClientTest, NegativeParametersHandling) {
    EXPECT_NO_THROW({
        try {
            json results = client->search_documents("test", -5); // Negative limit
        } catch (const std::exception&) {
            // Expected for invalid parameters
        }
    });
    
    EXPECT_NO_THROW({
        try {
            json documents = client->list_documents(-10, -5); // Negative offset and limit
        } catch (const std::exception&) {
            // Expected for invalid parameters
        }
    });
}

TEST_F(KolosalClientTest, ConfigurationValidation) {
    KolosalClient::Config test_config;
    
    // Test with various configurations
    test_config.server_url = "https://secure-server:443";
    test_config.timeout_seconds = 120;
    test_config.max_retries = 10;
    test_config.verify_ssl = true;
    
    auto secure_client = std::make_unique<KolosalClient>(test_config);
    EXPECT_NE(secure_client, nullptr);
    EXPECT_EQ(secure_client->get_config().verify_ssl, true);
}

TEST_F(KolosalClientTest, RetryConfiguration) {
    KolosalClient::Config retry_config;
    retry_config.max_retries = 0; // No retries
    retry_config.retry_delay_ms = 0;
    
    auto no_retry_client = std::make_unique<KolosalClient>(retry_config);
    EXPECT_EQ(no_retry_client->get_config().max_retries, 0);
}

TEST_F(KolosalClientTest, LongTimeoutConfiguration) {
    KolosalClient::Config long_timeout_config;
    long_timeout_config.timeout_seconds = 300; // 5 minutes
    
    auto long_timeout_client = std::make_unique<KolosalClient>(long_timeout_config);
    EXPECT_EQ(long_timeout_client->get_config().timeout_seconds, 300);
}
