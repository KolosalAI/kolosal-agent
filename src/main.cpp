#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>
#include <memory>
#include <filesystem>
#include <fstream>

// Platform-specific includes
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

// Enhanced includes
#include "integration/unified_server.hpp"
#include "api/agent_management_route.hpp"
#include "services/agent_service.hpp"
#include "yaml_config.hpp"

using namespace kolosal::integration;
using namespace kolosal::api;
using namespace kolosal::services;

// Global state for graceful shutdown
std::atomic<bool> keep_running{true};
std::unique_ptr<UnifiedKolosalServer> unified_server;

// Enhanced signal handler
void signal_handler(int signal) {
    std::cout << "\n🛑 Received signal " << signal << ", initiating graceful shutdown..." << std::endl;
    keep_running = false;
    
    if (unified_server) {
        std::cout << "⏳ Stopping unified server..." << std::endl;
        unified_server->stop();
    }
}

// Enhanced command line parsing
struct CommandLineArgs {
    std::string config_file = "config.yaml";
    int server_port = 8080;
    std::string server_host = "127.0.0.1";
    std::string server_path = "";
    bool no_server = false;
    bool demo_mode = false;
    bool verbose = false;
    bool development_mode = false;
    bool production_mode = false;
    std::string log_level = "INFO";
    bool enable_metrics = true;
    bool enable_health_monitoring = true;
    bool show_help = false;
    bool show_version = false;
};

CommandLineArgs parse_command_line(int argc, char* argv[]) {
    CommandLineArgs args;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            args.show_help = true;
        } else if (arg == "-v" || arg == "--version") {
            args.show_version = true;
        } else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                args.config_file = argv[++i];
            } else {
                throw std::runtime_error("--config requires a file path");
            }
        } else if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                args.server_port = std::stoi(argv[++i]);
                if (args.server_port < 1 || args.server_port > 65535) {
                    throw std::runtime_error("Port must be between 1 and 65535");
                }
            } else {
                throw std::runtime_error("--port requires a port number");
            }
        } else if (arg == "--host") {
            if (i + 1 < argc) {
                args.server_host = argv[++i];
            } else {
                throw std::runtime_error("--host requires a hostname");
            }
        } else if (arg == "-s" || arg == "--server") {
            if (i + 1 < argc) {
                args.server_path = argv[++i];
            } else {
                throw std::runtime_error("--server requires a file path");
            }
        } else if (arg == "--no-server") {
            args.no_server = true;
        } else if (arg == "--demo") {
            args.demo_mode = true;
        } else if (arg == "--verbose") {
            args.verbose = true;
        } else if (arg == "--dev" || arg == "--development") {
            args.development_mode = true;
        } else if (arg == "--prod" || arg == "--production") {
            args.production_mode = true;
        } else if (arg == "--log-level") {
            if (i + 1 < argc) {
                args.log_level = argv[++i];
            } else {
                throw std::runtime_error("--log-level requires a level (DEBUG, INFO, WARN, ERROR)");
            }
        } else if (arg == "--no-metrics") {
            args.enable_metrics = false;
        } else if (arg == "--no-health-monitoring") {
            args.enable_health_monitoring = false;
        } else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }
    
    return args;
}

void print_usage(const char* program_name) {
    std::cout << R"(
🤖 Kolosal Agent System v2.0.0 - Unified LLM & Multi-Agent Platform
=====================================================================

USAGE:
    )" << program_name << R"( [OPTIONS]

OPTIONS:
    -c, --config FILE          Use custom configuration file (default: config.yaml)
    -p, --port PORT           Server port (default: 8080)
        --host HOST           Server host (default: 127.0.0.1)
    -s, --server PATH         Path to kolosal-server executable (auto-detect if not specified)
        --no-server           Don't start LLM server (assume it's already running)
        --demo                Run system demonstration
        --verbose             Enable verbose logging
        --dev, --development  Run in development mode with enhanced debugging
        --prod, --production  Run in production mode with optimizations
        --log-level LEVEL     Set log level (DEBUG, INFO, WARN, ERROR)
        --no-metrics          Disable metrics collection
        --no-health-monitoring Disable health monitoring
    -h, --help                Show this help message
    -v, --version             Show version information

EXAMPLES:
    # Basic usage with default configuration
    )" << program_name << R"(

    # Custom configuration and port
    )" << program_name << R"( -c my_config.yaml -p 9090

    # Development mode with verbose output
    )" << program_name << R"( --dev --verbose --log-level DEBUG

    # Production mode
    )" << program_name << R"( --prod -p 8080

    # Run demonstration
    )" << program_name << R"( --demo

    # Connect to external LLM server
    )" << program_name << R"( --no-server --host external-server.com -p 8080

FEATURES:
    🚀 High-performance LLM inference server
    🤖 Multi-agent coordination and management
    📊 Real-time metrics and monitoring
    🔄 Automatic health checking and recovery
    🌐 REST API for agent management
    ⚙️  Hot configuration reloading
    📈 Performance analytics and optimization

API ENDPOINTS:
    GET    /v1/agents                    - List all agents
    POST   /v1/agents                    - Create new agent
    GET    /v1/agents/{id}               - Get agent details
    PUT    /v1/agents/{id}/start         - Start agent
    PUT    /v1/agents/{id}/stop          - Stop agent
    DELETE /v1/agents/{id}               - Delete agent
    POST   /v1/agents/{id}/execute       - Execute function
    GET    /v1/system/status             - System status
    POST   /v1/system/reload             - Reload configuration

For more information, visit: https://github.com/Evintkoo/kolosal-agent
)";
}

void print_banner() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════╗
║                    🤖 Kolosal Agent System v2.0                  ║
║            Unified LLM Inference & Multi-Agent Platform         ║
╠══════════════════════════════════════════════════════════════════╣
║  🚀 High-Performance    🤖 Multi-Agent    📊 Real-time Analytics ║
║  🔄 Auto-Recovery      🌐 REST API        ⚙️  Hot-Reload Config  ║
╚══════════════════════════════════════════════════════════════════╝
)" << std::endl;
}

void create_default_config_if_missing(const std::string& config_path) {
    if (std::filesystem::exists(config_path)) {
        return;
    }

    std::cout << "📝 Creating default configuration at: " << config_path << std::endl;
    
    std::ofstream config_file(config_path);
    config_file << R"DELIM(# Kolosal Agent System v2.0 Configuration
# Enhanced configuration with modern features and better organization

system:
  name: "Kolosal Multi-Agent System v2.0"
  version: "2.0.0"
  environment: "development"  # development, production, testing
  
  server:
    host: "127.0.0.1"
    port: 8080
    timeout: 30
    enable_cors: true
    allowed_origins: ["*"]
    
  logging:
    level: "INFO"  # DEBUG, INFO, WARN, ERROR
    file: "kolosal_agent_system.log"
    enable_console: true
    max_file_size_mb: 100
    max_backup_files: 5
    
  monitoring:
    enable_health_checks: true
    health_check_interval_seconds: 30
    enable_metrics: true
    enable_performance_analytics: true
    enable_auto_recovery: true
    max_recovery_attempts: 3

# Agent definitions with enhanced capabilities
agents:
  - name: "system_coordinator"
    id: "coord-001"
    type: "coordinator"
    role: "COORDINATOR"
    priority: 1
    
    specializations:
      - "TASK_PLANNING"
      - "RESOURCE_MANAGEMENT"
      - "SYSTEM_MONITORING"
      - "WORKFLOW_ORCHESTRATION"
    
    capabilities:
      - "plan_execution"
      - "task_delegation" 
      - "system_monitoring"
      - "resource_optimization"
      - "error_recovery"
    
    functions:
      - "plan_tasks"
      - "delegate_work"
      - "monitor_progress" 
      - "optimize_resources"
      - "handle_system_events"
    
    config:
      auto_start: true
      max_concurrent_tasks: 10
      memory_limit_mb: 512
      enable_persistence: true
      heartbeat_interval_seconds: 10

  - name: "data_analyst"
    id: "analyst-001"
    type: "specialist"
    role: "ANALYST"
    priority: 2
    
    specializations:
      - "DATA_ANALYSIS"
      - "RESEARCH"
      - "PATTERN_RECOGNITION"
      - "STATISTICAL_MODELING"
    
    capabilities:
      - "data_processing"
      - "research_synthesis"
      - "report_generation"
      - "trend_analysis"
      - "visualization"
    
    functions:
      - "analyze_data"
      - "research_topic"
      - "generate_report"
      - "identify_patterns"
      - "create_visualizations"
    
    config:
      auto_start: true
      max_concurrent_tasks: 5
      memory_limit_mb: 1024
      enable_persistence: true
      specialized_tools: ["python", "pandas", "matplotlib"]

  - name: "task_executor"
    id: "exec-001"
    type: "worker"
    role: "EXECUTOR"
    priority: 3
    
    specializations:
      - "TASK_EXECUTION"
      - "TOOL_USAGE"
      - "FILE_OPERATIONS"
      - "API_INTEGRATION"
    
    capabilities:
      - "execute_commands"
      - "use_tools"
      - "file_operations"
      - "api_calls"
      - "batch_processing"
    
    functions:
      - "execute_task"
      - "use_tool"
      - "process_files"
      - "make_api_call"
      - "batch_execute"
    
    config:
      auto_start: true
      max_concurrent_tasks: 20
      memory_limit_mb: 256
      enable_persistence: false
      timeout_seconds: 300

  - name: "knowledge_manager"
    id: "knowledge-001"
    type: "specialist"
    role: "SPECIALIST"
    priority: 2
    
    specializations:
      - "KNOWLEDGE_MANAGEMENT"
      - "MEMORY_OPERATIONS"
      - "INFORMATION_RETRIEVAL"
      - "CONTENT_CURATION"
    
    capabilities:
      - "knowledge_storage"
      - "information_retrieval"
      - "content_summarization"
      - "semantic_search"
      - "knowledge_graph_operations"
    
    functions:
      - "store_knowledge"
      - "retrieve_information"
      - "summarize_content"
      - "semantic_search"
      - "update_knowledge_graph"
    
    config:
      auto_start: true
      max_concurrent_tasks: 8
      memory_limit_mb: 2048
      enable_persistence: true
      vector_db_enabled: true

# Enhanced function definitions with better metadata
functions:
  - name: "plan_tasks"
    type: "builtin"
    category: "planning"
    description: "Create comprehensive execution plans for complex tasks"
    version: "2.0"
    
    parameters:
      - name: "goal"
        type: "string"
        required: true
        description: "The main objective to achieve"
      - name: "context"
        type: "string"
        required: false
        description: "Additional context and constraints"
      - name: "priority"
        type: "integer"
        required: false
        default: 5
        min: 1
        max: 10
        description: "Task priority (1-10)"
      - name: "deadline"
        type: "datetime"
        required: false
        description: "Task deadline in ISO format"
        
    returns:
      type: "object"
      description: "Execution plan with steps and dependencies"

  - name: "analyze_data"
    type: "builtin"
    category: "analysis"
    description: "Perform comprehensive data analysis with statistical insights"
    version: "2.0"
    
    parameters:
      - name: "data_source"
        type: "string"
        required: true
        description: "Path to data file or dataset identifier"
      - name: "analysis_type"
        type: "string"
        required: false
        default: "comprehensive"
        enum: ["basic", "comprehensive", "statistical", "predictive"]
        description: "Type of analysis to perform"
      - name: "output_format"
        type: "string"
        required: false
        default: "json"
        enum: ["json", "csv", "report", "visualization"]
        description: "Output format for results"
        
    returns:
      type: "object" 
      description: "Analysis results with insights and recommendations"

  - name: "execute_task"
    type: "builtin"
    category: "execution"
    description: "Execute specific tasks with comprehensive error handling"
    version: "2.0"
    
    parameters:
      - name: "task_definition"
        type: "object"
        required: true
        description: "Complete task definition with steps and requirements"
      - name: "execution_mode"
        type: "string"
        required: false
        default: "safe"
        enum: ["safe", "fast", "thorough"]
        description: "Execution mode balancing speed and safety"
      - name: "retry_policy"
        type: "object"
        required: false
        description: "Retry configuration for failed operations"
        
    returns:
      type: "object"
      description: "Execution results with status and output data"

# System-wide templates for quick agent creation
templates:
  basic_worker:
    type: "worker"
    role: "EXECUTOR"
    specializations: ["TASK_EXECUTION"]
    capabilities: ["execute_commands"]
    functions: ["execute_task"]
    config:
      auto_start: false
      max_concurrent_tasks: 5
      
  data_processor:
    type: "specialist"
    role: "ANALYST"
    specializations: ["DATA_ANALYSIS"]
    capabilities: ["data_processing", "report_generation"]
    functions: ["analyze_data", "generate_report"]
    config:
      auto_start: false
      max_concurrent_tasks: 3
      memory_limit_mb: 512

# Integration settings
integration:
  llm_server:
    auto_start: true
    startup_timeout_seconds: 60
    health_check_endpoint: "/v1/health"
    
  external_apis:
    enable_rate_limiting: true
    default_timeout_seconds: 30
    retry_attempts: 3
    
  database:
    enable_persistence: true
    connection_pool_size: 10
    backup_interval_hours: 24
)DELIM";

    config_file.close();
    std::cout << "✅ Default configuration created successfully!" << std::endl;
}

void setup_health_monitoring(UnifiedKolosalServer& server) {
    server.setHealthCheckCallback([](const UnifiedKolosalServer::SystemStatus& status) {
        static auto last_log_time = std::chrono::system_clock::now();
        auto now = std::chrono::system_clock::now();
        
        // Log health status every 5 minutes
        if (now - last_log_time >= std::chrono::minutes(5)) {
            std::cout << "💓 Health Check - LLM: " << (status.llm_server_healthy ? "✅" : "❌")
                      << ", Agents: " << status.running_agents << "/" << status.total_agents
                      << ", Avg Response: " << std::fixed << std::setprecision(1) 
                      << status.average_response_time_ms << "ms" << std::endl;
            last_log_time = now;
        }
        
        // Alert on errors
        if (!status.llm_server_healthy || !status.agent_system_running) {
            std::cout << "⚠️  System Health Alert: " << status.last_error << std::endl;
        }
    });
}

void run_demo_mode(UnifiedKolosalServer& server) {
    std::cout << "\n🎬 Starting System Demonstration" << std::endl;
    std::cout << "=================================" << std::endl;

    auto agent_service = server.getAgentService();
    
    // Show system status
    auto status = server.getSystemStatus();
    std::cout << "\n📊 System Status:" << std::endl;
    std::cout << "  LLM Server: " << (status.llm_server_healthy ? "✅ Healthy" : "❌ Unhealthy") << std::endl;
    std::cout << "  Agents: " << status.running_agents << "/" << status.total_agents << " running" << std::endl;
    
    // List all agents
    auto agents = agent_service->getAllAgentInfo();
    std::cout << "\n🤖 Active Agents:" << std::endl;
    for (const auto& agent : agents) {
        std::cout << "  • " << agent.name << " (" << agent.id << ") - " 
                  << (agent.running ? "✅ Running" : "⏸️  Stopped") << std::endl;
    }
    
    // Get system metrics
    auto metrics = server.getMetrics();
    std::cout << "\n📈 System Metrics:" << std::endl;
    std::cout << "  LLM Requests: " << metrics.total_llm_requests << " (Success: " 
              << metrics.successful_llm_requests << ")" << std::endl;
    std::cout << "  Agent Calls: " << metrics.total_agent_function_calls << " (Success: " 
              << metrics.successful_agent_function_calls << ")" << std::endl;
    
    std::cout << "\n✨ Demonstration complete! System is ready for use." << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        CommandLineArgs args = parse_command_line(argc, argv);
        
        if (args.show_help) {
            print_usage(argv[0]);
            return 0;
        }
        
        if (args.show_version) {
            std::cout << "Kolosal Agent System v2.0.0" << std::endl;
            return 0;
        }
        
        // Set up signal handlers
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
#ifdef _WIN32
        std::signal(SIGBREAK, signal_handler);
#endif

        print_banner();
        
        // Create default configuration if needed
        create_default_config_if_missing(args.config_file);
        
        // Build server configuration
        UnifiedKolosalServer::ServerConfig config;
        
        if (args.production_mode) {
            config = UnifiedServerFactory::buildProductionConfig(args.server_port);
        } else if (args.development_mode) {
            config = UnifiedServerFactory::buildDevelopmentConfig(args.server_port);
        } else {
            config = UnifiedServerFactory::buildDefaultConfig();
        }
        
        // Apply command line overrides
        config.server_host = args.server_host;
        config.server_port = args.server_port;
        config.server_executable_path = args.server_path;
        config.auto_start_server = !args.no_server;
        config.agent_config_file = args.config_file;
        config.enable_health_monitoring = args.enable_health_monitoring;
        config.enable_metrics_collection = args.enable_metrics;
        
        std::cout << "⚙️  Configuration:" << std::endl;
        std::cout << "  • Config File: " << args.config_file << std::endl;
        std::cout << "  • Server: " << config.server_host << ":" << config.server_port << std::endl;
        std::cout << "  • Mode: " << (args.production_mode ? "Production" : 
                                      args.development_mode ? "Development" : "Default") << std::endl;
        std::cout << "  • Auto-start Server: " << (config.auto_start_server ? "Yes" : "No") << std::endl;
        std::cout << std::endl;

        // Create and configure unified server
        unified_server = std::make_unique<UnifiedKolosalServer>(config);
        
        // Set up health monitoring
        if (config.enable_health_monitoring) {
            setup_health_monitoring(*unified_server);
            unified_server->enableAutoRecovery(true);
        }
        
        // Start the unified server
        std::cout << "🚀 Starting Kolosal unified server..." << std::endl;
        if (!unified_server->start()) {
            std::cerr << "❌ Failed to start unified server!" << std::endl;
            return 1;
        }
        
        std::cout << "✅ Unified server started successfully!" << std::endl;
        
        // Run demo if requested
        if (args.demo_mode) {
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Let system stabilize
            run_demo_mode(*unified_server);
        }
        
        // Main event loop
        std::cout << "\n🎯 System is operational!" << std::endl;
        std::cout << "   • LLM Server: http://" << config.server_host << ":" << config.server_port << std::endl;
        std::cout << "   • Agent API: http://" << config.server_host << ":" << config.server_port << "/v1/agents" << std::endl;
        std::cout << "   • System Status: http://" << config.server_host << ":" << config.server_port << "/v1/system/status" << std::endl;
        std::cout << "\n💡 Press Ctrl+C to stop..." << std::endl;
        
        // Event loop with periodic status updates
        auto last_status_update = std::chrono::system_clock::now();
        
        while (keep_running && unified_server->isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Periodic status updates in verbose mode
            if (args.verbose) {
                auto now = std::chrono::system_clock::now();
                if (now - last_status_update >= std::chrono::minutes(1)) {
                    auto status = unified_server->getSystemStatus();
                    std::cout << "📊 Status Update - Agents: " << status.running_agents << "/" 
                              << status.total_agents << ", Response: " 
                              << std::fixed << std::setprecision(1) 
                              << status.average_response_time_ms << "ms" << std::endl;
                    last_status_update = now;
                }
            }
        }
        
        // Graceful shutdown
        std::cout << "\n🛑 Initiating graceful shutdown..." << std::endl;
        
        if (unified_server) {
            unified_server->stop();
            unified_server.reset();
        }
        
        std::cout << "✅ Kolosal Agent System shutdown complete." << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal Error: " << e.what() << std::endl;
        
        if (unified_server) {
            unified_server->stop();
            unified_server.reset();
        }
        
        return 1;
    }
}
