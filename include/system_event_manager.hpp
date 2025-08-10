/**
 * @file system_event_manager.hpp
 * @brief System-wide event handling and distribution
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_SYSTEM_EVENT_MANAGER_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_SYSTEM_EVENT_MANAGER_HPP_INCLUDED

#include "export.hpp"
#include "agent/agent_interfaces.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

namespace kolosal::agents {

// Forward declaration
/**
 * @brief Represents logger functionality
 */
class Logger;

/**
 * @brief Manages events and event handlers in the agent system
 */
class KOLOSAL_SERVER_API EventSystem {
private:
    std::unordered_map<std::string, std::vector<std::shared_ptr<EventHandler>>> handlers;
    std::shared_ptr<Logger> logger;
    std::atomic<bool> running {false};
    mutable std::mutex handlers_mutex;

public:
    EventSystem(std::shared_ptr<Logger> log);

    void start();
    void stop();

    void emit(const std::string& event_type, const std::string& source, 
             const AgentData& data = AgentData());
    void subscribe(const std::string& event_type, std::shared_ptr<EventHandler> handler);
    void unsubscribe(const std::string& event_type, std::shared_ptr<EventHandler> handler);
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_SYSTEM_EVENT_MANAGER_HPP_INCLUDED
