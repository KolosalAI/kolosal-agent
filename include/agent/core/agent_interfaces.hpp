/**
 * @file agent_interfaces.hpp
 * @brief Core functionality for agent interfaces
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_AGENT_AGENT_INTERFACES_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_AGENT_AGENT_INTERFACES_HPP_INCLUDED

#include "../../export.hpp"
#include "agent_data.hpp"
#include <string>
#include <vector>
#include <chrono>

namespace kolosal::agents {

/**
 * @brief Result of function execution
 */
struct KOLOSAL_SERVER_API FunctionResult {
    bool success = false;
    std::string error_message;
    AgentData result_data;
    double execution_time_ms = 0.0;
    std::string llm_response; // For LLM-based functions

    FunctionResult() = default;
    FunctionResult(bool ok) : success(ok) {}
    FunctionResult(bool ok, const std::string& error) : success(ok), error_message(error) {}
};

/**
 * @brief Base class for all agent functions
 */
class KOLOSAL_SERVER_API AgentFunction {
public:
    virtual ~AgentFunction() = default;
    virtual std::string get__name() const = 0;
    virtual std::string get__description() const = 0;
    virtual std::string get__type() const { return "builtin"; }
    virtual FunctionResult execute(const AgentData& parameters) = 0;
};

/**
 * @brief Message for inter-agent communication
 */
struct KOLOSAL_SERVER_API AgentMessage {
    std::string id;
    std::string from_agent;
    std::string to_agent;
    std::string type;
    AgentData payload;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    int priority = 0;
    std::string correlation_id;

    AgentMessage(const std::string& from, const std::string& to, const std::string& msg_type);
};

/**
 * @brief Event for agent system notifications
 */
struct KOLOSAL_SERVER_API AgentEvent {
    std::string type;
    std::string source;
    AgentData data;
    
    AgentEvent(const std::string& event_type, const std::string& event_source)
        : type(event_type), source(event_source) {}
};

/**
 * @brief Base class for event handlers
 */
class KOLOSAL_SERVER_API EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void handle_event(const AgentEvent& event) = 0;
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_AGENT_AGENT_INTERFACES_HPP_INCLUDED
