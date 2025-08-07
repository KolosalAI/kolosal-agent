// File: src/server_logger_adapter.cpp
#include "server_logger_adapter.hpp"
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
