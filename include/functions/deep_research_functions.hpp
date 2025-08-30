#pragma once

#include <json.hpp>
#include <string>
#include <vector>
#include <memory>

using json = nlohmann::json;

/**
 * @brief Deep research functions for comprehensive multi-phase research workflows
 */
namespace DeepResearchFunctions {
    
    /**
     * @brief Research planning and strategy
     */
    struct ResearchPlan {
        std::string query;
        std::string scope;
        std::string depth_level;
        std::vector<std::string> research_phases;
        std::vector<std::string> key_questions;
        std::vector<std::string> required_sources;
        json metadata;
    };
    
    /**
     * @brief Research findings structure
     */
    struct ResearchFinding {
        std::string content;
        std::string source_url;
        std::string source_type;
        double credibility_score;
        std::vector<std::string> tags;
        json metadata;
    };
    
    /**
     * @brief Synthesis result structure
     */
    struct SynthesisResult {
        std::string summary;
        std::vector<std::string> key_insights;
        std::vector<std::string> research_gaps;
        std::vector<std::string> conflicting_information;
        std::vector<ResearchFinding> supporting_evidence;
        json metadata;
    };
    
    /**
     * @brief Plan comprehensive research strategy
     */
    ResearchPlan plan_research(const json& params);
    
    /**
     * @brief Conduct targeted research on specific gaps
     */
    json targeted_research(const json& params);
    
    /**
     * @brief Verify facts across multiple sources
     */
    json verify_facts(const json& params);
    
    /**
     * @brief Synthesize research from multiple sources
     */
    SynthesisResult synthesize_research(const json& params);
    
    /**
     * @brief Generate comprehensive research report
     */
    json generate_research_report(const json& params);
    
    /**
     * @brief Cross-reference search across multiple databases
     */
    json cross_reference_search(const json& params);
    
    /**
     * @brief Iteratively refine search queries based on results
     */
    json iterative_search_refinement(const json& params);
    
    /**
     * @brief Analyze source credibility
     */
    json source_credibility_analysis(const json& params);
    
    /**
     * @brief Helper functions
     */
    
    /**
     * @brief Extract key concepts from query for targeted searching
     */
    std::vector<std::string> extract_key_concepts(const std::string& query);
    
    /**
     * @brief Generate search variations for comprehensive coverage
     */
    std::vector<std::string> generate_search_variations(const std::string& query);
    
    /**
     * @brief Calculate information overlap between sources
     */
    double calculate_information_overlap(const std::vector<ResearchFinding>& findings);
    
    /**
     * @brief Identify contradictions in research findings
     */
    std::vector<std::string> identify_contradictions(const std::vector<ResearchFinding>& findings);
    
    /**
     * @brief Score source credibility based on multiple factors
     */
    double score_source_credibility(const std::string& source_url, const json& criteria);
    
    /**
     * @brief Generate citations in different formats
     */
    std::string generate_citation(const ResearchFinding& finding, const std::string& format = "APA");
    
    /**
     * @brief Create research timeline from findings
     */
    json create_research_timeline(const std::vector<ResearchFinding>& findings);
    
    /**
     * @brief Generate executive summary from detailed research
     */
    std::string generate_executive_summary(const SynthesisResult& synthesis, int max_words = 300);
}
