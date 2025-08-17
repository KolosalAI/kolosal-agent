/**
 * @file multi_agent_system.cpp
 * @brief Multi-agent coordination and orchestringingingingation
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "agent/core/multi_agent_system.hpp"
#include "tools/builtin_function_registry.hpp"
#include "tools/research_functions.hpp"
#include "logger/server_logger_integration.hpp"
#include "agent/core/agent_core.hpp"
#include "api/message_router.hpp"
#include "config/yaml_configuration_parser.hpp"
#include "agent/core/agent_config_validator.hpp"
#include <mutex>
#include <chrono>
#include <thread>
#include <sstream>
#include <algorithm>

namespace kolosal::agents {

// ConfigurableAgentFactory implementation
ConfigurableAgentFactory::ConfigurableAgentFactory(std::shared_ptr<Logger> log) 
    /**
     * @brief Perform logger operation
     * @return : Description of return value
     */
    : logger(log) {
}

void ConfigurableAgentFactory::register_function_config(const FunctionConfig& config) {
    function_configs[config.name] = config;
    logger->info("Registered function config: " + config.name + " (type: " + config.type + ")");
}

std::unique_ptr<AgentFunction> ConfigurableAgentFactory::create__function(const std::string& function_name) {
    const auto it = function_configs.find(function_name);
    if (it != function_configs.end()) {
        // Found explicit function config
        const FunctionConfig& config = it->second;
        
        if (config.type == "llm") {
            LLMConfig llm_config; // Use default or parse from config
            return std::make_unique<LLMFunction>(
                config.name, 
                config.description,
                "You are a helpful AI assistant performing the function: " + config.description,
                llm_config
            );
        } else if (config.type == "external_api") {
            return std::make_unique<ExternalAPIFunction>(
                config.name, 
                config.description,
                config.endpoint
            );
        } else if (config.type == "builtin") {
            return create__builtin_function(config);
        } else if (config.type == "inference") {
            return std::make_unique<InferenceFunction>();
        } else if (config.type == "retrieval") {
            return std::make_unique<RetrievalFunction>();
        } else if (config.type == "context_retrieval") {
            return std::make_unique<ContextRetrievalFunction>();
        }
        
        logger->warn("Unknown function type: " + config.type);
        return nullptr;
    }
    
    // If no explicit config found, try to create as builtin function
    logger->debug("No explicit config found for function '" + function_name + "', attempting to create as builtin");
    
    // Create a default config for builtin functions
    FunctionConfig default_config;
    default_config.name = function_name;
    default_config.type = "builtin";
    default_config.description = "Built-in function: " + function_name;
    
    auto builtin_function = create__builtin_function(default_config);
    if (builtin_function) {
        logger->info("Successfully created builtin function: " + function_name);
        return builtin_function;
    }
    
    logger->error("Function config not found and could not create as builtin: " + function_name);
    return nullptr;
}

std::unique_ptr<AgentFunction> ConfigurableAgentFactory::create__builtin_function(const FunctionConfig& config) {
    if (config.name == "add") {
        return std::make_unique<AddFunction>();
    } else if (config.name == "echo") {
        return std::make_unique<EchoFunction>();
    } else if (config.name == "delay") {
        return std::make_unique<DelayFunction>();
    } else if (config.name == "text_analysis" || config.name == "text_processing") {
        return std::make_unique<TextAnalysisFunction>();
    } else if (config.name == "data_analysis") {
        return std::make_unique<DataAnalysisFunction>();
    } else if (config.name == "data_transform") {
        return std::make_unique<DataTransformFunction>();
    } else if (config.name == "inference") {
        return std::make_unique<InferenceFunction>();
    } else if (config.name == "retrieval") {
        return std::make_unique<RetrievalFunction>();
    } else if (config.name == "context_retrieval") {
        return std::make_unique<ContextRetrievalFunction>();
    } else if (config.name == "add_document") {
        return std::make_unique<AddDocumentFunction>();
    } else if (config.name == "remove_document") {
        return std::make_unique<RemoveDocumentFunction>();
    } else if (config.name == "parse_pdf") {
        return std::make_unique<ParsePdfFunction>();
    } else if (config.name == "parse_docx") {
        return std::make_unique<ParseDocxFunction>();
    } else if (config.name == "get_embedding") {
        return std::make_unique<GetEmbeddingFunction>();
    } else if (config.name == "test_document_service") {
        return std::make_unique<TestDocumentServiceFunction>();
    
    // Research-specific functions
    } else if (config.name == "research_query_planning") {
        return std::make_unique<ResearchQueryPlanningFunction>();
    } else if (config.name == "methodology_selection") {
        return std::make_unique<MethodologySelectionFunction>();
    } else if (config.name == "source_credibility_analysis") {
        return std::make_unique<SourceCredibilityAnalysisFunction>();
    } else if (config.name == "information_synthesis") {
        return std::make_unique<InformationSynthesisFunction>();
    } else if (config.name == "fact_verification") {
        return std::make_unique<FactVerificationFunction>();
    } else if (config.name == "knowledge_graph_query") {
        return std::make_unique<KnowledgeGraphQueryFunction>();
    } else if (config.name == "research_report_generation") {
        return std::make_unique<ResearchReportGenerationFunction>();
    } else if (config.name == "entity_extraction") {
        return std::make_unique<EntityExtractionFunction>();
    } else if (config.name == "relationship_mapping") {
        return std::make_unique<RelationshipMappingFunction>();
    } else if (config.name == "citation_management") {
        return std::make_unique<CitationManagementFunction>();
    
    // Additional missing functions from config.yaml
    } else if (config.name == "analyze_data") {
        return std::make_unique<DataAnalysisFunction>();
    } else if (config.name == "research_topic") {
        // Create a generic research function as LLMFunction
        LLMConfig llm_config;
        return std::make_unique<LLMFunction>(
            "research_topic", 
            "Conduct research on a given topic",
            "You are a research assistant capable of conducting comprehensive research on various topics.",
            llm_config
        );
    } else if (config.name == "generate_report") {
        return std::make_unique<ResearchReportGenerationFunction>();
    } else if (config.name == "synthesize_information") {
        return std::make_unique<InformationSynthesisFunction>();
    } else if (config.name == "execute_task") {
        // Create a generic task execution function as LLMFunction
        LLMConfig llm_config;
        return std::make_unique<LLMFunction>(
            "execute_task", 
            "Execute a specific task with given parameters",
            "You are a task executor capable of performing specific operations and completing assigned tasks efficiently.",
            llm_config
        );
    } else if (config.name == "use_tool") {
        // Create a generic tool usage function as LLMFunction
        LLMConfig llm_config;
        return std::make_unique<LLMFunction>(
            "use_tool", 
            "Use external tools and utilities for various tasks",
            "You are capable of using various tools and utilities to complete tasks.",
            llm_config
        );
    } else if (config.name == "process_files") {
        // Create a generic file processing function as LLMFunction
        LLMConfig llm_config;
        return std::make_unique<LLMFunction>(
            "process_files", 
            "Process various file formats and extract content",
            "You are capable of processing various file formats and extracting content from them.",
            llm_config
        );
    } else if (config.name == "call_api") {
        // Create a generic API calling function as LLMFunction
        LLMConfig llm_config;
        return std::make_unique<LLMFunction>(
            "call_api", 
            "Make API calls to external services",
            "You are capable of making API calls to external services and processing responses.",
            llm_config
        );
    } else if (config.name == "project_coordination") {
        // Create a generic project coordination function as LLMFunction
        LLMConfig llm_config;
        return std::make_unique<LLMFunction>(
            "project_coordination", 
            "Coordinate project activities and manage resources",
            "You are a project coordinator capable of managing project activities and resources.",
            llm_config
        );
    } else if (config.name == "bias_detection") {
        // Create a generic bias detection function as LLMFunction
        LLMConfig llm_config;
        return std::make_unique<LLMFunction>(
            "bias_detection", 
            "Detect various types of bias in data, sources, or research",
            "You are an expert in detecting various types of bias in data, sources, and research materials.",
            llm_config
        );
    } else if (config.name == "citation_analysis") {
        // Create a generic citation analysis function as LLMFunction
        LLMConfig llm_config;
        return std::make_unique<LLMFunction>(
            "citation_analysis", 
            "Analyzes citations and bibliographic references for quality and relevance",
            "You are an expert in analyzing citations and bibliographic references for quality and relevance.",
            llm_config
        );
    } else if (config.name == "knowledge_retrieval") {
        return std::make_unique<RetrievalFunction>();
    }
    
    logger->warn("Unknown builtin function: " + config.name);
    return nullptr;
}

// YAMLConfigurableAgentManager implementation
YAMLConfigurableAgentManager::YAMLConfigurableAgentManager() {
    // Use the adapter to bridge ServerLogger singleton with agents::Logger
    logger = std::make_shared<ServerLoggerAdapter>();
    message_router = std::make_shared<MessageRouter>(logger);
    agent_factory = std::make_shared<ConfigurableAgentFactory>(logger);
}

YAMLConfigurableAgentManager::~YAMLConfigurableAgentManager() {
    stop();
}

bool YAMLConfigurableAgentManager::load_configuration(const std::string& yaml_file) {
    try {
        system_config = SystemConfig::from_file(yaml_file);
        
        // Validate configuration before proceeding
        logger->info("Validating agent system configuration...");
        
    const auto system_validation = AgentConfigValidator::validate_system_config(system_config);
    const auto engine_validation = AgentConfigValidator::validate_inference_engines(system_config.inference_engines);
        const auto function_validation = AgentConfigValidator::validate_function_configs(system_config.functions);
        // Log validation results
        if (!system_validation.is_valid || !engine_validation.is_valid || !function_validation.is_valid) {
            logger->error("Configuration validation failed:");
            
            for (const auto& error : system_validation.errors) {
                logger->error("System config error: " + error);
            }
            for (const auto& error : engine_validation.errors) {
                logger->error("Engine config error: " + error);
            }
            for (const auto& error : function_validation.errors) {
                logger->error("Function config error: " + error);
            }
            
            return false;
        }
        
        // Log warnings
        for (const auto& warning : system_validation.warnings) {
            logger->warn("System config warning: " + warning);
        }
        for (const auto& warning : engine_validation.warnings) {
            logger->warn("Engine config warning: " + warning);
        }
        for (const auto& warning : function_validation.warnings) {
            logger->warn("Function config warning: " + warning);
        }
        
        // Log suggestions
        for (const auto& suggestion : system_validation.suggestions) {
            logger->info("System config suggestion: " + suggestion);
        }
        for (const auto& suggestion : engine_validation.suggestions) {
            logger->info("Engine config suggestion: " + suggestion);
        }
        for (const auto& suggestion : function_validation.suggestions) {
            logger->info("Function config suggestion: " + suggestion);
        }
        
        // Validate individual agent configurations
    int valid_agents = 0;
        for (const auto& agent_config : system_config.agents) {
            const auto agent_validation = AgentConfigValidator::validate_agent_config(agent_config);
            if (!agent_validation.is_valid) {
                logger->error("Agent '" + agent_config.name + "' configuration is invalid:");
                for (const auto& error : agent_validation.errors) {
                    logger->error("  " + error);
                }
            } else {
                valid_agents++;
                
                // Log warnings for this agent
                for (const auto& warning : agent_validation.warnings) {
                    logger->warn("Agent '" + agent_config.name + "': " + warning);
                }
            }
            
            // Check agent dependencies
            if (!AgentConfigValidator::validate_agent_dependencies(agent_config, system_config.functions)) {
                logger->warn("Agent '" + agent_config.name + "' has missing function dependencies");
            }
        }
        
        if (valid_agents == 0) {
            logger->error("No valid agent configurations found");
            return false;
        }
        
        logger->info("Configuration validation completed: " + std::to_string(valid_agents) + "/" + 
                    std::to_string(system_config.agents.size()) + " agents are valid");
        
        // Check inference engine health - wrap in try-catch to prevent failures
        try {
            logger->info("DEBUG: About to check inference engine health");
            const auto engine_statuses = AgentConfigValidator::check_inference_engine_health();
            logger->info("DEBUG: Inference engine health check completed successfully");
            
            int healthy_engines = 0;
            for (const auto& status : engine_statuses) {
                if (status.available && status.healthy) {
                    healthy_engines++;
                    logger->info("Inference engine '" + status.name + "': " + status.status_message);
                } else if (status.available) {
                    logger->warn("Inference engine '" + status.name + "': " + status.status_message);
                } else {
                    logger->debug("Inference engine '" + status.name + "': " + status.status_message);
                }
            }
            
            if (healthy_engines == 0) {
                logger->warn("No healthy inference engines detected - LLM functions may not work properly");
            } else {
                logger->info("Found " + std::to_string(healthy_engines) + " healthy inference engine(s)");
            }
        } catch (const std::bad_alloc& e) {
            logger->error("Memory allocation failed during health check: " + std::string(e.what()));
            throw; // Re-throw memory errors as they indicate serious problems
        } catch (const std::exception& e) {
            logger->warn("Failed to check inference engine health: " + std::string(e.what()));
            logger->warn("Continuing with configuration loading...");
        } catch (...) {
            logger->error("Unknown exception while checking inference engine health - this may indicate a serious system issue");
            logger->warn("Continuing with configuration loading, but system stability may be compromised");
        }
        
        // Register function configurations
        logger->info("DEBUG: About to register function configurations");
        try {
            for (const auto& func_config : system_config.functions) {
                logger->debug("Registering function: " + func_config.name);
                agent_factory->register_function_config(func_config);
            }
            logger->info("DEBUG: Function configurations registered successfully");
        } catch (const std::exception& e) {
            logger->error("Exception registering function configurations: " + std::string(e.what()));
            throw;
        }
        
        logger->info("Configuration loaded successfully from: " + yaml_file);
        logger->info("Found " + std::to_string(system_config.agents.size()) + " agent configurations");
        logger->info("Found " + std::to_string(system_config.functions.size()) + " function configurations");
        logger->info("Found " + std::to_string(system_config.inference_engines.size()) + " inference engine configurations");
        
        logger->info("DEBUG: About to return true from load_configuration");
        return true;
    } catch (const std::exception& e) {
        logger->error("Failed to load configuration: " + std::string(e.what()));
        logger->error("DEBUG: About to return false from load_configuration due to exception");
        return false;
    }
}

bool YAMLConfigurableAgentManager::load_configuration(const SystemConfig& system_config_param) {
    try {
    // Store the provided system configuration
    system_config = system_config_param;
        // Validate configuration before proceeding
        logger->info("Validating agent system configuration...");
        
        auto system_validation = AgentConfigValidator::validate_system_config(system_config);
        auto engine_validation = AgentConfigValidator::validate_inference_engines(system_config.inference_engines);
        const auto function_validation = AgentConfigValidator::validate_function_configs(system_config.functions);
        // Log validation results
        if (!system_validation.is_valid || !engine_validation.is_valid || !function_validation.is_valid) {
            logger->error("Configuration validation failed:");
            
            for (const auto& error : system_validation.errors) {
                logger->error("System config error: " + error);
            }
            for (const auto& error : engine_validation.errors) {
                logger->error("Engine config error: " + error);
            }
            for (const auto& error : function_validation.errors) {
                logger->error("Function config error: " + error);
            }
            
            return false;
        }
        
        // Log warnings
        for (const auto& warning : system_validation.warnings) {
            logger->warn("System config warning: " + warning);
        }
        for (const auto& warning : engine_validation.warnings) {
            logger->warn("Engine config warning: " + warning);
        }
        for (const auto& warning : function_validation.warnings) {
            logger->warn("Function config warning: " + warning);
        }
        
        // Log suggestions
        for (const auto& suggestion : system_validation.suggestions) {
            logger->info("System config suggestion: " + suggestion);
        }
        for (const auto& suggestion : engine_validation.suggestions) {
            logger->info("Engine config suggestion: " + suggestion);
        }
        for (const auto& suggestion : function_validation.suggestions) {
            logger->info("Function config suggestion: " + suggestion);
        }
        
        logger->info("Agent system configuration loaded successfully from SystemConfig");
        logger->info("Found " + std::to_string(system_config.agents.size()) + " agent configurations");
        logger->info("Found " + std::to_string(system_config.functions.size()) + " function configurations");
        logger->info("Found " + std::to_string(system_config.inference_engines.size()) + " inference engine configurations");
        
        return true;
    } catch (const std::exception& e) {
        logger->error("Failed to load configuration from SystemConfig: " + std::string(e.what()));
        return false;
    }
}

void YAMLConfigurableAgentManager::start() {
    if (running.load()) {
        logger->warn("Agent manager is already running");
        return;
    }

    running.store(true);
    message_router->start();
    
    // Create and start agents from configuration
    for (const auto& agent_config : system_config.agents) {
        std::string agent_id = create__agent_from_config(agent_config);
        if (!agent_id.empty() && agent_config.auto_start) {
            start_agent(agent_id);
        }
    }
    
    logger->info("YAML-configurable agent manager started");
}

void YAMLConfigurableAgentManager::stop() {
    if (!running.load()) {
        return;
    }

    logger->info("Stopping YAML-configurable agent manager");
    running.store(false);

    {
        std::lock_guard<std::mutex> lock(agents_mutex);
        // Stop all agents
        for (const auto& pair : active_agents) {
            try {
                if (pair.second && pair.second->is__running()) {
                    pair.second->stop();
                }
            } catch (const std::exception& e) {
                logger->error("Error stopping agent " + pair.first + ": " + e.what());
            }
        }
    }

    message_router->stop();
    logger->info("YAML-configurable agent manager stopped");
}

std::string YAMLConfigurableAgentManager::create__agent_from_config(const AgentConfig& config) {
    if (config.name.empty() || config.type.empty()) {
        logger->error("Invalid agent configuration: name and type are required");
        return "";
    }

    try {
        const auto agent = std::make_shared<AgentCore>(config.name, config.type);
        // Set up agent capabilities
        for (const auto& capability : config.capabilities) {
            agent->add_capability(capability);
        }
        
        // Register functions for this agent
        for (const auto& function_name : config.functions) {
            auto function = agent_factory->create__function(function_name);
            if (function) {
                agent->get__function_manager()->register_function(std::move(function));
            } else {
                logger->warn("Failed to create function: " + function_name + " for agent: " + config.name);
            }
        }
        
        // Set up message handling
        agent->set__message_router(message_router);
        
    // Get agent ID before locking to minimize lock time
    std::string agent_id = agent->get__agent_id();
        // Store the agent thread-safely
        {
            std::lock_guard<std::mutex> lock(agents_mutex);
            active_agents[agent_id] = agent;
        }
        
        logger->info("Created agent from config: " + config.name + " (ID: " + agent_id.substr(0, 8) + "...)");
        return agent_id;
        
    } catch (const std::exception& e) {
        logger->error("Failed to create agent from config: " + config.name + ": " + std::string(e.what()));
        return "";
    }
}

bool YAMLConfigurableAgentManager::start_agent(const std::string& agent_id) {
    if (agent_id.empty()) {
        logger->error("Invalid agent ID provided");
        return false;
    }

    std::lock_guard<std::mutex> lock(agents_mutex);
    const auto it = active_agents.find(agent_id);
    if (it == active_agents.end()) {
        logger->error("Agent not found: " + agent_id);
        return false;
    }

    if (!it->second) {
        logger->error("Agent instance is null: " + agent_id);
        return false;
    }

    if (it->second->is__running()) {
        logger->warn("Agent is already running: " + agent_id);
        return true;
    }

    try {
        it->second->start();
        logger->info("Agent started: " + agent_id.substr(0, 8) + "...");
        return true;
    } catch (const std::exception& e) {
        logger->error("Failed to start agent " + agent_id + ": " + e.what());
        return false;
    }
}

bool YAMLConfigurableAgentManager::stop_agent(const std::string& agent_id) {
    if (agent_id.empty()) {
        logger->error("Invalid agent ID provided");
        return false;
    }

    std::lock_guard<std::mutex> lock(agents_mutex);
    auto it = active_agents.find(agent_id);
    if (it == active_agents.end()) {
        logger->error("Agent not found: " + agent_id);
        return false;
    }

    if (!it->second) {
        logger->error("Agent instance is null: " + agent_id);
        return false;
    }

    if (!it->second->is__running()) {
        logger->warn("Agent is not running: " + agent_id);
        return true;
    }

    try {
        it->second->stop();
        logger->info("Agent stopped: " + agent_id.substr(0, 8) + "...");
        return true;
    } catch (const std::exception& e) {
        logger->error("Failed to stop agent " + agent_id + ": " + e.what());
        return false;
    }
}

bool YAMLConfigurableAgentManager::delete_agent(const std::string& agent_id) {
    if (agent_id.empty()) {
        logger->error("Invalid agent ID provided");
        return false;
    }

    std::lock_guard<std::mutex> lock(agents_mutex);
    auto it = active_agents.find(agent_id);
    if (it == active_agents.end()) {
        logger->error("Agent not found: " + agent_id);
        return false;
    }

    if (!it->second) {
        logger->error("Agent instance is null: " + agent_id);
        return false;
    }

    try {
        // Stop the agent first if it's running
        if (it->second->is__running()) {
            it->second->stop();
        }
        
        // Remove from active agents map
        active_agents.erase(it);
        
        logger->info("Agent deleted: " + agent_id.substr(0, 8) + "...");
        return true;
    } catch (const std::exception& e) {
        logger->error("Failed to delete agent " + agent_id + ": " + e.what());
        return false;
    }
}

bool YAMLConfigurableAgentManager::reload_configuration(const std::string& yaml_file) {
    logger->info("Reloading configuration from: " + yaml_file);
    
    std::lock_guard<std::mutex> lock(agents_mutex);
    
    // Stop all current agents
    for (const auto& pair : active_agents) {
        try {
            if (pair.second && pair.second->is__running()) {
                pair.second->stop();
            }
        } catch (const std::exception& e) {
            logger->error("Error stopping agent " + pair.first + ": " + e.what());
        }
    }
    active_agents.clear();
    
    // Load new configuration
    if (!load_configuration(yaml_file)) {
        logger->error("Failed to reload configuration");
        return false;
    }
    
    // Create and start new agents
    for (const auto& agent_config : system_config.agents) {
        std::string agent_id = create__agent_from_config(agent_config);
        if (!agent_id.empty() && agent_config.auto_start) {
            start_agent(agent_id);
        }
    }
    
    logger->info("Configuration reloaded successfully");
    return true;
}

std::vector<std::string> YAMLConfigurableAgentManager::list_agents() const {
    std::vector<std::string> agent_ids;
    std::lock_guard<std::mutex> lock(agents_mutex);
    
    for (const auto& pair : active_agents) {
        agent_ids.push_back(pair.first);
    }
    return agent_ids;
}

std::shared_ptr<AgentCore> YAMLConfigurableAgentManager::get__agent(const std::string& agent_id) {
    if (agent_id.empty()) {
        logger->error("Invalid agent ID provided");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(agents_mutex);
    auto it = active_agents.find(agent_id);
    return (it != active_agents.end()) ? it->second : nullptr;
}

std::shared_ptr<AgentCore> YAMLConfigurableAgentManager::get_agent_by_name(const std::string& agent_name) {
    if (agent_name.empty()) {
        logger->error("Invalid agent name provided");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(agents_mutex);
    for (const auto& pair : active_agents) {
        if (pair.second && pair.second->get__agent_name() == agent_name) {
            return pair.second;
        }
    }
    return nullptr;
}

std::string YAMLConfigurableAgentManager::get__system_status() const {
    std::lock_guard<std::mutex> lock(agents_mutex);
    
    std::ostringstream status;
    status << "=== YAML-Configurable Agent Manager Status ===\n";
    status << "Total Agents: " << active_agents.size() << "\n";
    status << "Running Agents: ";
    
    int running_count = 0;
    for (const auto& pair : active_agents) {
        if (pair.second && pair.second->is__running()) {
            running_count++;
        }
    }
    status << running_count << "\n";
    
    status << "Loaded Functions: " << system_config.functions.size() << "\n";
    status << "Worker Threads: " << system_config.worker_threads << "\n";
    status << "Log Level: " << system_config.log_level << "\n";

    return status.str();
}

void YAMLConfigurableAgentManager::demonstrate_system() {
    logger->info("=== YAML-Configurable Multi-Agent System Demo ===");
    // Show system status
    logger->info(get__system_status());
    
    // List all agents - Note: This already includes mutex locking
    const auto agent_ids = list_agents();
    logger->info("Active Agents: " + std::to_string(agent_ids.size()));
    
    // No need for additional lock since we're using get_agent which has its own locking
    for (const auto& agent_id : agent_ids) {
        const auto agent = get__agent(agent_id);
        if (agent) {
            std::ostringstream agent_info;
            agent_info << "  - " << agent->get__agent_name() 
                      << " (ID: " << agent_id.substr(0, 8) << "...)" 
                      << " Type: " << agent->get__agent_type()
                      << " Status: " << (agent->is__running() ? "RUNNING" : "STOPPED");
            logger->info(agent_info.str());
            
            // Show capabilities
            const auto capabilities = agent->get__capabilities();
            if (!capabilities.empty()) {
                std::ostringstream caps;
                caps << "    Capabilities: ";
                for (size_t i = 0; i < capabilities.size(); ++i) {
                    if (i > 0) caps << ", ";
                    caps << capabilities[i];
                }
                logger->info(caps.str());
            }
            
            // Show available functions
            const auto function_names = agent->get__function_manager()->get__function_names();
            if (!function_names.empty()) {
                std::ostringstream funcs;
                funcs << "    Functions: ";
                for (size_t i = 0; i < function_names.size(); ++i) {
                    if (i > 0) funcs << ", ";
                    funcs << function_names[i];
                }
                logger->info(funcs.str());
            }
        }
    }
    
    logger->info("=== Demo completed ===");
}

} // namespace kolosal::agents
