/**
 * @file enhanced_agent_example.hpp
 * @brief Example of agent with enhanced kolosal-server integrated functions
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Example showing how to create agents with web search and retrieval capabilities.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_ENHANCED_AGENT_EXAMPLE_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_ENHANCED_AGENT_EXAMPLE_HPP_INCLUDED

#include "export.hpp"
#include "agent/core/agent_core.hpp"
#include "tools/enhanced_function_registry.hpp"
#include <memory>
#include <string>

namespace kolosal::agents {

/**
 * @brief Enhanced Agent with kolosal-server integrated functions
 * 
 * This class demonstrates how to create agents that can perform real
 * web searches and document retrieval using kolosal-server endpoints.
 */
class KOLOSAL_SERVER_API EnhancedAgent : public AgentCore {
private:
    std::shared_ptr<EnhancedFunctionRegistry> enhanced_registry;
    std::string server_url;
    bool server_functions_enabled;
    
public:
    /**
     * @brief Constructor for enhanced agent
     * @param name Agent name
     * @param role Agent role
     * @param server_endpoint kolosal-server URL
     * @param enable_server_functions Whether to enable server-integrated functions
     */
    EnhancedAgent(const std::string& name = "EnhancedAgent", 
                  AgentRole role = AgentRole::RESEARCHER,
                  const std::string& server_endpoint = "http://localhost:8080",
                  bool enable_server_functions = true);
    
    /**
     * @brief Initialize the agent with enhanced functions
     */
    void initialize_enhanced_functions();
    
    /**
     * @brief Perform an internet search using real web search
     * @param query The search query
     * @param max_results Maximum number of results
     * @return Search results
     */
    FunctionResult perform_web_search(const std::string& query, int max_results = 10);
    
    /**
     * @brief Retrieve documents from the knowledge base
     * @param query The retrieval query
     * @param max_docs Maximum number of documents
     * @return Retrieved documents
     */
    FunctionResult retrieve_documents(const std::string& query, int max_docs = 5);
    
    /**
     * @brief Perform hybrid knowledge retrieval (web + documents)
     * @param query The search/retrieval query
     * @param max_results Maximum total results
     * @return Combined knowledge results
     */
    FunctionResult get_comprehensive_knowledge(const std::string& query, int max_results = 15);
    
    /**
     * @brief Test connection to kolosal-server
     * @return True if connection is successful
     */
    bool test_server_connection();
    
    /**
     * @brief Enable or disable server functions
     * @param enabled Whether to enable server functions
     */
    void set_server_functions_enabled(bool enabled);
    
    /**
     * @brief Get the server URL
     * @return The current server URL
     */
    const std::string& get_server_url() const { return server_url; }
    
    /**
     * @brief Update the server URL
     * @param url New server URL
     */
    void set_server_url(const std::string& url);
    
    /**
     * @brief Get list of available enhanced functions
     * @return Vector of function names
     */
    std::vector<std::string> get_enhanced_functions() const;
};

/**
 * @brief Factory for creating enhanced agents with different configurations
 */
class KOLOSAL_SERVER_API EnhancedAgentFactory {
public:
    /**
     * @brief Create a research agent with web search capabilities
     * @param server_url kolosal-server URL
     * @return Unique pointer to the agent
     */
    static std::unique_ptr<EnhancedAgent> create_research_agent(
        const std::string& server_url = "http://localhost:8080");
    
    /**
     * @brief Create a knowledge assistant with document retrieval
     * @param server_url kolosal-server URL
     * @return Unique pointer to the agent
     */
    static std::unique_ptr<EnhancedAgent> create_knowledge_assistant(
        const std::string& server_url = "http://localhost:8080");
    
    /**
     * @brief Create a hybrid agent with both web search and document retrieval
     * @param server_url kolosal-server URL
     * @return Unique pointer to the agent
     */
    static std::unique_ptr<EnhancedAgent> create_hybrid_agent(
        const std::string& server_url = "http://localhost:8080");
    
    /**
     * @brief Create an agent with custom function selection
     * @param role Agent role
     * @param server_url kolosal-server URL
     * @param enable_web_search Whether to enable web search functions
     * @param enable_document_retrieval Whether to enable document retrieval
     * @return Unique pointer to the agent
     */
    static std::unique_ptr<EnhancedAgent> create_custom_agent(
        AgentRole role,
        const std::string& server_url = "http://localhost:8080",
        bool enable_web_search = true,
        bool enable_document_retrieval = true);
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_ENHANCED_AGENT_EXAMPLE_HPP_INCLUDED
