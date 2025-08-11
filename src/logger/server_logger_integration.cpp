/**
 * @file server_logger_integration.cpp
 * @brief Core functionality for server logger integration
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "logger/server_logger_integration.hpp"
#include "kolosal/logger.hpp"

namespace kolosal::agents {

ServerLoggerAdapter::ServerLoggerAdapter() 
    : serverLogger(&::ServerLogger::instance()) {}

void ServerLoggerAdapter::debug(const std::string& message) {
    serverLogger->debug(message);
}

void ServerLoggerAdapter::info(const std::string& message) {
    serverLogger->info(message);
}

void ServerLoggerAdapter::warn(const std::string& message) {
    serverLogger->warning(message);  // Map warn -> warning
}

void ServerLoggerAdapter::error(const std::string& message) {
    serverLogger->error(message);
}

} // namespace kolosal::agents
