#include "retrieval_functions.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <set>

namespace RetrievalFunctions {

std::vector<std::string> chunk_text(const std::string& text, int chunk_size, int overlap) {
    std::vector<std::string> chunks;
    
    if (text.empty() || chunk_size <= 0) {
        return chunks;
    }
    
    if (text.length() <= static_cast<size_t>(chunk_size)) {
        chunks.push_back(text);
        return chunks;
    }
    
    size_t start = 0;
    while (start < text.length()) {
        size_t end = std::min(start + chunk_size, text.length());
        
        // Try to break at sentence or word boundaries
        if (end < text.length()) {
            // Look for sentence ending
            size_t sentence_end = text.find_last_of(".!?", end);
            if (sentence_end != std::string::npos && sentence_end > start + chunk_size/2) {
                end = sentence_end + 1;
            } else {
                // Look for word boundary
                size_t word_end = text.find_last_of(" \t\n", end);
                if (word_end != std::string::npos && word_end > start + chunk_size/2) {
                    end = word_end;
                }
            }
        }
        
        std::string chunk = text.substr(start, end - start);
        if (!chunk.empty()) {
            chunks.push_back(chunk);
        }
        
        start = end - overlap;
        if (start >= end) break;
    }
    
    return chunks;
}

json extract_metadata(const std::string& content) {
    json metadata;
    
    // Basic statistics
    metadata["length"] = content.length();
    metadata["word_count"] = std::count(content.begin(), content.end(), ' ') + 1;
    metadata["line_count"] = std::count(content.begin(), content.end(), '\n') + 1;
    metadata["paragraph_count"] = std::count(content.begin(), content.end(), '\n') / 2 + 1;
    
    // Extract potential title (first line or first sentence)
    std::istringstream stream(content);
    std::string first_line;
    if (std::getline(stream, first_line) && !first_line.empty()) {
        metadata["potential_title"] = first_line.substr(0, std::min(100, static_cast<int>(first_line.length())));
    }
    
    // Language detection (basic)
    std::regex english_pattern(R"([a-zA-Z\s.,!?;:'"()-]+)");
    std::regex code_pattern(R"(\{|\}|\[|\]|#include|function|class|def |import |from )");
    
    if (std::regex_search(content, code_pattern)) {
        metadata["content_type"] = "code";
    } else if (std::regex_search(content, english_pattern)) {
        metadata["content_type"] = "text";
    } else {
        metadata["content_type"] = "unknown";
    }
    
    // Extract keywords (simple frequency-based approach)
    std::regex word_regex(R"(\b[a-zA-Z]{4,}\b)");
    std::map<std::string, int> word_freq;
    std::sregex_iterator iter(content.begin(), content.end(), word_regex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::string word = iter->str();
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        word_freq[word]++;
    }
    
    // Get top keywords
    std::vector<std::pair<int, std::string>> freq_pairs;
    for (const auto& [word, freq] : word_freq) {
        freq_pairs.emplace_back(freq, word);
    }
    
    std::sort(freq_pairs.rbegin(), freq_pairs.rend());
    
    json keywords = json::array();
    for (size_t i = 0; i < std::min(size_t(10), freq_pairs.size()); ++i) {
        keywords.push_back(freq_pairs[i].second);
    }
    metadata["keywords"] = keywords;
    
    return metadata;
}

json analyze_document_structure(const std::string& content) {
    json analysis;
    
    // Extract metadata
    analysis["metadata"] = extract_metadata(content);
    
    // Analyze structure
    std::vector<std::string> lines;
    std::istringstream stream(content);
    std::string line;
    
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    
    analysis["structure"] = {
        {"total_lines", lines.size()},
        {"empty_lines", std::count_if(lines.begin(), lines.end(), [](const std::string& l) { return l.empty(); })},
        {"average_line_length", lines.empty() ? 0 : content.length() / lines.size()}
    };
    
    // Identify sections (lines that look like headers)
    json sections = json::array();
    std::regex header_pattern(R"(^#+\s+.*|^[A-Z][A-Z\s]+$|^\d+\.\s+.*|^[IVX]+\.\s+.*)");
    
    for (size_t i = 0; i < lines.size(); ++i) {
        if (std::regex_match(lines[i], header_pattern)) {
            sections.push_back({
                {"line", i + 1},
                {"text", lines[i]},
                {"type", "header"}
            });
        }
    }
    
    analysis["sections"] = sections;
    
    // Chunking recommendation
    int recommended_chunk_size = std::max(256, std::min(1024, static_cast<int>(content.length() / 10)));
    analysis["chunking_recommendation"] = {
        {"chunk_size", recommended_chunk_size},
        {"estimated_chunks", (content.length() + recommended_chunk_size - 1) / recommended_chunk_size},
        {"overlap", recommended_chunk_size / 10}
    };
    
    return analysis;
}

json advanced_similarity_search(const json& query_params, const json& filters) {
    json result;
    
    // This would integrate with the actual vector database
    // For now, return a structured response format
    
    std::string query = query_params.value("query", "");
    int limit = query_params.value("limit", 10);
    double threshold = query_params.value("threshold", 0.7);
    
    result["query"] = query;
    result["limit"] = limit;
    result["threshold"] = threshold;
    result["filters_applied"] = filters;
    result["results"] = json::array();
    result["search_time"] = "0.123s";
    result["total_matches"] = 0;
    
    // Placeholder for actual implementation
    result["status"] = "placeholder_implementation";
    result["message"] = "Advanced similarity search would be implemented here";
    
    return result;
}

json batch_add_documents(const json& documents) {
    json result;
    
    if (!documents.is_array()) {
        throw std::runtime_error("Documents parameter must be an array");
    }
    
    result["total_documents"] = documents.size();
    result["processed"] = 0;
    result["failed"] = 0;
    result["errors"] = json::array();
    result["document_ids"] = json::array();
    
    // Process each document
    for (size_t i = 0; i < documents.size(); ++i) {
        const auto& doc = documents[i];
        
        try {
            // Validate document structure
            if (!doc.contains("content")) {
                throw std::runtime_error("Document missing 'content' field");
            }
            
            std::string content = doc["content"];
            if (content.empty()) {
                throw std::runtime_error("Document content is empty");
            }
            
            // Generate metadata if not provided
            json metadata = doc.value("metadata", json::object());
            if (metadata.empty()) {
                metadata = extract_metadata(content);
            }
            
            // Add document ID
            std::string doc_id = "doc_" + std::to_string(i) + "_" + std::to_string(std::time(nullptr));
            result["document_ids"].push_back(doc_id);
            
            result["processed"] = result["processed"].get<int>() + 1;
            
        } catch (const std::exception& e) {
            result["failed"] = result["failed"].get<int>() + 1;
            result["errors"].push_back({
                {"document_index", i},
                {"error", e.what()}
            });
        }
    }
    
    result["success_rate"] = static_cast<double>(result["processed"]) / documents.size();
    result["status"] = result["failed"].get<int>() == 0 ? "success" : "partial_success";
    
    return result;
}

std::vector<std::string> generate_search_suggestions(const std::string& query) {
    std::vector<std::string> suggestions;
    
    // Basic query expansion
    std::string lower_query = query;
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);
    
    // Add variations
    suggestions.push_back(query + " definition");
    suggestions.push_back(query + " explanation");
    suggestions.push_back(query + " examples");
    suggestions.push_back("what is " + query);
    suggestions.push_back("how to " + query);
    
    // Add related terms (simplified)
    if (lower_query.find("program") != std::string::npos) {
        suggestions.push_back(query + " code");
        suggestions.push_back(query + " algorithm");
    }
    
    if (lower_query.find("data") != std::string::npos) {
        suggestions.push_back(query + " analysis");
        suggestions.push_back(query + " structure");
    }
    
    return suggestions;
}

json organize_documents_by_similarity(const json& params) {
    json result;
    
    double similarity_threshold = params.value("threshold", 0.8);
    int max_clusters = params.value("max_clusters", 10);
    
    result["threshold"] = similarity_threshold;
    result["max_clusters"] = max_clusters;
    result["clusters"] = json::array();
    result["unclustered"] = json::array();
    
    // Placeholder for clustering algorithm
    result["status"] = "placeholder_implementation";
    result["message"] = "Document clustering would be implemented here using vector similarities";
    
    return result;
}

json extract_knowledge_graph(const json& documents) {
    json graph;
    
    graph["nodes"] = json::array();
    graph["edges"] = json::array();
    graph["entities"] = json::array();
    graph["relationships"] = json::array();
    
    // Basic entity extraction (simplified)
    std::regex entity_pattern(R"(\b[A-Z][a-z]+(?:\s+[A-Z][a-z]+)*\b)");
    std::set<std::string> entities;
    
    if (documents.is_array()) {
        for (const auto& doc : documents) {
            if (doc.contains("content")) {
                std::string content = doc["content"];
                
                std::sregex_iterator iter(content.begin(), content.end(), entity_pattern);
                std::sregex_iterator end;
                
                for (; iter != end; ++iter) {
                    entities.insert(iter->str());
                }
            }
        }
    }
    
    // Convert entities to nodes
    for (const auto& entity : entities) {
        graph["nodes"].push_back({
            {"id", entity},
            {"label", entity},
            {"type", "entity"}
        });
        
        graph["entities"].push_back(entity);
    }
    
    graph["total_entities"] = entities.size();
    graph["status"] = "placeholder_implementation";
    graph["message"] = "Knowledge graph extraction would be enhanced with NLP models";
    
    return graph;
}

} // namespace RetrievalFunctions
