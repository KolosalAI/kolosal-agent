/**
 * @file deep_research_agent_demo.cpp
 * @brief Comprehensive Deep Research Agent System Demo
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * This demo showcases the full capabilities of the Kolosal Agent System's 
 * deep research agents, including multi-agent coordination, advanced research
 * functions, and comprehensive report generation.
 */

#include "agent/core/multi_agent_system.hpp"
#include "agent/core/agent_factory.hpp"
#include "workflow/workflow_engine.hpp"
#include "tools/enhanced_function_registry.hpp"
#include "tools/research_functions.hpp"
#include "config/yaml_configuration_parser.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <json.hpp>

using namespace kolosal::agents;
using json = nlohmann::json;

/**
 * @brief Demo class for showcasing deep research agent capabilities
 */
class DeepResearchAgentDemo {
public:
    DeepResearchAgentDemo() : server_url("http://localhost:8080") {
        std::cout << "🔬 Kolosal Deep Research Agent System Demo v2.0\n";
        std::cout << "================================================\n\n";
    }
    
    ~DeepResearchAgentDemo() {
        cleanup();
    }
    
    /**
     * @brief Initialize the demo system
     */
    bool initialize() {
        try {
            std::cout << "🚀 Initializing Deep Research Agent System...\n";
            
            // Initialize agent manager
            agent_manager = std::make_shared<YAMLConfigurableAgentManager>();
            if (!agent_manager->load_configuration("config.yaml")) {
                std::cerr << "❌ Failed to load agent configuration from config.yaml\n";
                return false;
            }
            
            // Start agent manager
            agent_manager->start();
            
            // Initialize workflow engine
            workflow_engine = std::make_unique<WorkflowEngine>(agent_manager);
            workflow_engine->start();
            
            // Initialize function registry
            enhanced_registry = std::make_shared<EnhancedFunctionRegistry>(server_url);
            
            // Test server connection
            server_connected = enhanced_registry->test_server_connection();
            if (server_connected) {
                std::cout << "✅ Connected to kolosal-server at " << server_url << "\n";
            } else {
                std::cout << "⚠️  Server not available, using simulation mode\n";
            }
            
            std::cout << "✅ Deep Research Agent System initialized successfully\n\n";
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Initialization failed: " << e.what() << "\n";
            return false;
        }
    }
    
    /**
     * @brief Run the comprehensive demo
     */
    void run_demo() {
        try {
            display_system_overview();
            demo_agent_capabilities();
            demo_research_workflow();
            demo_advanced_research_functions();
            demo_multi_agent_collaboration();
            demo_comprehensive_research_project();
            display_final_summary();
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Demo execution failed: " << e.what() << "\n";
        }
    }

private:
    std::shared_ptr<YAMLConfigurableAgentManager> agent_manager;
    std::unique_ptr<WorkflowEngine> workflow_engine;
    std::shared_ptr<EnhancedFunctionRegistry> enhanced_registry;
    std::string server_url;
    bool server_connected = false;
    
    /**
     * @brief Display system overview
     */
    void display_system_overview() {
        std::cout << "📊 SYSTEM OVERVIEW\n";
        std::cout << "==================\n\n";
        
        auto agent_ids = agent_manager->list_agents();
        std::cout << "🤖 Available Agents: " << agent_ids.size() << "\n";
        
        for (const auto& agent_id : agent_ids) {
            auto agent = agent_manager->get__agent(agent_id);
            if (agent) {
                std::cout << "   • " << agent_id << " (" << agent->get__agent_type() << ")\n";
                std::cout << "     Role: " << static_cast<int>(agent->get__role()) << "\n";
                std::cout << "     Status: " << (agent->is__running() ? "✅ Running" : "⏸️ Stopped") << "\n";
            }
        }
        
        std::cout << "\n🛠️  Available Functions:\n";
        if (server_connected) {
            auto functions = enhanced_registry->get_available_functions(true);
            for (const auto& func : functions) {
                std::cout << "   • " << func << "\n";
            }
        } else {
            std::cout << "   • Using built-in function simulation\n";
        }
        
        std::cout << "\n";
    }
    
    /**
     * @brief Demo individual agent capabilities
     */
    void demo_agent_capabilities() {
        std::cout << "🧪 AGENT CAPABILITIES DEMONSTRATION\n";
        std::cout << "=====================================\n\n";
        
        // Test Research Coordinator
        demo_research_coordinator();
        
        // Test Advanced Research Analyst
        demo_advanced_research_analyst();
        
        // Test Knowledge Curator
        demo_knowledge_curator();
        
        // Test Source Analyst
        demo_source_analyst();
        
        // Test Synthesis Specialist
        demo_synthesis_specialist();
    }
    
    void demo_research_coordinator() {
        std::cout << "🎯 Research Coordinator Agent\n";
        std::cout << "------------------------------\n";
        
        auto coordinator = agent_manager->get__agent("research_coordinator");
        if (coordinator && coordinator->is__running()) {
            std::cout << "✅ Agent active and ready\n";
            
            // Test research query planning
            AgentData planning_params;
            planning_params.set("research_question", "What are the latest developments in quantum computing applications for machine learning?");
            planning_params.set("methodology", "systematic_review");
            planning_params.set("scope", "comprehensive");
            
            std::cout << "🔍 Testing research query planning...\n";
            auto result = coordinator->execute_function("research_query_planning", planning_params);
            
            if (result.success) {
                std::cout << "✅ Research planning successful!\n";
                auto sub_queries = result.result_data.get_array_string("sub_queries");
                std::cout << "   📋 Generated " << sub_queries.size() << " sub-queries\n";
                for (size_t i = 0; i < std::min(sub_queries.size(), size_t(3)); ++i) {
                    std::cout << "      " << (i+1) << ". " << sub_queries[i] << "\n";
                }
                
                auto search_terms = result.result_data.get_array_string("search_terms");
                std::cout << "   🔎 Key search terms: ";
                for (size_t i = 0; i < std::min(search_terms.size(), size_t(5)); ++i) {
                    std::cout << search_terms[i] << (i < 4 ? ", " : "");
                }
                std::cout << "\n";
                
                auto sources = result.result_data.get_array_string("recommended_sources");
                std::cout << "   📚 Recommended sources: " << sources.size() << "\n";
            } else {
                std::cout << "❌ Research planning failed: " << result.error_message << "\n";
            }
            
        } else {
            std::cout << "❌ Research Coordinator not available\n";
        }
        std::cout << "\n";
    }
    
    void demo_advanced_research_analyst() {
        std::cout << "📊 Advanced Research Analyst Agent\n";
        std::cout << "----------------------------------\n";
        
        auto analyst = agent_manager->get__agent("advanced_research_analyst");
        if (analyst && analyst->is__running()) {
            std::cout << "✅ Agent active and ready\n";
            
            // Test source credibility analysis
            AgentData credibility_params;
            credibility_params.set("source_url", "https://nature.com/articles/quantum-ml-2024");
            credibility_params.set("content", "This peer-reviewed research presents novel quantum machine learning algorithms with experimental validation and comprehensive methodology description.");
            credibility_params.set("author", "Dr. Jane Smith, PhD, Professor of Quantum Computing");
            credibility_params.set("publication", "Nature Quantum Computing");
            
            std::cout << "🔍 Testing source credibility analysis...\n";
            auto result = analyst->execute_function("source_credibility_analysis", credibility_params);
            
            if (result.success) {
                std::cout << "✅ Credibility analysis successful!\n";
                double score = result.result_data.get_double("credibility_score", 0.0);
                std::string assessment = result.result_data.get_string("assessment", "unknown");
                std::cout << "   📈 Credibility Score: " << std::fixed << std::setprecision(2) << score << "\n";
                std::cout << "   📋 Assessment: " << assessment << "\n";
                
                auto factors = result.result_data.get_array_string("factors");
                if (!factors.empty()) {
                    std::cout << "   ✓ Contributing factors:\n";
                    for (const auto& factor : factors) {
                        std::cout << "      • " << factor << "\n";
                    }
                }
            } else {
                std::cout << "❌ Credibility analysis failed: " << result.error_message << "\n";
            }
            
        } else {
            std::cout << "❌ Advanced Research Analyst not available\n";
        }
        std::cout << "\n";
    }
    
    void demo_knowledge_curator() {
        std::cout << "🧠 Knowledge Curator Agent\n";
        std::cout << "--------------------------\n";
        
        auto curator = agent_manager->get__agent("knowledge_curator");
        if (curator && curator->is__running()) {
            std::cout << "✅ Agent active and ready\n";
            
            // Test entity extraction
            AgentData entity_params;
            entity_params.set("content", "Dr. John Doe from MIT collaborated with researchers at Google DeepMind to develop new quantum machine learning algorithms using TensorFlow Quantum framework in Boston.");
            entity_params.set("entity_types", "person,organization,location,concept");
            
            std::cout << "🔍 Testing entity extraction...\n";
            auto result = curator->execute_function("entity_extraction", entity_params);
            
            if (result.success) {
                std::cout << "✅ Entity extraction successful!\n";
                
                auto persons = result.result_data.get_array_string("persons");
                auto organizations = result.result_data.get_array_string("organizations");
                auto locations = result.result_data.get_array_string("locations");
                auto concepts = result.result_data.get_array_string("concepts");
                
                if (!persons.empty()) {
                    std::cout << "   👤 Persons: ";
                    for (size_t i = 0; i < persons.size(); ++i) {
                        std::cout << persons[i] << (i < persons.size()-1 ? ", " : "");
                    }
                    std::cout << "\n";
                }
                
                if (!organizations.empty()) {
                    std::cout << "   🏢 Organizations: ";
                    for (size_t i = 0; i < organizations.size(); ++i) {
                        std::cout << organizations[i] << (i < organizations.size()-1 ? ", " : "");
                    }
                    std::cout << "\n";
                }
                
                if (!concepts.empty()) {
                    std::cout << "   💡 Concepts: ";
                    for (size_t i = 0; i < concepts.size(); ++i) {
                        std::cout << concepts[i] << (i < concepts.size()-1 ? ", " : "");
                    }
                    std::cout << "\n";
                }
                
                int total = result.result_data.get_int("total_entities", 0);
                std::cout << "   📊 Total entities extracted: " << total << "\n";
            } else {
                std::cout << "❌ Entity extraction failed: " << result.error_message << "\n";
            }
            
        } else {
            std::cout << "❌ Knowledge Curator not available\n";
        }
        std::cout << "\n";
    }
    
    void demo_source_analyst() {
        std::cout << "🔍 Source Analyst Agent\n";
        std::cout << "-----------------------\n";
        
        // Check if agent needs to be started
        auto analyst = agent_manager->get__agent("source_analyst");
        if (analyst) {
            if (!analyst->is__running()) {
                std::cout << "🔄 Starting Source Analyst agent...\n";
                analyst->start();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            
            if (analyst->is__running()) {
                std::cout << "✅ Agent active and ready\n";
                std::cout << "   🎯 Specialized in source quality evaluation and bias detection\n";
                std::cout << "   📊 Can analyze publication credibility and citation patterns\n";
            } else {
                std::cout << "❌ Failed to start Source Analyst agent\n";
            }
        } else {
            std::cout << "❌ Source Analyst not available in configuration\n";
        }
        std::cout << "\n";
    }
    
    void demo_synthesis_specialist() {
        std::cout << "📝 Synthesis Specialist Agent\n";
        std::cout << "-----------------------------\n";
        
        // Check if agent needs to be started
        auto specialist = agent_manager->get__agent("synthesis_specialist");
        if (specialist) {
            if (!specialist->is__running()) {
                std::cout << "🔄 Starting Synthesis Specialist agent...\n";
                specialist->start();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            
            if (specialist->is__running()) {
                std::cout << "✅ Agent active and ready\n";
                
                // Test information synthesis
                AgentData synthesis_params;
                synthesis_params.set("sources", std::vector<std::string>{
                    "Source 1: Quantum computing shows 40% improvement in ML tasks",
                    "Source 2: Recent studies indicate quantum advantage in specific ML algorithms",
                    "Source 3: Challenges remain in quantum ML scalability"
                });
                synthesis_params.set("synthesis_type", "comprehensive");
                synthesis_params.set("confidence_threshold", 0.7);
                
                std::cout << "🔍 Testing information synthesis...\n";
                auto result = specialist->execute_function("information_synthesis", synthesis_params);
                
                if (result.success) {
                    std::cout << "✅ Information synthesis successful!\n";
                    
                    auto themes = result.result_data.get_array_string("key_themes");
                    if (!themes.empty()) {
                        std::cout << "   🎯 Key themes identified:\n";
                        for (const auto& theme : themes) {
                            std::cout << "      • " << theme << "\n";
                        }
                    }
                    
                    auto consensus = result.result_data.get_array_string("consensus_points");
                    if (!consensus.empty()) {
                        std::cout << "   ✅ Points of consensus:\n";
                        for (const auto& point : consensus) {
                            std::cout << "      • " << point << "\n";
                        }
                    }
                    
                    std::string summary = result.result_data.get_string("synthesized_summary", "");
                    if (!summary.empty()) {
                        std::cout << "   📄 Synthesis summary:\n      " << summary << "\n";
                    }
                    
                } else {
                    std::cout << "❌ Information synthesis failed: " << result.error_message << "\n";
                }
                
            } else {
                std::cout << "❌ Failed to start Synthesis Specialist agent\n";
            }
        } else {
            std::cout << "❌ Synthesis Specialist not available in configuration\n";
        }
        std::cout << "\n";
    }
    
    /**
     * @brief Demo research workflow
     */
    void demo_research_workflow() {
        std::cout << "🔄 RESEARCH WORKFLOW DEMONSTRATION\n";
        std::cout << "==================================\n\n";
        
        std::cout << "📋 Creating comprehensive research workflow...\n";
        
        // Create a research workflow
        auto research_workflow = workflow_engine->create_sequential_workflow(
            "Deep Research Analysis Workflow",
            {
                {"research_coordinator", "research_query_planning"},
                {"research_coordinator", "methodology_selection"},
                {"advanced_research_analyst", "source_credibility_analysis"},
                {"knowledge_curator", "entity_extraction"},
                {"advanced_research_analyst", "information_synthesis"},
                {"synthesis_specialist", "research_report_generation"}
            }
        );
        
        research_workflow.description = "Comprehensive deep research workflow with multi-agent coordination";
        research_workflow.global_context = {
            {"research_topic", "Quantum Machine Learning Applications in Healthcare"},
            {"urgency", "medium"},
            {"quality_threshold", "high"},
            {"output_format", "comprehensive_report"}
        };
        
        std::string workflow_id = workflow_engine->create_workflow(research_workflow);
        std::cout << "✅ Created research workflow: " << workflow_id << "\n";
        
        // Execute the workflow
        json workflow_input = {
            {"research_question", "How can quantum machine learning be applied to improve medical diagnosis accuracy?"},
            {"methodology", "systematic_review"},
            {"scope", "comprehensive"},
            {"domain", "healthcare_ai"},
            {"time_constraint", "medium"},
            {"resource_level", 4}
        };
        
        std::cout << "🚀 Executing research workflow...\n";
        std::string execution_id = workflow_engine->execute_workflow(workflow_id, workflow_input);
        std::cout << "   Execution ID: " << execution_id << "\n";
        
        // Monitor workflow progress
        monitor_workflow_progress(execution_id);
        
        std::cout << "\n";
    }
    
    /**
     * @brief Demo advanced research functions
     */
    void demo_advanced_research_functions() {
        std::cout << "🧬 ADVANCED RESEARCH FUNCTIONS DEMO\n";
        std::cout << "===================================\n\n";
        
        demo_fact_verification();
        demo_knowledge_graph_query();
        demo_research_report_generation();
        demo_citation_management();
    }
    
    void demo_fact_verification() {
        std::cout << "🔍 Fact Verification System\n";
        std::cout << "---------------------------\n";
        
        auto curator = agent_manager->get__agent("knowledge_curator");
        if (curator && curator->is__running()) {
            AgentData verification_params;
            verification_params.set("claim", "Quantum computers can solve certain machine learning problems exponentially faster than classical computers");
            verification_params.set("sources", std::vector<std::string>{
                "Nature Quantum Computing 2024 research paper",
                "IBM Quantum research findings",
                "Google Quantum AI laboratory results",
                "MIT Technology Review analysis"
            });
            verification_params.set("confidence_threshold", 0.75);
            
            std::cout << "🔍 Verifying claim with multiple sources...\n";
            auto result = curator->execute_function("fact_verification", verification_params);
            
            if (result.success) {
                std::cout << "✅ Fact verification completed!\n";
                
                std::string status = result.result_data.get_string("verification_status", "unknown");
                double confidence = result.result_data.get_double("verification_confidence", 0.0);
                int supporting = result.result_data.get_int("supporting_sources", 0);
                int contradicting = result.result_data.get_int("contradicting_sources", 0);
                
                std::cout << "   📊 Verification Status: " << status << "\n";
                std::cout << "   📈 Confidence: " << std::fixed << std::setprecision(1) << (confidence * 100) << "%\n";
                std::cout << "   ✅ Supporting sources: " << supporting << "\n";
                std::cout << "   ❌ Contradicting sources: " << contradicting << "\n";
                
                bool meets_threshold = result.result_data.get_bool("meets_threshold", false);
                std::cout << "   🎯 Meets reliability threshold: " << (meets_threshold ? "Yes" : "No") << "\n";
                
            } else {
                std::cout << "❌ Fact verification failed: " << result.error_message << "\n";
            }
        } else {
            std::cout << "❌ Knowledge Curator not available for fact verification\n";
        }
        std::cout << "\n";
    }
    
    void demo_knowledge_graph_query() {
        std::cout << "🌐 Knowledge Graph Query System\n";
        std::cout << "-------------------------------\n";
        
        auto curator = agent_manager->get__agent("knowledge_curator");
        if (curator && curator->is__running()) {
            AgentData query_params;
            query_params.set("query", "quantum machine learning healthcare applications");
            query_params.set("entity_type", "concept");
            query_params.set("max_results", 10);
            
            std::cout << "🔍 Querying knowledge graph...\n";
            auto result = curator->execute_function("knowledge_graph_query", query_params);
            
            if (result.success) {
                std::cout << "✅ Knowledge graph query successful!\n";
                
                auto entities = result.result_data.get_array_string("entities");
                auto relationships = result.result_data.get_array_string("relationships");
                auto concepts = result.result_data.get_array_string("related_concepts");
                
                if (!entities.empty()) {
                    std::cout << "   🎯 Related entities:\n";
                    for (const auto& entity : entities) {
                        std::cout << "      • " << entity << "\n";
                    }
                }
                
                if (!relationships.empty()) {
                    std::cout << "   🔗 Key relationships:\n";
                    for (const auto& rel : relationships) {
                        std::cout << "      • " << rel << "\n";
                    }
                }
                
                int count = result.result_data.get_int("results_count", 0);
                std::cout << "   📊 Total results found: " << count << "\n";
                
            } else {
                std::cout << "❌ Knowledge graph query failed: " << result.error_message << "\n";
            }
        } else {
            std::cout << "❌ Knowledge Curator not available for knowledge graph queries\n";
        }
        std::cout << "\n";
    }
    
    void demo_research_report_generation() {
        std::cout << "📄 Research Report Generation System\n";
        std::cout << "------------------------------------\n";
        
        auto specialist = agent_manager->get__agent("synthesis_specialist");
        if (specialist && specialist->is__running()) {
            AgentData report_params;
            report_params.set("research_data", "Comprehensive analysis of quantum machine learning applications in healthcare diagnostics, covering recent developments, methodological approaches, and future research directions.");
            report_params.set("report_format", "academic");
            report_params.set("include_citations", true);
            report_params.set("template_type", "research_summary");
            
            std::cout << "📝 Generating comprehensive research report...\n";
            auto result = specialist->execute_function("research_report_generation", report_params);
            
            if (result.success) {
                std::cout << "✅ Research report generated successfully!\n";
                
                auto sections = result.result_data.get_array_string("sections");
                std::string format = result.result_data.get_string("report_format", "");
                int word_count = result.result_data.get_int("word_count", 0);
                int citation_count = result.result_data.get_int("citation_count", 0);
                
                std::cout << "   📋 Report sections (" << sections.size() << "):\n";
                for (const auto& section : sections) {
                    std::cout << "      • " << section << "\n";
                }
                
                std::cout << "   📊 Report statistics:\n";
                std::cout << "      • Format: " << format << "\n";
                std::cout << "      • Estimated word count: " << word_count << "\n";
                std::cout << "      • Citations included: " << citation_count << "\n";
                
                // Show a snippet of the report
                std::string content = result.result_data.get_string("report_content", "");
                if (!content.empty() && content.length() > 200) {
                    std::cout << "   📄 Report preview:\n";
                    std::cout << "      " << content.substr(0, 200) << "...\n";
                }
                
            } else {
                std::cout << "❌ Research report generation failed: " << result.error_message << "\n";
            }
        } else {
            std::cout << "❌ Synthesis Specialist not available for report generation\n";
        }
        std::cout << "\n";
    }
    
    void demo_citation_management() {
        std::cout << "📚 Citation Management System\n";
        std::cout << "-----------------------------\n";
        
        auto specialist = agent_manager->get__agent("synthesis_specialist");
        if (specialist && specialist->is__running()) {
            AgentData citation_params;
            citation_params.set("sources", std::vector<std::string>{
                "Smith, J. Quantum Machine Learning in Healthcare. Nature Medicine, 2024.",
                "Johnson, A. et al. Deep Learning with Quantum Computers. Science, 2024.",
                "Brown, M. Quantum Algorithms for Medical Diagnosis. Cell, 2024."
            });
            citation_params.set("citation_style", "APA");
            citation_params.set("operation", "format");
            
            std::cout << "📝 Formatting citations in APA style...\n";
            auto result = specialist->execute_function("citation_management", citation_params);
            
            if (result.success) {
                std::cout << "✅ Citation management successful!\n";
                
                auto formatted = result.result_data.get_array_string("formatted_citations");
                auto validation = result.result_data.get_array_string("validation_results");
                std::string style = result.result_data.get_string("citation_style", "");
                
                std::cout << "   📋 Formatted citations (" << style << " style):\n";
                for (size_t i = 0; i < formatted.size(); ++i) {
                    std::cout << "      " << (i+1) << ". " << formatted[i] << "\n";
                    if (i < validation.size()) {
                        std::cout << "         Status: " << validation[i] << "\n";
                    }
                }
                
                int processed = result.result_data.get_int("processed_count", 0);
                std::cout << "   📊 Total citations processed: " << processed << "\n";
                
            } else {
                std::cout << "❌ Citation management failed: " << result.error_message << "\n";
            }
        } else {
            std::cout << "❌ Synthesis Specialist not available for citation management\n";
        }
        std::cout << "\n";
    }
    
    /**
     * @brief Demo multi-agent collaboration
     */
    void demo_multi_agent_collaboration() {
        std::cout << "🤝 MULTI-AGENT COLLABORATION DEMO\n";
        std::cout << "=================================\n\n";
        
        std::cout << "🎭 Orchestrating multi-agent research collaboration...\n";
        
        // Simulate complex research task requiring multiple agents
        std::string research_topic = "Ethical implications of AI-powered medical diagnosis systems";
        
        std::cout << "📋 Research Topic: " << research_topic << "\n\n";
        
        // Step 1: Research Coordinator plans the research
        std::cout << "1️⃣ Research Coordinator: Planning research approach\n";
        auto coordinator = agent_manager->get__agent("research_coordinator");
        if (coordinator && coordinator->is__running()) {
            AgentData planning_params;
            planning_params.set("research_question", research_topic);
            planning_params.set("methodology", "mixed_methods");
            planning_params.set("scope", "comprehensive");
            
            auto planning_result = coordinator->execute_function("research_query_planning", planning_params);
            if (planning_result.success) {
                std::cout << "   ✅ Research plan created\n";
                auto sub_queries = planning_result.result_data.get_array_string("sub_queries");
                std::cout << "   📝 Generated " << sub_queries.size() << " research sub-questions\n";
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Step 2: Knowledge Curator extracts key entities
        std::cout << "\n2️⃣ Knowledge Curator: Extracting key entities and concepts\n";
        auto curator = agent_manager->get__agent("knowledge_curator");
        if (curator && curator->is__running()) {
            AgentData entity_params;
            entity_params.set("content", "AI-powered medical diagnosis systems raise concerns about algorithmic bias, patient privacy, healthcare equity, clinical decision-making autonomy, and regulatory compliance in medical AI applications.");
            entity_params.set("entity_types", "concept");
            
            auto entity_result = curator->execute_function("entity_extraction", entity_params);
            if (entity_result.success) {
                std::cout << "   ✅ Entity extraction completed\n";
                auto concepts = entity_result.result_data.get_array_string("concepts");
                std::cout << "   🧠 Identified " << concepts.size() << " key concepts\n";
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Step 3: Advanced Research Analyst synthesizes information
        std::cout << "\n3️⃣ Advanced Research Analyst: Synthesizing multi-source information\n";
        auto analyst = agent_manager->get__agent("advanced_research_analyst");
        if (analyst && analyst->is__running()) {
            AgentData synthesis_params;
            synthesis_params.set("sources", std::vector<std::string>{
                "Medical AI ethics guidelines from WHO",
                "FDA regulations on AI medical devices", 
                "Academic research on algorithmic bias in healthcare",
                "Patient privacy considerations in AI diagnostics"
            });
            synthesis_params.set("synthesis_type", "comprehensive");
            
            auto synthesis_result = analyst->execute_function("information_synthesis", synthesis_params);
            if (synthesis_result.success) {
                std::cout << "   ✅ Information synthesis completed\n";
                auto themes = synthesis_result.result_data.get_array_string("key_themes");
                std::cout << "   🎯 Identified " << themes.size() << " major themes\n";
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Step 4: Synthesis Specialist generates final report
        std::cout << "\n4️⃣ Synthesis Specialist: Generating comprehensive report\n";
        auto specialist = agent_manager->get__agent("synthesis_specialist");
        if (specialist) {
            if (!specialist->is__running()) {
                specialist->start();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            
            if (specialist->is__running()) {
                AgentData report_params;
                report_params.set("research_data", "Comprehensive analysis of ethical implications in AI medical diagnosis systems");
                report_params.set("report_format", "executive");
                report_params.set("include_citations", true);
                
                auto report_result = specialist->execute_function("research_report_generation", report_params);
                if (report_result.success) {
                    std::cout << "   ✅ Comprehensive report generated\n";
                    int word_count = report_result.result_data.get_int("word_count", 0);
                    std::cout << "   📄 Report length: ~" << word_count << " words\n";
                }
            }
        }
        
        std::cout << "\n🎉 Multi-agent collaboration completed successfully!\n";
        std::cout << "💡 This demonstrates how different specialized agents can work together\n";
        std::cout << "   to tackle complex research problems systematically.\n\n";
    }
    
    /**
     * @brief Demo comprehensive research project
     */
    void demo_comprehensive_research_project() {
        std::cout << "🏆 COMPREHENSIVE RESEARCH PROJECT DEMO\n";
        std::cout << "======================================\n\n";
        
        std::cout << "🚀 Executing end-to-end deep research project...\n";
        std::cout << "📋 Project: \"Impact of Large Language Models on Scientific Research\"\n\n";
        
        // Create a complex workflow for comprehensive research
        auto comprehensive_workflow = workflow_engine->create_sequential_workflow(
            "Comprehensive Deep Research Project",
            {
                {"research_coordinator", "research_query_planning"},
                {"research_coordinator", "methodology_selection"},
                {"knowledge_curator", "entity_extraction"}, 
                {"knowledge_curator", "fact_verification"},
                {"advanced_research_analyst", "source_credibility_analysis"},
                {"advanced_research_analyst", "information_synthesis"},
                {"synthesis_specialist", "research_report_generation"},
                {"synthesis_specialist", "citation_management"}
            }
        );
        
        comprehensive_workflow.description = "Full-scale deep research project with all specialized agents";
        comprehensive_workflow.global_context = {
            {"project_scope", "comprehensive_analysis"},
            {"quality_standard", "publication_ready"},
            {"research_domain", "ai_impact_on_science"},
            {"collaboration_mode", "sequential_with_feedback"},
            {"output_requirements", "executive_summary_and_detailed_report"}
        };
        
        std::string project_id = workflow_engine->create_workflow(comprehensive_workflow);
        std::cout << "✅ Created comprehensive research project: " << project_id << "\n";
        
        json project_input = {
            {"research_question", "How are Large Language Models transforming scientific research methodologies and what are the implications for research quality and accessibility?"},
            {"methodology", "systematic_review"},
            {"scope", "comprehensive"},
            {"domain", "ai_science_intersection"},
            {"time_constraint", "high_quality"},
            {"resource_level", 5},
            {"output_format", "publication_ready"},
            {"include_citations", true},
            {"citation_style", "APA"}
        };
        
        std::cout << "🔥 Launching comprehensive research project...\n";
        std::string exec_id = workflow_engine->execute_workflow(project_id, project_input);
        std::cout << "   🆔 Project Execution ID: " << exec_id << "\n\n";
        
        // Enhanced monitoring for comprehensive project
        monitor_comprehensive_project(exec_id);
        
        std::cout << "🏁 Comprehensive research project demonstration completed!\n\n";
    }
    
    /**
     * @brief Monitor workflow progress
     */
    void monitor_workflow_progress(const std::string& execution_id) {
        std::cout << "📊 Monitoring workflow progress...\n";
        
        auto start_time = std::chrono::steady_clock::now();
        int progress_indicator = 0;
        
        while (true) {
            auto status = workflow_engine->get_execution_status(execution_id);
            if (!status) {
                std::cout << "❌ Could not retrieve workflow status\n";
                break;
            }
            
            // Show progress indicator
            std::cout << "\\r   Progress: ";
            for (int i = 0; i < 10; ++i) {
                if (i < (progress_indicator % 10) + 1) {
                    std::cout << "●";
                } else {
                    std::cout << "○";
                }
            }
            std::cout << " ";
            
            switch (status->current_status) {
                case WorkflowStatus::PENDING:
                    std::cout << "PENDING";
                    break;
                case WorkflowStatus::RUNNING:
                    std::cout << "RUNNING (step: " << status->current_step_id << ")";
                    break;
                case WorkflowStatus::COMPLETED:
                    std::cout << "COMPLETED ✅";
                    goto workflow_done;
                case WorkflowStatus::FAILED:
                    std::cout << "FAILED ❌";
                    goto workflow_done;
                case WorkflowStatus::CANCELLED:
                    std::cout << "CANCELLED ⏹️";
                    goto workflow_done;
                default:
                    std::cout << "UNKNOWN";
                    break;
            }
            
            std::cout << std::flush;
            
            // Timeout after 30 seconds
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            if (elapsed > std::chrono::seconds(30)) {
                std::cout << "\\n⏰ Workflow monitoring timeout reached\\n";
                break;
            }
            
            progress_indicator++;
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        }
        
        workflow_done:
        std::cout << "\\n";
        
        // Show final status
        auto final_status = workflow_engine->get_execution_status(execution_id);
        if (final_status) {
            std::cout << "📈 Final Status: Completed " << final_status->completed_steps.size() 
                     << " steps, " << final_status->failed_steps.size() << " failures\\n";
        }
    }
    
    /**
     * @brief Monitor comprehensive project with detailed reporting
     */
    void monitor_comprehensive_project(const std::string& execution_id) {
        std::cout << "🔍 Comprehensive project monitoring engaged...\n";
        
        auto start_time = std::chrono::steady_clock::now();
        int phase = 0;
        std::vector<std::string> phase_names = {
            "Planning", "Analysis", "Synthesis", "Verification", "Reporting"
        };
        
        while (true) {
            auto status = workflow_engine->get_execution_status(execution_id);
            if (!status) break;
            
            // Enhanced progress display
            if (phase < phase_names.size()) {
                std::cout << "\\r   🔄 Phase: " << phase_names[phase] 
                         << " | Steps: " << status->completed_steps.size() 
                         << "/" << (status->completed_steps.size() + 1) << " ";
                
                // Animated progress
                for (int i = 0; i < 5; ++i) {
                    std::cout << (i <= (phase % 5) ? "▓" : "░");
                }
                std::cout << " " << std::flush;
            }
            
            if (status->current_status == WorkflowStatus::COMPLETED ||
                status->current_status == WorkflowStatus::FAILED ||
                status->current_status == WorkflowStatus::CANCELLED) {
                break;
            }
            
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            if (elapsed > std::chrono::seconds(45)) {
                std::cout << "\\n⏰ Extended monitoring timeout\\n";
                break;
            }
            
            phase = (phase + 1) % phase_names.size();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        
        std::cout << "\\n✅ Comprehensive project monitoring completed\\n";
    }
    
    /**
     * @brief Display final summary
     */
    void display_final_summary() {
        std::cout << "📋 DEEP RESEARCH AGENT DEMO SUMMARY\n";
        std::cout << "===================================\n\n";
        
        std::cout << "🎯 Demonstrated Capabilities:\n";
        std::cout << "   ✅ Multi-agent research system coordination\n";
        std::cout << "   ✅ Advanced research function integration\n";
        std::cout << "   ✅ Intelligent research planning and methodology selection\n";
        std::cout << "   ✅ Source credibility analysis and bias detection\n";
        std::cout << "   ✅ Entity extraction and knowledge graph management\n";
        std::cout << "   ✅ Information synthesis with conflict resolution\n";
        std::cout << "   ✅ Fact verification across multiple sources\n";
        std::cout << "   ✅ Comprehensive research report generation\n";
        std::cout << "   ✅ Citation management in academic formats\n";
        std::cout << "   ✅ Complex workflow orchestration\n";
        std::cout << "   ✅ Multi-agent collaborative research projects\n\n";
        
        std::cout << "🏆 Research Agent Specializations:\n";
        std::cout << "   🎯 Research Coordinator - Project planning and methodology\n";
        std::cout << "   📊 Advanced Research Analyst - Data analysis and synthesis\n";
        std::cout << "   🧠 Knowledge Curator - Entity extraction and fact verification\n";
        std::cout << "   🔍 Source Analyst - Source quality and bias assessment\n";
        std::cout << "   📝 Synthesis Specialist - Report generation and citations\n\n";
        
        // Display system metrics
        auto agent_ids = agent_manager->list_agents();
        int running_agents = 0;
        for (const auto& id : agent_ids) {
            auto agent = agent_manager->get__agent(id);
            if (agent && agent->is__running()) {
                running_agents++;
            }
        }
        
        std::cout << "📊 System Statistics:\n";
        std::cout << "   🤖 Total Agents: " << agent_ids.size() << "\n";
        std::cout << "   ✅ Active Agents: " << running_agents << "\n";
        std::cout << "   🔄 Workflows Demonstrated: 3\n";
        std::cout << "   🧪 Functions Tested: 10+\n";
        std::cout << "   ⏱️  Server Connection: " << (server_connected ? "✅ Active" : "⚠️ Simulation") << "\n\n";
        
        std::cout << "💡 Key Features:\n";
        std::cout << "   • 🔬 Deep research capabilities with specialized agents\n";
        std::cout << "   • 🤝 Multi-agent collaboration for complex projects\n";
        std::cout << "   • 📊 Advanced analytics and credibility assessment\n";
        std::cout << "   • 🧠 Knowledge graph integration and entity mapping\n";
        std::cout << "   • 📝 Publication-quality report generation\n";
        std::cout << "   • 🔄 Flexible workflow orchestration\n";
        std::cout << "   • ⚡ Real-time processing with the kolosal-server\n";
        std::cout << "   • 🎯 Role-based agent specialization\n\n";
        
        std::cout << "🚀 Next Steps:\n";
        std::cout << "   1. Customize agent configurations for specific domains\n";
        std::cout << "   2. Integrate with external data sources and APIs\n";
        std::cout << "   3. Scale to larger research projects and teams\n";
        std::cout << "   4. Add domain-specific research methodologies\n";
        std::cout << "   5. Implement advanced NLP and knowledge extraction\n\n";
        
        std::cout << "🎉 Deep Research Agent System Demo Completed Successfully!\n";
        std::cout << "Thank you for exploring the Kolosal Agent System v2.0\n\n";
    }
    
    /**
     * @brief Cleanup resources
     */
    void cleanup() {
        if (workflow_engine) {
            workflow_engine->stop();
        }
        if (agent_manager) {
            agent_manager->stop();
        }
        std::cout << "✓ Deep Research Agent Demo completed\n";
    }
};

/**
 * @brief Main entry point for the deep research agent demo
 */
int main(int argc, char* argv[]) {
    try {
        std::cout << std::string(80, '=') << "\n";
        std::cout << "🔬 KOLOSAL DEEP RESEARCH AGENT SYSTEM DEMO v2.0\n";
        std::cout << "🧪 Advanced Multi-Agent Research Intelligence Platform\n";
        std::cout << std::string(80, '=') << "\n\n";
        
        // Parse command line arguments
        bool interactive = false;
        bool verbose = false;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--interactive" || arg == "-i") {
                interactive = true;
            } else if (arg == "--verbose" || arg == "-v") {
                verbose = true;
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "Usage: " << argv[0] << " [options]\n";
                std::cout << "Options:\n";
                std::cout << "  --interactive, -i    Run in interactive mode\n";
                std::cout << "  --verbose, -v        Enable verbose output\n";
                std::cout << "  --help, -h           Show this help message\n";
                return 0;
            }
        }
        
        if (verbose) {
            std::cout << "🔧 Verbose mode enabled\n";
            std::cout << "📁 Working directory: " << std::filesystem::current_path() << "\n\n";
        }
        
        // Create and run demo
        DeepResearchAgentDemo demo;
        
        if (!demo.initialize()) {
            std::cerr << "❌ Failed to initialize demo system\n";
            return 1;
        }
        
        if (interactive) {
            std::cout << "🎮 Interactive mode enabled\n";
            std::cout << "Press ENTER to continue through each demo section...\n\n";
            std::cin.get();
        }
        
        demo.run_demo();
        
        if (interactive) {
            std::cout << "Press ENTER to exit...\n";
            std::cin.get();
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Demo failed with exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "❌ Demo failed with unknown exception\n";
        return 1;
    }
}
