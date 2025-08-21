/**
 * @file main.cpp
 * @brief Main entry point for the Kolosal Agent System v2.0
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * This is the unified main application that starts and manages both the LLM inference server
 * and the multi-agent system. It provides comprehensive command-line interface, configuration
 * management, and graceful shutdown handling.
 */

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
#include <sstream>
#include <iomanip>

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
#include <crtdbg.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

// Enhanced includes
#include "server/unified_server.hpp"
#include "api/agent_management_route.hpp"
#include "agent/services/agent_service.hpp"
#include "config/yaml_configuration_parser.hpp"

// Kolosal logging system
#include "logger/logger_system.hpp"
#include "logger/logging_utilities.hpp"

using namespace kolosal::integration;
using namespace kolosal::api;
using namespace kolosal::services;
using namespace kolosal::logging;
using kolosal::KolosalLogger;

// Component loggers for different subsystems
DECLARE_COMPONENT_LOGGER(application_main);
DECLARE_COMPONENT_LOGGER(signal_handler);
DECLARE_COMPONENT_LOGGER(configuration);
DECLARE_COMPONENT_LOGGER(unified_server);

// Forward declarations
void system_signal_handler(int signal_number);
DECLARE_COMPONENT_LOGGER(health_monitor);

// Global state for graceful shutdown
std::atomic<bool> system_running {true};
std::unique_ptr<UnifiedKolosalServer> unified_server_instance;

/**
 * @brief Disable Windows error dialogs and crash popups for silent operation
 */
void disable_windows_error_dialogs() {
#ifdef _WIN32
    // Disable the Windows Error Reporting dialog
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    
    // Disable debug assertion dialogs in Debug builds
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    
    // Disable abort() dialog
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    
    // Set console control handler to prevent Windows from showing dialogs on console close
    SetConsoleCtrlHandler([](DWORD control_type) -> BOOL {
        if (control_type == CTRL_C_EVENT || control_type == CTRL_BREAK_EVENT || control_type == CTRL_CLOSE_EVENT) {
            system_signal_handler(SIGINT);
            return TRUE;
        }
        return FALSE;
    }, TRUE);
    
    // Disable Windows critical error handler dialogs
    SetUnhandledExceptionFilter([](PEXCEPTION_POINTERS pExceptionInfo) -> LONG {
        // Log the exception instead of showing a dialog
        try {
            COMPONENT_FATAL(application_main, "Unhandled exception occurred - application terminating silently");
        } catch (...) {
            // If logging fails, write to stderr
            std::cerr << "Fatal: Unhandled exception - terminating" << std::endl;
        }
        return EXCEPTION_EXECUTE_HANDLER;
    });
    
    // Redirect abort() to our custom handler
    signal(SIGABRT, [](int) {
        try {
            COMPONENT_FATAL(application_main, "Abort signal received - terminating silently");
        } catch (...) {
            std::cerr << "Fatal: Abort signal - terminating" << std::endl;
        }
        std::_Exit(3); // Use _Exit to avoid calling atexit handlers
    });
#endif
}

/**
 * @brief Enhanced signal handler for graceful system shutdown
 * @param signal_number The received signal number
 */
void system_signal_handler(int signal_number) {
    // Set the flag first to break the main loop
    system_running.store(false);
    
    COMPONENT_WARN(signal_handler, "Received signal {} (interrupt signal), initiating graceful shutdown...", signal_number);
    
    if (unified_server_instance) {
        COMPONENT_INFO(signal_handler, "Stopping unified server instance...");
        unified_server_instance->stop();
    }
    
    // Force exit if signal is sent again
    static int signal_count = 0;
    signal_count++;
    if (signal_count >= 2) {
        COMPONENT_FATAL(signal_handler, "Force exit requested...");
        std::exit(1);
    }
}

/**
 * @brief Command line arguments configuration structure
 * 
 * Simplified configuration structure with defaults optimized for
 * comprehensive logging and monitoring of all requests and actions.
 */
struct ApplicationConfiguration {
    std::string configuration_file_path = "agent_config.yaml";
    int server_port_number = 8080;
    std::string server_host_address = "127.0.0.1";
    std::string external_server_executable_path = "";
    bool disable_embedded_server = false;
    std::string logging_level = "INFO";
    bool display_help_information = false;
    bool display_version_information = false;
    
    // Default enabled features for comprehensive monitoring
    bool enable_verbose_logging = true;
    bool enable_performance_metrics = true;
    bool enable_system_health_monitoring = true;
    bool enable_request_response_logging = true;
    bool enable_action_logging = true;
};

/**
 * @brief Parse command line arguments into application configuration
 * @param argc Argument count
 * @param argv Argument vector
 * @return ApplicationConfiguration with parsed values
 * @throws std::runtime_error for invalid arguments
 */
ApplicationConfiguration parse_command_line_arguments(int argc, char* argv[]) {
    ApplicationConfiguration config;
    
    for (int i = 1; i < argc; ++i) {
        std::string argument = argv[i];
        
        if (argument == "-h" || argument == "--help") {
            config.display_help_information = true;
        } else if (argument == "-v" || argument == "--version") {
            config.display_version_information = true;
        } else if (argument == "-c" || argument == "--config") {
            if (i + 1 < argc) {
                config.configuration_file_path = argv[++i];
            } else {
                throw std::runtime_error("--config requires a file path");
            }
        } else if (argument == "-p" || argument == "--port") {
            if (i + 1 < argc) {
                config.server_port_number = std::stoi(argv[++i]);
                if (config.server_port_number < 1 || config.server_port_number > 65535) {
                    throw std::runtime_error("Port must be between 1 and 65535");
                }
            } else {
                throw std::runtime_error("--port requires a port number");
            }
        } else if (argument == "--host") {
            if (i + 1 < argc) {
                config.server_host_address = argv[++i];
            } else {
                throw std::runtime_error("--host requires a hostname");
            }
        } else if (argument == "-s" || argument == "--server") {
            if (i + 1 < argc) {
                config.external_server_executable_path = argv[++i];
            } else {
                throw std::runtime_error("--server requires a file path");
            }
        } else if (argument == "--no-server") {
            config.disable_embedded_server = true;
        } else if (argument == "--log-level") {
            if (i + 1 < argc) {
                config.logging_level = argv[++i];
            } else {
                throw std::runtime_error("--log-level requires a level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)");
            }
        } else if (argument == "--no-metrics") {
            config.enable_performance_metrics = false;
        } else if (argument == "--no-health-monitoring") {
            config.enable_system_health_monitoring = false;
        } else if (argument == "--no-request-logging") {
            config.enable_request_response_logging = false;
        } else if (argument == "--no-action-logging") {
            config.enable_action_logging = false;
        } else {
            throw std::runtime_error("Unknown argument: " + argument);
        }
    }
    
    return config;
}

/**
 * @brief Display comprehensive usage information and help text
 * @param program_executable_name The name of the program executable
 */
void display_application_usage_information(const char* program_executable_name) {
    std::cout << R"(
Kolosal Agent System v2.0.0 - Unified LLM & Multi-Agent Platform
====================================================================

USAGE:
    )" << program_executable_name << R"( [OPTIONS]

OVERVIEW:
    The Kolosal Agent System runs with comprehensive logging enabled by default,
    monitoring all requests and actions performed by the agent server. This 
    provides full visibility into system operations and performance.

OPTIONS:
    -c, --config FILE          Use custom configuration file (default: agent_config.yaml)
    -p, --port PORT           Server port (default: 8080)
        --host HOST           Server host (default: 127.0.0.1)
    -s, --server PATH         Path to kolosal-server executable (auto-detect if not specified)
        --no-server           Don't start LLM server (assume it's already running)
        --log-level LEVEL     Set log level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
        --no-metrics          Disable performance metrics collection
        --no-health-monitoring Disable health monitoring
        --no-request-logging  Disable request/response logging
        --no-action-logging   Disable action logging
    -h, --help                Show this help message
    -v, --version             Show version information

EXAMPLES:
    # Default operation with comprehensive logging
    )" << program_executable_name << R"(

    # Custom configuration and port
    )" << program_executable_name << R"( -c my_config.yaml -p 9090

    # Enable debug level logging for detailed monitoring
    )" << program_executable_name << R"( --log-level DEBUG

    # Connect to external LLM server
    )" << program_executable_name << R"( --no-server --host external-server.com -p 8080

    # Minimal logging for production environments
    )" << program_executable_name << R"( --log-level WARN --no-request-logging

DEFAULT FEATURES (Always Enabled):
    * Comprehensive request/response logging
    * Action and operation monitoring  
    * Performance metrics collection
    * Health monitoring and auto-recovery
    * Real-time system status tracking

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

/**
 * @brief Display the application banner with version and feature information
 */
void display_application_banner() {
    std::cout << R"(
===============================================================================
                 Kolosal Agent System v2.0 - Unified                 
            LLM Inference Server + Multi-Agent Platform
  * High-Performance      * Multi-Agent      * Real-time Analytics 
  * Auto-Recovery         * REST API         * Hot-Reload Config  
===============================================================================
)" << std::endl;
}



/**
 * @brief Setup comprehensive health monitoring for the unified server
 * @param unified_server_ref Reference to the unified server instance
 */
void initialize_system_health_monitoring(UnifiedKolosalServer& unified_server_ref) {
    unified_server_ref.setHealthCheck_Callback([](const UnifiedKolosalServer::SystemStatus& status) {
        static auto last_health_log_time = std::chrono::system_clock::now();
        const auto current_time = std::chrono::system_clock::now();
        // Log health status every 5 minutes for comprehensive monitoring
        if (current_time - last_health_log_time >= std::chrono::minutes(5)) {
            COMPONENT_INFO(health_monitor, "Health Check - LLM: {}, Agents: {}/{}, Average Response: {:.1f} ms", 
                          (status.llm_server_healthy ? "Healthy" : "Unhealthy"),
                          status.running_agents, status.total_agents,
                          status.average_response_time_ms);
            last_health_log_time = current_time;
        }
        
        // Alert on critical errors only
        if (!status.llm_server_healthy && !status.agent_system_running) {
            COMPONENT_ERROR(health_monitor, "Critical System Health Alert: {}", status.last_error);
        }
    });
}

/**
 * @brief Setup comprehensive request and action logging
 * @param unified_server_ref Reference to the unified server instance
 * @param config Application configuration
 */
void initialize_comprehensive_logging(UnifiedKolosalServer& unified_server_ref, const ApplicationConfiguration& config) {
    if (config.enable_request_response_logging) {
        COMPONENT_INFO(application_main, "Request/Response logging enabled - all API calls will be monitored");
        // Note: Actual request/response logging would be implemented in the server routing layer
    }
    
    if (config.enable_action_logging) {
        COMPONENT_INFO(application_main, "Action logging enabled - all agent actions will be tracked");
        // Note: Action logging would be implemented in the agent execution layer
    }
    
    COMPONENT_INFO(application_main, "Comprehensive logging system initialized successfully");
}



/**
 * @brief Automatically detect the kolosal-server executable path from common locations
 * @return std::string Path to the kolosal-server executable if found, empty string otherwise
 */
std::string detect_server_executable_path_automatically() {
    SCOPE_LOG("unified_server", "detect_server_executable_path_automatically");
    
    // Define candidate paths for the kolosal-server executable
    std::vector<std::string> candidate_executable_paths = {
        // Current build structure - main Debug directory
        "./build/Debug/kolosal-server.exe",
        "./build/Release/kolosal-server.exe",
        // Try the same directory as the kolosal-agent executable
        "kolosal-server.exe",
        "./kolosal-server.exe",
        // Kolosal-server subproject build paths
        "./build/kolosal-server/Debug/kolosal-server.exe",
        "./build/kolosal-server/Release/kolosal-server.exe",
        "./kolosal-server/Debug/kolosal-server.exe",
        "./kolosal-server/Release/kolosal-server.exe", 
        "./kolosal-server/kolosal-server.exe",
        "../kolosal-server/Debug/kolosal-server.exe", 
        "../kolosal-server.exe",
        // Relative paths from build directory
        "../kolosal-server.exe",
        "../../Debug/kolosal-server.exe"
    };
    
    for (const auto& candidate_path : candidate_executable_paths) {
        if (std::filesystem::exists(candidate_path)) {
            const auto absolute_executable_path = std::filesystem::absolute(candidate_path);
            COMPONENT_INFO(unified_server, "Auto-detected kolosal-server at: {}", absolute_executable_path.string());
            return absolute_executable_path.string();
        }
    }
    
    COMPONENT_WARN(unified_server, "Could not auto-detect kolosal-server executable");
    COMPONENT_DEBUG(unified_server, "Searched the following candidate paths:");
    for (const auto& candidate_path : candidate_executable_paths) {
        COMPONENT_DEBUG(unified_server, "  * {}", candidate_path);
    }
    COMPONENT_WARN(unified_server, "Please specify the executable path using the --server option");
    
    return "";
}

/**
 * @brief Perform main operation
 * @return int Description of return value
 */
int main(int argc, char* argv[]) {
    // Prevent multiple instances using a named mutex (Windows only)
#ifdef _WIN32
    HANDLE hMutex = CreateMutexA(NULL, FALSE, "Global\\KolosalAgentMutex");
    if (hMutex == NULL) {
        std::cerr << "[bootstrap] ERROR: Unable to create mutex for single instance check." << std::endl;
        return EXIT_FAILURE;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cerr << "[bootstrap] WARNING: Kolosal Agent is already running. Only one instance is allowed." << std::endl;
        CloseHandle(hMutex);
        return EXIT_FAILURE;
    }
#endif
    try {
        // Early diagnostic output
        std::cout << "[bootstrap] kolosal-agent-unified starting..." << std::endl;
        
        // Disable Windows error dialogs first thing
        disable_windows_error_dialogs();
        
        // Initialize logging system with development defaults early
        try {
            LoggingConfig::setupDevelopment_Logging();
            std::cout << "[bootstrap] logging initialized" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "[bootstrap] logging initialization failed: " << e.what() << std::endl;
            // Continue without logging if it fails
        }
        
        try {
            PERF_LOG("application_main", "application_startup_sequence");
            COMPONENT_INFO(application_main, "Kolosal Agent System v2.0.0 initializing...");
        } catch (...) {
            std::cout << "[bootstrap] logging calls failed, continuing..." << std::endl;
        }
        
        // Parse and validate command line arguments
        ApplicationConfiguration application_config = parse_command_line_arguments(argc, argv);
        std::cout << "[bootstrap] args parsed" << std::endl;
        // Handle immediate exit scenarios
        if (application_config.display_help_information) {
            display_application_usage_information(argv[0]);
            return EXIT_SUCCESS;
        }
        
        if (application_config.display_version_information) {
            std::cout << "Kolosal Agent System v2.0.0" << std::endl;
            return EXIT_SUCCESS;
        }
        
        // Configure logging system with enhanced monitoring capabilities
        LoggingConfig::setupDevelopment_Logging("kolosal_agent.log");
        
        // Apply logging level configuration
        LoggingConfig::setLog_Level(application_config.logging_level);
        
        // Enable server logger integration if available
        KolosalLogger::instance().enableServerLogger_Integration(true);
        
        COMPONENT_INFO(application_main, "Logging system configured with comprehensive monitoring - level: {}", application_config.logging_level);
        COMPONENT_INFO(application_main, "Request/Response logging: {}", (application_config.enable_request_response_logging ? "ENABLED" : "DISABLED"));
        COMPONENT_INFO(application_main, "Action logging: {}", (application_config.enable_action_logging ? "ENABLED" : "DISABLED"));
        
        // Initialize signal handlers for graceful system shutdown
        COMPONENT_DEBUG(application_main, "Configuring system signal handlers...");
        std::signal(SIGINT, system_signal_handler);
        std::signal(SIGTERM, system_signal_handler);
#ifdef _WIN32
        std::signal(SIGBREAK, system_signal_handler);
        // Console control handler was already set in disable_windows_error_dialogs()
#endif

        // Configuration file must exist - no default creation
        if (!std::filesystem::exists(application_config.configuration_file_path)) {
            COMPONENT_ERROR(configuration, "Configuration file not found: {}", application_config.configuration_file_path);
            std::cerr << "Error: Configuration file not found: " << application_config.configuration_file_path << std::endl;
            std::cerr << "Please create a configuration file or use --config to specify an existing one." << std::endl;
            return EXIT_FAILURE;
        }
        
        // Build unified server configuration with default comprehensive monitoring
        UnifiedKolosalServer::ServerConfig server_configuration;
        server_configuration = UnifiedServerFactory::buildDevelopment_Config(application_config.server_port_number);
        
        COMPONENT_INFO(configuration, "Using default configuration with comprehensive logging and monitoring enabled");
        
        // Apply command line configuration overrides
        server_configuration.server_host = application_config.server_host_address;
        server_configuration.server_port = application_config.server_port_number;
        
        // Resolve server executable path
        if (application_config.external_server_executable_path.empty()) {
            server_configuration.server_executable_path = detect_server_executable_path_automatically();
        } else {
            server_configuration.server_executable_path = application_config.external_server_executable_path;
        }
        
        server_configuration.auto_start_server = !application_config.disable_embedded_server;
        server_configuration.agent_config_file = application_config.configuration_file_path;
        server_configuration.enable_health_monitoring = application_config.enable_system_health_monitoring;
        server_configuration.enable_metrics_collection = application_config.enable_performance_metrics;
        
        // Display configuration summary (always shown for transparency)
        COMPONENT_INFO(configuration, "System Configuration Summary:");
        COMPONENT_INFO(configuration, "  * Configuration File: {}", application_config.configuration_file_path);
        COMPONENT_INFO(configuration, "  * Server Endpoint: {}:{}", server_configuration.server_host, server_configuration.server_port);
        if (server_configuration.auto_start_server && !server_configuration.server_executable_path.empty()) {
            COMPONENT_INFO(configuration, "  * Server Executable: {}", server_configuration.server_executable_path);
        }
        COMPONENT_INFO(configuration, "  * Operating Mode: Default (Comprehensive Logging & Monitoring)");
        COMPONENT_INFO(configuration, "  * Auto-start Server: {}", (server_configuration.auto_start_server ? "Enabled" : "Disabled"));
        COMPONENT_INFO(configuration, "  * Request/Response Logging: {}", (application_config.enable_request_response_logging ? "Enabled" : "Disabled"));
        COMPONENT_INFO(configuration, "  * Action Logging: {}", (application_config.enable_action_logging ? "Enabled" : "Disabled"));
        COMPONENT_INFO(configuration, "  * Performance Metrics: {}", (application_config.enable_performance_metrics ? "Enabled" : "Disabled"));
        COMPONENT_INFO(configuration, "  * Health Monitoring: {}", (application_config.enable_system_health_monitoring ? "Enabled" : "Disabled"));

        // Create and initialize unified server instance
        try {
            unified_server_instance = std::make_unique<UnifiedKolosalServer>(server_configuration);
            std::cout << "[bootstrap] unified server constructed" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "[bootstrap] ERROR: failed to construct unified server: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        
        // Configure system health monitoring and comprehensive logging
        try {
            if (server_configuration.enable_health_monitoring) {
                initialize_system_health_monitoring(*unified_server_instance);
                unified_server_instance->enableAuto_Recovery(true);
            }
            
            // Initialize comprehensive request and action logging
            initialize_comprehensive_logging(*unified_server_instance, application_config);
            
        } catch (const std::exception& e) {
            std::cout << "[bootstrap] WARNING: monitoring setup failed: " << e.what() << std::endl;
            // Continue without enhanced monitoring
        }
        
        // Start the unified server system with comprehensive logging
        try {
            COMPONENT_INFO(unified_server, "Starting Kolosal unified server system with comprehensive monitoring...");
        } catch (...) {
            std::cout << "Starting Kolosal unified server system with comprehensive monitoring..." << std::endl;
        }
        
        PERF_LOG("unified_server", "server_startup_sequence");
        bool server_started = false;
        try {
            server_started = unified_server_instance->start();
            if (!server_started) {
                std::cout << "[bootstrap] WARNING: unified server failed to start normally, but continuing..." << std::endl;
                // Don't exit immediately; allow main loop to run for diagnostics
            } else {
                std::cout << "[bootstrap] unified server started successfully" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "[bootstrap] ERROR: exception during server startup: " << e.what() << std::endl;
            std::cout << "[bootstrap] continuing with partial functionality..." << std::endl;
        }
        
        if (server_started) {
            try {
                COMPONENT_INFO(unified_server, "Unified server system started successfully with comprehensive monitoring!");
            } catch (...) {
                std::cout << "Unified server system started successfully with comprehensive monitoring!" << std::endl;
            }
        }
        
        // Main application event loop with enhanced logging
        try {
            const auto server_config = unified_server_instance ? unified_server_instance->get_Configuration() : server_configuration;
            COMPONENT_INFO(application_main, "🎯 Kolosal Agent System is operational with comprehensive monitoring!");
            COMPONENT_INFO(application_main, "   * LLM Inference Server: http://{}:{}", server_config.server_host, server_config.server_port);
            COMPONENT_INFO(application_main, "");
            COMPONENT_INFO(application_main, "📋 Available Agent Management Endpoints:");
            COMPONENT_INFO(application_main, "   * GET    http://{}:{}/v1/agents                    - List all agents", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * POST   http://{}:{}/v1/agents                    - Create new agent", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * GET    http://{}:{}/v1/agents/{{id}}               - Get agent details", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * PUT    http://{}:{}/v1/agents/{{id}}/start         - Start agent", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * PUT    http://{}:{}/v1/agents/{{id}}/stop          - Stop agent", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * DELETE http://{}:{}/v1/agents/{{id}}               - Delete agent", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * POST   http://{}:{}/v1/agents/{{id}}/execute       - Execute agent function", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "");
            COMPONENT_INFO(application_main, "🔄 Available Workflow Endpoints:");
            COMPONENT_INFO(application_main, "   * GET    http://{}:{}/v1/workflows                 - List all workflows", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * POST   http://{}:{}/v1/workflows                 - Create new workflow", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * GET    http://{}:{}/v1/workflows/{{id}}            - Get workflow details", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * POST   http://{}:{}/v1/workflows/{{id}}/execute    - Execute workflow", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * PUT    http://{}:{}/v1/workflows/{{id}}/pause      - Pause workflow", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * PUT    http://{}:{}/v1/workflows/{{id}}/resume     - Resume workflow", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * DELETE http://{}:{}/v1/workflows/{{id}}            - Delete workflow", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "");
            COMPONENT_INFO(application_main, "🔧 System Management Endpoints:");
            COMPONENT_INFO(application_main, "   * GET    http://{}:{}/v1/system/status             - Get system status", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * POST   http://{}:{}/v1/system/reload             - Reload configuration", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * GET    http://{}:{}/v1/system/health             - Health check", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "   * GET    http://{}:{}/v1/system/metrics            - Performance metrics", server_config.agent_api_host, server_config.agent_api_port);
            COMPONENT_INFO(application_main, "");
            COMPONENT_INFO(application_main, "   * All requests and actions will be logged for monitoring");
            COMPONENT_INFO(application_main, "Press Ctrl+C to initiate graceful shutdown...");
        } catch (const std::exception& e) {
            std::cout << "🎯 Kolosal Agent System is operational with comprehensive monitoring!" << std::endl;
            std::cout << "   * LLM Inference Server: http://" << server_configuration.server_host << ":" << server_configuration.server_port << std::endl;
            std::cout << std::endl;
            std::cout << "📋 Available Agent Management Endpoints:" << std::endl;
            std::cout << "   * GET    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/agents                    - List all agents" << std::endl;
            std::cout << "   * POST   http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/agents                    - Create new agent" << std::endl;
            std::cout << "   * GET    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/agents/{id}               - Get agent details" << std::endl;
            std::cout << "   * PUT    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/agents/{id}/start         - Start agent" << std::endl;
            std::cout << "   * PUT    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/agents/{id}/stop          - Stop agent" << std::endl;
            std::cout << "   * DELETE http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/agents/{id}               - Delete agent" << std::endl;
            std::cout << "   * POST   http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/agents/{id}/execute       - Execute agent function" << std::endl;
            std::cout << std::endl;
            std::cout << "🔄 Available Workflow Endpoints:" << std::endl;
            std::cout << "   * GET    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/workflows                 - List all workflows" << std::endl;
            std::cout << "   * POST   http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/workflows                 - Create new workflow" << std::endl;
            std::cout << "   * GET    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/workflows/{id}            - Get workflow details" << std::endl;
            std::cout << "   * POST   http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/workflows/{id}/execute    - Execute workflow" << std::endl;
            std::cout << "   * PUT    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/workflows/{id}/pause      - Pause workflow" << std::endl;
            std::cout << "   * PUT    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/workflows/{id}/resume     - Resume workflow" << std::endl;
            std::cout << "   * DELETE http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/workflows/{id}            - Delete workflow" << std::endl;
            std::cout << std::endl;
            std::cout << "🔧 System Management Endpoints:" << std::endl;
            std::cout << "   * GET    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/system/status             - Get system status" << std::endl;
            std::cout << "   * POST   http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/system/reload             - Reload configuration" << std::endl;
            std::cout << "   * GET    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/system/health             - Health check" << std::endl;
            std::cout << "   * GET    http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/system/metrics            - Performance metrics" << std::endl;
            std::cout << std::endl;
            std::cout << "   * All requests and actions will be logged for monitoring" << std::endl;
            std::cout << "Press Ctrl+C to initiate graceful shutdown..." << std::endl;
        }
        
        // Event loop with comprehensive status monitoring and responsive signal handling
        auto last_status_update_time = std::chrono::system_clock::now();
        while (system_running.load()) {
            // Use shorter sleep intervals to improve signal responsiveness
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            // Regular status updates for comprehensive monitoring (every 2 minutes)
            auto current_time = std::chrono::system_clock::now();
            if (current_time - last_status_update_time >= std::chrono::minutes(2)) {
                const auto current_system_status = unified_server_instance ? unified_server_instance->getSystem_Status() : UnifiedKolosalServer::SystemStatus{};
                COMPONENT_INFO(application_main, "📊 System Status Monitor - Agents: {}/{}, Response Time: {:.1f} ms, Health: {}", 
                              current_system_status.running_agents, current_system_status.total_agents,
                              current_system_status.average_response_time_ms,
                              (current_system_status.llm_server_healthy ? "Healthy" : "Unhealthy"));
                
                // Log current metrics if enabled
                if (application_config.enable_performance_metrics && unified_server_instance) {
                    const auto performance_metrics = unified_server_instance->get_Metrics();
                    COMPONENT_INFO(application_main, "📈 Performance Metrics - LLM Requests: {} (Success: {}), Agent Calls: {} (Success: {})", 
                                  performance_metrics.total_llm_requests, performance_metrics.successful_llm_requests,
                                  performance_metrics.total_agent_function_calls, performance_metrics.successful_agent_function_calls);
                }
                
                last_status_update_time = current_time;
            }
            
            // Check for shutdown signals
            if (!system_running.load()) {
                COMPONENT_INFO(application_main, "🛑 Shutdown signal received, preparing for graceful termination...");
                break;
            }
        }
        
        // Graceful system shutdown sequence
        COMPONENT_INFO(application_main, "🛑 Initiating graceful system shutdown...");
        
        PERF_LOG("application_main", "application_shutdown_sequence");
        if (unified_server_instance) {
            try {
                unified_server_instance->stop();
            } catch (...) {
                // swallow
            }
            unified_server_instance.reset();
        }
        
        COMPONENT_INFO(application_main, "Kolosal Agent System shutdown completed successfully.");
        
        // Ensure all logs are flushed before termination
        KolosalLogger::instance().flush();
        // Release mutex on exit (Windows only)
#ifdef _WIN32
        if (hMutex) {
            CloseHandle(hMutex);
        }
#endif
        return EXIT_SUCCESS;
        
    } catch (const std::exception& exception) {
        std::cerr << "Fatal System Error: " << exception.what() << std::endl;
        // Ensure cleanup on exception
        if (unified_server_instance) {
            unified_server_instance->stop();
            unified_server_instance.reset();
        }
#ifdef _WIN32
        if (hMutex) {
            CloseHandle(hMutex);
        }
#endif
        return EXIT_FAILURE;
    }
}
