/**
 * @file agent_interfaces.cpp
 * @brief Core functionality for agent interfaces
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "agent/agent_interfaces.hpp"
#include "agent/agent_data.hpp"

namespace kolosal::agents {

AgentMessage::AgentMessage(const std::string& from, const std::string& to, const std::string& msg_type)
    : id(UUIDGenerator::generate()), from_agent(from), to_agent(to), type(msg_type),
      timestamp(std::chrono::system_clock::now()) {}

} // namespace kolosal::agents
