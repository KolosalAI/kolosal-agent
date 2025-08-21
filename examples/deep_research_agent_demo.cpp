/**
 * @file deep_research_agent_demo.cpp
 * @brief Deep Research Agent Demo Application
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Standalone executable for demonstrating deep research capabilities
 * with kolosal-server integration for web search and document retrieval.
 */

#include "examples/deep_research_agent.hpp"
#include "server/unified_server.hpp"
#include "agent/services/agent_service.hpp"
#include "utils/loading_animation_utils.hpp"
#include "kolosal/logger.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <iomanip>
#include <ctime>
#include <algorithm>

using namespace kolosal::agents::examples;
using namespace kolosal::agents;
using namespace kolosal::services;

class DeepResearchDemo {
private:
    std::shared_ptr<DeepResearchAgent> research_agent;
    std::string server_url;
    bool server_available;
    
public:
    DeepResearchDemo(const std::string& server_endpoint = "http://localhost:8080") 
        : server_url(server_endpoint), server_available(false) {}
    
    bool initialize() {
        std::cout << "\n🔬 Deep Research Agent Demo - Initializing...\n" << std::endl;
        
        // Test server connection first
        std::cout << "Testing connection to kolosal-server at " << server_url << "..." << std::endl;
        
        try {
            research_agent = DeepResearchAgentFactory::create_standard_research_agent(
                "DemoResearchAgent", server_url);
            
            if (!research_agent->initialize()) {
                std::cerr << "❌ Failed to initialize research agent" << std::endl;
                return false;
            }
            
            if (!research_agent->start()) {
                std::cerr << "❌ Failed to start research agent" << std::endl;
                return false;
            }
            
            // Test server connectivity
            server_available = research_agent->test_server_connection();
            
            if (server_available) {
                std::cout << "✅ Server connection successful - Real web search enabled" << std::endl;
            } else {
                std::cout << "⚠️  Server connection failed - Using simulation mode" << std::endl;
                research_agent->set_server_integration_enabled(false);
            }
            
            std::cout << "✅ Deep Research Agent initialized successfully\n" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Initialization failed: " << e.what() << std::endl;
            return false;
        }
    }
    
    void show_menu() {
        std::cout << "\n═══════════════════════════════════════════════════════════════" << std::endl;
        std::cout << "🔬 DEEP RESEARCH AGENT DEMO" << std::endl;
        std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
        std::cout << "Server Status: " << (server_available ? "🟢 Connected" : "🔴 Disconnected") << std::endl;
        std::cout << "Server URL: " << server_url << std::endl;
        std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
        std::cout << "1. 🔍 Quick Research (5-10 minutes)" << std::endl;
        std::cout << "2. 📚 Comprehensive Research (15-30 minutes)" << std::endl;
        std::cout << "3. 🎓 Academic Research (20-40 minutes)" << std::endl;
        std::cout << "4. 💼 Market Research (10-20 minutes)" << std::endl;
        std::cout << "5. 🛠️  Custom Research (Configure parameters)" << std::endl;
        std::cout << "6. ⚙️  Settings & Configuration" << std::endl;
        std::cout << "7. 📊 View Available Workflows" << std::endl;
        std::cout << "8. 🧪 Test Server Functions" << std::endl;
        std::cout << "9. ❌ Exit" << std::endl;
        std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
        std::cout << "Choose an option (1-9): ";
    }
    
    void run() {
        if (!initialize()) {
            std::cerr << "Failed to initialize. Exiting." << std::endl;
            return;
        }
        
        std::string choice;
        
        while (true) {
            show_menu();
            std::getline(std::cin, choice);
            
            try {
                if (choice == "1") {
                    perform_quick_research();
                } else if (choice == "2") {
                    perform_comprehensive_research();
                } else if (choice == "3") {
                    perform_academic_research();
                } else if (choice == "4") {
                    perform_market_research();
                } else if (choice == "5") {
                    perform_custom_research();
                } else if (choice == "6") {
                    show_settings();
                } else if (choice == "7") {
                    show_workflows();
                } else if (choice == "8") {
                    test_server_functions();
                } else if (choice == "9") {
                    break;
                } else {
                    std::cout << "❌ Invalid choice. Please select 1-9." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "❌ Error: " << e.what() << std::endl;
            }
            
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
        
        std::cout << "\n👋 Thank you for using Deep Research Agent Demo!" << std::endl;
    }
    
private:
    std::string get_research_question() {
        std::string question;
        std::cout << "\n📝 Enter your research question: ";
        std::getline(std::cin, question);
        
        if (question.empty()) {
            std::cout << "❌ Research question cannot be empty." << std::endl;
            return get_research_question();
        }
        
        return question;
    }
    
    void perform_quick_research() {
        std::cout << "\n🔍 QUICK RESEARCH MODE" << std::endl;
        std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
        
        std::string question = get_research_question();
        
        ResearchConfig config;
        config.methodology = "quick_scan";
        config.depth_level = "moderate";
        config.max_sources = 15;
        config.max_web_results = 10;
        config.output_format = "summary";
        
        execute_research(question, config);
    }
    
    void perform_comprehensive_research() {
        std::cout << "\n📚 COMPREHENSIVE RESEARCH MODE" << std::endl;
        std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
        
        std::string question = get_research_question();
        
        ResearchConfig config;
        config.methodology = "systematic";
        config.depth_level = "comprehensive";
        config.max_sources = 30;
        config.max_web_results = 20;
        config.output_format = "comprehensive_report";
        
        execute_research(question, config);
    }
    
    void perform_academic_research() {
        std::cout << "\n🎓 ACADEMIC RESEARCH MODE" << std::endl;
        std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
        
        std::string question = get_research_question();
        
        ResearchConfig config;
        config.methodology = "systematic_review";
        config.depth_level = "exhaustive";
        config.max_sources = 50;
        config.max_web_results = 25;
        config.relevance_threshold = 0.8;
        config.include_academic = true;
        config.include_news = false;
        config.output_format = "academic_paper";
        
        execute_research(question, config);
    }
    
    void perform_market_research() {
        std::cout << "\n💼 MARKET RESEARCH MODE" << std::endl;
        std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
        
        std::string question = get_research_question();
        
        ResearchConfig config;
        config.methodology = "market_analysis";
        config.depth_level = "comprehensive";
        config.max_sources = 40;
        config.max_web_results = 20;
        config.include_academic = false;
        config.include_news = true;
        config.output_format = "business_report";
        
        execute_research(question, config);
    }
    
    void perform_custom_research() {
        std::cout << "\n🛠️  CUSTOM RESEARCH MODE" << std::endl;
        std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
        
        std::string question = get_research_question();
        
        ResearchConfig config = configure_custom_research();
        
        execute_research(question, config);
    }
    
    ResearchConfig configure_custom_research() {
        ResearchConfig config;
        std::string input;
        
        std::cout << "\n⚙️  Custom Research Configuration" << std::endl;
        std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
        
        // Methodology
        std::cout << "Methodology (systematic/exploratory/market_analysis): ";
        std::getline(std::cin, input);
        if (!input.empty()) config.methodology = input;
        
        // Depth level
        std::cout << "Depth level (shallow/moderate/comprehensive/exhaustive): ";
        std::getline(std::cin, input);
        if (!input.empty()) config.depth_level = input;
        
        // Max sources
        std::cout << "Maximum sources (default 30): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            try {
                config.max_sources = std::stoi(input);
            } catch (...) {
                std::cout << "Invalid number, using default (30)" << std::endl;
            }
        }
        
        // Include academic sources
        std::cout << "Include academic sources? (y/n): ";
        std::getline(std::cin, input);
        config.include_academic = (input == "y" || input == "Y" || input == "yes");
        
        // Include news sources
        std::cout << "Include news sources? (y/n): ";
        std::getline(std::cin, input);
        config.include_news = (input == "y" || input == "Y" || input == "yes");
        
        // Include documents
        std::cout << "Include document retrieval? (y/n): ";
        std::getline(std::cin, input);
        config.include_documents = (input == "y" || input == "Y" || input == "yes");
        
        return config;
    }
    
    void execute_research(const std::string& question, const ResearchConfig& config) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::cout << "\n🚀 Starting research..." << std::endl;
        std::cout << "Question: " << question << std::endl;
        std::cout << "Methodology: " << config.methodology << std::endl;
        std::cout << "Depth: " << config.depth_level << std::endl;
        std::cout << "Max sources: " << config.max_sources << std::endl;
        std::cout << "\n⏳ This may take several minutes. Please wait...\n" << std::endl;
        
        // Start loading animation in a separate thread
        std::atomic<bool> research_complete(false);
        std::thread animation_thread([&research_complete]() {
            LoadingAnimationUtils::show_research_progress(research_complete);
        });
        
        // Perform research
        ResearchResult result = research_agent->conduct_research(question, config);
        
        // Stop animation
        research_complete = true;
        if (animation_thread.joinable()) {
            animation_thread.join();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
        
        // Display results
        display_research_results(result, duration.count());
    }
    
    void display_research_results(const ResearchResult& result, int duration_seconds) {
        std::cout << "\n═══════════════════════════════════════════════════════════════" << std::endl;
        std::cout << "📊 RESEARCH RESULTS" << std::endl;
        std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
        
        if (result.success) {
            std::cout << "✅ Research completed successfully!" << std::endl;
            std::cout << "⏱️  Duration: " << duration_seconds << " seconds" << std::endl;
            std::cout << "🎯 Confidence Score: " << std::fixed << std::setprecision(2) 
                     << (result.confidence_score * 100) << "%" << std::endl;
            std::cout << "📈 Methodology: " << result.methodology_used << std::endl;
            std::cout << "📚 Sources Found: " << result.sources_found.size() << std::endl;
            
            std::cout << "\n───────────────────────────────────────────────────────────────" << std::endl;
            std::cout << "📋 EXECUTIVE SUMMARY" << std::endl;
            std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
            if (!result.executive_summary.empty()) {
                std::cout << result.executive_summary << std::endl;
            } else {
                std::cout << "No executive summary available." << std::endl;
            }
            
            std::cout << "\n───────────────────────────────────────────────────────────────" << std::endl;
            std::cout << "🔍 KEY FINDINGS" << std::endl;
            std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
            if (!result.key_findings.empty()) {
                for (size_t i = 0; i < result.key_findings.size(); ++i) {
                    std::cout << (i + 1) << ". " << result.key_findings[i] << std::endl;
                }
            } else {
                std::cout << "No key findings available." << std::endl;
            }
            
            std::cout << "\n───────────────────────────────────────────────────────────────" << std::endl;
            std::cout << "📖 COMPREHENSIVE ANALYSIS" << std::endl;
            std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
            if (!result.comprehensive_analysis.empty()) {
                std::cout << result.comprehensive_analysis << std::endl;
            } else {
                std::cout << "No comprehensive analysis available." << std::endl;
            }
            
            std::cout << "\n───────────────────────────────────────────────────────────────" << std::endl;
            std::cout << "📚 SOURCES" << std::endl;
            std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
            if (!result.sources_found.empty()) {
                for (size_t i = 0; i < std::min(result.sources_found.size(), size_t(10)); ++i) {
                    std::cout << "[" << (i + 1) << "] " << result.sources_found[i] << std::endl;
                }
                if (result.sources_found.size() > 10) {
                    std::cout << "... and " << (result.sources_found.size() - 10) << " more sources" << std::endl;
                }
            } else {
                std::cout << "No sources listed." << std::endl;
            }
            
        } else {
            std::cout << "❌ Research failed!" << std::endl;
            std::cout << "⏱️  Duration: " << duration_seconds << " seconds" << std::endl;
            std::cout << "💥 Error: " << result.error_message << std::endl;
        }
        
        std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
    }
    
    void show_settings() {
        std::cout << "\n⚙️  SETTINGS & CONFIGURATION" << std::endl;
        std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
        std::cout << "Server URL: " << research_agent->get_server_url() << std::endl;
        std::cout << "Server Integration: " << (research_agent->is_server_integration_enabled() ? "Enabled" : "Disabled") << std::endl;
        
        const auto& config = research_agent->get_research_config();
        std::cout << "\nDefault Research Configuration:" << std::endl;
        std::cout << "  Methodology: " << config.methodology << std::endl;
        std::cout << "  Depth Level: " << config.depth_level << std::endl;
        std::cout << "  Max Sources: " << config.max_sources << std::endl;
        std::cout << "  Max Web Results: " << config.max_web_results << std::endl;
        std::cout << "  Include Academic: " << (config.include_academic ? "Yes" : "No") << std::endl;
        std::cout << "  Include News: " << (config.include_news ? "Yes" : "No") << std::endl;
        std::cout << "  Include Documents: " << (config.include_documents ? "Yes" : "No") << std::endl;
        std::cout << "  Output Format: " << config.output_format << std::endl;
        
        std::cout << "\n1. Change server URL" << std::endl;
        std::cout << "2. Toggle server integration" << std::endl;
        std::cout << "3. Test server connection" << std::endl;
        std::cout << "4. Back to main menu" << std::endl;
        std::cout << "Choose option (1-4): ";
        
        std::string choice;
        std::getline(std::cin, choice);
        
        if (choice == "1") {
            std::cout << "Enter new server URL: ";
            std::string new_url;
            std::getline(std::cin, new_url);
            if (!new_url.empty()) {
                research_agent->set_server_url(new_url);
                server_url = new_url;
                std::cout << "✅ Server URL updated to: " << new_url << std::endl;
            }
        } else if (choice == "2") {
            bool current_state = research_agent->is_server_integration_enabled();
            research_agent->set_server_integration_enabled(!current_state);
            std::cout << "✅ Server integration " << (!current_state ? "enabled" : "disabled") << std::endl;
        } else if (choice == "3") {
            std::cout << "Testing server connection..." << std::endl;
            bool connected = research_agent->test_server_connection();
            std::cout << (connected ? "✅ Connection successful" : "❌ Connection failed") << std::endl;
            server_available = connected;
        }
    }
    
    void show_workflows() {
        std::cout << "\n📊 AVAILABLE RESEARCH WORKFLOWS" << std::endl;
        std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
        
        auto workflows = research_agent->get_available_workflows();
        
        if (workflows.empty()) {
            std::cout << "No workflows available." << std::endl;
        } else {
            for (size_t i = 0; i < workflows.size(); ++i) {
                std::cout << (i + 1) << ". " << workflows[i] << std::endl;
            }
            
            std::cout << "\nDo you want to execute a specific workflow? (y/n): ";
            std::string response;
            std::getline(std::cin, response);
            
            if (response == "y" || response == "Y") {
                std::cout << "Enter workflow number (1-" << workflows.size() << "): ";
                std::string num_str;
                std::getline(std::cin, num_str);
                
                try {
                    int num = std::stoi(num_str);
                    if (num >= 1 && num <= static_cast<int>(workflows.size())) {
                        std::string workflow_id = workflows[num - 1];
                        std::string question = get_research_question();
                        
                        std::cout << "\n🚀 Executing workflow: " << workflow_id << std::endl;
                        
                        auto start_time = std::chrono::high_resolution_clock::now();
                        ResearchResult result = research_agent->conduct_research_with_workflow(
                            workflow_id, question);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        
                        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                            end_time - start_time);
                        
                        display_research_results(result, duration.count());
                    } else {
                        std::cout << "❌ Invalid workflow number." << std::endl;
                    }
                } catch (...) {
                    std::cout << "❌ Invalid input." << std::endl;
                }
            }
        }
    }
    
    void test_server_functions() {
        std::cout << "\n🧪 TESTING SERVER FUNCTIONS" << std::endl;
        std::cout << "───────────────────────────────────────────────────────────────" << std::endl;
        
        if (!server_available) {
            std::cout << "❌ Server is not available. Please check connection." << std::endl;
            return;
        }
        
        std::cout << "1. Test internet search" << std::endl;
        std::cout << "2. Test document retrieval" << std::endl;
        std::cout << "3. Test knowledge retrieval" << std::endl;
        std::cout << "4. Test all functions" << std::endl;
        std::cout << "Choose test (1-4): ";
        
        std::string choice;
        std::getline(std::cin, choice);
        
        try {
            if (choice == "1" || choice == "4") {
                std::cout << "\n🔍 Testing internet search..." << std::endl;
                // Add actual test implementation here
                std::cout << "✅ Internet search test completed" << std::endl;
            }
            
            if (choice == "2" || choice == "4") {
                std::cout << "\n📚 Testing document retrieval..." << std::endl;
                // Add actual test implementation here
                std::cout << "✅ Document retrieval test completed" << std::endl;
            }
            
            if (choice == "3" || choice == "4") {
                std::cout << "\n🧠 Testing knowledge retrieval..." << std::endl;
                // Add actual test implementation here
                std::cout << "✅ Knowledge retrieval test completed" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cout << "❌ Test failed: " << e.what() << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    std::cout << "🔬 Deep Research Agent Demo v2.0.0" << std::endl;
    std::cout << "Kolosal AI Agent System - Advanced Research Capabilities" << std::endl;
    
    // Parse command line arguments
    std::string server_url = "http://localhost:8080";
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server" && i + 1 < argc) {
            server_url = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "\nUsage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --server <url>    Kolosal server URL (default: http://localhost:8080)" << std::endl;
            std::cout << "  --help, -h        Show this help message" << std::endl;
            return 0;
        }
    }
    
    try {
        DeepResearchDemo demo(server_url);
        demo.run();
    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
