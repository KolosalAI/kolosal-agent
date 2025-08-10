/**
 * @file server_logger_integration.hpp
 * @brief Core functionality for server logger integration
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_SERVER_LOGGER_INTEGRATION_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_SERVER_LOGGER_INTEGRATION_HPP_INCLUDED

#include "export.hpp"
#include <string>
#include <memory>

// Forward declare ServerLogger from global namespace
/**
 * @brief Represents server logger functionality
 */
class ServerLogger;

namespace kolosal::agents {

/**
 * @brief Logger interface for agent system (moved from removed logger.hpp)
 */
class Logger {
public:
    virtual ~Logger() = default;
    virtual void debug(const std::string& message) = 0;
    virtual void info(const std::string& message) = 0;
    virtual void warn(const std::string& message) = 0;
    virtual void error(const std::string& message) = 0;
};

/**
 * Adapter class that wraps ServerLogger to implement the agents::Logger interface
 * This bridges the gap between the singleton ServerLogger and the agents system
 */
class KOLOSAL_AGENT_API ServerLoggerAdapter : public Logger {
public:
    ServerLoggerAdapter();
    virtual ~ServerLoggerAdapter() = default;
    
    // Implement the agents::Logger interface
    void debug(const std::string& message) override;
    void info(const std::string& message) override;
    void warn(const std::string& message) override;
    void error(const std::string& message) override;
private:
    // Reference to singleton - we don't own it (global namespace ServerLogger)
    ::ServerLogger* serverLogger;
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_SERVER_LOGGER_INTEGRATION_HPP_INCLUDED
