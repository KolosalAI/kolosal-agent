/**
 * @file function_execution_manager.cpp
 * @brief Function execution and lifecycle management
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "function_execution_manager.hpp"
#include "kolosal/logger.hpp"
#include <chrono>
#include <sstream>

namespace kolosal::agents {

// Bridge Logger interface to ServerLogger
/**
 * @brief Represents logger functionality
 */
class Logger {
public:
    void info(const std::string& message) { ServerLogger::logInfo("%s", message.c_str()); }
    void debug(const std::string& message) { ServerLogger::logDebug("%s", message.c_str()); }
    void warn(const std::string& message) { ServerLogger::logWarning("%s", message.c_str()); }
    void error(const std::string& message) { ServerLogger::logError("%s", message.c_str()); }
};

FunctionManager::FunctionManager(std::shared_ptr<Logger> log) : logger(std::move(log)) {}

bool FunctionManager::register_function(std::unique_ptr<AgentFunction> function) {
    std::lock_guard<std::mutex> lock(functions_mutex);
    const std::string name = function->get__name();
    functions[name] = std::move(function);
    logger->info("Registered function: " + name);
    return true;
}

FunctionResult FunctionManager::execute_function(const std::string& name, const AgentData& parameters) {
    std::lock_guard<std::mutex> lock(functions_mutex);
    const auto it = functions.find(name);
    if (it == functions.end()) {
        return FunctionResult(false, "Function not found: " + name);
    }

    const auto start_time = std::chrono::high_resolution_clock::now();
    FunctionResult result = it->second->execute(parameters);
    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    if (result.execution_time_ms == 0.0) {
        result.execution_time_ms = duration.count() / 1000.0;
    }

    logger->debug("Function '" + name + "' executed in " + std::to_string(result.execution_time_ms) + "ms");
    return result;
}

std::vector<std::string> FunctionManager::get__function_names() const {
    std::lock_guard<std::mutex> lock(functions_mutex);
    std::vector<std::string> names;
    for (const auto& pair : functions) {
        names.push_back(pair.first);
    }
    return names;
}

bool FunctionManager::has__function(const std::string& name) const {
    std::lock_guard<std::mutex> lock(functions_mutex);
    return functions.find(name) != functions.end();
}

std::string FunctionManager::get__function_description(const std::string& name) const {
    std::lock_guard<std::mutex> lock(functions_mutex);
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second->get__description();
    }
    return "";
}

std::string FunctionManager::get__available_tools_summary() const {
    std::lock_guard<std::mutex> lock(functions_mutex);
    std::ostringstream summary;
    summary << "Available Tools/Functions (" << functions.size() << " total):\n";
    
    for (const auto& [name, function] : functions) {
        summary << "- " << name << " (" << function->get__type() << "): " << function->get__description() << "\n";
    }
    
    return summary.str();
}

std::vector<std::pair<std::string, std::string>> FunctionManager::get__all_functions_with_descriptions() const {
    std::lock_guard<std::mutex> lock(functions_mutex);
    std::vector<std::pair<std::string, std::string>> result;
    
    for (const auto& [name, function] : functions) {
        result.emplace_back(name, function->get__description());
    }
    
    return result;
}

} // namespace kolosal::agents
