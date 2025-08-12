/**
 * @file mock_agent_components.hpp
 * @brief Mock objects for agent components testing
 */

#pragma once

#include <gmock/gmock.h>
#include "agent/core/agent_interfaces.hpp"
#include "logger/server_logger_integration.hpp"

namespace kolosal::agents::test {

using namespace kolosal::agents;

/**
 * @brief Mock Logger for testing
 */
class MockLogger : public Logger {
public:
    MOCK_METHOD(void, debug, (const std::string& message), (override));
    MOCK_METHOD(void, info, (const std::string& message), (override));
    MOCK_METHOD(void, warn, (const std::string& message), (override));
    MOCK_METHOD(void, error, (const std::string& message), (override));
};

// TODO: Add more mock classes when interfaces are stabilized
// For now, we'll focus on basic functionality

} // namespace kolosal::agents::test
