/**
 * @file research_functions.hpp
 * @brief Research-specific agent functions
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_TOOLS_RESEARCH_FUNCTIONS_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_TOOLS_RESEARCH_FUNCTIONS_HPP_INCLUDED

#include "../export.hpp"
#include "../agent/core/agent_interfaces.hpp"
#include "../agent/core/agent_data.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace kolosal::agents {

// Forward declarations
class Logger;

/**
 * @brief Research query planning and decomposition function
 */
class KOLOSAL_SERVER_API ResearchQueryPlanningFunction : public AgentFunction {
public:
    std::string get__name() const override { return "research_query_planning"; }
    std::string get__description() const override { 
        return "Decomposes complex research queries into manageable sub-tasks and selects appropriate methodologies"; 
    }
    std::string get__type() const override { return "research_intelligence"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Research methodology selection function
 */
class KOLOSAL_SERVER_API MethodologySelectionFunction : public AgentFunction {
public:
    std::string get__name() const override { return "methodology_selection"; }
    std::string get__description() const override { 
        return "Selects appropriate research methodologies based on research questions and constraints"; 
    }
    std::string get__type() const override { return "research_intelligence"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Source credibility analysis function
 */
class KOLOSAL_SERVER_API SourceCredibilityAnalysisFunction : public AgentFunction {
public:
    std::string get__name() const override { return "source_credibility_analysis"; }
    std::string get__description() const override { 
        return "Analyzes and scores the credibility and reliability of information sources"; 
    }
    std::string get__type() const override { return "analysis_synthesis"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Information synthesis function
 */
class KOLOSAL_SERVER_API InformationSynthesisFunction : public AgentFunction {
public:
    std::string get__name() const override { return "information_synthesis"; }
    std::string get__description() const override { 
        return "Synthesizes information from multiple sources, identifying patterns and resolving conflicts"; 
    }
    std::string get__type() const override { return "analysis_synthesis"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Fact verification function
 */
class KOLOSAL_SERVER_API FactVerificationFunction : public AgentFunction {
public:
    std::string get__name() const override { return "fact_verification"; }
    std::string get__description() const override { 
        return "Cross-references facts against multiple sources and assesses claim reliability"; 
    }
    std::string get__type() const override { return "knowledge_graph"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Knowledge graph query function
 */
class KOLOSAL_SERVER_API KnowledgeGraphQueryFunction : public AgentFunction {
public:
    std::string get__name() const override { return "knowledge_graph_query"; }
    std::string get__description() const override { 
        return "Queries the knowledge graph for entity relationships and semantic connections"; 
    }
    std::string get__type() const override { return "knowledge_graph"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Entity extraction function
 */
class KOLOSAL_SERVER_API EntityExtractionFunction : public AgentFunction {
public:
    std::string get__name() const override { return "entity_extraction"; }
    std::string get__description() const override { 
        return "Extracts named entities (people, organizations, concepts) from research content"; 
    }
    std::string get__type() const override { return "knowledge_graph"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Relationship mapping function
 */
class KOLOSAL_SERVER_API RelationshipMappingFunction : public AgentFunction {
public:
    std::string get__name() const override { return "relationship_mapping"; }
    std::string get__description() const override { 
        return "Maps and analyzes relationships between extracted entities"; 
    }
    std::string get__type() const override { return "knowledge_graph"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Research report generation function
 */
class KOLOSAL_SERVER_API ResearchReportGenerationFunction : public AgentFunction {
public:
    std::string get__name() const override { return "research_report_generation"; }
    std::string get__description() const override { 
        return "Generates structured research reports with proper citations and evidence presentation"; 
    }
    std::string get__type() const override { return "reporting"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Citation management function
 */
class KOLOSAL_SERVER_API CitationManagementFunction : public AgentFunction {
public:
    std::string get__name() const override { return "citation_management"; }
    std::string get__description() const override { 
        return "Manages citations and bibliographic references in various academic formats"; 
    }
    std::string get__type() const override { return "reporting"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Research quality assessment function
 */
class KOLOSAL_SERVER_API ResearchQualityAssessmentFunction : public AgentFunction {
public:
    std::string get__name() const override { return "research_quality_assessment"; }
    std::string get__description() const override { 
        return "Assesses the quality and completeness of research reports and findings"; 
    }
    std::string get__type() const override { return "quality_control"; }
    FunctionResult execute(const AgentData& parameters) override;
};

/**
 * @brief Research function registry utilities
 */
class KOLOSAL_SERVER_API ResearchFunctionRegistry {
public:
    /**
     * @brief Register all research functions with a function manager
     * @param function_manager The function manager to register with
     */
    static void register_all_research_functions(std::shared_ptr<class FunctionManager> function_manager);
    
    /**
     * @brief Get recommended functions for research roles
     * @param role The agent role
     * @return Vector of recommended function names
     */
    static std::vector<std::string> get_recommended_functions_for_role(const std::string& role);
    
    /**
     * @brief Register functions for specific research capabilities
     * @param function_manager The function manager to register with
     * @param capabilities List of research capabilities needed
     */
    static void register_functions_for_capabilities(
        std::shared_ptr<class FunctionManager> function_manager,
        const std::vector<std::string>& capabilities);
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_TOOLS_RESEARCH_FUNCTIONS_HPP_INCLUDED
