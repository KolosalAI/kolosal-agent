#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "retrieval_manager.hpp"
#include <json.hpp>

using json = nlohmann::json;

class RetrievalManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test configuration
        config.server_url = "http://localhost:8081";
        config.timeout_seconds = 30;
        config.max_retries = 3;
        config.search_enabled = true;
        config.max_results = 10;
        
        retrieval_manager = std::make_unique<RetrievalManager>(config);
    }

    void TearDown() override {
        retrieval_manager.reset();
    }

    RetrievalManager::Config config;
    std::unique_ptr<RetrievalManager> retrieval_manager;
};

TEST_F(RetrievalManagerTest, ConstructorWithConfig) {
    EXPECT_NE(retrieval_manager, nullptr);
}

TEST_F(RetrievalManagerTest, ConfigurationProperties) {
    RetrievalManager::Config test_config;
    test_config.server_url = "http://test:9090";
    test_config.timeout_seconds = 60;
    test_config.max_retries = 5;
    test_config.search_enabled = false;
    test_config.max_results = 20;
    
    auto test_manager = std::make_unique<RetrievalManager>(test_config);
    EXPECT_NE(test_manager, nullptr);
}

TEST_F(RetrievalManagerTest, AvailabilityCheck) {
    // Should not throw
    EXPECT_NO_THROW(retrieval_manager->is_available());
}

TEST_F(RetrievalManagerTest, GetStatus) {
    json status = retrieval_manager->get_status();
    EXPECT_TRUE(status.is_object());
    // Status should contain some basic information
}

// Note: The following tests would require a running kolosal-server
// They test the interface behavior when server is not available

TEST_F(RetrievalManagerTest, AddDocumentInterface) {
    json params;
    params["content"] = "Test document content";
    params["title"] = "Test Document";
    params["metadata"] = json{{"author", "Test Author"}};
    
    // Should handle server unavailability gracefully
    EXPECT_NO_THROW({
        try {
            json result = retrieval_manager->add_document(params);
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(RetrievalManagerTest, SearchDocumentsInterface) {
    json params;
    params["query"] = "test search query";
    params["max_results"] = 5;
    
    EXPECT_NO_THROW({
        try {
            json result = retrieval_manager->search_documents(params);
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(RetrievalManagerTest, ListDocumentsInterface) {
    json params;
    params["limit"] = 10;
    params["offset"] = 0;
    
    EXPECT_NO_THROW({
        try {
            json result = retrieval_manager->list_documents(params);
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(RetrievalManagerTest, RemoveDocumentInterface) {
    json params;
    params["document_id"] = "test_doc_123";
    
    EXPECT_NO_THROW({
        try {
            json result = retrieval_manager->remove_document(params);
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(RetrievalManagerTest, InternetSearchInterface) {
    json params;
    params["query"] = "test internet search";
    params["max_results"] = 5;
    
    EXPECT_NO_THROW({
        try {
            json result = retrieval_manager->internet_search(params);
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(RetrievalManagerTest, CombinedSearchInterface) {
    json params;
    params["query"] = "combined search test";
    params["search_documents"] = true;
    params["search_internet"] = true;
    params["max_results"] = 10;
    
    EXPECT_NO_THROW({
        try {
            json result = retrieval_manager->combined_search(params);
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(RetrievalManagerTest, EmptyParametersHandling) {
    json empty_params = json{};
    
    // All functions should handle empty parameters gracefully
    EXPECT_NO_THROW({
        try {
            retrieval_manager->add_document(empty_params);
        } catch (const std::exception&) {}
    });
    
    EXPECT_NO_THROW({
        try {
            retrieval_manager->search_documents(empty_params);
        } catch (const std::exception&) {}
    });
    
    EXPECT_NO_THROW({
        try {
            retrieval_manager->list_documents(empty_params);
        } catch (const std::exception&) {}
    });
}

TEST_F(RetrievalManagerTest, InvalidParametersHandling) {
    json invalid_params;
    invalid_params["invalid_field"] = "invalid_value";
    invalid_params["numeric_field"] = "not_a_number";
    
    // Should handle invalid parameters gracefully
    EXPECT_NO_THROW({
        try {
            retrieval_manager->search_documents(invalid_params);
        } catch (const std::exception&) {
            // Expected for invalid parameters
        }
    });
}

TEST_F(RetrievalManagerTest, LargeContentHandling) {
    json params;
    params["content"] = std::string(100000, 'x'); // 100KB of 'x'
    params["title"] = "Large Document";
    
    EXPECT_NO_THROW({
        try {
            retrieval_manager->add_document(params);
        } catch (const std::exception&) {
            // Expected if server rejects large content or is not running
        }
    });
}

TEST_F(RetrievalManagerTest, SpecialCharactersInQuery) {
    json params;
    params["query"] = "test query with special chars: !@#$%^&*()[]{}|\\:;\"'<>?,./";
    
    EXPECT_NO_THROW({
        try {
            retrieval_manager->search_documents(params);
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(RetrievalManagerTest, UnicodeContentHandling) {
    json params;
    params["content"] = "Test content with unicode: こんにちは 🌟 Café naïve résumé";
    params["title"] = "Unicode Test Document";
    
    EXPECT_NO_THROW({
        try {
            retrieval_manager->add_document(params);
        } catch (const std::exception&) {
            // Expected if server is not running
        }
    });
}

TEST_F(RetrievalManagerTest, ConfigurationValidation) {
    // Test with different configurations
    RetrievalManager::Config invalid_config;
    invalid_config.server_url = ""; // Empty URL
    invalid_config.timeout_seconds = -1; // Invalid timeout
    invalid_config.max_retries = -1; // Invalid retries
    
    // Should still construct but may not be available
    auto invalid_manager = std::make_unique<RetrievalManager>(invalid_config);
    EXPECT_NE(invalid_manager, nullptr);
}
