/**
 * @file system_event_manager.cpp
 * @brief System-wide event handling and distribution
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "tools/system_event_manager.hpp"
#include "logger/server_logger_integration.hpp"
#include <algorithm>

namespace kolosal::agents {

EventSystem::EventSystem(std::shared_ptr<Logger> log) : logger(log) {}

void EventSystem::start() {
    running.store(true);
    logger->info("Event system started");
}

void EventSystem::stop() {
    running.store(false);
    logger->info("Event system stopped");
}

void EventSystem::emit(const std::string& event_type, const std::string& source, 
                      const AgentData& data) {
    if (!running.load()) return;
    
    std::lock_guard<std::mutex> lock(handlers_mutex);
    const auto it = handlers.find(event_type);
    if (it != handlers.end()) {
        AgentEvent event(event_type, source);
        event.data = data;
        
        for (auto& handler : it->second) {
            try {
                handler->handle_event(event);
            } catch (const std::exception& e) {
                logger->error("Event handler error: " + std::string(e.what()));
            }
        }
        
        logger->debug("Event emitted: " + event_type + " from " + source + 
                     " (" + std::to_string(it->second.size()) + " handlers)");
    } else {
        logger->debug("Event emitted with no handlers: " + event_type + " from " + source);
    }
}

void EventSystem::subscribe(const std::string& event_type, std::shared_ptr<EventHandler> handler) {
    std::lock_guard<std::mutex> lock(handlers_mutex);
    handlers[event_type].push_back(handler);
    logger->debug("Handler subscribed to event type: " + event_type);
}

void EventSystem::unsubscribe(const std::string& event_type, std::shared_ptr<EventHandler> handler) {
    std::lock_guard<std::mutex> lock(handlers_mutex);
    auto it = handlers.find(event_type);
    if (it != handlers.end()) {
        auto& handler_list = it->second;
        handler_list.erase(
            std::remove_if(handler_list.begin(), handler_list.end(),
                          [&handler](const std::shared_ptr<EventHandler>& h) {
                              return h == handler;
                          }),
            handler_list.end());
        
        if (handler_list.empty()) {
            handlers.erase(it);
        }
        
        logger->debug("Handler unsubscribed from event type: " + event_type);
    }
}

} // namespace kolosal::agents
