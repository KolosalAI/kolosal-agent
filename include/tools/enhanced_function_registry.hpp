/**
 * @file enhanced_function_registry.hpp
 * @brief Enhanced function registry with kolosal-server integration
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for enhanced function registration that includes both
 * built-in functions and kolosal-server integrated functions.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_ENHANCED_FUNCTION_REGISTRY_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_ENHANCED_FUNCTION_REGISTRY_HPP_INCLUDED

#include "export.hpp"
#include "tools/builtin_function_registry.hpp"
#include "tools/kolosal_server_functions.hpp"
#include "execution/function_execution_manager.hpp"
#include <memory>
#include <string>
#include <vector>

namespace kolosal::agents {

// Forward declarations
class FunctionManager;

/**
 * @brief Enhanced function registry that combines built-in and server-integrated functions
 * 
 * This registry provides a unified interface for registering and managing both
 * local built-in functions and functions that integrate with kolosal-server endpoints.
 */
class KOLOSAL_SERVER_API EnhancedFunctionRegistry {
private:
    std::shared_ptr<KolosalServerFunctionRegistry> server_registry;
    std::string server_url;
    bool server_functions_enabled;
    
public:
    EnhancedFunctionRegistry(const std::string& server_endpoint = "http://localhost:8080");
    ~EnhancedFunctionRegistry() = default;
    
    /**
     * @brief Register all built-in functions with a function manager
     * @param manager The function execution manager to register functions with
     */
    void register_builtin_functions(std::shared_ptr<FunctionManager> manager);
    
    /**
     * @brief Register all kolosal-server integrated functions with a function manager
     * @param manager The function execution manager to register functions with
     */
    void register_server_functions(std::shared_ptr<FunctionManager> manager);
    
    /**
     * @brief Register all functions (built-in and server-integrated) with a function manager
     * @param manager The function execution manager to register functions with
     * @param include_server_functions Whether to include server-integrated functions
     */
    void register_all_functions(std::shared_ptr<FunctionManager> manager, 
                               bool include_server_functions = true);
    
    /**
     * @brief Enable or disable server-integrated functions
     * @param enabled Whether to enable server functions
     */
    void set_server_functions_enabled(bool enabled) { server_functions_enabled = enabled; }
    
    /**
     * @brief Check if server functions are enabled
     * @return True if server functions are enabled
     */
    bool are_server_functions_enabled() const { return server_functions_enabled; }
    
    /**
     * @brief Set the kolosal-server URL
     * @param url The server URL to connect to
     */
    void set_server_url(const std::string& url);
    
    /**
     * @brief Get the current server URL
     * @return The server URL
     */
    const std::string& get_server_url() const { return server_url; }
    
    /**
     * @brief Get a list of all available function names
     * @param include_server_functions Whether to include server function names
     * @return Vector of function names
     */
    std::vector<std::string> get_available_functions(bool include_server_functions = true) const;
    
    /**
     * @brief Test connection to kolosal-server
     * @return True if connection is successful
     */
    bool test_server_connection();
};

/**
 * @brief Utility functions for function registration
 */
namespace FunctionRegistryUtils {
    
    /**
     * @brief Register web search and retrieval functions with an agent
     * @param manager The function execution manager
     * @param server_url The kolosal-server URL (optional)
     * @param enable_real_search Whether to enable real internet search (vs simulation)
     */
    void register_web_search_functions(std::shared_ptr<FunctionManager> manager,
                                      const std::string& server_url = "http://localhost:8080",
                                      bool enable_real_search = true);
    
    /**
     * @brief Register document retrieval functions with an agent
     * @param manager The function execution manager
     * @param server_url The kolosal-server URL (optional)
     * @param collection_name Default collection name for document operations
     */
    void register_document_functions(std::shared_ptr<FunctionManager> manager,
                                    const std::string& server_url = "http://localhost:8080",
                                    const std::string& collection_name = "documents");
    
    /**
     * @brief Register hybrid knowledge functions with an agent
     * @param manager The function execution manager
     * @param server_url The kolosal-server URL (optional)
     */
    void register_knowledge_functions(std::shared_ptr<FunctionManager> manager,
                                     const std::string& server_url = "http://localhost:8080");
    
    /**
     * @brief Get the recommended function set for different agent roles
     * @param role The agent role
     * @return Vector of recommended function names
     */
    std::vector<std::string> get_recommended_functions_for_role(const std::string& role);
}

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_ENHANCED_FUNCTION_REGISTRY_HPP_INCLUDED
