#include <gtest/gtest.h>
#include "../src/functions/deep_research_functions.hpp"
#include "../include/workflow_types.hpp"
#include <chrono>
#include <thread>

using namespace DeepResearchFunctions;

class DeepResearchTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test environment
    }
    
    void TearDown() override {
        // Cleanup after tests
    }
};

// Test Research Planning Function
TEST_F(DeepResearchTest, PlanResearchBasic) {
    json params;
    params["query"] = "artificial intelligence";
    params["research_scope"] = "broad";
    params["depth_level"] = "basic";
    
    ResearchPlan plan = plan_research(params);
    
    EXPECT_EQ(plan.query, "artificial intelligence");
    EXPECT_EQ(plan.scope, "broad");
    EXPECT_EQ(plan.depth_level, "basic");
    EXPECT_GT(plan.research_phases.size(), 0);
    EXPECT_GT(plan.key_questions.size(), 0);
    EXPECT_GT(plan.required_sources.size(), 0);
}

TEST_F(DeepResearchTest, PlanResearchAdvanced) {
    json params;
    params["query"] = "machine learning algorithms";
    params["research_scope"] = "comprehensive";
    params["depth_level"] = "advanced";
    
    ResearchPlan plan = plan_research(params);
    
    EXPECT_EQ(plan.depth_level, "advanced");
    EXPECT_GT(plan.key_questions.size(), 5); // Advanced should have more questions
    EXPECT_GT(plan.required_sources.size(), 3); // Comprehensive should have more sources
}

TEST_F(DeepResearchTest, PlanResearchExpert) {
    json params;
    params["query"] = "quantum computing";
    params["research_scope"] = "comprehensive";
    params["depth_level"] = "expert";
    
    ResearchPlan plan = plan_research(params);
    
    EXPECT_EQ(plan.depth_level, "expert");
    EXPECT_GT(plan.key_questions.size(), 7); // Expert should have most questions
    EXPECT_TRUE(plan.metadata.contains("estimated_duration_minutes"));
}

// Test Targeted Research Function
TEST_F(DeepResearchTest, TargetedResearch) {
    json params;
    params["research_gaps"] = json::array({"gap1", "gap2", "gap3"});
    params["search_terms"] = json::array({"term1", "term2"});
    params["sources"] = json::array({"source1", "source2"});
    
    json result = targeted_research(params);
    
    EXPECT_TRUE(result.contains("research_gaps_addressed"));
    EXPECT_TRUE(result.contains("findings"));
    EXPECT_EQ(result["status"], "completed");
    EXPECT_EQ(result["findings"].size(), 3); // Should have findings for each gap
}

// Test Fact Verification Function
TEST_F(DeepResearchTest, VerifyFacts) {
    json params;
    params["findings"] = json::array({
        "Fact 1: AI was invented in 1956",
        "Fact 2: Machine learning is a subset of AI",
        "Fact 3: Neural networks mimic human brain"
    });
    params["sources"] = json::array({"source1", "source2", "source3"});
    params["verification_depth"] = "thorough";
    
    json result = verify_facts(params);
    
    EXPECT_TRUE(result.contains("verification_results"));
    EXPECT_TRUE(result.contains("overall_verification_rate"));
    EXPECT_EQ(result["verification_results"].size(), 3);
    EXPECT_EQ(result["verification_depth"], "thorough");
}

// Test Research Synthesis Function
TEST_F(DeepResearchTest, SynthesizeResearch) {
    json params;
    params["primary_data"] = {{"source1", "data1"}, {"source2", "data2"}};
    params["knowledge_base_data"] = {{"kb1", "kb_data1"}};
    params["synthesis_type"] = "comprehensive";
    
    SynthesisResult result = synthesize_research(params);
    
    EXPECT_FALSE(result.summary.empty());
    EXPECT_GT(result.key_insights.size(), 0);
    EXPECT_GT(result.research_gaps.size(), 0);
    EXPECT_TRUE(result.metadata.contains("synthesis_type"));
    EXPECT_EQ(result.metadata["synthesis_type"], "comprehensive");
}

// Test Report Generation Function
TEST_F(DeepResearchTest, GenerateResearchReport) {
    json params;
    params["research_data"] = {{"findings", "comprehensive research data"}};
    params["analysis_results"] = {{"insights", "key analytical insights"}};
    params["report_format"] = "detailed";
    params["include_citations"] = true;
    
    json report = generate_research_report(params);
    
    EXPECT_TRUE(report.contains("title"));
    EXPECT_TRUE(report.contains("executive_summary"));
    EXPECT_TRUE(report.contains("sections"));
    EXPECT_TRUE(report.contains("citations"));
    EXPECT_EQ(report["format"], "detailed");
    EXPECT_EQ(report["include_citations"], true);
    EXPECT_GT(report["sections"].size(), 0);
}

// Test Cross-Reference Search Function
TEST_F(DeepResearchTest, CrossReferenceSearch) {
    json params;
    params["query"] = "deep learning applications";
    params["databases"] = json::array({"internet", "knowledge_base", "documents"});
    params["correlation_threshold"] = 0.8;
    
    json result = cross_reference_search(params);
    
    EXPECT_EQ(result["query"], "deep learning applications");
    EXPECT_TRUE(result.contains("cross_references"));
    EXPECT_TRUE(result.contains("overall_correlation_score"));
    EXPECT_EQ(result["databases_searched"].size(), 3);
    EXPECT_EQ(result["status"], "completed");
}

// Test Iterative Search Refinement Function
TEST_F(DeepResearchTest, IterativeSearchRefinement) {
    json params;
    params["initial_query"] = "artificial intelligence";
    params["previous_results"] = {{"iteration1", "some results"}};
    params["refinement_strategy"] = "narrow";
    
    json result = iterative_search_refinement(params);
    
    EXPECT_EQ(result["initial_query"], "artificial intelligence");
    EXPECT_EQ(result["refinement_strategy"], "narrow");
    EXPECT_TRUE(result.contains("iterations"));
    EXPECT_GT(result["iterations"].size(), 0);
    EXPECT_TRUE(result.contains("overall_improvement"));
}

// Test Source Credibility Analysis Function
TEST_F(DeepResearchTest, SourceCredibilityAnalysis) {
    json params;
    params["sources"] = json::array({
        "https://www.nature.com/articles/nature123",
        "https://en.wikipedia.org/wiki/AI",
        "https://blog.example.com/ai-post"
    });
    params["criteria"] = json::array({"authority", "accuracy", "currency", "objectivity"});
    
    json result = source_credibility_analysis(params);
    
    EXPECT_EQ(result["sources_analyzed"], 3);
    EXPECT_TRUE(result.contains("credibility_scores"));
    EXPECT_TRUE(result.contains("average_credibility"));
    EXPECT_EQ(result["credibility_scores"].size(), 3);
}

// Test Helper Functions
TEST_F(DeepResearchTest, ExtractKeyConcepts) {
    std::string query = "machine learning artificial intelligence neural networks";
    
    auto concepts = extract_key_concepts(query);
    
    EXPECT_GT(concepts.size(), 0);
    // Should extract words longer than 3 characters
    bool found_machine = std::find(concepts.begin(), concepts.end(), "machine") != concepts.end();
    bool found_learning = std::find(concepts.begin(), concepts.end(), "learning") != concepts.end();
    EXPECT_TRUE(found_machine || found_learning);
}

TEST_F(DeepResearchTest, GenerateSearchVariations) {
    std::string query = "deep learning";
    
    auto variations = generate_search_variations(query);
    
    EXPECT_GT(variations.size(), 3);
    // Should include exact phrase search
    bool found_exact = std::find(variations.begin(), variations.end(), "\"deep learning\"") != variations.end();
    EXPECT_TRUE(found_exact);
}

TEST_F(DeepResearchTest, ScoreSourceCredibility) {
    json criteria = json::array({"authority", "accuracy"});
    
    // Test academic source (.edu)
    double edu_score = score_source_credibility("https://university.edu/research", criteria);
    EXPECT_GT(edu_score, 0.5);
    
    // Test government source (.gov)
    double gov_score = score_source_credibility("https://government.gov/data", criteria);
    EXPECT_GT(gov_score, 0.5);
    
    // Test regular website
    double regular_score = score_source_credibility("http://example.com/blog", criteria);
    EXPECT_GE(regular_score, 0.0);
    EXPECT_LE(regular_score, 1.0);
}

TEST_F(DeepResearchTest, GenerateCitation) {
    ResearchFinding finding;
    finding.source_url = "https://example.com/article";
    finding.content = "Sample research finding";
    
    // Test APA format
    std::string apa_citation = generate_citation(finding, "APA");
    EXPECT_TRUE(apa_citation.find("2024") != std::string::npos);
    EXPECT_TRUE(apa_citation.find(finding.source_url) != std::string::npos);
    
    // Test MLA format
    std::string mla_citation = generate_citation(finding, "MLA");
    EXPECT_TRUE(mla_citation.find("2024") != std::string::npos);
    
    // Test default format
    std::string default_citation = generate_citation(finding);
    EXPECT_FALSE(default_citation.empty());
}

TEST_F(DeepResearchTest, CreateResearchTimeline) {
    std::vector<ResearchFinding> findings(3);
    findings[0].content = "Finding 1";
    findings[1].content = "Finding 2"; 
    findings[2].content = "Finding 3";
    
    json timeline = create_research_timeline(findings);
    
    EXPECT_TRUE(timeline.contains("timeline"));
    EXPECT_EQ(timeline["timeline"].size(), 3);
    
    // Check timeline structure
    for (const auto& entry : timeline["timeline"]) {
        EXPECT_TRUE(entry.contains("phase"));
        EXPECT_TRUE(entry.contains("timestamp"));
        EXPECT_TRUE(entry.contains("description"));
    }
}

TEST_F(DeepResearchTest, GenerateExecutiveSummary) {
    SynthesisResult synthesis;
    synthesis.summary = "This is a comprehensive research synthesis.";
    synthesis.key_insights = {
        "Insight 1: Important finding",
        "Insight 2: Critical discovery", 
        "Insight 3: Key observation"
    };
    
    std::string summary = generate_executive_summary(synthesis, 50);
    
    EXPECT_FALSE(summary.empty());
    EXPECT_TRUE(summary.find("comprehensive research synthesis") != std::string::npos);
    EXPECT_TRUE(summary.find("Key insights include") != std::string::npos);
}

// Integration Test for Deep Research Workflow
TEST_F(DeepResearchTest, DeepResearchWorkflowIntegration) {
    // Test the complete workflow integration
    
    // 1. Plan research
    json plan_params;
    plan_params["query"] = "blockchain technology applications";
    plan_params["research_scope"] = "comprehensive";
    plan_params["depth_level"] = "advanced";
    
    ResearchPlan plan = plan_research(plan_params);
    EXPECT_FALSE(plan.query.empty());
    
    // 2. Conduct targeted research based on plan
    json target_params;
    target_params["research_gaps"] = json::array({"technical details", "practical applications"});
    target_params["search_terms"] = json::array({"blockchain", "distributed ledger"});
    
    json target_result = targeted_research(target_params);
    EXPECT_EQ(target_result["status"], "completed");
    
    // 3. Synthesize results
    json synthesis_params;
    synthesis_params["primary_data"] = target_result;
    synthesis_params["synthesis_type"] = "comprehensive";
    
    SynthesisResult synthesis = synthesize_research(synthesis_params);
    EXPECT_FALSE(synthesis.summary.empty());
    
    // 4. Generate final report
    json report_params;
    report_params["research_data"] = target_result;
    report_params["analysis_results"] = {{"synthesis", synthesis.summary}};
    report_params["report_format"] = "detailed";
    report_params["include_citations"] = true;
    
    json final_report = generate_research_report(report_params);
    EXPECT_TRUE(final_report.contains("title"));
    EXPECT_TRUE(final_report.contains("sections"));
    EXPECT_TRUE(final_report.contains("citations"));
}
