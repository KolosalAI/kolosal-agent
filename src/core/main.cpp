#include "agent.hpp"
#include "agent_manager.hpp"
#include "agent_config.hpp"
#include "http_server.hpp"
#include "workflow_manager.hpp"
#include "workflow_types.hpp"
#include "kolosal_server_launcher.hpp"
#include "logger.hpp"
#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> system_running{true};
std::unique_ptr<HTTPServer> http_server;
std::shared_ptr<WorkflowManager> workflow_manager;
std::shared_ptr<WorkflowOrchestrator> workflow_orchestrator;
std::unique_ptr<KolosalServerLauncher> kolosal_server_launcher;

void signal_handler(int signal) {
    LOG_INFO_F("Received signal %d, shutting down gracefully...", signal);
    system_running.store(false);
    
    // Stop in proper order: HTTP server -> workflow system -> agents -> kolosal server
    if (http_server) {
        LOG_DEBUG("Stopping HTTP server...");
        http_server->stop();
    }
    if (workflow_orchestrator) {
        LOG_DEBUG("Stopping workflow orchestrator...");
        workflow_orchestrator->stop();
    }
    if (workflow_manager) {
        LOG_DEBUG("Stopping workflow manager...");
        workflow_manager->stop();
    }
    if (kolosal_server_launcher) {
        LOG_DEBUG("Stopping Kolosal Server...");
        kolosal_server_launcher->stop();
    }
}

void print_banner() {
    std::cout << R"(
===============================================================================
                          Kolosal Agent System v1.0
                         Multi-Agent Platform
===============================================================================
)" << std::endl;
    
    // Configure logging based on build type
    auto& logger = KolosalAgent::Logger::instance();
    
#ifdef DEBUG_BUILD
    logger.set_level(KolosalAgent::LogLevel::DEBUG_LVL);
    logger.enable_function_tracing(true);
    logger.enable_thread_id(true);
    logger.set_file_output("kolosal_agent_debug.log");
    LOG_INFO("Debug build detected - enabling verbose logging");
    LOG_DEBUG("Function tracing and debug features enabled");
#else
    logger.set_level(KolosalAgent::LogLevel::INFO_LVL);
    logger.enable_function_tracing(false);
    logger.enable_thread_id(false);
    LOG_INFO("Release build - using standard logging level");
#endif
    
    LOG_INFO("Kolosal Agent System v1.0 initializing...");
}

void print_usage(const char* program_name) {
    std::cout << R"(
Usage: )" << program_name << R"( [OPTIONS]

Kolosal Agent System - A multi-agent platform

OPTIONS:
    --host HOST        Server host (default: from agent.yaml or 127.0.0.1)
    --port PORT        Server port (default: from agent.yaml or 8080)
    --config FILE      Agent configuration file (default: agent.yaml)
    --workflow FILE    Workflow configuration file (default: workflow.yaml)
    --help             Show this help message

EXAMPLES:
    )" << program_name << R"(                         # Start with default settings
    )" << program_name << R"( --port 9090             # Start on custom port
    )" << program_name << R"( --host 0.0.0.0          # Listen on all interfaces
    )" << program_name << R"( --config my-config.yaml # Use custom config file
    )" << program_name << R"( --workflow my-workflows.yaml # Use custom workflow file

API ENDPOINTS:
    GET    /agents              - List all agents
    POST   /agents              - Create new agent
    GET    /agents/{id}         - Get agent info
    PUT    /agents/{id}/start   - Start agent
    PUT    /agents/{id}/stop    - Stop agent
    DELETE /agents/{id}         - Delete agent
    POST   /agents/{id}/execute - Execute function
    GET    /status              - System status

WORKFLOW ORCHESTRATION:
    GET    /workflows             - List workflow definitions
    POST   /workflows             - Register workflow definition
    POST   /workflows/{id}/execute     - Execute workflow
    GET    /workflows/executions/{id} - Get execution status
    PUT    /workflows/executions/{id}/{action} - Control execution
    GET    /workflows/executions  - List workflow executions

Configuration:
    - Agent system: agent.yaml (or specified with --config)
    - Workflow definitions: workflow.yaml (or specified with --workflow)
    - Kolosal server: config.yaml (separate component)

For more information, visit: https://github.com/KolosalAI/kolosal-agent
)";
}

int main(int argc, char* argv[]) {
    TRACE_FUNCTION(); // Function tracing for debug builds
    
    std::string host;
    int port = 0;
    std::string config_file = "agent.yaml";
    std::string workflow_config_file = "workflow.yaml";
    bool host_override = false;
    bool port_override = false;
    
    LOG_DEBUG_F("Starting with %d command line arguments", argc);
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        LOG_DEBUG_F("Processing argument %d: %s", i, arg.c_str());
        
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
            host_override = true;
            LOG_DEBUG_F("Host override set to: %s", host.c_str());
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
            port_override = true;
            LOG_DEBUG_F("Port override set to: %d", port);
        } else if (arg == "--config" && i + 1 < argc) {
            config_file = argv[++i];
            LOG_DEBUG_F("Config file set to: %s", config_file.c_str());
        } else if (arg == "--workflow" && i + 1 < argc) {
            workflow_config_file = argv[++i];
            LOG_DEBUG_F("Workflow config file set to: %s", workflow_config_file.c_str());
        } else {
            LOG_ERROR_F("Unknown argument: %s", arg.c_str());
            print_usage(argv[0]);
            return 1;
        }
    }
    
    try {
        print_banner();
        
        // Setup signal handlers
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        LOG_DEBUG("Signal handlers registered");
        
        // Start Kolosal Server FIRST before anything else
        LOG_INFO("Starting Kolosal Server...");
        {
            SCOPED_TIMER("kolosal_server_startup");
            
            // Create server configuration
            auto server_config = create_default_server_config();
            server_config.host = "127.0.0.1";
            server_config.port = 8081;
            server_config.quiet_mode = true; // Run in background quietly
            server_config.log_level = "INFO";
            server_config.timeout = 45; // Give more time for startup
            
            kolosal_server_launcher = std::make_unique<KolosalServerLauncher>(server_config);
            
            if (kolosal_server_launcher->start()) {
                LOG_INFO_F("✓ Kolosal Server started successfully at %s", 
                          kolosal_server_launcher->get_server_url().c_str());
                
                // Wait for server to be fully ready
                LOG_INFO("Waiting for Kolosal Server to be fully ready...");
                if (kolosal_server_launcher->wait_for_ready(30)) {
                    LOG_INFO("✓ Kolosal Server is ready for requests");
                } else {
                    LOG_WARN("⚠ Kolosal Server startup timeout - proceeding anyway");
                }
            } else {
                LOG_ERROR("✗ Failed to start Kolosal Server - system will proceed without it");
                LOG_WARN("Agents will use fallback responses only");
            }
        }
        
        // Now initialize the agent system
        LOG_INFO("Initializing Agent System...");
        
        // Create configuration manager and load configuration
        LOG_DEBUG_F("Loading configuration from: %s", config_file.c_str());
        auto config_manager = std::make_shared<AgentConfigManager>();
        {
            SCOPED_TIMER("config_load");
            config_manager->load_config(config_file);
        }
        LOG_INFO_F("Configuration loaded from: %s", config_file.c_str());
        
        // Create agent manager with configuration
        LOG_DEBUG("Creating agent manager");
        auto agent_manager = std::make_shared<AgentManager>(config_manager);
        
        // Get host and port from config if not overridden
        if (!host_override) {
            host = config_manager->get_host();
            LOG_DEBUG_F("Using host from config: %s", host.c_str());
        }
        if (!port_override) {
            port = config_manager->get_port();
            LOG_DEBUG_F("Using port from config: %d", port);
        }
        
        // Initialize default agents from configuration
        LOG_INFO("Initializing default agents from configuration");
        {
            SCOPED_TIMER("agent_initialization");
            agent_manager->initialize_default_agents();
        }
        LOG_DEBUG("✓ Default agents initialized successfully");
        
        // Create and start workflow system
        LOG_INFO("Initializing workflow system...");
        
        // Create workflow manager
        LOG_DEBUG("Creating workflow manager");
        workflow_manager = std::make_shared<WorkflowManager>(agent_manager);
        
        // Load function configurations from agent config
        const auto& config_data = config_manager->get_config();
        if (!config_data.functions.empty()) {
            LOG_DEBUG_F("Loading %zu function configurations", config_data.functions.size());
            // Convert functions map to JSON for compatibility
            json functions_json;
            for (const auto& [name, func_config] : config_data.functions) {
                functions_json["functions"][name] = {
                    {"description", func_config.description},
                    {"timeout", func_config.timeout},
                    {"parameters", func_config.parameters}
                };
                LOG_DEBUG_F("Loaded function config: %s", name.c_str());
            }
            workflow_manager->load_function_configs(functions_json);
        }
        
        // Start workflow manager
        LOG_DEBUG("Starting workflow manager");
        if (!workflow_manager->start()) {
            LOG_FATAL("Failed to start workflow manager");
            return 1;
        }
        LOG_DEBUG("Workflow manager started successfully");
        
        // Create and start workflow orchestrator
        LOG_DEBUG("Creating workflow orchestrator");
        workflow_orchestrator = std::make_shared<WorkflowOrchestrator>(workflow_manager);
        
        // Load workflow configuration
        std::string workflow_config_path = workflow_config_file;
        LOG_DEBUG_F("Loading workflow configuration from: %s", workflow_config_path.c_str());
        if (workflow_orchestrator->load_workflow_config(workflow_config_path)) {
            LOG_INFO_F("Workflow configuration loaded from %s", workflow_config_path.c_str());
        } else {
            LOG_WARN_F("Could not load workflow configuration from %s, using built-in workflows only", workflow_config_path.c_str());
        }
        
        LOG_DEBUG("Starting workflow orchestrator");
        if (!workflow_orchestrator->start()) {
            LOG_FATAL("Failed to start workflow orchestrator");
            return 1;
        }
        LOG_DEBUG("Workflow orchestrator started successfully");
        
        LOG_INFO("Workflow system initialized successfully");
        
        // Create and start HTTP server with workflow support (no kolosal-server endpoints)
        LOG_DEBUG_F("Creating HTTP server on %s:%d", host.c_str(), port);
        http_server = std::make_unique<HTTPServer>(agent_manager, workflow_manager, workflow_orchestrator, host, port);
        
        LOG_DEBUG("Starting HTTP server");
        if (!http_server->start()) {
            LOG_FATAL("Failed to start HTTP server");
            return 1;
        }
        LOG_DEBUG("HTTP server started successfully");
        
        LOG_INFO("Kolosal Agent System is now running!");
        std::cout << "   * Server: http://" << host << ":" << port << "\n";
        std::cout << "   * Agent Configuration: " << config_manager->get_config_file_path() << "\n";
        std::cout << "   * Workflow Configuration: " << workflow_config_path << "\n";
        std::cout << "   * API Documentation: Available at endpoints above\n";
        std::cout << "   * Default agents created and ready\n";
        std::cout << "   * Workflow system active with built-in templates\n\n";
        
        // Get the first available agent for examples
        auto agents_list = agent_manager->list_agents();
        std::string example_agent_id;
        if (agents_list.contains("agents") && !agents_list["agents"].empty()) {
            example_agent_id = agents_list["agents"][0]["id"];
        }
        
        std::cout << "📋 Quick Start Examples:\n";
        std::cout << "   # List all agents\n";
        std::cout << "   curl http://" << host << ":" << port << "/agents\n\n";
        
        if (!example_agent_id.empty()) {
            std::cout << "   # Chat with agent (specify model)\n";
            std::cout << "   curl -X POST http://" << host << ":" << port << "/agents/" << example_agent_id << "/execute \\\n";
            std::cout << "     -H \"Content-Type: application/json\" \\\n";
            std::cout << "     -d '{\"function\": \"chat\", \"model\": \"your_model_name\", \"params\": {\"message\": \"Hello!\"}}'\n\n";
            
            std::cout << "   # Execute simple research workflow with agent-LLM pairing\n";
            std::cout << "   curl -X POST http://" << host << ":" << port << "/workflows/simple_research/execute \\\n";
            std::cout << "     -H \"Content-Type: application/json\" \\\n";
            std::cout << "     -d '{\"input_data\": {\"question\": \"What is AI?\"}}'\n\n";
            
            std::cout << "   # Execute analysis workflow with multiple agents\n";
            std::cout << "   curl -X POST http://" << host << ":" << port << "/workflows/analysis_workflow/execute \\\n";
            std::cout << "     -H \"Content-Type: application/json\" \\\n";
            std::cout << "     -d '{\"input_data\": {\"text\": \"Sample text to analyze\"}}'\n\n";
        }
        
        std::cout << "   # List workflow templates\n";
        std::cout << "   curl http://" << host << ":" << port << "/workflows\n\n";
        
        std::cout << "   # System status\n";
        std::cout << "   curl http://" << host << ":" << port << "/status\n\n";
        
        std::cout << "Press Ctrl+C to shutdown gracefully...\n\n";
        
        // Main event loop
        LOG_DEBUG("Entering main event loop");
        while (system_running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Graceful shutdown
        LOG_INFO("Shutting down system...");
        
        // Stop HTTP server first
        if (http_server) {
            LOG_DEBUG("Stopping HTTP server...");
            http_server->stop();
            http_server.reset();
        }
        
        // Stop workflow system
        if (workflow_orchestrator) {
            LOG_INFO("Stopping workflow orchestrator...");
            workflow_orchestrator->stop();
            workflow_orchestrator.reset();
        }
        
        if (workflow_manager) {
            LOG_INFO("Stopping workflow manager...");
            workflow_manager->stop();
            workflow_manager.reset();
        }
        
        // Stop all agents
        LOG_DEBUG("Stopping all agents...");
        agent_manager->stop_all_agents();
        
        // Stop Kolosal Server LAST
        if (kolosal_server_launcher && kolosal_server_launcher->is_running()) {
            LOG_INFO("Stopping Kolosal Server...");
            kolosal_server_launcher->stop();
            LOG_INFO("✓ Kolosal Server stopped");
        }
        
        LOG_INFO("Kolosal Agent System shutdown complete.");
        return 0;
        
    } catch (const std::exception& e) {
        LOG_FATAL_F("Fatal error: %s", e.what());
        return 1;
    }
}
