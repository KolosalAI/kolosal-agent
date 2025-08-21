/**
 * @file test_deep_research_agent_integration.cpp
 * @brief Integration tests for DeepResearchAgent with server connectivity
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 */

#include <gtest/gtest.h>
#include "examples/deep_research_agent.hpp"
#include <chrono>
#include <thread>
#include <future>

using namespace kolosal::agents::examples;
using namespace kolosal::agents;

class DeepResearchAgentIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Check if integration tests should run
        const char* env = std::getenv("KOLOSAL_INTEGRATION_TESTS");
        integration_enabled = env && std::string(env) == "1";
        
        if (!integration_enabled) {
            GTEST_SKIP() << "Integration tests disabled. Set KOLOSAL_INTEGRATION_TESTS=1 to enable.";
        }
        
        // Get server URL from environment or use default
        const char* server_env = std::getenv("KOLOSAL_SERVER_URL");
        server_url = server_env ? server_env : "http://localhost:8080";
        
        // Create agent with server integration enabled
        agent = std::make_unique<DeepResearchAgent>(
            "IntegrationTestAgent", 
            server_url, 
            true
        );
        
        // Setup test configuration
        test_config.methodology = "comprehensive";
        test_config.max_sources = 15;
        test_config.max_web_results = 10;
        test_config.relevance_threshold = 0.75;
        test_config.include_academic = true;
        test_config.include_news = true;
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

    bool integration_enabled = false;
    std::string server_url;
    std::unique_ptr<DeepResearchAgent> agent;
    ResearchConfig test_config;
};

// Server Connectivity Tests
TEST_F(DeepResearchAgentIntegrationTest, ServerConnectivity) {
    ASSERT_TRUE(agent->initialize());
    ASSERT_TRUE(agent->start());
    
    // Test server connection
    bool connected = agent->test_server_connection();
    EXPECT_TRUE(connected) << "Failed to connect to kolosal-server at " << server_url;
    
    if (!connected) {
        GTEST_SKIP() << "Server not available, skipping integration tests";
    }
}

TEST_F(DeepResearchAgentIntegrationTest, ServerFunctionAvailability) {
    ASSERT_TRUE(agent->start());
    
    // Verify server integration is enabled
    EXPECT_TRUE(agent->is_server_integration_enabled());
    
    // Test that server functions are accessible
    auto core = agent->get_agent_core();
    ASSERT_NE(core, nullptr);
    
    auto function_manager = core->get__function_manager();
    ASSERT_NE(function_manager, nullptr);
    
    // Check for enhanced web search function
    bool has_web_search = function_manager->has__function("enhanced_web_search");
    EXPECT_TRUE(has_web_search) << "Enhanced web search function not available";
}

// End-to-End Research Tests
TEST_F(DeepResearchAgentIntegrationTest, ComprehensiveResearchExecution) {
    ASSERT_TRUE(agent->start());
    
    if (!agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    const std::string research_question = "What are the recent advances in artificial intelligence and machine learning?";
    
    auto start_time = std::chrono::steady_clock::now();
    auto result = agent->conduct_research(research_question, test_config);
    auto end_time = std::chrono::steady_clock::now();
    
    // Verify basic result structure
    EXPECT_TRUE(result.success) << "Research failed: " << result.error_message;
    EXPECT_EQ(result.research_question, research_question);
    EXPECT_EQ(result.methodology_used, test_config.methodology);
    
    // Verify content quality
    EXPECT_FALSE(result.full_report.empty()) << "Full report is empty";
    EXPECT_FALSE(result.comprehensive_analysis.empty()) << "Analysis is empty";
    EXPECT_GT(result.confidence_score, 0.5) << "Low confidence score: " << result.confidence_score;
    
    // Verify timing (should complete within reasonable time)
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    EXPECT_LT(duration.count(), 300) << "Research took too long: " << duration.count() << " seconds";
    
    // Content length checks
    EXPECT_GT(result.full_report.length(), 100) << "Report too short";
    EXPECT_GT(result.comprehensive_analysis.length(), 50) << "Analysis too short";
}

TEST_F(DeepResearchAgentIntegrationTest, TechnicalResearchQuery) {
    ASSERT_TRUE(agent->start());
    
    if (!agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    const std::string research_question = "Quantum computing developments in 2024 and 2025";
    
    ResearchConfig tech_config = test_config;
    tech_config.include_academic = true;
    tech_config.max_sources = 20;
    tech_config.relevance_threshold = 0.8;
    
    auto result = agent->conduct_research(research_question, tech_config);
    
    EXPECT_TRUE(result.success) << "Technical research failed: " << result.error_message;
    EXPECT_FALSE(result.full_report.empty());
    
    // Technical queries should have high relevance
    EXPECT_GT(result.confidence_score, 0.6) << "Low confidence for technical query";
    
    // Report should contain technical terms
    std::string report_lower = result.full_report;
    std::transform(report_lower.begin(), report_lower.end(), report_lower.begin(), ::tolower);
    
    bool has_quantum_terms = report_lower.find("quantum") != std::string::npos ||
                            report_lower.find("computing") != std::string::npos ||
                            report_lower.find("technology") != std::string::npos;
    
    EXPECT_TRUE(has_quantum_terms) << "Report doesn't contain relevant technical terms";
}

TEST_F(DeepResearchAgentIntegrationTest, AcademicResearchFocus) {
    ASSERT_TRUE(agent->start());
    
    if (!agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    const std::string research_question = "Climate change impacts on biodiversity";
    
    ResearchConfig academic_config = test_config;
    academic_config.methodology = "systematic";
    academic_config.include_academic = true;
    academic_config.include_news = false;  // Focus on academic sources
    academic_config.max_sources = 25;
    
    auto result = agent->conduct_research(research_question, academic_config);
    
    EXPECT_TRUE(result.success) << "Academic research failed: " << result.error_message;
    EXPECT_FALSE(result.full_report.empty());
    
    // Academic research should be thorough
    EXPECT_GT(result.full_report.length(), 200) << "Academic report too brief";
    EXPECT_GT(result.confidence_score, 0.7) << "Low confidence for academic research";
}

// Workflow Integration Tests
TEST_F(DeepResearchAgentIntegrationTest, DefaultWorkflowExecution) {
    ASSERT_TRUE(agent->start());
    
    if (!agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    const std::string research_question = "Sustainable energy technologies";
    
    AgentData workflow_params;
    workflow_params.set("priority", "high");
    workflow_params.set("focus", "technology");
    
    auto result = agent->conduct_research_with_workflow(
        "comprehensive", 
        research_question, 
        workflow_params
    );
    
    EXPECT_TRUE(result.success) << "Workflow research failed: " << result.error_message;
    EXPECT_EQ(result.research_question, research_question);
    EXPECT_FALSE(result.full_report.empty());
}

TEST_F(DeepResearchAgentIntegrationTest, CustomWorkflowCreationAndExecution) {
    ASSERT_TRUE(agent->start());
    
    if (!agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    // Create custom workflow
    std::vector<std::string> workflow_steps = {
        "research_planning",
        "enhanced_web_search",
        "research_synthesis",
        "research_report_generator"
    };
    
    bool workflow_created = agent->create_research_workflow(
        "integration_test_workflow",
        "Integration Test Custom Workflow",
        workflow_steps
    );
    
    EXPECT_TRUE(workflow_created) << "Failed to create custom workflow";
    
    // Verify workflow is available
    auto available_workflows = agent->get_available_workflows();
    bool workflow_found = std::find(available_workflows.begin(), available_workflows.end(), 
                                   "integration_test_workflow") != available_workflows.end();
    EXPECT_TRUE(workflow_found) << "Custom workflow not found in available workflows";
    
    // Execute custom workflow
    const std::string research_question = "Blockchain applications in healthcare";
    auto result = agent->conduct_research_with_workflow(
        "integration_test_workflow", 
        research_question, 
        AgentData()
    );
    
    EXPECT_EQ(result.research_question, research_question);
    // Note: Success may depend on server function availability
}

// Performance and Load Tests
TEST_F(DeepResearchAgentIntegrationTest, ConcurrentResearchQueries) {
    ASSERT_TRUE(agent->start());
    
    if (!agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    const std::vector<std::string> research_questions = {
        "Renewable energy trends",
        "Artificial intelligence ethics",
        "Space exploration updates"
    };
    
    std::vector<std::future<ResearchResult>> futures;
    
    // Launch concurrent research operations
    for (const auto& question : research_questions) {
        futures.push_back(std::async(std::launch::async, [this, question]() {
            ResearchConfig concurrent_config = test_config;
            concurrent_config.max_sources = 5;  // Reduce load for concurrent tests
            concurrent_config.max_web_results = 3;
            
            return agent->conduct_research(question, concurrent_config);
        }));
    }
    
    // Wait for all to complete and verify results
    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            auto result = futures[i].get();
            EXPECT_EQ(result.research_question, research_questions[i]);
            EXPECT_FALSE(result.full_report.empty()) << "Empty report for question: " << research_questions[i];
        } catch (const std::exception& e) {
            FAIL() << "Concurrent research failed for question " << i << ": " << e.what();
        }
    }
}

TEST_F(DeepResearchAgentIntegrationTest, LargeScaleResearch) {
    ASSERT_TRUE(agent->start());
    
    if (!agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    const std::string research_question = "Comprehensive analysis of global economic trends";
    
    ResearchConfig large_config = test_config;
    large_config.max_sources = 50;
    large_config.max_web_results = 30;
    large_config.depth_level = "exhaustive";
    large_config.include_academic = true;
    large_config.include_news = true;
    large_config.include_documents = true;
    
    auto start_time = std::chrono::steady_clock::now();
    auto result = agent->conduct_research(research_question, large_config);
    auto end_time = std::chrono::steady_clock::now();
    
    EXPECT_TRUE(result.success) << "Large-scale research failed: " << result.error_message;
    
    // Should generate substantial content
    EXPECT_GT(result.full_report.length(), 500) << "Large-scale report too short";
    EXPECT_GT(result.confidence_score, 0.6) << "Low confidence for large-scale research";
    
    // Should complete within reasonable time even for large requests
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(end_time - start_time);
    EXPECT_LT(duration.count(), 10) << "Large-scale research took too long: " << duration.count() << " minutes";
}

// Error Handling and Recovery Tests
TEST_F(DeepResearchAgentIntegrationTest, ServerConnectionRecovery) {
    // Test agent behavior when server becomes unavailable
    agent->set_server_url("http://nonexistent-server:9999");
    ASSERT_TRUE(agent->start());
    
    // Should handle server unavailability gracefully
    EXPECT_FALSE(agent->test_server_connection());
    
    const std::string research_question = "Test with unavailable server";
    auto result = agent->conduct_research(research_question, test_config);
    
    // Should still provide some response even without server
    EXPECT_EQ(result.research_question, research_question);
    EXPECT_FALSE(result.full_report.empty()) << "Should provide fallback response";
    
    // Restore proper server URL
    agent->set_server_url(server_url);
}

TEST_F(DeepResearchAgentIntegrationTest, InvalidRequestHandling) {
    ASSERT_TRUE(agent->start());
    
    if (!agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    // Test with malformed or problematic queries
    const std::vector<std::string> problematic_queries = {
        "",  // Empty query
        std::string(10000, 'x'),  // Very long query
        "Special chars: !@#$%^&*(){}[]|\\:;\"'<>?,./`~",  // Special characters
        "Query with\nnewlines\tand\ttabs",  // Control characters
    };
    
    for (const auto& query : problematic_queries) {
        auto result = agent->conduct_research(query, test_config);
        
        // Should handle gracefully without crashing
        EXPECT_EQ(result.research_question, query);
        
        // May or may not succeed, but should not crash
        if (!result.success) {
            EXPECT_FALSE(result.error_message.empty()) << "Should provide error message for: " << query;
        }
    }
}

// Data Quality and Validation Tests
TEST_F(DeepResearchAgentIntegrationTest, ResearchContentQuality) {
    ASSERT_TRUE(agent->start());
    
    if (!agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    const std::string research_question = "Machine learning applications in healthcare";
    
    ResearchConfig quality_config = test_config;
    quality_config.relevance_threshold = 0.9;  // High relevance requirement
    quality_config.include_academic = true;
    
    auto result = agent->conduct_research(research_question, quality_config);
    
    EXPECT_TRUE(result.success) << "Quality research failed: " << result.error_message;
    
    // Quality checks
    EXPECT_GT(result.confidence_score, 0.7) << "Low confidence score";
    
    // Content should be relevant
    std::string content_lower = result.full_report;
    std::transform(content_lower.begin(), content_lower.end(), content_lower.begin(), ::tolower);
    
    bool has_ml_terms = content_lower.find("machine learning") != std::string::npos ||
                       content_lower.find("artificial intelligence") != std::string::npos ||
                       content_lower.find("ai") != std::string::npos;
    
    bool has_healthcare_terms = content_lower.find("healthcare") != std::string::npos ||
                               content_lower.find("medical") != std::string::npos ||
                               content_lower.find("health") != std::string::npos;
    
    EXPECT_TRUE(has_ml_terms) << "Report missing machine learning terms";
    EXPECT_TRUE(has_healthcare_terms) << "Report missing healthcare terms";
}

TEST_F(DeepResearchAgentIntegrationTest, TimestampAccuracy) {
    ASSERT_TRUE(agent->start());
    
    if (!agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    auto before_research = std::chrono::system_clock::now();
    
    auto result = agent->conduct_research("Timestamp test query", test_config);
    
    auto after_research = std::chrono::system_clock::now();
    
    // Timestamp should be within the research period
    EXPECT_GE(result.timestamp, before_research);
    EXPECT_LE(result.timestamp, after_research);
}

// Configuration Validation Tests
TEST_F(DeepResearchAgentIntegrationTest, ConfigurationPersistence) {
    ASSERT_TRUE(agent->start());
    
    ResearchConfig custom_config;
    custom_config.methodology = "exploratory";
    custom_config.max_sources = 35;
    custom_config.relevance_threshold = 0.6;
    custom_config.language = "en";
    
    agent->set_research_config(custom_config);
    
    // Verify configuration persists
    const auto& retrieved_config = agent->get_research_config();
    EXPECT_EQ(retrieved_config.methodology, custom_config.methodology);
    EXPECT_EQ(retrieved_config.max_sources, custom_config.max_sources);
    EXPECT_EQ(retrieved_config.relevance_threshold, custom_config.relevance_threshold);
    EXPECT_EQ(retrieved_config.language, custom_config.language);
}

// Cleanup and Resource Management Tests
TEST_F(DeepResearchAgentIntegrationTest, ProperResourceCleanup) {
    auto local_agent = std::make_unique<DeepResearchAgent>(
        "CleanupTestAgent", 
        server_url, 
        true
    );
    
    ASSERT_TRUE(local_agent->start());
    
    if (!local_agent->test_server_connection()) {
        GTEST_SKIP() << "Server not available";
    }
    
    // Perform research
    auto result = local_agent->conduct_research("Cleanup test", test_config);
    
    // Explicit cleanup
    local_agent->stop();
    local_agent.reset();
    
    // If we reach here without crash, cleanup was successful
    SUCCEED();
}
