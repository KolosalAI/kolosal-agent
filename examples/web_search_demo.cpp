/**
 * @file web_search_demo.cpp
 * @brief Demonstration of web search and retrieval function integration
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * This demo shows how to use the new kolosal-server integrated functions
 * for real web search and document retrieval capabilities.
 */

#include "agent/core/agent_core.hpp"
#include "tools/enhanced_function_registry.hpp"
#include "tools/kolosal_server_functions.hpp"
#include "execution/function_execution_manager.hpp"
#include "logger/kolosal_logger.hpp"
#include <iostream>
#include <memory>

using namespace kolosal::agents;

/**
 * @brief Demo function showing how to register and use web search functions
 */
void demo_web_search_integration() {
    std::cout << "\\n=== Web Search and Retrieval Demo ===\\n" << std::endl;
    
    // Configure kolosal-server endpoint (adjust as needed)
    std::string server_url = "http://localhost:8080";
    
    // Create an enhanced function registry
    auto enhanced_registry = std::make_shared<EnhancedFunctionRegistry>(server_url);
    
    // Create a function manager
    auto function_manager = std::make_shared<FunctionManager>();
    
    // Test server connection first
    std::cout << "Testing connection to kolosal-server at " << server_url << "..." << std::endl;
    bool connected = enhanced_registry->test_server_connection();
    
    if (connected) {
        std::cout << "✓ Server connection successful!" << std::endl;
        
        // Register all functions including server-integrated ones
        enhanced_registry->register_all_functions(function_manager, true);
        
        std::cout << "✓ Registered enhanced functions with server integration" << std::endl;
        
    } else {
        std::cout << "⚠ Server connection failed. Using built-in functions only." << std::endl;
        
        // Register only built-in functions (simulation mode)
        enhanced_registry->register_builtin_functions(function_manager);
        
        std::cout << "✓ Registered built-in functions (simulation mode)" << std::endl;
    }
    
    // List available functions
    auto available_functions = enhanced_registry->get_available_functions(connected);
    std::cout << "\\nAvailable functions (" << available_functions.size() << "):" << std::endl;
    for (const auto& func_name : available_functions) {
        std::cout << "  - " << func_name << std::endl;
    }
    
    // Demo web search
    std::cout << "\\n--- Web Search Demo ---" << std::endl;
    
    if (function_manager->has__function("internet_search")) {
        std::cout << "Testing real internet search..." << std::endl;
        
        AgentData search_params;
        search_params.set("query", "artificial intelligence recent developments");
        search_params.set("results", 3);
        search_params.set("safe_search", true);
        
        auto search_result = function_manager->execute__function("internet_search", search_params);
        
        if (search_result.success) {
            std::cout << "✓ Internet search successful!" << std::endl;
            std::cout << "Results count: " << search_result.result_data.get_int("results_count", 0) << std::endl;
            std::cout << "First result snippet: " << std::endl;
            auto snippets = search_result.result_data.get_array_string("snippets");
            if (!snippets.empty()) {
                std::cout << "  " << snippets[0].substr(0, 200) << "..." << std::endl;
            }
        } else {
            std::cout << "✗ Internet search failed: " << search_result.error_message << std::endl;
        }
    } else if (function_manager->has__function("web_search")) {
        std::cout << "Using simulated web search..." << std::endl;
        
        AgentData search_params;
        search_params.set("query", "artificial intelligence recent developments");
        search_params.set("limit", 3);
        
        auto search_result = function_manager->execute__function("web_search", search_params);
        
        if (search_result.success) {
            std::cout << "✓ Simulated web search completed!" << std::endl;
            std::cout << "Results: " << search_result.result_data.get_string("result", "No result") << std::endl;
        } else {
            std::cout << "✗ Web search failed: " << search_result.error_message << std::endl;
        }
    }
    
    // Demo document retrieval
    std::cout << "\\n--- Document Retrieval Demo ---" << std::endl;
    
    if (function_manager->has__function("server_document_retrieval")) {
        std::cout << "Testing server document retrieval..." << std::endl;
        
        AgentData retrieval_params;
        retrieval_params.set("query", "machine learning algorithms");
        retrieval_params.set("limit", 3);
        retrieval_params.set("threshold", 0.7);
        
        auto retrieval_result = function_manager->execute__function("server_document_retrieval", retrieval_params);
        
        if (retrieval_result.success) {
            std::cout << "✓ Document retrieval successful!" << std::endl;
            std::cout << "Documents found: " << retrieval_result.result_data.get_int("documents_count", 0) << std::endl;
        } else {
            std::cout << "✗ Document retrieval failed: " << retrieval_result.error_message << std::endl;
        }
    } else if (function_manager->has__function("retrieval")) {
        std::cout << "Using basic retrieval function..." << std::endl;
        
        AgentData retrieval_params;
        retrieval_params.set("query", "machine learning algorithms");
        retrieval_params.set("limit", 3);
        
        auto retrieval_result = function_manager->execute__function("retrieval", retrieval_params);
        
        if (retrieval_result.success) {
            std::cout << "✓ Basic retrieval completed!" << std::endl;
            std::cout << "Result: " << retrieval_result.result_data.get_string("result", "No result") << std::endl;
        } else {
            std::cout << "✗ Retrieval failed: " << retrieval_result.error_message << std::endl;
        }
    }
    
    // Demo hybrid knowledge retrieval
    std::cout << "\\n--- Hybrid Knowledge Retrieval Demo ---" << std::endl;
    
    if (function_manager->has__function("knowledge_retrieval")) {
        std::cout << "Testing hybrid knowledge retrieval..." << std::endl;
        
        AgentData knowledge_params;
        knowledge_params.set("query", "quantum computing applications");
        knowledge_params.set("max_results", 8);
        knowledge_params.set("web_only", false);
        knowledge_params.set("local_only", false);
        
        auto knowledge_result = function_manager->execute__function("knowledge_retrieval", knowledge_params);
        
        if (knowledge_result.success) {
            std::cout << "✓ Hybrid knowledge retrieval successful!" << std::endl;
            std::cout << "Total results: " << knowledge_result.result_data.get_int("total_results", 0) << std::endl;
            std::cout << "Web results: " << knowledge_result.result_data.get_int("web_results_found", 0) << std::endl;
            std::cout << "Local documents: " << knowledge_result.result_data.get_int("local_documents_found", 0) << std::endl;
        } else {
            std::cout << "✗ Hybrid knowledge retrieval failed: " << knowledge_result.error_message << std::endl;
        }
    } else {
        std::cout << "Hybrid knowledge retrieval not available." << std::endl;
    }
    
    std::cout << "\\n=== Demo Complete ===\\n" << std::endl;
}

/**
 * @brief Demo function showing role-based function registration
 */
void demo_role_based_functions() {
    std::cout << "\\n=== Role-Based Function Registration Demo ===\\n" << std::endl;
    
    // Show recommended functions for different roles
    std::vector<std::string> roles = {"researcher", "assistant", "developer", "specialist"};
    
    for (const auto& role : roles) {
        auto recommended = FunctionRegistryUtils::get_recommended_functions_for_role(role);
        
        std::cout << "Recommended functions for " << role << " role:" << std::endl;
        for (const auto& func_name : recommended) {
            std::cout << "  - " << func_name << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Demo selective function registration
    std::cout << "--- Selective Function Registration ---" << std::endl;
    
    auto function_manager = std::make_shared<FunctionManager>();
    
    // Register web search functions only
    FunctionRegistryUtils::register_web_search_functions(function_manager, "http://localhost:8080", true);
    std::cout << "✓ Registered web search functions" << std::endl;
    
    // Register document functions
    FunctionRegistryUtils::register_document_functions(function_manager, "http://localhost:8080", "documents");
    std::cout << "✓ Registered document functions" << std::endl;
    
    // Register knowledge functions
    FunctionRegistryUtils::register_knowledge_functions(function_manager, "http://localhost:8080");
    std::cout << "✓ Registered hybrid knowledge functions" << std::endl;
    
    std::cout << "\\nTotal registered functions: " << function_manager->get_available_functions().size() << std::endl;
}

/**
 * @brief Main demo function
 */
int main() {
    try {
        std::cout << "Kolosal Agent System - Web Search & Retrieval Integration Demo" << std::endl;
        std::cout << "================================================================" << std::endl;
        
        // Initialize logging
        ServerLogger::initialize("console");
        
        // Run demos
        demo_web_search_integration();
        demo_role_based_functions();
        
        std::cout << "All demos completed successfully!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Demo failed with error: " << e.what() << std::endl;
        return 1;
    }
}
