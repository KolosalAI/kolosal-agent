/**
 * @file deep_research_agent.hpp
 * @brief Deep Research Agent for comprehensive information gathering and analysis
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Deep Research Agent implementation.
 * Integrates with kolosal-server for web search and document retrieval.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_EXAMPLES_DEEP_RESEARCH_AGENT_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_EXAMPLES_DEEP_RESEARCH_AGENT_HPP_INCLUDED

#include "export.hpp"
#include "agent/core/agent_core.hpp"
#include "agent/core/agent_data.hpp"
#include "agent/core/agent_interfaces.hpp"
#include "workflow/sequential_workflow.hpp"
#include "tools/enhanced_function_registry.hpp"
#include "tools/kolosal_server_functions.hpp"
#include "execution/function_execution_manager.hpp"
#include "logger/server_logger_integration.hpp"
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace kolosal::agents::examples {

/**
 * @brief Research configuration for deep research operations
 */
struct ResearchConfig {
    std::string research_question;          // Main research question
    std::vector<std::string> keywords;      // Key terms to search for
    std::vector<std::string> domains;       // Specific domains to focus on
    std::string methodology;                // Research methodology (systematic, exploratory, etc.)
    std::string depth_level;                // shallow, moderate, comprehensive, exhaustive
    int max_sources;                        // Maximum number of sources to consider
    int max_web_results;                    // Maximum web search results per query
    double relevance_threshold;             // Minimum relevance score for results
    bool include_academic;                  // Include academic sources
    bool include_news;                      // Include news sources
    bool include_documents;                 // Include local document retrieval
    std::string output_format;              // report, summary, analysis, full
    std::string language;                   // Research language preference
    
    ResearchConfig() :
        methodology("systematic"),
        depth_level("comprehensive"),
        max_sources(50),
        max_web_results(20),
        relevance_threshold(0.7),
        include_academic(true),
        include_news(true),
        include_documents(true),
        output_format("comprehensive_report"),
        language("en") {}
};

/**
 * @brief Research result containing findings and analysis
 */
struct ResearchResult {
    bool success;
    std::string research_question;
    std::string methodology_used;
    std::vector<std::string> sources_found;
    std::vector<std::string> key_findings;
    std::string comprehensive_analysis;
    std::string executive_summary;
    std::string full_report;
    std::map<std::string, std::string> source_details;
    std::vector<std::string> related_questions;
    std::string error_message;
    double confidence_score;
    std::chrono::system_clock::time_point timestamp;
    
    ResearchResult() : success(false), confidence_score(0.0) {}
};

/**
 * @brief Deep Research Agent with advanced research capabilities
 * 
 * This agent specializes in comprehensive research operations, combining:
 * - Web search through kolosal-server integration
 * - Document retrieval from knowledge base
 * - Multi-step analysis and synthesis
 * - Report generation with citations
 */
class KOLOSAL_AGENT_API DeepResearchAgent {
private:
    std::shared_ptr<AgentCore> agent_core;
    std::shared_ptr<EnhancedFunctionRegistry> function_registry;
    std::shared_ptr<SequentialWorkflowExecutor> workflow_executor;
    std::shared_ptr<Logger> logger;
    std::string server_url;
    bool server_integration_enabled;
    ResearchConfig default_config;
    
    // Internal workflow management
    std::map<std::string, SequentialWorkflow> research_workflows;
    
public:
    /**
     * @brief Constructor for Deep Research Agent
     * @param agent_name Name of the research agent
     * @param server_endpoint URL of the kolosal-server
     * @param enable_server_functions Whether to enable server integration
     */
    explicit DeepResearchAgent(const std::string& agent_name = "DeepResearchAgent",
                              const std::string& server_endpoint = "http://localhost:8080",
                              bool enable_server_functions = true);
    
    /**
     * @brief Destructor
     */
    ~DeepResearchAgent();
    
    /**
     * @brief Initialize the research agent
     * @return True if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Start the research agent
     * @return True if started successfully
     */
    bool start();
    
    /**
     * @brief Stop the research agent
     */
    void stop();
    
    /**
     * @brief Conduct comprehensive research on a given topic
     * @param research_question The main research question
     * @param config Research configuration (optional)
     * @return Research result with findings and analysis
     */
    ResearchResult conduct_research(const std::string& research_question,
                                   const ResearchConfig& config = ResearchConfig());
    
    /**
     * @brief Conduct research using a predefined workflow
     * @param workflow_id ID of the workflow to use
     * @param research_question The research question
     * @param additional_params Additional parameters for the workflow
     * @return Research result
     */
    ResearchResult conduct_research_with_workflow(const std::string& workflow_id,
                                                 const std::string& research_question,
                                                 const AgentData& additional_params = AgentData());
    
    /**
     * @brief Create a custom research workflow
     * @param workflow_id Unique identifier for the workflow
     * @param workflow_name Human-readable name
     * @param research_steps List of research steps to perform
     * @return True if workflow created successfully
     */
    bool create_research_workflow(const std::string& workflow_id,
                                 const std::string& workflow_name,
                                 const std::vector<std::string>& research_steps);
    
    /**
     * @brief Get available research workflows
     * @return List of workflow IDs
     */
    std::vector<std::string> get_available_workflows() const;
    
    /**
     * @brief Test server connectivity and functionality
     * @return True if server is accessible and functional
     */
    bool test_server_connection();
    
    /**
     * @brief Set research configuration
     * @param config New default configuration
     */
    void set_research_config(const ResearchConfig& config) { default_config = config; }
    
    /**
     * @brief Get current research configuration
     * @return Current configuration
     */
    const ResearchConfig& get_research_config() const { return default_config; }
    
    /**
     * @brief Enable or disable server integration
     * @param enabled Whether to enable server functions
     */
    void set_server_integration_enabled(bool enabled);
    
    /**
     * @brief Check if server integration is enabled
     * @return True if server integration is enabled
     */
    bool is_server_integration_enabled() const { return server_integration_enabled; }
    
    /**
     * @brief Set the kolosal-server URL
     * @param url Server URL
     */
    void set_server_url(const std::string& url);
    
    /**
     * @brief Get the current server URL
     * @return Server URL
     */
    const std::string& get_server_url() const { return server_url; }
    
    /**
     * @brief Get the underlying agent core
     * @return Shared pointer to agent core
     */
    std::shared_ptr<AgentCore> get_agent_core() const { return agent_core; }

private:
    /**
     * @brief Initialize built-in research workflows
     */
    void initialize_default_workflows();
    
    /**
     * @brief Create the comprehensive research workflow
     * @return True if created successfully
     */
    bool create_comprehensive_research_workflow();
    
    /**
     * @brief Create the quick research workflow
     * @return True if created successfully
     */
    bool create_quick_research_workflow();
    
    /**
     * @brief Create the academic research workflow
     * @return True if created successfully
     */
    bool create_academic_research_workflow();
    
    /**
     * @brief Execute web search phase of research
     * @param research_question The research question
     * @param config Research configuration
     * @return Web search results
     */
    FunctionResult execute_web_search_phase(const std::string& research_question,
                                           const ResearchConfig& config);
    
    /**
     * @brief Execute document retrieval phase of research
     * @param research_question The research question
     * @param config Research configuration
     * @return Document retrieval results
     */
    FunctionResult execute_document_retrieval_phase(const std::string& research_question,
                                                   const ResearchConfig& config);
    
    /**
     * @brief Synthesize information from multiple sources
     * @param web_results Results from web search
     * @param doc_results Results from document retrieval
     * @param research_question Original research question
     * @param config Research configuration
     * @return Synthesized analysis
     */
    FunctionResult synthesize_research_findings(const FunctionResult& web_results,
                                               const FunctionResult& doc_results,
                                               const std::string& research_question,
                                               const ResearchConfig& config);
    
    /**
     * @brief Generate comprehensive research report
     * @param synthesis_result Results from synthesis phase
     * @param research_question Original research question
     * @param config Research configuration
     * @return Generated report
     */
    FunctionResult generate_research_report(const FunctionResult& synthesis_result,
                                           const std::string& research_question,
                                           const ResearchConfig& config);
    
    /**
     * @brief Validate and score research results
     * @param results Research findings to validate
     * @param research_question Original research question
     * @return Validation score and feedback
     */
    double validate_research_quality(const ResearchResult& results,
                                   const std::string& research_question);
    
    /**
     * @brief Convert function result to research result
     * @param func_result Function execution result
     * @param research_question Original research question
     * @param config Used configuration
     * @return Structured research result
     */
    ResearchResult convert_to_research_result(const FunctionResult& func_result,
                                             const std::string& research_question,
                                             const ResearchConfig& config);
};

/**
 * @brief Factory for creating deep research agents
 */
class KOLOSAL_AGENT_API DeepResearchAgentFactory {
public:
    /**
     * @brief Create a standard deep research agent
     * @param name Agent name
     * @param server_url Kolosal server URL
     * @return Shared pointer to deep research agent
     */
    static std::shared_ptr<DeepResearchAgent> create_standard_research_agent(
        const std::string& name = "StandardResearcher",
        const std::string& server_url = "http://localhost:8080");
    
    /**
     * @brief Create an academic research agent optimized for scholarly research
     * @param name Agent name
     * @param server_url Kolosal server URL
     * @return Shared pointer to deep research agent
     */
    static std::shared_ptr<DeepResearchAgent> create_academic_research_agent(
        const std::string& name = "AcademicResearcher",
        const std::string& server_url = "http://localhost:8080");
    
    /**
     * @brief Create a market research agent optimized for business intelligence
     * @param name Agent name
     * @param server_url Kolosal server URL
     * @return Shared pointer to deep research agent
     */
    static std::shared_ptr<DeepResearchAgent> create_market_research_agent(
        const std::string& name = "MarketResearcher",
        const std::string& server_url = "http://localhost:8080");
};

} // namespace kolosal::agents::examples

#endif // KOLOSAL_AGENT_INCLUDE_EXAMPLES_DEEP_RESEARCH_AGENT_HPP_INCLUDED
