#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <json.hpp>

using json = nlohmann::json;

/**
 * @brief Research brief source information
 */
struct ResearchSource {
    std::string title;
    std::string publisher;
    std::string url;
    std::string accessed_date;
    std::string content_excerpt;
    double credibility_score;
    std::vector<std::string> topics;
    std::string source_type; // "web", "academic", "news", "government", etc.
    
    ResearchSource() : credibility_score(0.0) {}
    
    json to_json() const {
        return json{
            {"title", title},
            {"publisher", publisher},
            {"url", url},
            {"accessed_date", accessed_date},
            {"content_excerpt", content_excerpt},
            {"credibility_score", credibility_score},
            {"topics", topics},
            {"source_type", source_type}
        };
    }
};

/**
 * @brief Research claim with evidence and confidence
 */
struct ResearchClaim {
    std::string claim_text;
    std::vector<std::string> supporting_source_urls;
    std::vector<std::string> contradicting_source_urls;
    double confidence_score;
    std::string evidence_summary;
    std::vector<std::string> tags;
    
    ResearchClaim() : confidence_score(0.0) {}
    
    json to_json() const {
        return json{
            {"claim", claim_text},
            {"supporting_sources", supporting_source_urls},
            {"contradicting_sources", contradicting_source_urls},
            {"confidence", confidence_score},
            {"evidence_summary", evidence_summary},
            {"tags", tags}
        };
    }
};

/**
 * @brief Detected contradiction between sources
 */
struct ResearchContradiction {
    std::string topic;
    std::string description;
    std::vector<ResearchSource> conflicting_sources;
    std::string resolution_strategy;
    double severity_score;
    
    ResearchContradiction() : severity_score(0.0) {}
    
    json to_json() const {
        json sources_json = json::array();
        for (const auto& source : conflicting_sources) {
            sources_json.push_back(source.to_json());
        }
        
        return json{
            {"topic", topic},
            {"description", description},
            {"conflicting_sources", sources_json},
            {"resolution_strategy", resolution_strategy},
            {"severity", severity_score}
        };
    }
};

/**
 * @brief Complete research brief structure
 */
struct DecisionGradeResearchBrief {
    // Metadata
    std::string topic;
    std::string audience;
    std::string depth_level;
    std::string generated_date;
    int min_sources_requirement;
    
    // Core deliverables
    std::string executive_summary;
    std::vector<std::string> key_findings;
    std::vector<ResearchSource> sources;
    std::vector<ResearchContradiction> contradictions;
    std::vector<std::string> research_gaps;
    
    // Analysis results
    std::vector<ResearchClaim> claims;
    double overall_confidence;
    std::map<std::string, double> topic_confidence_scores;
    
    // Quality metrics
    int total_sources_found;
    int high_credibility_sources;
    int verification_attempts;
    std::chrono::system_clock::time_point research_duration;
    
    DecisionGradeResearchBrief() : min_sources_requirement(5), overall_confidence(0.0), 
                                  total_sources_found(0), high_credibility_sources(0), 
                                  verification_attempts(0) {}
    
    json to_json() const {
        json sources_json = json::array();
        for (const auto& source : sources) {
            sources_json.push_back(source.to_json());
        }
        
        json contradictions_json = json::array();
        for (const auto& contradiction : contradictions) {
            contradictions_json.push_back(contradiction.to_json());
        }
        
        json claims_json = json::array();
        for (const auto& claim : claims) {
            claims_json.push_back(claim.to_json());
        }
        
        return json{
            {"metadata", {
                {"topic", topic},
                {"audience", audience},
                {"depth_level", depth_level},
                {"generated_date", generated_date},
                {"min_sources_requirement", min_sources_requirement}
            }},
            {"executive_summary", executive_summary},
            {"key_findings", key_findings},
            {"sources", sources_json},
            {"contradictions", contradictions_json},
            {"research_gaps", research_gaps},
            {"claims", claims_json},
            {"confidence", {
                {"overall", overall_confidence},
                {"by_topic", topic_confidence_scores}
            }},
            {"quality_metrics", {
                {"total_sources", total_sources_found},
                {"high_credibility_sources", high_credibility_sources},
                {"verification_attempts", verification_attempts}
            }}
        };
    }
};

/**
 * @brief Research brief generator and processor
 */
class ResearchBriefProcessor {
private:
    std::string timezone_;
    std::string date_format_;
    int default_min_sources_;
    double confidence_threshold_;
    
public:
    explicit ResearchBriefProcessor(const std::string& timezone = "Asia/Jakarta",
                                  const std::string& date_format = "YYYY-MM-DD",
                                  int default_min_sources = 5,
                                  double confidence_threshold = 0.7);
    
    // Core processing functions
    json validate_parameters(const json& input_params);
    json plan_research_strategy(const json& params);
    json analyze_source_credibility(const json& sources);
    json detect_contradictions(const json& research_data);
    json calculate_confidence_scores(const json& claims_data, const json& source_scores);
    json format_decision_brief(const json& research_report, const json& format_specs);
    json validate_brief_quality(const json& formatted_brief, const json& validation_criteria);
    
    // Helper functions
    std::string get_current_date_jakarta() const;
    std::vector<std::string> extract_key_terms(const std::string& topic) const;
    double calculate_source_credibility(const ResearchSource& source) const;
    std::vector<ResearchClaim> extract_claims_from_text(const std::string& text) const;
    std::vector<ResearchContradiction> find_contradictions(const std::vector<ResearchClaim>& claims, 
                                                          const std::vector<ResearchSource>& sources) const;
    
    // Formatting and citation functions
    std::string format_citation(const ResearchSource& source) const;
    std::string format_executive_summary(const std::string& content, int max_words) const;
    std::string format_key_findings(const std::vector<std::string>& findings) const;
    std::string format_source_list(const std::vector<ResearchSource>& sources) const;
    
    // Validation functions
    bool validate_executive_summary_length(const std::string& summary, int max_words) const;
    bool validate_minimum_sources(const std::vector<ResearchSource>& sources, int min_required) const;
    bool validate_citation_format(const std::string& citation) const;
    
    // Configuration
    void set_timezone(const std::string& timezone) { timezone_ = timezone; }
    void set_date_format(const std::string& format) { date_format_ = format; }
    void set_default_min_sources(int min_sources) { default_min_sources_ = min_sources; }
    void set_confidence_threshold(double threshold) { confidence_threshold_ = threshold; }
};

/**
 * @brief Research brief workflow utilities
 */
namespace ResearchBriefUtils {
    // Date and time functions
    std::string get_jakarta_timestamp();
    std::string format_date(const std::chrono::system_clock::time_point& time_point, 
                           const std::string& timezone = "Asia/Jakarta");
    
    // Text processing functions
    int count_words(const std::string& text);
    std::string truncate_to_words(const std::string& text, int max_words);
    std::vector<std::string> extract_sentences(const std::string& text);
    
    // URL and source validation
    bool is_valid_url(const std::string& url);
    std::string extract_domain(const std::string& url);
    std::string classify_source_type(const std::string& url, const std::string& content);
    
    // Quality assessment
    double assess_text_quality(const std::string& text);
    double calculate_information_density(const std::string& text);
    std::vector<std::string> extract_key_topics(const std::string& text);
    
    // Citation and reference formatting
    std::string format_url_citation(const std::string& url, const std::string& title, 
                                   const std::string& accessed_date);
    std::string generate_inline_citation(int citation_number);
    std::map<std::string, int> build_citation_map(const std::vector<ResearchSource>& sources);
}
