/**
 * @file function_execution_manager.hpp
 * @brief Function execution and lifecycle management
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */


#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_FUNCTION_EXECUTION_MANAGER_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_FUNCTION_EXECUTION_MANAGER_HPP_INCLUDED

#include "export.hpp"
#include "agent/core/agent_interfaces.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>

namespace kolosal::agents {

// Forward declaration
/**
 * @brief Represents logger functionality
 */
class Logger;

/**
 * @brief Manages agent functions and their execution
 */
class KOLOSAL_SERVER_API FunctionManager {
private:
    std::unordered_map<std::string, std::unique_ptr<AgentFunction>> functions;
    std::shared_ptr<Logger> logger;
    mutable std::mutex functions_mutex;

public:
    FunctionManager(std::shared_ptr<Logger> log);

    bool register_function(std::unique_ptr<AgentFunction> function);
    FunctionResult execute_function(const std::string& name, const AgentData& parameters);
    std::vector<std::string> get__function_names() const;
    bool has__function(const std::string& name) const;
    std::string get__function_description(const std::string& name) const;
    
    // Enhanced tool management
    std::string get__available_tools_summary() const;
    std::vector<std::pair<std::string, std::string>> get__all_functions_with_descriptions() const;
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_FUNCTION_EXECUTION_MANAGER_HPP_INCLUDED
