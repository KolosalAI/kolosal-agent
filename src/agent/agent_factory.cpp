/**
 * @file agent_factory.cpp
 * @brief Core functionality for agent factory
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "agent/agent_factory.hpp"
#include "system_tool_registry.hpp"

namespace kolosal::agents {

std::shared_ptr<AgentCore> AgentFactory::create__researcher_agent(const std::string& name) {
    const std::string agent_name = name.empty() ? "Researcher" : name;
    const auto agent = std::make_shared<AgentCore>(agent_name, "researcher", AgentRole::RESEARCHER);
    agent->add_specialization(AgentSpecialization::WEB_RESEARCH);
    agent->add_specialization(AgentSpecialization::DOCUMENT_ANALYSIS);
    
    configure_agent_for_role(agent, AgentRole::RESEARCHER);
    return agent;
}

std::shared_ptr<AgentCore> AgentFactory::create__analyst_agent(const std::string& name) {
    const std::string agent_name = name.empty() ? "Analyst" : name;
    const auto agent = std::make_shared<AgentCore>(agent_name, "analyst", AgentRole::ANALYST);
    agent->add_specialization(AgentSpecialization::DATA_ANALYSIS);
    agent->add_specialization(AgentSpecialization::REASONING);
    
    configure_agent_for_role(agent, AgentRole::ANALYST);
    return agent;
}

std::shared_ptr<AgentCore> AgentFactory::create__writer_agent(const std::string& name) {
    const std::string agent_name = name.empty() ? "Writer" : name;
    const auto agent = std::make_shared<AgentCore>(agent_name, "writer", AgentRole::WRITER);
    agent->add_specialization(AgentSpecialization::TEXT_PROCESSING);
    agent->add_specialization(AgentSpecialization::CODE_GENERATION);
    
    configure_agent_for_role(agent, AgentRole::WRITER);
    return agent;
}

std::shared_ptr<AgentCore> AgentFactory::create__critic_agent(const std::string& name) {
    const std::string agent_name = name.empty() ? "Critic" : name;
    const auto agent = std::make_shared<AgentCore>(agent_name, "critic", AgentRole::CRITIC);
    agent->add_specialization(AgentSpecialization::REASONING);
    
    configure_agent_for_role(agent, AgentRole::CRITIC);
    return agent;
}

std::shared_ptr<AgentCore> AgentFactory::create__coordinator_agent(const std::string& name) {
    const std::string agent_name = name.empty() ? "Coordinator" : name;
    const auto agent = std::make_shared<AgentCore>(agent_name, "coordinator", AgentRole::COORDINATOR);
    agent->add_specialization(AgentSpecialization::PLANNING);
    agent->add_specialization(AgentSpecialization::EXECUTION);
    
    configure_agent_for_role(agent, AgentRole::COORDINATOR);
    return agent;
}

std::shared_ptr<AgentCore> AgentFactory::create__agent(const AgentConfiguration& config) {
    const auto agent = std::make_shared<AgentCore>(config.name, config.type, config.role);
    // Add specializations
    for (auto spec : config.specializations) {
        agent->add_specialization(spec);
    }
    
    // Add custom capabilities
    for (const auto& capability : config.custom_capabilities) {
        agent->add_capability(capability);
    }
    
    configure_agent_for_role(agent, config.role);
    return agent;
}

std::shared_ptr<AgentCore> AgentFactory::create__document_processing_agent(const std::string& name) {
    AgentConfiguration config(name.empty() ? "DocumentProcessor" : name, AgentRole::SPECIALIST);
    config.specializations = {AgentSpecialization::DOCUMENT_ANALYSIS, AgentSpecialization::TEXT_PROCESSING};
    config.custom_capabilities = {"pdf_parsing", "document_extraction", "content_analysis"};
    
    return create__agent(config);
}

std::shared_ptr<AgentCore> AgentFactory::create__web_research_agent(const std::string& name) {
    AgentConfiguration config(name.empty() ? "WebResearcher" : name, AgentRole::RESEARCHER);
    config.specializations = {AgentSpecialization::WEB_RESEARCH, AgentSpecialization::DATA_ANALYSIS};
    config.custom_capabilities = {"web_scraping", "search_optimization", "source_validation"};
    
    return create__agent(config);
}

std::shared_ptr<AgentCore> AgentFactory::create__code_generation_agent(const std::string& name) {
    AgentConfiguration config(name.empty() ? "CodeGenerator" : name, AgentRole::WRITER);
    config.specializations = {AgentSpecialization::CODE_GENERATION, AgentSpecialization::REASONING};
    config.custom_capabilities = {"code_analysis", "refactoring", "testing", "documentation"};
    
    return create__agent(config);
}

std::shared_ptr<AgentCore> AgentFactory::create__data_analysis_agent(const std::string& name) {
    AgentConfiguration config(name.empty() ? "DataAnalyst" : name, AgentRole::ANALYST);
    config.specializations = {AgentSpecialization::DATA_ANALYSIS, AgentSpecialization::REASONING};
    config.custom_capabilities = {"statistical_analysis", "data_visualization", "pattern_recognition"};
    
    return create__agent(config);
}

std::vector<std::shared_ptr<AgentCore>> AgentFactory::create__research_team() {
    std::vector<std::shared_ptr<AgentCore>> team;
    
    team.emplace_back(create__researcher_agent("Researcher-1"));
    team.emplace_back(create__analyst_agent("Analyst-1"));
    team.emplace_back(create__writer_agent("Writer-1"));
    team.emplace_back(create__critic_agent("Critic-1"));
    team.emplace_back(create__coordinator_agent("Coordinator-1"));
    
    return team;
}

std::vector<std::shared_ptr<AgentCore>> AgentFactory::create__content_creation_team() {
    std::vector<std::shared_ptr<AgentCore>> team;
    
    team.emplace_back(create__writer_agent("Writer-Lead"));
    team.emplace_back(create__researcher_agent("Researcher-1"));
    team.emplace_back(create__analyst_agent("Analyst-1"));
    team.emplace_back(create__critic_agent("Editor"));
    team.emplace_back(create__coordinator_agent("Coordinator"));
    
    return team;
}

std::vector<std::shared_ptr<AgentCore>> AgentFactory::create__analysis_team() {
    std::vector<std::shared_ptr<AgentCore>> team;
    
    team.emplace_back(create__analyst_agent("Analyst-Lead"));
    team.emplace_back(create__researcher_agent("Researcher-1"));
    team.emplace_back(create__critic_agent("Reviewer"));
    team.emplace_back(create__coordinator_agent("Coordinator"));
    
    return team;
}

void AgentFactory::configure_agent_for_role(std::shared_ptr<AgentCore> agent, AgentRole role) {
    // Add role-specific tools and configurations
    add_role_specific_tools(agent, role);
    
    // Configure memory settings based on role
    if (const auto memory_manager = agent->get__memory_manager()) {
        // Different roles might have different memory requirements
        switch (role) {
            case AgentRole::RESEARCHER:
                // Researchers need more long-term memory
                break;
            case AgentRole::ANALYST:
                // Analysts need working memory for calculations
                break;
            case AgentRole::WRITER:
                // Writers need conversation memory for context
                break;
            default:
                break;
        }
    }
}

void AgentFactory::add_role_specific_tools(std::shared_ptr<AgentCore> agent, AgentRole role) {
    const auto tool_registry = agent->get__tool_registry();
    if (!tool_registry) return;
    
    // Add role-specific custom tools here
    // This is where you would register specialized tools for each role
    
    switch (role) {
        case AgentRole::RESEARCHER:
            // Add research-specific tools
            break;
        case AgentRole::ANALYST:
            // Add analysis-specific tools
            break;
        case AgentRole::WRITER:
            // Add writing-specific tools
            break;
        case AgentRole::CRITIC:
            // Add evaluation-specific tools
            break;
        default:
            break;
    }
}

} // namespace kolosal::agents
