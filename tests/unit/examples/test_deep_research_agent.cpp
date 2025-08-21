/**
 * @file test_deep_research_agent.cpp
 * @brief Comprehensive unit tests for DeepResearchAgent class
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 */

#include <gtest/gtest.h>
#include "examples/deep_research_agent.hpp"
#include "agent/core/agent_data.hpp"
#include <chrono>
#include <thread>
#include <future>

using namespace kolosal::agents::examples;
using namespace kolosal::agents;

class DeepResearchAgentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create agent with test configuration
        agent = std::make_unique<DeepResearchAgent>(
            "TestResearchAgent", 
            "http://localhost:8080", 
            false  // Disable server integration for unit tests
        );
        
        // Configure test research settings
        test_config.research_question = "Test research question";
        test_config.methodology = "systematic";
        test_config.max_sources = 10;
        test_config.max_web_results = 5;
        test_config.relevance_threshold = 0.8;
        test_config.include_academic = true;
        test_config.include_news = false;
        test_config.include_documents = true;
        test_config.output_format = "comprehensive_report";
        test_config.language = "en";
    }

    void TearDown() override {
        if (agent) {
            agent->stop();
            agent.reset();
        }
    }

    std::unique_ptr<DeepResearchAgent> agent;
    ResearchConfig test_config;
};

// Constructor and Initialization Tests
TEST_F(DeepResearchAgentTest, ConstructorInitializesCorrectly) {
    EXPECT_NE(agent, nullptr);
    EXPECT_EQ(agent->get_server_url(), "http://localhost:8080");
    EXPECT_FALSE(agent->is_server_integration_enabled());
}

TEST_F(DeepResearchAgentTest, InitializationSucceeds) {
    EXPECT_TRUE(agent->initialize());
    
    // Verify agent core is initialized
    auto core = agent->get_agent_core();
    EXPECT_NE(core, nullptr);
}

TEST_F(DeepResearchAgentTest, StartAndStopLifecycle) {
    EXPECT_TRUE(agent->start());
    
    // Verify agent is running
    auto core = agent->get_agent_core();
    EXPECT_TRUE(core->is__running());
    
    agent->stop();
    EXPECT_FALSE(core->is__running());
}

// Configuration Tests
TEST_F(DeepResearchAgentTest, ConfigurationManagement) {
    // Test setting configuration
    agent->set_research_config(test_config);
    
    const auto& current_config = agent->get_research_config();
    EXPECT_EQ(current_config.methodology, test_config.methodology);
    EXPECT_EQ(current_config.max_sources, test_config.max_sources);
    EXPECT_EQ(current_config.relevance_threshold, test_config.relevance_threshold);
}

TEST_F(DeepResearchAgentTest, ServerUrlConfiguration) {
    const std::string new_url = "http://test-server:9090";
    agent->set_server_url(new_url);
    EXPECT_EQ(agent->get_server_url(), new_url);
}

TEST_F(DeepResearchAgentTest, ServerIntegrationToggle) {
    EXPECT_FALSE(agent->is_server_integration_enabled());
    
    agent->set_server_integration_enabled(true);
    EXPECT_TRUE(agent->is_server_integration_enabled());
    
    agent->set_server_integration_enabled(false);
    EXPECT_FALSE(agent->is_server_integration_enabled());
}

// Research Functionality Tests
TEST_F(DeepResearchAgentTest, BasicResearchExecution) {
    ASSERT_TRUE(agent->start());
    
    const std::string research_question = "What are the latest trends in artificial intelligence?";
    auto result = agent->conduct_research(research_question, test_config);
    
    EXPECT_EQ(result.research_question, research_question);
    EXPECT_EQ(result.methodology_used, test_config.methodology);
    EXPECT_FALSE(result.full_report.empty());
    EXPECT_GE(result.confidence_score, 0.0);
    EXPECT_LE(result.confidence_score, 1.0);
}

TEST_F(DeepResearchAgentTest, ResearchWithDefaultConfig) {
    ASSERT_TRUE(agent->start());
    
    const std::string research_question = "Climate change impacts";
    auto result = agent->conduct_research(research_question);
    
    EXPECT_EQ(result.research_question, research_question);
    EXPECT_FALSE(result.full_report.empty());
    // Should use default configuration
    EXPECT_EQ(result.methodology_used, "systematic");
}

TEST_F(DeepResearchAgentTest, ResearchErrorHandling) {
    // Don't start the agent to trigger an error condition
    const std::string research_question = "Test question";
    auto result = agent->conduct_research(research_question, test_config);
    
    // Should handle gracefully even without proper initialization
    EXPECT_EQ(result.research_question, research_question);
}

// Workflow Management Tests
TEST_F(DeepResearchAgentTest, CustomWorkflowCreation) {
    ASSERT_TRUE(agent->start());
    
    std::vector<std::string> steps = {"web_search", "document_analysis", "synthesis"};
    bool success = agent->create_research_workflow("custom_workflow", "Custom Research", steps);
    
    EXPECT_TRUE(success);
    
    auto workflows = agent->get_available_workflows();
    EXPECT_TRUE(std::find(workflows.begin(), workflows.end(), "custom_workflow") != workflows.end());
}

TEST_F(DeepResearchAgentTest, WorkflowExecution) {
    ASSERT_TRUE(agent->start());
    
    // Create a simple workflow
    std::vector<std::string> steps = {"planning", "search"};
    ASSERT_TRUE(agent->create_research_workflow("test_workflow", "Test Workflow", steps));
    
    AgentData params;
    params.set("test_param", "test_value");
    
    auto result = agent->conduct_research_with_workflow(
        "test_workflow", 
        "Test research question", 
        params
    );
    
    EXPECT_EQ(result.research_question, "Test research question");
}

TEST_F(DeepResearchAgentTest, DefaultWorkflowsAvailable) {
    ASSERT_TRUE(agent->start());
    
    auto workflows = agent->get_available_workflows();
    EXPECT_FALSE(workflows.empty());
    
    // Should have at least the comprehensive workflow
    EXPECT_TRUE(std::find(workflows.begin(), workflows.end(), "comprehensive") != workflows.end());
}

// Server Integration Tests
TEST_F(DeepResearchAgentTest, ServerConnectionTest) {
    // Without server integration, this should return false
    EXPECT_FALSE(agent->test_server_connection());
}

TEST_F(DeepResearchAgentTest, ServerIntegrationDisabled) {
    ASSERT_TRUE(agent->start());
    
    // With server integration disabled, research should still work using fallbacks
    auto result = agent->conduct_research("Test question", test_config);
    EXPECT_FALSE(result.full_report.empty());
}

// Quality Validation Tests
TEST_F(DeepResearchAgentTest, QualityValidation) {
    ResearchResult test_result;
    test_result.success = true;
    test_result.full_report = "This is a comprehensive research report with detailed findings and analysis.";
    test_result.comprehensive_analysis = "Detailed analysis of the research findings.";
    test_result.executive_summary = "Executive summary of key findings.";
    
    // Access the private method through public interface
    // Since validate_research_quality is private, we test it indirectly through conduct_research
    ASSERT_TRUE(agent->start());
    auto result = agent->conduct_research("Quality test question", test_config);
    
    // Confidence score should be calculated
    EXPECT_GE(result.confidence_score, 0.0);
    EXPECT_LE(result.confidence_score, 1.0);
}

// Edge Cases and Error Handling
TEST_F(DeepResearchAgentTest, EmptyResearchQuestion) {
    ASSERT_TRUE(agent->start());
    
    auto result = agent->conduct_research("", test_config);
    EXPECT_EQ(result.research_question, "");
    // Should handle empty question gracefully
}

TEST_F(DeepResearchAgentTest, VeryLongResearchQuestion) {
    ASSERT_TRUE(agent->start());
    
    std::string long_question(5000, 'a'); // 5000 character string
    auto result = agent->conduct_research(long_question, test_config);
    
    EXPECT_EQ(result.research_question, long_question);
    // Should handle long questions without crashing
}

TEST_F(DeepResearchAgentTest, InvalidWorkflowExecution) {
    ASSERT_TRUE(agent->start());
    
    auto result = agent->conduct_research_with_workflow(
        "nonexistent_workflow", 
        "Test question", 
        AgentData()
    );
    
    // Should handle invalid workflow gracefully
    EXPECT_EQ(result.research_question, "Test question");
}

// Configuration Validation Tests
TEST_F(DeepResearchAgentTest, ConfigurationBoundaryValues) {
    ResearchConfig edge_config;
    edge_config.max_sources = 0;
    edge_config.max_web_results = 0;
    edge_config.relevance_threshold = -1.0;
    
    agent->set_research_config(edge_config);
    
    ASSERT_TRUE(agent->start());
    auto result = agent->conduct_research("Boundary test", edge_config);
    
    // Should handle boundary values gracefully
    EXPECT_FALSE(result.full_report.empty());
}

TEST_F(DeepResearchAgentTest, ConfigurationExtremeLimits) {
    ResearchConfig extreme_config;
    extreme_config.max_sources = 100000;
    extreme_config.max_web_results = 100000;
    extreme_config.relevance_threshold = 2.0;
    
    agent->set_research_config(extreme_config);
    
    ASSERT_TRUE(agent->start());
    auto result = agent->conduct_research("Extreme test", extreme_config);
    
    // Should handle extreme values gracefully
    EXPECT_FALSE(result.full_report.empty());
}

// Performance and Timeout Tests
TEST_F(DeepResearchAgentTest, ResearchTimeout) {
    ASSERT_TRUE(agent->start());
    
    // Test with very restrictive timeout through configuration
    ResearchConfig timeout_config = test_config;
    // Note: We can't directly test timeouts without modifying the implementation
    // This test verifies the research completes in reasonable time
    
    auto start_time = std::chrono::steady_clock::now();
    auto result = agent->conduct_research("Timeout test", timeout_config);
    auto end_time = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    EXPECT_LT(duration.count(), 60); // Should complete within 60 seconds
}

// Memory and Resource Management Tests
TEST_F(DeepResearchAgentTest, MultipleResearchOperations) {
    ASSERT_TRUE(agent->start());
    
    // Perform multiple research operations to test resource management
    for (int i = 0; i < 5; ++i) {
        std::string question = "Research question " + std::to_string(i);
        auto result = agent->conduct_research(question, test_config);
        
        EXPECT_EQ(result.research_question, question);
        EXPECT_FALSE(result.full_report.empty());
    }
}

TEST_F(DeepResearchAgentTest, ConcurrentResearchAttempts) {
    ASSERT_TRUE(agent->start());
    
    // Note: The current implementation may not support true concurrency
    // This test ensures thread safety at the API level
    std::vector<std::future<ResearchResult>> futures;
    
    for (int i = 0; i < 3; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i]() {
            return agent->conduct_research(
                "Concurrent question " + std::to_string(i), 
                test_config
            );
        }));
    }
    
    // Wait for all to complete
    for (auto& future : futures) {
        auto result = future.get();
        EXPECT_FALSE(result.full_report.empty());
    }
}

// Research Result Structure Tests
TEST_F(DeepResearchAgentTest, ResearchResultStructure) {
    ASSERT_TRUE(agent->start());
    
    auto result = agent->conduct_research("Structure test", test_config);
    
    // Verify all expected fields are populated
    EXPECT_FALSE(result.research_question.empty());
    EXPECT_FALSE(result.methodology_used.empty());
    EXPECT_FALSE(result.full_report.empty());
    EXPECT_GE(result.confidence_score, 0.0);
    
    // Timestamp should be recent
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::minutes>(now - result.timestamp);
    EXPECT_LT(diff.count(), 5); // Should be within 5 minutes
}

// Default Configuration Tests
TEST_F(DeepResearchAgentTest, DefaultConfigurationValues) {
    ResearchConfig default_config;
    
    EXPECT_EQ(default_config.methodology, "systematic");
    EXPECT_EQ(default_config.depth_level, "comprehensive");
    EXPECT_EQ(default_config.max_sources, 50);
    EXPECT_EQ(default_config.max_web_results, 20);
    EXPECT_DOUBLE_EQ(default_config.relevance_threshold, 0.7);
    EXPECT_TRUE(default_config.include_academic);
    EXPECT_TRUE(default_config.include_news);
    EXPECT_TRUE(default_config.include_documents);
    EXPECT_EQ(default_config.output_format, "comprehensive_report");
    EXPECT_EQ(default_config.language, "en");
}

// Integration Test Placeholders
// Note: These would be moved to integration tests in a real project
class DeepResearchAgentIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Only run if integration testing is enabled
        if (!integration_tests_enabled()) {
            GTEST_SKIP() << "Integration tests disabled";
        }
    }
    
private:
    bool integration_tests_enabled() {
        // Check environment variable or configuration
        const char* env = std::getenv("KOLOSAL_INTEGRATION_TESTS");
        return env && std::string(env) == "1";
    }
};

TEST_F(DeepResearchAgentIntegrationTest, DISABLED_ServerIntegrationEndToEnd) {
    auto agent = std::make_unique<DeepResearchAgent>(
        "IntegrationAgent", 
        "http://localhost:8080", 
        true
    );
    
    ASSERT_TRUE(agent->start());
    EXPECT_TRUE(agent->test_server_connection());
    
    auto result = agent->conduct_research(
        "Latest developments in machine learning", 
        ResearchConfig()
    );
    
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.full_report.empty());
    EXPECT_GT(result.confidence_score, 0.5);
}

TEST_F(DeepResearchAgentIntegrationTest, DISABLED_WorkflowIntegration) {
    auto agent = std::make_unique<DeepResearchAgent>(
        "WorkflowAgent", 
        "http://localhost:8080", 
        true
    );
    
    ASSERT_TRUE(agent->start());
    
    auto result = agent->conduct_research_with_workflow(
        "comprehensive",
        "Blockchain technology trends",
        AgentData()
    );
    
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.full_report.empty());
}
