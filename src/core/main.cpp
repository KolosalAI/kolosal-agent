#include "../include/agent.hpp"
#include "../include/agent_manager.hpp"
#include "../include/http_server.hpp"
#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> system_running{true};
std::unique_ptr<HTTPServer> http_server;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully...\n";
    system_running.store(false);
    if (http_server) {
        http_server->stop();
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
    --host HOST     Server host (default: 127.0.0.1)
    --port PORT     Server port (default: 8080)
    --help          Show this help message

EXAMPLES:
    )" << program_name << R"(                    # Start with default settings
    )" << program_name << R"( --port 9090        # Start on custom port
    )" << program_name << R"( --host 0.0.0.0     # Listen on all interfaces

API ENDPOINTS:
    GET    /agents              - List all agents
    POST   /agents              - Create new agent
    GET    /agents/{id}         - Get agent info
    PUT    /agents/{id}/start   - Start agent
    PUT    /agents/{id}/stop    - Stop agent
    DELETE /agents/{id}         - Delete agent
    POST   /agents/{id}/execute - Execute function
    GET    /status              - System status

For more information, visit: https://github.com/KolosalAI/kolosal-agent
)";
}

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 8080;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
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
        
        // Create agent manager
        auto agent_manager = std::make_shared<AgentManager>();
        
        // Create some default agents
        std::string assistant_id = agent_manager->create_agent("Assistant", {"chat", "analysis"});
        std::string analyzer_id = agent_manager->create_agent("Analyzer", {"analysis", "data_processing"});
        
        // Start the agents
        agent_manager->start_agent(assistant_id);
        agent_manager->start_agent(analyzer_id);
        
        std::cout << "Created and started default agents:\n";
        std::cout << "  - Assistant (" << assistant_id << ")\n";
        std::cout << "  - Analyzer (" << analyzer_id << ")\n\n";
        
        // Create and start HTTP server
        http_server = std::make_unique<HTTPServer>(agent_manager, host, port);
        
        if (!http_server->start()) {
            std::cerr << "Failed to start HTTP server\n";
            return 1;
        }
        
        std::cout << "🎯 Kolosal Agent System is now running!\n";
        std::cout << "   * Server: http://" << host << ":" << port << "\n";
        std::cout << "   * API Documentation: Available at endpoints above\n";
        std::cout << "   * Default agents created and ready\n\n";
        
        std::cout << "📋 Quick Start Examples:\n";
        std::cout << "   # List all agents\n";
        std::cout << "   curl http://" << host << ":" << port << "/agents\n\n";
        std::cout << "   # Chat with assistant\n";
        std::cout << "   curl -X POST http://" << host << ":" << port << "/agents/" << assistant_id << "/execute \\\n";
        std::cout << "     -H \"Content-Type: application/json\" \\\n";
        std::cout << "     -d '{\"function\": \"chat\", \"params\": {\"message\": \"Hello!\"}}'\n\n";
        std::cout << "   # Analyze text\n";
        std::cout << "   curl -X POST http://" << host << ":" << port << "/agents/" << analyzer_id << "/execute \\\n";
        std::cout << "     -H \"Content-Type: application/json\" \\\n";
        std::cout << "     -d '{\"function\": \"analyze\", \"params\": {\"text\": \"This is a test\"}}'\n\n";
        
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
        
        agent_manager->stop_all_agents();
        
        std::cout << "Kolosal Agent System shutdown complete.\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
