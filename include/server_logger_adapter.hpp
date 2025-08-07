// File: include/server_logger_adapter.hpp
#pragma once

#include "export.hpp"
#include <string>
#include <memory>

// Forward declare ServerLogger from global namespace
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
