#include "../include/agent.hpp"
#include "../include/agent_manager.hpp"
#include "../include/agent_config.hpp"
#include "../include/http_server.hpp"
#include "../include/workflow_manager.hpp"
#include "../include/workflow_types.hpp"
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

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully...\n";
    system_running.store(false);
    if (http_server) {
        http_server->stop();
    }
    if (workflow_orchestrator) {
        workflow_orchestrator->stop();
    }
    if (workflow_manager) {
        workflow_manager->stop();
    }
}

void print_banner() {
    std::cout << R"(
===============================================================================
                          Kolosal Agent System v1.0
                         Multi-Agent Platform
===============================================================================
)" << std::endl;
}

void print_usage(const char* program_name) {
    std::cout << R"(
Usage: )" << program_name << R"( [OPTIONS]

Kolosal Agent System - A multi-agent platform

OPTIONS:
    --host HOST     Server host (default: from agent.yaml or 127.0.0.1)
    --port PORT     Server port (default: from agent.yaml or 8080)
    --config FILE   Configuration file (default: agent.yaml)
    --help          Show this help message

EXAMPLES:
    )" << program_name << R"(                         # Start with default settings
    )" << program_name << R"( --port 9090             # Start on custom port
    )" << program_name << R"( --host 0.0.0.0          # Listen on all interfaces
    )" << program_name << R"( --config my-config.yaml # Use custom config file

API ENDPOINTS:
    GET    /agents              - List all agents
    POST   /agents              - Create new agent
    GET    /agents/{id}         - Get agent info
    PUT    /agents/{id}/start   - Start agent
    PUT    /agents/{id}/stop    - Stop agent
    DELETE /agents/{id}         - Delete agent
    POST   /agents/{id}/execute - Execute function
    GET    /status              - System status

WORKFLOW ENDPOINTS:
    POST   /workflow/execute      - Submit workflow request
    GET    /workflow/status/{id}  - Get request status
    DELETE /workflow/cancel/{id}  - Cancel request
    GET    /workflow/requests     - List workflow requests
    GET    /workflow/status       - Workflow system status

WORKFLOW ORCHESTRATION:
    GET    /workflows             - List workflow definitions
    POST   /workflows             - Register workflow definition
    POST   /workflows/execute     - Execute workflow
    GET    /workflows/executions/{id} - Get execution status
    PUT    /workflows/executions/{id}/{action} - Control execution
    GET    /workflows/executions  - List workflow executions

Configuration:
    - Agent system: agent.yaml (this file)
    - Kolosal server: config.yaml (separate component)

For more information, visit: https://github.com/KolosalAI/kolosal-agent
)";
}

int main(int argc, char* argv[]) {
    std::string host;
    int port = 0;
    std::string config_file = "agent.yaml";
    bool host_override = false;
    bool port_override = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
            host_override = true;
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
            port_override = true;
        } else if (arg == "--config" && i + 1 < argc) {
            config_file = argv[++i];
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }
    
    try {
        print_banner();
        
        // Setup signal handlers
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        
        // Create configuration manager and load configuration
        auto config_manager = std::make_shared<AgentConfigManager>();
        config_manager->load_config(config_file);
        
        // Create agent manager with configuration
        auto agent_manager = std::make_shared<AgentManager>(config_manager);
        
        // Get host and port from config if not overridden
        if (!host_override) {
            host = config_manager->get_host();
        }
        if (!port_override) {
            port = config_manager->get_port();
        }
        
        // Initialize default agents from configuration
        agent_manager->initialize_default_agents();
        
        // Create and start workflow system
        std::cout << "🚀 Initializing workflow system...\n";
        
        // Create workflow manager
        workflow_manager = std::make_shared<WorkflowManager>(agent_manager);
        
        // Load function configurations from agent config
        const auto& config_data = config_manager->get_config();
        if (!config_data.functions.empty()) {
            // Convert functions map to JSON for compatibility
            json functions_json;
            for (const auto& [name, func_config] : config_data.functions) {
                functions_json["functions"][name] = {
                    {"description", func_config.description},
                    {"timeout", func_config.timeout},
                    {"parameters", func_config.parameters}
                };
            }
            workflow_manager->load_function_configs(functions_json);
        }
        
        // Start workflow manager
        if (!workflow_manager->start()) {
            std::cerr << "Failed to start workflow manager\n";
            return 1;
        }
        
        // Create and start workflow orchestrator
        workflow_orchestrator = std::make_shared<WorkflowOrchestrator>(workflow_manager);
        
        // Load workflow configuration
        std::string workflow_config_path = "workflow.yaml";
        if (workflow_orchestrator->load_workflow_config(workflow_config_path)) {
            std::cout << "✅ Workflow configuration loaded from " << workflow_config_path << "\n";
        } else {
            std::cout << "⚠️  Could not load workflow configuration from " << workflow_config_path << ", using built-in workflows only\n";
        }
        
        if (!workflow_orchestrator->start()) {
            std::cerr << "Failed to start workflow orchestrator\n";
            return 1;
        }
        
        std::cout << "✅ Workflow system initialized successfully\n";
        
        // Create and start HTTP server with workflow support
        http_server = std::make_unique<HTTPServer>(agent_manager, workflow_manager, workflow_orchestrator, host, port);
        
        if (!http_server->start()) {
            std::cerr << "Failed to start HTTP server\n";
            return 1;
        }
        
        std::cout << "🎯 Kolosal Agent System is now running!\n";
        std::cout << "   * Server: http://" << host << ":" << port << "\n";
        std::cout << "   * Configuration: " << config_manager->get_config_file_path() << "\n";
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
            
            std::cout << "   # Submit workflow request\n";
            std::cout << "   curl -X POST http://" << host << ":" << port << "/workflow/execute \\\n";
            std::cout << "     -H \"Content-Type: application/json\" \\\n";
            std::cout << "     -d '{\"agent_name\": \"Assistant\", \"function_name\": \"chat\", \"parameters\": {\"message\": \"Hello\", \"model\": \"gemma3-1b\"}}'\n\n";
            
            std::cout << "   # Execute simple research workflow with agent-LLM pairing\n";
            std::cout << "   curl -X POST http://" << host << ":" << port << "/workflows/execute \\\n";
            std::cout << "     -H \"Content-Type: application/json\" \\\n";
            std::cout << "     -d '{\"workflow_id\": \"simple_research\", \"input_data\": {\"question\": \"What is AI?\"}}'\n\n";
            
            std::cout << "   # Execute analysis workflow with multiple agents\n";
            std::cout << "   curl -X POST http://" << host << ":" << port << "/workflows/execute \\\n";
            std::cout << "     -H \"Content-Type: application/json\" \\\n";
            std::cout << "     -d '{\"workflow_id\": \"analysis_workflow\", \"input_data\": {\"text\": \"Sample text to analyze\"}}'\n\n";
        }
        
        std::cout << "   # List workflow templates\n";
        std::cout << "   curl http://" << host << ":" << port << "/workflows\n\n";
        
        std::cout << "   # System status\n";
        std::cout << "   curl http://" << host << ":" << port << "/status\n\n";
        
        std::cout << "Press Ctrl+C to shutdown gracefully...\n\n";
        
        // Main event loop
        while (system_running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Graceful shutdown
        std::cout << "Shutting down system...\n";
        
        if (http_server) {
            http_server->stop();
            http_server.reset();
        }
        
        if (workflow_orchestrator) {
            std::cout << "Stopping workflow orchestrator...\n";
            workflow_orchestrator->stop();
            workflow_orchestrator.reset();
        }
        
        if (workflow_manager) {
            std::cout << "Stopping workflow manager...\n";
            workflow_manager->stop();
            workflow_manager.reset();
        }
        
        agent_manager->stop_all_agents();
        
        std::cout << "Kolosal Agent System shutdown complete.\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
