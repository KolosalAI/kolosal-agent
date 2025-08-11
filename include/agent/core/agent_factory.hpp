/**
 * @file agent_factory.hpp
 * @brief Core functionality for agent factory
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_AGENT_AGENT_FACTORY_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_AGENT_AGENT_FACTORY_HPP_INCLUDED

#include "../../export.hpp"
#include "agent_core.hpp"
#include "agent_roles.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace kolosal::agents {

/**
 * @brief Configuration for creating specialized agents
 */
struct KOLOSAL_SERVER_API AgentConfiguration {
    std::string name;
    std::string type;
    AgentRole role;
    std::vector<AgentSpecialization> specializations;
    std::vector<std::string> custom_capabilities;
    std::unordered_map<std::string, std::string> metadata;
    
    AgentConfiguration(const std::string& agent_name = "", 
                       const AgentRole agent_role = AgentRole::GENERIC)
        : name(agent_name), type("generic"), role(agent_role) {}
};
/**
 * @brief Factory for creating and configurationuring agents
 */
class KOLOSAL_SERVER_API AgentFactory {
public:
    // Predefined agent creation methods
    static std::shared_ptr<AgentCore> create__researcher_agent(const std::string& name = "");
    static std::shared_ptr<AgentCore> create__analyst_agent(const std::string& name = "");
    static std::shared_ptr<AgentCore> create__writer_agent(const std::string& name = "");
    static std::shared_ptr<AgentCore> create__critic_agent(const std::string& name = "");
    static std::shared_ptr<AgentCore> create__coordinator_agent(const std::string& name = "");
    
    // Generic agent creation
    static std::shared_ptr<AgentCore> create__agent(const AgentConfiguration& config);
    
    // Specialized agent creation
    static std::shared_ptr<AgentCore> create__document_processing_agent(const std::string& name = "");
    static std::shared_ptr<AgentCore> create__web_research_agent(const std::string& name = "");
    static std::shared_ptr<AgentCore> create__code_generation_agent(const std::string& name = "");
    static std::shared_ptr<AgentCore> create__data_analysis_agent(const std::string& name = "");
    
    // Agent team creation
    static std::vector<std::shared_ptr<AgentCore>> create__research_team();
    static std::vector<std::shared_ptr<AgentCore>> create__content_creation_team();
    static std::vector<std::shared_ptr<AgentCore>> create__analysis_team();

private:
    static void configure_agent_for_role(std::shared_ptr<AgentCore> agent, AgentRole role);
    static void add_role_specific_tools(std::shared_ptr<AgentCore> agent, AgentRole role);
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_AGENT_AGENT_FACTORY_HPP_INCLUDED
