/**
 * @file message_router.hpp
 * @brief Core functionality for message router
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_ROUTING_MESSAGE_ROUTER_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_ROUTING_MESSAGE_ROUTER_HPP_INCLUDED

#include "../export.hpp"
#include "../agent/agent_interfaces.hpp"
#include <queue>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

namespace kolosal::agents {

// Forward declaration
/**
 * @brief Represents logger functionality
 */
class Logger;

/**
 * @brief Routes messages between agents
 */
class KOLOSAL_SERVER_API MessageRouter {
private:
    std::queue<AgentMessage> message_queue;
    std::unordered_map<std::string, std::function<void(const AgentMessage&)>> message_handlers;
    mutable std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::atomic<bool> running {false};
    std::thread router_thread;
    std::shared_ptr<Logger> logger;

public:
    MessageRouter(std::shared_ptr<Logger> log);
    ~MessageRouter();

    void start();
    void stop();

    void register_agent_handler(const std::string& agent_id, 
                               std::function<void(const AgentMessage&)> handler);
    void unregister_agent_handler(const std::string& agent_id);
    void route_message(const AgentMessage& message);
    void broadcast_message(const AgentMessage& message);

private:
    void routing_loop();
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_ROUTING_MESSAGE_ROUTER_HPP_INCLUDED
