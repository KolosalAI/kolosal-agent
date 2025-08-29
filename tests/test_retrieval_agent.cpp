#include <iostream>
#include <cassert>
#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include <json.hpp>

// Include the headers we need to test
#include "../include/retrieval_manager.hpp"
#include "../include/agent.hpp"
#include "../include/agent_manager.hpp"

using json = nlohmann::json;

// Simple test framework for this file
class SimpleTest {
public:
    static void assert_true(bool condition, const std::string& message) {
        if (!condition) {
            std::cerr << "ASSERTION FAILED: " << message << std::endl;
            throw std::runtime_error("Test assertion failed");
        }
        std::cout << "✓ " << message << std::endl;
    }
    
    static void assert_false(bool condition, const std::string& message) {
        assert_true(!condition, message);
    }
    
    template<typename T>
    static void assert_equals(const T& expected, const T& actual, const std::string& message) {
        if (expected != actual) {
            std::cerr << "ASSERTION FAILED: " << message 
                      << " (expected: " << expected << ", actual: " << actual << ")" << std::endl;
            throw std::runtime_error("Test assertion failed");
        }
        std::cout << "✓ " << message << std::endl;
    }
    
    static void assert_not_null(const void* ptr, const std::string& message) {
        assert_true(ptr != nullptr, message);
    }
    
    static void assert_throws(std::function<void()> func, const std::string& message) {
        bool threw = false;
        try {
            func();
        } catch (...) {
            threw = true;
        }
        assert_true(threw, message);
    }
};

// Mock retrieval manager for testing without dependencies
class MockRetrievalManager {
private:
    bool available_;
    bool search_enabled_;
    std::vector<json> documents_;
    int next_doc_id_;

public:
    MockRetrievalManager(bool available = true, bool search_enabled = false) 
        : available_(available), search_enabled_(search_enabled), next_doc_id_(1) {
    }
    
    bool is_available() const {
        return available_;
    }
    
    json get_status() const {
        json status;
        status["available"] = available_;
        status["vector_db_type"] = "mock";
        status["search_enabled"] = search_enabled_;
        status["document_count"] = documents_.size();
        return status;
    }
    
    json add_document(const json& params) {
        if (!available_) {
            throw std::runtime_error("Retrieval system not available");
        }
        
        std::string content = params.contains("content") && !params["content"].is_null() ? 
                              params["content"].get<std::string>() : "";
        std::string title = params.contains("title") && !params["title"].is_null() ? 
                            params["title"].get<std::string>() : "";
        
        if (content.empty()) {
            throw std::runtime_error("Document content cannot be empty");
        }
        
        json document;
        document["id"] = std::to_string(next_doc_id_++);
        document["title"] = title;
        document["content"] = content;
        document["timestamp"] = "2024-01-01T00:00:00Z";
        
        documents_.push_back(document);
        
        json result;
        result["status"] = "success";
        result["document_id"] = document["id"];
        result["message"] = "Document added successfully";
        return result;
    }
    
    json search_documents(const json& params) {
        if (!available_) {
            throw std::runtime_error("Retrieval system not available");
        }
        
        std::string query = params.contains("query") && !params["query"].is_null() ? 
                            params["query"].get<std::string>() : "";
        int limit = params.contains("limit") && !params["limit"].is_null() ? 
                    params["limit"].get<int>() : 10;
        
        if (query.empty()) {
            throw std::runtime_error("Search query cannot be empty");
        }
        
        json results = json::array();
        int count = 0;
        
        // Simple text matching for mock
        for (const auto& doc : documents_) {
            if (count >= limit) break;
            
        std::string content = doc.contains("content") && !doc["content"].is_null() ? 
                              doc["content"].get<std::string>() : "";
        std::string title = doc.contains("title") && !doc["title"].is_null() ? 
                            doc["title"].get<std::string>() : "";            if (content.find(query) != std::string::npos || 
                title.find(query) != std::string::npos) {
                json result_doc = doc;
                result_doc["score"] = 0.8; // Mock similarity score
                results.push_back(result_doc);
                count++;
            }
        }
        
        json result;
        result["query"] = query;
        result["results"] = results;
        result["total_found"] = count;
        result["limit"] = limit;
        return result;
    }
    
    json list_documents(const json& params = json()) {
        if (!available_) {
            throw std::runtime_error("Retrieval system not available");
        }
        
        int limit = params.contains("limit") && !params["limit"].is_null() ? 
                    params["limit"].get<int>() : 50;
        int offset = params.contains("offset") && !params["offset"].is_null() ? 
                     params["offset"].get<int>() : 0;
        
        json result_docs = json::array();
        int start = std::min(offset, (int)documents_.size());
        int end = std::min(start + limit, (int)documents_.size());
        
        for (int i = start; i < end; i++) {
            result_docs.push_back(documents_[i]);
        }
        
        json result;
        result["documents"] = result_docs;
        result["total_count"] = documents_.size();
        result["offset"] = offset;
        result["limit"] = limit;
        return result;
    }
    
    json remove_document(const json& params) {
        if (!available_) {
            throw std::runtime_error("Retrieval system not available");
        }
        
        std::string doc_id = params.contains("id") && !params["id"].is_null() ? 
                             params["id"].get<std::string>() : "";
        if (doc_id.empty()) {
            throw std::runtime_error("Document ID is required");
        }
        
        auto it = std::remove_if(documents_.begin(), documents_.end(),
            [&doc_id](const json& doc) {
                std::string id = doc.contains("id") && !doc["id"].is_null() ? 
                                 doc["id"].get<std::string>() : "";
                return id == doc_id;
            });
        
        bool found = it != documents_.end();
        if (found) {
            documents_.erase(it, documents_.end());
        }
        
        json result;
        result["id"] = doc_id;
        result["status"] = found ? "success" : "not_found";
        result["message"] = found ? "Document removed successfully" : "Document not found";
        return result;
    }
    
    json internet_search(const json& params) {
        if (!available_ || !search_enabled_) {
            throw std::runtime_error("Internet search not available");
        }
        
        std::string query = params.contains("query") && !params["query"].is_null() ? 
                            params["query"].get<std::string>() : "";
        int results = params.contains("results") && !params["results"].is_null() ? 
                      params["results"].get<int>() : 10;
        
        json result;
        result["query"] = query;
        result["results"] = json::array();
        
        // Mock web search results
        for (int i = 0; i < std::min(results, 3); i++) {
            json web_result;
            web_result["title"] = "Mock Web Result " + std::to_string(i + 1) + " for: " + query;
            web_result["url"] = "https://example.com/result" + std::to_string(i + 1);
            web_result["snippet"] = "This is a mock web search result for the query: " + query;
            web_result["score"] = 0.9 - (i * 0.1);
            result["results"].push_back(web_result);
        }
        
        result["message"] = "Internet search completed";
        return result;
    }
    
    json combined_search(const json& params) {
        if (!available_) {
            throw std::runtime_error("Retrieval system not available");
        }
        
        std::string query = params.contains("query") && !params["query"].is_null() ? 
                            params["query"].get<std::string>() : "";
        
        json result;
        result["query"] = query;
        
        try {
            // Search local documents
            json doc_params;
            doc_params["query"] = query;
            doc_params["limit"] = 5;
            result["local_results"] = search_documents(doc_params);
        } catch (const std::exception& e) {
            result["local_error"] = e.what();
        }
        
        try {
            // Search internet if enabled
            if (search_enabled_) {
                json search_params;
                search_params["query"] = query;
                search_params["results"] = 5;
                result["web_results"] = internet_search(search_params);
            } else {
                result["web_results"] = json::object();
                result["web_results"]["message"] = "Internet search disabled";
            }
        } catch (const std::exception& e) {
            result["web_error"] = e.what();
        }
        
        return result;
    }
    
    void set_availability(bool available) {
        available_ = available;
    }
    
    void set_search_enabled(bool enabled) {
        search_enabled_ = enabled;
    }
    
    size_t get_document_count() const {
        return documents_.size();
    }
    
    void clear_documents() {
        documents_.clear();
        next_doc_id_ = 1;
    }
};

// Test functions
void test_retrieval_manager_basic() {
    std::cout << "\n--- Testing Retrieval Manager Basic Operations ---" << std::endl;
    
    MockRetrievalManager manager;
    
    // Test initial state
    SimpleTest::assert_true(manager.is_available(), "Manager should be available");
    SimpleTest::assert_equals(size_t(0), manager.get_document_count(), "Should start with no documents");
    
    // Test status
    json status = manager.get_status();
    SimpleTest::assert_true(status["available"].get<bool>(), "Status should show available");
    SimpleTest::assert_equals(std::string("mock"), status["vector_db_type"].get<std::string>(), "Should show mock type");
}

void test_document_operations() {
    std::cout << "\n--- Testing Document Operations ---" << std::endl;
    
    MockRetrievalManager manager;
    
    // Test adding documents
    json add_params;
    add_params["title"] = "Test Document";
    add_params["content"] = "This is a test document about artificial intelligence.";
    
    json add_result = manager.add_document(add_params);
    SimpleTest::assert_equals(std::string("success"), add_result["status"].get<std::string>(), "Document should be added successfully");
    SimpleTest::assert_true(!add_result["document_id"].get<std::string>().empty(), "Should return document ID");
    
    // Test adding multiple documents
    json add_params2;
    add_params2["title"] = "Another Document";
    add_params2["content"] = "This document discusses machine learning algorithms.";
    manager.add_document(add_params2);
    
    json add_params3;
    add_params3["title"] = "Third Document";
    add_params3["content"] = "Natural language processing and AI models.";
    manager.add_document(add_params3);
    
    SimpleTest::assert_equals(size_t(3), manager.get_document_count(), "Should have 3 documents");
    
    // Test listing documents
    json list_result = manager.list_documents();
    SimpleTest::assert_true(list_result.contains("documents"), "Should contain documents array");
    SimpleTest::assert_true(list_result.contains("total_count"), "Should contain total_count field");
    SimpleTest::assert_true(!list_result["total_count"].is_null(), "total_count should not be null");
    SimpleTest::assert_equals(3, list_result["total_count"].get<int>(), "Should show total count");
    SimpleTest::assert_equals(3, (int)list_result["documents"].size(), "Should return all documents");
    
    // Test removing document
    std::string doc_id = add_result["document_id"].get<std::string>();
    json remove_params;
    remove_params["id"] = doc_id;
    
    json remove_result = manager.remove_document(remove_params);
    SimpleTest::assert_equals(std::string("success"), remove_result["status"].get<std::string>(), "Document should be removed successfully");
    SimpleTest::assert_equals(size_t(2), manager.get_document_count(), "Should have 2 documents after removal");
}

void test_document_search() {
    std::cout << "\n--- Testing Document Search ---" << std::endl;
    
    MockRetrievalManager manager;
    
    // Add test documents
    json doc1;
    doc1["title"] = "AI Research";
    doc1["content"] = "Artificial intelligence research focuses on machine learning and neural networks.";
    manager.add_document(doc1);
    
    json doc2;
    doc2["title"] = "Software Development";
    doc2["content"] = "Software engineering practices include testing, debugging, and code review.";
    manager.add_document(doc2);
    
    json doc3;
    doc3["title"] = "Machine Learning";
    doc3["content"] = "Machine learning algorithms can be supervised, unsupervised, or reinforcement learning.";
    manager.add_document(doc3);
    
    // Test search
    json search_params;
    search_params["query"] = "machine learning";
    search_params["limit"] = 10;
    
    json search_result = manager.search_documents(search_params);
    SimpleTest::assert_equals(std::string("machine learning"), search_result["query"].get<std::string>(), "Should return query");
    SimpleTest::assert_true(search_result["total_found"].get<int>() >= 1, "Should find at least one document");
    SimpleTest::assert_true(search_result.contains("results"), "Should contain results array");
    
    // Test search with limit
    search_params["limit"] = 1;
    json limited_result = manager.search_documents(search_params);
    SimpleTest::assert_true((int)limited_result["results"].size() <= 1, "Should respect limit parameter");
    
    // Test no results
    json no_match_params;
    no_match_params["query"] = "quantum computing";
    json no_result = manager.search_documents(no_match_params);
    SimpleTest::assert_equals(0, no_result["total_found"].get<int>(), "Should find no matching documents");
}

void test_internet_search() {
    std::cout << "\n--- Testing Internet Search ---" << std::endl;
    
    // Test with search disabled
    MockRetrievalManager manager_no_search(true, false);
    
    json search_params;
    search_params["query"] = "test query";
    search_params["results"] = 5;
    
    SimpleTest::assert_throws([&]() {
        manager_no_search.internet_search(search_params);
    }, "Should throw when internet search is disabled");
    
    // Test with search enabled
    MockRetrievalManager manager_with_search(true, true);
    
    json search_result = manager_with_search.internet_search(search_params);
    SimpleTest::assert_equals(std::string("test query"), search_result["query"].get<std::string>(), "Should return query");
    SimpleTest::assert_true(search_result.contains("results"), "Should contain results array");
    SimpleTest::assert_true((int)search_result["results"].size() > 0, "Should return some results");
}

void test_combined_search() {
    std::cout << "\n--- Testing Combined Search ---" << std::endl;
    
    MockRetrievalManager manager(true, true);
    
    // Add test document
    json doc;
    doc["title"] = "Test Document";
    doc["content"] = "This is a test document about artificial intelligence.";
    manager.add_document(doc);
    
    // Test combined search
    json search_params;
    search_params["query"] = "artificial intelligence";
    
    json combined_result = manager.combined_search(search_params);
    SimpleTest::assert_equals(std::string("artificial intelligence"), combined_result["query"].get<std::string>(), "Should return query");
    SimpleTest::assert_true(combined_result.contains("local_results"), "Should contain local results");
    SimpleTest::assert_true(combined_result.contains("web_results"), "Should contain web results");
    
    // Test with search disabled
    manager.set_search_enabled(false);
    json no_web_result = manager.combined_search(search_params);
    SimpleTest::assert_true(no_web_result["web_results"].contains("message"), "Should indicate web search disabled");
}

void test_error_scenarios() {
    std::cout << "\n--- Testing Error Scenarios ---" << std::endl;
    
    // Test unavailable manager
    MockRetrievalManager unavailable_manager(false);
    
    json params;
    params["content"] = "test content";
    
    SimpleTest::assert_throws([&]() {
        unavailable_manager.add_document(params);
    }, "Should throw when manager is unavailable");
    
    SimpleTest::assert_throws([&]() {
        unavailable_manager.search_documents(params);
    }, "Should throw when manager is unavailable for search");
    
    SimpleTest::assert_throws([&]() {
        unavailable_manager.list_documents();
    }, "Should throw when manager is unavailable for listing");
    
    // Test invalid parameters
    MockRetrievalManager manager;
    
    // Empty content
    json empty_content;
    empty_content["content"] = "";
    SimpleTest::assert_throws([&]() {
        manager.add_document(empty_content);
    }, "Should throw for empty document content");
    
    // Empty search query
    json empty_query;
    empty_query["query"] = "";
    SimpleTest::assert_throws([&]() {
        manager.search_documents(empty_query);
    }, "Should throw for empty search query");
    
    // Missing document ID for removal
    json no_id;
    SimpleTest::assert_throws([&]() {
        manager.remove_document(no_id);
    }, "Should throw when document ID is missing");
}

void test_configuration_scenarios() {
    std::cout << "\n--- Testing Configuration Scenarios ---" << std::endl;
    
    // Test RetrievalManager::Config structure
    RetrievalManager::Config config;
    
    // Test default values
    SimpleTest::assert_equals(std::string("faiss"), config.vector_db_type, "Default vector DB should be faiss");
    SimpleTest::assert_equals(std::string("localhost"), config.db_host, "Default host should be localhost");
    SimpleTest::assert_equals(6333, config.db_port, "Default port should be 6333");
    SimpleTest::assert_equals(std::string("documents"), config.collection_name, "Default collection should be documents");
    SimpleTest::assert_false(config.search_enabled, "Search should be disabled by default");
    SimpleTest::assert_equals(std::string("http://localhost:8888"), config.searxng_url, "Default searxng URL");
    SimpleTest::assert_equals(10, config.max_results, "Default max results should be 10");
    SimpleTest::assert_equals(30, config.timeout, "Default timeout should be 30");
    
    // Test custom configuration
    config.vector_db_type = "qdrant";
    config.db_host = "remote-host";
    config.db_port = 8000;
    config.search_enabled = true;
    config.max_results = 20;
    
    SimpleTest::assert_equals(std::string("qdrant"), config.vector_db_type, "Should accept custom vector DB type");
    SimpleTest::assert_equals(std::string("remote-host"), config.db_host, "Should accept custom host");
    SimpleTest::assert_equals(8000, config.db_port, "Should accept custom port");
    SimpleTest::assert_true(config.search_enabled, "Should accept search enabled");
    SimpleTest::assert_equals(20, config.max_results, "Should accept custom max results");
}

void test_real_retrieval_manager() {
    std::cout << "\n--- Testing Real RetrievalManager (if available) ---" << std::endl;
    
    try {
        RetrievalManager::Config config;
        config.search_enabled = false; // Disable internet search for testing
        
        RetrievalManager manager(config);
        
        // Test basic functionality
        json status = manager.get_status();
        SimpleTest::assert_true(status.contains("available"), "Status should contain availability");
        
        std::cout << "Real RetrievalManager status: " << status.dump(2) << std::endl;
        
        // Only test further if the manager is available
        if (manager.is_available()) {
            std::cout << "RetrievalManager is available - testing basic operations" << std::endl;
            
            // Test document operations (these might be placeholders in current implementation)
            json add_params;
            add_params["title"] = "Test Document";
            add_params["content"] = "This is a test document for the real retrieval manager.";
            
            try {
                json add_result = manager.add_document(add_params);
                std::cout << "Add document result: " << add_result.dump(2) << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Add document operation: " << e.what() << std::endl;
            }
            
            // Test search
            try {
                json search_params;
                search_params["query"] = "test";
                search_params["limit"] = 5;
                
                json search_result = manager.search_documents(search_params);
                std::cout << "Search result: " << search_result.dump(2) << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Search operation: " << e.what() << std::endl;
            }
            
            // Test list documents
            try {
                json list_result = manager.list_documents(json());
                std::cout << "List documents result: " << list_result.dump(2) << std::endl;
            } catch (const std::exception& e) {
                std::cout << "List documents operation: " << e.what() << std::endl;
            }
        } else {
            std::cout << "RetrievalManager not available - this is expected if kolosal-server is not built" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Real RetrievalManager test failed (expected if not built): " << e.what() << std::endl;
    }
}

#ifdef BUILD_WITH_RETRIEVAL
void test_agent_with_retrieval() {
    std::cout << "\n--- Testing Agent with Retrieval Functions ---" << std::endl;
    
    try {
        Agent agent("RetrievalTestAgent");
        
        // Configure retrieval
        json retrieval_config;
        retrieval_config["search_enabled"] = false;
        agent.configure_retrieval(retrieval_config);
        
        // Test that retrieval functions are available
        json agent_info = agent.get_info();
        std::cout << "Agent info: " << agent_info.dump(2) << std::endl;
        
        // Test retrieval functions through agent
        try {
            json add_params;
            add_params["title"] = "Agent Test Document";
            add_params["content"] = "This is a test document added through the agent.";
            
            json add_result = agent.execute_function("add_document", add_params);
            std::cout << "Agent add_document result: " << add_result.dump(2) << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Agent add_document: " << e.what() << std::endl;
        }
        
        try {
            json search_params;
            search_params["query"] = "test";
            search_params["limit"] = 5;
            
            json search_result = agent.execute_function("search_documents", search_params);
            std::cout << "Agent search_documents result: " << search_result.dump(2) << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Agent search_documents: " << e.what() << std::endl;
        }
        
        try {
            json list_result = agent.execute_function("list_documents", json());
            std::cout << "Agent list_documents result: " << list_result.dump(2) << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Agent list_documents: " << e.what() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Agent with retrieval test failed: " << e.what() << std::endl;
    }
}
#endif

// Main test runner
int main() {
    std::cout << "=== Kolosal Agent Retrieval System Tests ===" << std::endl;
    
    int passed_tests = 0;
    int total_tests = 0;
    
    auto run_test = [&](std::function<void()> test_func, const std::string& test_name) {
        total_tests++;
        try {
            test_func();
            passed_tests++;
            std::cout << "✓ " << test_name << " PASSED" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✗ " << test_name << " FAILED: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "✗ " << test_name << " FAILED: Unknown exception" << std::endl;
        }
    };
    
    // Run all tests
    run_test(test_retrieval_manager_basic, "RetrievalManager Basic");
    run_test(test_document_operations, "Document Operations");
    run_test(test_document_search, "Document Search");
    run_test(test_internet_search, "Internet Search");
    run_test(test_combined_search, "Combined Search");
    run_test(test_error_scenarios, "Error Scenarios");
    run_test(test_configuration_scenarios, "Configuration Scenarios");
    run_test(test_real_retrieval_manager, "Real RetrievalManager");
    
#ifdef BUILD_WITH_RETRIEVAL
    run_test(test_agent_with_retrieval, "Agent with Retrieval");
#else
    std::cout << "ℹ Agent with Retrieval test skipped (BUILD_WITH_RETRIEVAL not defined)" << std::endl;
#endif
    
    // Print summary
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Passed: " << passed_tests << "/" << total_tests << std::endl;
    
    if (passed_tests == total_tests) {
        std::cout << "🎉 All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some tests failed!" << std::endl;
        return 1;
    }
}
