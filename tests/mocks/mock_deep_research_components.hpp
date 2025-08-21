/**
 * @file mock_deep_research_components.hpp
 * @brief Mock objects for Deep Research Agent testing
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 */

#pragma once

#include <gmock/gmock.h>
#include "examples/deep_research_agent.hpp"
#include "agent/core/agent_interfaces.hpp"
#include "execution/function_execution_manager.hpp"
#include "workflow/sequential_workflow.hpp"

namespace kolosal::agents::test {

using namespace kolosal::agents;
using namespace kolosal::agents::examples;

/**
 * @brief Mock Function Manager for testing research functions
 */
class MockFunctionManager : public FunctionManager {
public:
    MockFunctionManager() : FunctionManager(nullptr) {}
    
    MOCK_METHOD(bool, has__function, (const std::string& function_name), (const, override));
    MOCK_METHOD(FunctionResult, execute_function, (const std::string& function_name, const AgentData& parameters), (override));
    MOCK_METHOD(std::vector<std::string>, get__function_names, (), (const, override));
    MOCK_METHOD(std::string, get__function_description, (const std::string& function_name), (const, override));
    MOCK_METHOD(std::string, get__available_tools_summary, (), (const, override));
    
    // Setup default behavior for common research functions
    void setup_research_mocks() {
        using ::testing::Return;
        using ::testing::_;
        
        // Mock enhanced web search
        ON_CALL(*this, has__function("enhanced_web_search"))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, execute_function("enhanced_web_search", _))
            .WillByDefault([](const std::string&, const AgentData& params) {
                FunctionResult result(true);
                result.llm_response = "Mock web search results for query: " + 
                                    params.get_string("query", "unknown");
                return result;
            });
        
        // Mock document retrieval
        ON_CALL(*this, has__function("document_retrieval"))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, execute_function("document_retrieval", _))
            .WillByDefault([](const std::string&, const AgentData& params) {
                FunctionResult result(true);
                result.llm_response = "Mock document retrieval results for query: " + 
                                    params.get_string("query", "unknown");
                return result;
            });
        
        // Mock research synthesis
        ON_CALL(*this, has__function("research_synthesis"))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, execute_function("research_synthesis", _))
            .WillByDefault([](const std::string&, const AgentData&) {
                FunctionResult result(true);
                result.llm_response = "Mock research synthesis: Combined analysis of web and document sources "
                                    "providing comprehensive insights on the research topic.";
                return result;
            });
        
        // Mock report generation
        ON_CALL(*this, has__function("research_report_generator"))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, execute_function("research_report_generator", _))
            .WillByDefault([](const std::string&, const AgentData& params) {
                FunctionResult result(true);
                std::string question = params.get_string("research_question", "Unknown Research Question");
                result.llm_response = "# Mock Research Report: " + question + "\n\n"
                                    "## Executive Summary\n"
                                    "This is a mock comprehensive research report generated for testing purposes.\n\n"
                                    "## Detailed Analysis\n"
                                    "Mock analysis content with detailed findings and insights.\n\n"
                                    "## Conclusions\n"
                                    "Mock conclusions based on the research analysis.";
                return result;
            });
        
        // Mock research planning
        ON_CALL(*this, has__function("research_planning"))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, execute_function("research_planning", _))
            .WillByDefault([](const std::string&, const AgentData&) {
                FunctionResult result(true);
                result.llm_response = "Mock research plan: Strategic approach for comprehensive information gathering.";
                return result;
            });
    }
    
    // Setup failure scenarios for testing error handling
    void setup_failure_mocks() {
        using ::testing::Return;
        using ::testing::_;
        
        ON_CALL(*this, has__function(_))
            .WillByDefault(Return(false));
        
        ON_CALL(*this, execute_function(_, _))
            .WillByDefault([](const std::string& name, const AgentData&) {
                return FunctionResult(false, "Mock function execution failed: " + name);
            });
    }
};

/**
 * @brief Mock Workflow Executor for testing workflow functionality
 */
class MockWorkflowExecutor : public SequentialWorkflowExecutor {
public:
    MockWorkflowExecutor() : SequentialWorkflowExecutor(nullptr) {}
    
    MOCK_METHOD(SequentialWorkflowResult, execute_workflow, 
                (const std::string& workflow_id, const AgentData& input_context), (override));
    MOCK_METHOD(bool, register_workflow, 
                (const SequentialWorkflow& workflow), (override));
    MOCK_METHOD(std::vector<std::string>, list_workflows, (), (const, override));
    
    void setup_workflow_mocks() {
        using ::testing::Return;
        using ::testing::_;
        
        ON_CALL(*this, execute_workflow(_, _))
            .WillByDefault([](const std::string& workflow_id, const AgentData& input) {
                SequentialWorkflowResult result;
                result.success = true;
                result.workflow_id = workflow_id;
                result.execution_time_ms = 1000;
                result.final_output = "Mock workflow execution completed for: " + workflow_id;
                return result;
            });
        
        ON_CALL(*this, register_workflow(_))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, list_workflows())
            .WillByDefault(Return(std::vector<std::string>{"comprehensive", "quick", "academic"}));
    }
};

/**
 * @brief Mock Agent Core for testing
 */
class MockAgentCore {
public:
    MockAgentCore() = default;
    
    MOCK_METHOD(std::shared_ptr<FunctionManager>, get_function_manager, ());
    MOCK_METHOD(bool, is_running, (), (const));
    MOCK_METHOD(void, start, ());
    MOCK_METHOD(void, stop, ());
    
    void setup_core_mocks(std::shared_ptr<MockFunctionManager> function_manager) {
        using ::testing::Return;
        
        ON_CALL(*this, get_function_manager())
            .WillByDefault(Return(function_manager));
        
        ON_CALL(*this, is_running())
            .WillByDefault(Return(true));
        
        ON_CALL(*this, start())
            .WillByDefault(::testing::Return());
        
        ON_CALL(*this, stop())
            .WillByDefault(::testing::Return());
    }
};

/**
 * @brief Test fixture with mocked components for Deep Research Agent
 */
class MockedDeepResearchAgentTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_function_manager = std::make_shared<MockFunctionManager>();
        mock_workflow_executor = std::make_shared<MockWorkflowExecutor>();
        mock_agent_core = std::make_shared<MockAgentCore>();
        
        // Setup default mock behavior
        mock_function_manager->setup_research_mocks();
        mock_workflow_executor->setup_workflow_mocks();
        mock_agent_core->setup_core_mocks(mock_function_manager);
        
        // Create test configuration
        test_config.methodology = "systematic";
        test_config.max_sources = 10;
        test_config.max_web_results = 5;
        test_config.relevance_threshold = 0.8;
        test_config.include_academic = true;
        test_config.include_news = true;
        test_config.include_documents = true;
        test_config.output_format = "comprehensive_report";
        test_config.language = "en";
    }

    void TearDown() override {
        mock_function_manager.reset();
        mock_workflow_executor.reset();
        mock_agent_core.reset();
    }

    std::shared_ptr<MockFunctionManager> mock_function_manager;
    std::shared_ptr<MockWorkflowExecutor> mock_workflow_executor;
    std::shared_ptr<MockAgentCore> mock_agent_core;
    ResearchConfig test_config;
};

/**
 * @brief Helper class for creating test research results
 */
class ResearchResultTestHelper {
public:
    static ResearchResult create_successful_result(const std::string& question) {
        ResearchResult result;
        result.success = true;
        result.research_question = question;
        result.methodology_used = "systematic";
        result.full_report = "Comprehensive research report for: " + question;
        result.comprehensive_analysis = "Detailed analysis of " + question;
        result.executive_summary = "Executive summary of " + question + " research";
        result.confidence_score = 0.85;
        result.timestamp = std::chrono::system_clock::now();
        
        result.sources_found = {
            "Source 1: Academic paper on " + question,
            "Source 2: Industry report on " + question,
            "Source 3: News article about " + question
        };
        
        result.key_findings = {
            "Finding 1: Key insight about " + question,
            "Finding 2: Important trend in " + question,
            "Finding 3: Future implications of " + question
        };
        
        result.source_details["source1"] = "Detailed information about source 1";
        result.source_details["source2"] = "Detailed information about source 2";
        
        result.related_questions = {
            "What are the implications of " + question + "?",
            "How does " + question + " compare to alternatives?",
            "What is the future of " + question + "?"
        };
        
        return result;
    }
    
    static ResearchResult create_failed_result(const std::string& question, const std::string& error) {
        ResearchResult result;
        result.success = false;
        result.research_question = question;
        result.error_message = error;
        result.confidence_score = 0.0;
        result.timestamp = std::chrono::system_clock::now();
        return result;
    }
    
    static ResearchConfig create_test_config(const std::string& methodology = "systematic") {
        ResearchConfig config;
        config.methodology = methodology;
        config.max_sources = 15;
        config.max_web_results = 8;
        config.relevance_threshold = 0.75;
        config.include_academic = true;
        config.include_news = true;
        config.include_documents = true;
        config.output_format = "comprehensive_report";
        config.language = "en";
        return config;
    }
};

/**
 * @brief Mock Server Client for testing server interactions
 */
class MockServerClient {
public:
    MOCK_METHOD(bool, test_connection, (), (const));
    MOCK_METHOD(std::string, send_request, (const std::string& endpoint, const std::string& data), ());
    MOCK_METHOD(bool, is_function_available, (const std::string& function_name), (const));
    
    void setup_server_mocks() {
        using ::testing::Return;
        using ::testing::_;
        
        ON_CALL(*this, test_connection())
            .WillByDefault(Return(true));
        
        ON_CALL(*this, is_function_available(_))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, send_request(_, _))
            .WillByDefault([](const std::string& endpoint, const std::string&) {
                return "Mock server response for endpoint: " + endpoint;
            });
    }
    
    void setup_connection_failure() {
        using ::testing::Return;
        using ::testing::_;
        
        ON_CALL(*this, test_connection())
            .WillByDefault(Return(false));
        
        ON_CALL(*this, is_function_available(_))
            .WillByDefault(Return(false));
        
        ON_CALL(*this, send_request(_, _))
            .WillByDefault(::testing::Throw(std::runtime_error("Connection failed")));
    }
};

} // namespace kolosal::agents::test
