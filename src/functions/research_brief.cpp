#include "research_brief.hpp"
#include <regex>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <random>
#include <set>

// ResearchBriefProcessor Implementation

ResearchBriefProcessor::ResearchBriefProcessor(const std::string& timezone,
                                              const std::string& date_format,
                                              int default_min_sources,
                                              double confidence_threshold)
    : timezone_(timezone), date_format_(date_format), 
      default_min_sources_(default_min_sources), 
      confidence_threshold_(confidence_threshold) {}

json ResearchBriefProcessor::validate_parameters(const json& input_params) {
    json result;
    result["status"] = "valid";
    result["errors"] = json::array();
    result["warnings"] = json::array();
    
    try {
        // Check required parameters
        if (!input_params.contains("topic") || input_params["topic"].empty()) {
            result["errors"].push_back("Missing required parameter: topic");
            result["status"] = "invalid";
        }
        
        if (!input_params.contains("audience") || input_params["audience"].empty()) {
            result["errors"].push_back("Missing required parameter: audience");
            result["status"] = "invalid";
        }
        
        if (!input_params.contains("depth") || input_params["depth"].empty()) {
            result["errors"].push_back("Missing required parameter: depth");
            result["status"] = "invalid";
        }
        
        // Validate depth level
        if (input_params.contains("depth")) {
            std::string depth = input_params["depth"];
            if (depth != "basic" && depth != "intermediate" && depth != "advanced" && depth != "expert") {
                result["warnings"].push_back("Depth level should be one of: basic, intermediate, advanced, expert");
            }
        }
        
        // Validate minimum sources
        int min_sources = input_params.value("min_sources", default_min_sources_);
        if (min_sources < 3) {
            result["warnings"].push_back("Minimum sources is less than 3, which may affect research quality");
        }
        if (min_sources > 50) {
            result["warnings"].push_back("Minimum sources is very high (>50), which may affect performance");
        }
        
        result["validated_params"] = {
            {"topic", input_params.value("topic", "")},
            {"audience", input_params.value("audience", "")},
            {"depth", input_params.value("depth", "intermediate")},
            {"min_sources", min_sources}
        };
        
    } catch (const std::exception& e) {
        result["status"] = "error";
        result["errors"].push_back(std::string("Validation error: ") + e.what());
    }
    
    return result;
}

json ResearchBriefProcessor::plan_research_strategy(const json& params) {
    json result;
    
    try {
        std::string topic = params.value("query", "");
        std::string depth = params.value("depth_level", "intermediate");
        std::string audience = params.value("audience", "general");
        int min_sources = params.value("min_sources", default_min_sources_);
        
        // Extract key terms and concepts
        auto key_terms = extract_key_terms(topic);
        
        // Generate primary search query
        std::string primary_query = topic;
        
        // Generate secondary search terms based on depth and audience
        std::vector<std::string> secondary_terms;
        
        // Add depth-specific terms
        if (depth == "basic") {
            secondary_terms.push_back(topic + " overview");
            secondary_terms.push_back(topic + " introduction");
        } else if (depth == "intermediate") {
            secondary_terms.push_back(topic + " analysis");
            secondary_terms.push_back(topic + " trends");
            secondary_terms.push_back(topic + " implications");
        } else if (depth == "advanced") {
            secondary_terms.push_back(topic + " technical analysis");
            secondary_terms.push_back(topic + " research findings");
            secondary_terms.push_back(topic + " methodology");
        } else if (depth == "expert") {
            secondary_terms.push_back(topic + " peer review");
            secondary_terms.push_back(topic + " academic research");
            secondary_terms.push_back(topic + " cutting edge");
        }
        
        // Add audience-specific terms
        if (audience.find("business") != std::string::npos || 
            audience.find("executive") != std::string::npos) {
            secondary_terms.push_back(topic + " business impact");
            secondary_terms.push_back(topic + " market analysis");
            secondary_terms.push_back(topic + " ROI");
        } else if (audience.find("technical") != std::string::npos ||
                   audience.find("developer") != std::string::npos) {
            secondary_terms.push_back(topic + " implementation");
            secondary_terms.push_back(topic + " architecture");
            secondary_terms.push_back(topic + " best practices");
        } else if (audience.find("academic") != std::string::npos ||
                   audience.find("research") != std::string::npos) {
            secondary_terms.push_back(topic + " literature review");
            secondary_terms.push_back(topic + " empirical studies");
            secondary_terms.push_back(topic + " theoretical framework");
        }
        
        // Generate research questions based on topic complexity
        std::vector<std::string> research_questions;
        research_questions.push_back("What is " + topic + "?");
        research_questions.push_back("How does " + topic + " work?");
        research_questions.push_back("What are the implications of " + topic + "?");
        research_questions.push_back("What are the current trends in " + topic + "?");
        research_questions.push_back("What are the challenges and limitations of " + topic + "?");
        
        if (depth == "advanced" || depth == "expert") {
            research_questions.push_back("What is the current state of research on " + topic + "?");
            research_questions.push_back("What are the future directions for " + topic + "?");
            research_questions.push_back("How does " + topic + " compare to alternatives?");
        }
        
        result = {
            {"primary_query", primary_query},
            {"secondary_terms", secondary_terms},
            {"key_terms", key_terms},
            {"research_questions", research_questions},
            {"recommended_sources", min_sources + 5}, // Add buffer for quality filtering
            {"search_strategy", {
                {"broad_search_first", true},
                {"specific_follow_up", true},
                {"cross_reference_required", true},
                {"fact_verification_required", true}
            }},
            {"quality_criteria", {
                {"min_credibility_score", 0.6},
                {"require_recent_sources", depth == "expert"},
                {"prefer_primary_sources", depth == "advanced" || depth == "expert"},
                {"require_peer_review", depth == "expert"}
            }}
        };
        
    } catch (const std::exception& e) {
        result = {
            {"error", std::string("Research planning error: ") + e.what()},
            {"status", "failed"}
        };
    }
    
    return result;
}

json ResearchBriefProcessor::analyze_source_credibility(const json& sources) {
    json result;
    json source_scores = json::array();
    json high_credibility_sources = json::array();
    
    try {
        if (!sources.is_array()) {
            result = {
                {"error", "Sources must be an array"},
                {"status", "failed"}
            };
            return result;
        }
        
        for (const auto& source_data : sources) {
            ResearchSource source;
            
            // Extract source information
            source.title = source_data.value("title", "");
            source.publisher = source_data.value("publisher", "");
            source.url = source_data.value("url", "");
            source.content_excerpt = source_data.value("content", "");
            source.accessed_date = get_current_date_jakarta();
            
            // Calculate credibility score
            source.credibility_score = calculate_source_credibility(source);
            
            // Classify source type
            source.source_type = ResearchBriefUtils::classify_source_type(source.url, source.content_excerpt);
            
            json scored_source = source.to_json();
            source_scores.push_back(scored_source);
            
            // Add to high credibility list if score is above threshold
            if (source.credibility_score >= confidence_threshold_) {
                high_credibility_sources.push_back(scored_source);
            }
        }
        
        // Calculate overall credibility metrics
        double avg_credibility = 0.0;
        int count = 0;
        for (const auto& scored : source_scores) {
            avg_credibility += scored["credibility_score"];
            count++;
        }
        if (count > 0) avg_credibility /= count;
        
        result = {
            {"source_scores", source_scores},
            {"high_credibility_sources", high_credibility_sources},
            {"verified_sources", high_credibility_sources}, // Alias for compatibility
            {"credibility_metrics", {
                {"average_credibility", avg_credibility},
                {"high_credibility_count", high_credibility_sources.size()},
                {"total_sources", source_scores.size()},
                {"credibility_ratio", count > 0 ? (double)high_credibility_sources.size() / count : 0.0}
            }},
            {"status", "success"}
        };
        
    } catch (const std::exception& e) {
        result = {
            {"error", std::string("Source credibility analysis error: ") + e.what()},
            {"status", "failed"}
        };
    }
    
    return result;
}

json ResearchBriefProcessor::detect_contradictions(const json& research_data) {
    json result;
    json contradictions = json::array();
    
    try {
        // Extract claims and findings from research data
        std::vector<std::string> findings;
        std::vector<ResearchSource> sources;
        
        if (research_data.contains("synthesized_data")) {
            const auto& synth_data = research_data["synthesized_data"];
            if (synth_data.contains("key_findings") && synth_data["key_findings"].is_array()) {
                for (const auto& finding : synth_data["key_findings"]) {
                    if (finding.is_string()) {
                        findings.push_back(finding);
                    }
                }
            }
        }
        
        // Simple contradiction detection based on opposing keywords
        std::vector<std::pair<std::string, std::string>> opposing_pairs = {
            {"increase", "decrease"},
            {"positive", "negative"},
            {"beneficial", "harmful"},
            {"effective", "ineffective"},
            {"safe", "dangerous"},
            {"approved", "rejected"},
            {"supports", "opposes"},
            {"confirms", "denies"},
            {"proven", "disproven"},
            {"successful", "failed"}
        };
        
        for (size_t i = 0; i < findings.size(); ++i) {
            for (size_t j = i + 1; j < findings.size(); ++j) {
                const std::string& finding1 = findings[i];
                const std::string& finding2 = findings[j];
                
                // Check for opposing keywords
                for (const auto& pair : opposing_pairs) {
                    bool found_contradiction = false;
                    std::string contradiction_topic;
                    
                    // Convert to lowercase for comparison
                    std::string f1_lower = finding1;
                    std::string f2_lower = finding2;
                    std::transform(f1_lower.begin(), f1_lower.end(), f1_lower.begin(), ::tolower);
                    std::transform(f2_lower.begin(), f2_lower.end(), f2_lower.begin(), ::tolower);
                    
                    if ((f1_lower.find(pair.first) != std::string::npos && 
                         f2_lower.find(pair.second) != std::string::npos) ||
                        (f1_lower.find(pair.second) != std::string::npos && 
                         f2_lower.find(pair.first) != std::string::npos)) {
                        
                        found_contradiction = true;
                        contradiction_topic = pair.first + " vs " + pair.second;
                    }
                    
                    if (found_contradiction) {
                        json contradiction = {
                            {"topic", contradiction_topic},
                            {"description", "Conflicting information found regarding " + contradiction_topic},
                            {"conflicting_statements", json::array({finding1, finding2})},
                            {"resolution_strategy", "Seek additional sources to clarify contradiction"},
                            {"severity", 0.7}
                        };
                        contradictions.push_back(contradiction);
                        break; // Avoid duplicate contradictions for the same pair
                    }
                }
            }
        }
        
        result = {
            {"contradictions", contradictions},
            {"contradiction_count", contradictions.size()},
            {"status", "success"}
        };
        
    } catch (const std::exception& e) {
        result = {
            {"error", std::string("Contradiction detection error: ") + e.what()},
            {"status", "failed"}
        };
    }
    
    return result;
}

json ResearchBriefProcessor::calculate_confidence_scores(const json& claims_data, const json& source_scores) {
    json result;
    json scored_claims = json::array();
    
    try {
        // Extract key claims from the claims data
        std::vector<std::string> claims;
        if (claims_data.contains("key_claims") && claims_data["key_claims"].is_array()) {
            for (const auto& claim : claims_data["key_claims"]) {
                if (claim.is_string()) {
                    claims.push_back(claim);
                }
            }
        } else if (claims_data.is_array()) {
            for (const auto& claim : claims_data) {
                if (claim.is_string()) {
                    claims.push_back(claim);
                }
            }
        }
        
        // Calculate average source credibility
        double avg_source_credibility = 0.0;
        int source_count = 0;
        if (source_scores.contains("credibility_metrics")) {
            avg_source_credibility = source_scores["credibility_metrics"].value("average_credibility", 0.0);
        }
        
        // Score each claim
        for (const auto& claim : claims) {
            // Simple confidence scoring based on:
            // 1. Source credibility
            // 2. Claim specificity (longer, more detailed claims get higher scores)
            // 3. Presence of quantitative data
            
            double confidence = avg_source_credibility * 0.6; // Base confidence from sources
            
            // Adjust for claim specificity
            if (claim.length() > 100) confidence += 0.1;
            if (claim.length() > 200) confidence += 0.1;
            
            // Look for quantitative indicators
            std::regex number_pattern(R"(\d+(?:\.\d+)?(?:%|percent|million|billion|trillion|thousand))");
            if (std::regex_search(claim, number_pattern)) {
                confidence += 0.1;
            }
            
            // Look for temporal indicators (recent data)
            std::regex date_pattern(R"(202[0-9]|2019|recent|latest|current)");
            if (std::regex_search(claim, date_pattern)) {
                confidence += 0.05;
            }
            
            // Cap confidence at 1.0
            confidence = std::min(confidence, 1.0);
            
            json scored_claim = {
                {"claim", claim},
                {"confidence", confidence},
                {"factors", {
                    {"source_credibility", avg_source_credibility},
                    {"claim_specificity", claim.length() > 100 ? "high" : "medium"},
                    {"has_quantitative_data", std::regex_search(claim, number_pattern)},
                    {"has_recent_data", std::regex_search(claim, date_pattern)}
                }}
            };
            scored_claims.push_back(scored_claim);
        }
        
        // Calculate overall confidence
        double overall_confidence = 0.0;
        if (scored_claims.size() > 0) {
            for (const auto& claim : scored_claims) {
                overall_confidence += claim["confidence"];
            }
            overall_confidence /= scored_claims.size();
        }
        
        result = {
            {"scored_claims", scored_claims},
            {"overall_confidence", overall_confidence},
            {"confidence_distribution", {
                {"high_confidence", 0}, // Count of claims with confidence > 0.8
                {"medium_confidence", 0}, // Count of claims with confidence 0.6-0.8
                {"low_confidence", 0} // Count of claims with confidence < 0.6
            }},
            {"status", "success"}
        };
        
        // Calculate distribution
        int high_conf = 0, med_conf = 0, low_conf = 0;
        for (const auto& claim : scored_claims) {
            double conf = claim["confidence"];
            if (conf > 0.8) high_conf++;
            else if (conf > 0.6) med_conf++;
            else low_conf++;
        }
        
        result["confidence_distribution"]["high_confidence"] = high_conf;
        result["confidence_distribution"]["medium_confidence"] = med_conf;
        result["confidence_distribution"]["low_confidence"] = low_conf;
        
    } catch (const std::exception& e) {
        result = {
            {"error", std::string("Confidence scoring error: ") + e.what()},
            {"status", "failed"}
        };
    }
    
    return result;
}

json ResearchBriefProcessor::format_decision_brief(const json& research_report, const json& format_specs) {
    json result;
    
    try {
        std::string formatted_brief;
        
        // Extract specifications
        int max_summary_words = format_specs.value("executive_summary_max", 200);
        std::string citation_style = format_specs.value("citation_style", "url_with_date");
        std::string timezone = format_specs.value("timezone", "Asia/Jakarta");
        bool include_json = format_specs.value("include_json_output", true);
        
        // Format header
        formatted_brief += "# DECISION-GRADE RESEARCH BRIEF\n\n";
        formatted_brief += "**Generated:** " + get_current_date_jakarta() + "\n";
        formatted_brief += "**Timezone:** " + timezone + "\n\n";
        
        // 1. Executive Summary
        if (research_report.contains("executive_summary")) {
            std::string summary = research_report["executive_summary"];
            summary = format_executive_summary(summary, max_summary_words);
            formatted_brief += "## 1. EXECUTIVE SUMMARY\n\n";
            formatted_brief += summary + "\n\n";
        }
        
        // 2. Key Findings
        if (research_report.contains("key_findings")) {
            formatted_brief += "## 2. KEY FINDINGS\n\n";
            auto findings = research_report["key_findings"];
            int finding_num = 1;
            
            if (findings.is_array()) {
                for (const auto& finding : findings) {
                    formatted_brief += std::to_string(finding_num++) + ". ";
                    formatted_brief += finding.get<std::string>() + " [" + std::to_string(finding_num - 1) + "]\n\n";
                }
            }
        }
        
        // 3. Sources
        if (research_report.contains("sources")) {
            formatted_brief += "## 3. SOURCES\n\n";
            auto sources = research_report["sources"];
            if (sources.is_array()) {
                int source_num = 1;
                for (const auto& source : sources) {
                    formatted_brief += "[" + std::to_string(source_num++) + "] ";
                    
                    std::string title = source.value("title", "Untitled");
                    std::string url = source.value("url", "");
                    std::string accessed = source.value("accessed_date", get_current_date_jakarta());
                    
                    formatted_brief += title + ". " + url + " (Accessed: " + accessed + ")\n\n";
                }
            }
        }
        
        // 4. Contradictions and Gaps
        if (research_report.contains("contradictions")) {
            formatted_brief += "## 4. DISAGREEMENTS AND GAPS\n\n";
            auto contradictions = research_report["contradictions"];
            if (contradictions.is_array() && contradictions.size() > 0) {
                for (const auto& contradiction : contradictions) {
                    formatted_brief += "**" + contradiction.value("topic", "Unknown") + "**: ";
                    formatted_brief += contradiction.value("description", "") + "\n\n";
                }
            } else {
                formatted_brief += "No significant contradictions detected in the reviewed sources.\n\n";
            }
            
            if (research_report.contains("research_gaps")) {
                auto gaps = research_report["research_gaps"];
                if (gaps.is_array() && gaps.size() > 0) {
                    formatted_brief += "**Research Gaps Identified:**\n";
                    for (const auto& gap : gaps) {
                        formatted_brief += "- " + gap.get<std::string>() + "\n";
                    }
                    formatted_brief += "\n";
                }
            }
        }
        
        // 5. JSON Output
        if (include_json) {
            formatted_brief += "## 5. STRUCTURED DATA (JSON)\n\n";
            formatted_brief += "```json\n";
            
            json structured_output = {
                {"claims", research_report.value("claims", json::array())},
                {"contradictions", research_report.value("contradictions", json::array())},
                {"sources", research_report.value("sources", json::array())},
                {"confidence", research_report.value("confidence_metrics", json::object())}
            };
            
            formatted_brief += structured_output.dump(2);
            formatted_brief += "\n```\n\n";
        }
        
        result = {
            {"formatted_brief", formatted_brief},
            {"word_count", ResearchBriefUtils::count_words(formatted_brief)},
            {"sections_included", json::array({
                "executive_summary", "key_findings", "sources", 
                "contradictions", include_json ? "json_output" : ""
            })},
            {"status", "success"}
        };
        
    } catch (const std::exception& e) {
        result = {
            {"error", std::string("Brief formatting error: ") + e.what()},
            {"status", "failed"}
        };
    }
    
    return result;
}

json ResearchBriefProcessor::validate_brief_quality(const json& formatted_brief, const json& validation_criteria) {
    json result;
    json validation_results = json::object();
    bool passed = true;
    
    try {
        std::string brief_text = formatted_brief.value("formatted_brief", "");
        
        // Validate minimum sources
        if (validation_criteria.contains("min_sources")) {
            int min_sources = validation_criteria["min_sources"];
            std::regex source_pattern(R"(\[\d+\])");
            auto source_matches = std::sregex_iterator(brief_text.begin(), brief_text.end(), source_pattern);
            auto source_end = std::sregex_iterator();
            int source_count = std::distance(source_matches, source_end);
            
            validation_results["source_count"] = {
                {"required", min_sources},
                {"found", source_count},
                {"passed", source_count >= min_sources}
            };
            
            if (source_count < min_sources) passed = false;
        }
        
        // Validate executive summary length
        if (validation_criteria.contains("max_executive_summary_words")) {
            int max_words = validation_criteria["max_executive_summary_words"];
            
            // Extract executive summary section
            std::regex summary_pattern(R"(## 1\. EXECUTIVE SUMMARY\n\n(.*?)\n\n##)");
            std::smatch summary_match;
            int summary_words = 0;
            
            if (std::regex_search(brief_text, summary_match, summary_pattern)) {
                std::string summary = summary_match[1].str();
                summary_words = ResearchBriefUtils::count_words(summary);
            }
            
            validation_results["executive_summary"] = {
                {"max_words", max_words},
                {"actual_words", summary_words},
                {"passed", summary_words <= max_words && summary_words > 0}
            };
            
            if (summary_words > max_words || summary_words == 0) passed = false;
        }
        
        // Validate required sections
        if (validation_criteria.contains("required_sections")) {
            auto required_sections = validation_criteria["required_sections"];
            json section_validation = json::object();
            
            for (const auto& section : required_sections) {
                std::string section_name = section;
                bool section_found = false;
                
                if (section_name == "executive_summary") {
                    section_found = brief_text.find("## 1. EXECUTIVE SUMMARY") != std::string::npos;
                } else if (section_name == "key_findings") {
                    section_found = brief_text.find("## 2. KEY FINDINGS") != std::string::npos;
                } else if (section_name == "sources") {
                    section_found = brief_text.find("## 3. SOURCES") != std::string::npos;
                } else if (section_name == "contradictions") {
                    section_found = brief_text.find("## 4. DISAGREEMENTS AND GAPS") != std::string::npos;
                } else if (section_name == "json_output") {
                    section_found = brief_text.find("## 5. STRUCTURED DATA (JSON)") != std::string::npos;
                }
                
                section_validation[section_name] = section_found;
                if (!section_found) passed = false;
            }
            
            validation_results["required_sections"] = section_validation;
        }
        
        // Overall quality metrics
        validation_results["overall_quality"] = {
            {"total_words", ResearchBriefUtils::count_words(brief_text)},
            {"has_citations", brief_text.find("[1]") != std::string::npos},
            {"has_structured_content", brief_text.find("##") != std::string::npos},
            {"information_density", ResearchBriefUtils::calculate_information_density(brief_text)}
        };
        
        result = {
            {"validation_passed", passed},
            {"validation_results", validation_results},
            {"quality_score", passed ? 0.9 : 0.6}, // Simple binary scoring for now
            {"status", "success"}
        };
        
    } catch (const std::exception& e) {
        result = {
            {"error", std::string("Quality validation error: ") + e.what()},
            {"status", "failed"}
        };
    }
    
    return result;
}

// Helper function implementations

std::string ResearchBriefProcessor::get_current_date_jakarta() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    // Convert to Jakarta time (UTC+7)
    time_t += 7 * 3600; // Add 7 hours
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d");
    return ss.str();
}

std::vector<std::string> ResearchBriefProcessor::extract_key_terms(const std::string& topic) const {
    std::vector<std::string> terms;
    std::stringstream ss(topic);
    std::string word;
    
    while (ss >> word) {
        // Remove punctuation and convert to lowercase
        word.erase(std::remove_if(word.begin(), word.end(), 
                   [](char c) { return std::ispunct(c); }), word.end());
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        
        if (word.length() > 2) { // Only include words longer than 2 characters
            terms.push_back(word);
        }
    }
    
    return terms;
}

double ResearchBriefProcessor::calculate_source_credibility(const ResearchSource& source) const {
    double score = 0.5; // Base score
    
    // URL-based scoring
    if (source.url.find(".edu") != std::string::npos) score += 0.3;
    else if (source.url.find(".org") != std::string::npos) score += 0.2;
    else if (source.url.find(".gov") != std::string::npos) score += 0.3;
    else if (source.url.find(".com") != std::string::npos) score += 0.1;
    
    // Content quality indicators
    if (source.content_excerpt.length() > 500) score += 0.1;
    if (source.content_excerpt.find("research") != std::string::npos ||
        source.content_excerpt.find("study") != std::string::npos) score += 0.1;
    
    // Title quality
    if (source.title.length() > 20) score += 0.05;
    
    return std::min(score, 1.0);
}

std::string ResearchBriefProcessor::format_executive_summary(const std::string& content, int max_words) const {
    return ResearchBriefUtils::truncate_to_words(content, max_words);
}

// ResearchBriefUtils Implementation

namespace ResearchBriefUtils {
    
    std::string get_jakarta_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        time_t += 7 * 3600; // Jakarta is UTC+7
        
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S JST");
        return ss.str();
    }
    
    int count_words(const std::string& text) {
        std::stringstream ss(text);
        std::string word;
        int count = 0;
        while (ss >> word) {
            count++;
        }
        return count;
    }
    
    std::string truncate_to_words(const std::string& text, int max_words) {
        std::stringstream ss(text);
        std::string word;
        std::string result;
        int count = 0;
        
        while (ss >> word && count < max_words) {
            if (count > 0) result += " ";
            result += word;
            count++;
        }
        
        if (count == max_words && ss >> word) {
            result += "...";
        }
        
        return result;
    }
    
    bool is_valid_url(const std::string& url) {
        std::regex url_pattern(R"(^https?://[^\s/$.?#].[^\s]*$)");
        return std::regex_match(url, url_pattern);
    }
    
    std::string extract_domain(const std::string& url) {
        std::regex domain_pattern(R"(https?://([^/]+))");
        std::smatch match;
        if (std::regex_search(url, match, domain_pattern)) {
            return match[1].str();
        }
        return "";
    }
    
    std::string classify_source_type(const std::string& url, const std::string& /* content */) {
        if (url.find(".edu") != std::string::npos) return "academic";
        if (url.find(".gov") != std::string::npos) return "government";
        if (url.find(".org") != std::string::npos) return "organization";
        if (url.find("news") != std::string::npos || 
            url.find("cnn") != std::string::npos ||
            url.find("bbc") != std::string::npos) return "news";
        return "web";
    }
    
    double calculate_information_density(const std::string& text) {
        // Simple metric: ratio of non-common words to total words
        std::set<std::string> common_words = {
            "the", "and", "or", "but", "in", "on", "at", "to", "for", "of", "with", "by",
            "a", "an", "is", "are", "was", "were", "be", "been", "have", "has", "had",
            "this", "that", "these", "those", "it", "its", "he", "she", "they", "we", "you"
        };
        
        std::stringstream ss(text);
        std::string word;
        int total_words = 0;
        int content_words = 0;
        
        while (ss >> word) {
            total_words++;
            
            // Clean word
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            word.erase(std::remove_if(word.begin(), word.end(), 
                       [](char c) { return std::ispunct(c); }), word.end());
            
            if (common_words.find(word) == common_words.end() && word.length() > 2) {
                content_words++;
            }
        }
        
        return total_words > 0 ? (double)content_words / total_words : 0.0;
    }
    
    std::string format_url_citation(const std::string& url, const std::string& title, 
                                   const std::string& accessed_date) {
        return title + ". " + url + " (Accessed: " + accessed_date + ")";
    }
    
    std::string generate_inline_citation(int citation_number) {
        return "[" + std::to_string(citation_number) + "]";
    }
}
