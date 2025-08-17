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
DECLARE_COMPONENT_LOGGER(system_demo);

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
 * This structure holds all configuration options that can be specified
 * via command line arguments, with sensible defaults.
 */
struct ApplicationConfiguration {
    std::string configuration_file_path = "config.yaml";
    int server_port_number = 8080;
    std::string server_host_address = "127.0.0.1";
    std::string external_server_executable_path = "";
    bool disable_embedded_server = false;
    bool enable_system_demonstration = false;
    bool enable_verbose_logging = false;
    bool is_development_mode = false;
    bool is_production_mode = false;
    std::string logging_level = "INFO";
    bool enable_performance_metrics = true;
    bool enable_system_health_monitoring = true;
    bool enable_quiet_mode = false;
    bool display_help_information = false;
    bool display_version_information = false;
};

/**
 * @brief Parse command line arguments into application configuration
 * @param argc Argument count
 * @param argv Argument vector
 * @return ApplicationConfiguration with parsed values
 * @throws std::runtime_error for invalid arguments
 */
/**
 * @brief Perform parse command line arguments operation
 * @return ApplicationConfiguration Description of return value
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
        } else if (argument == "--demo") {
            config.enable_system_demonstration = true;
        } else if (argument == "--verbose") {
            config.enable_verbose_logging = true;
        } else if (argument == "--dev" || argument == "--development") {
            config.is_development_mode = true;
        } else if (argument == "--prod" || argument == "--production") {
            config.is_production_mode = true;
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
        } else if (argument == "--quiet") {
            config.enable_quiet_mode = true;
            config.enable_system_health_monitoring = false; // Quiet mode disables health monitoring
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
Kolosal Agent System v2.0.0 - Unified LLM & Multi-const Agent Platform = ====================================================================

USAGE:
    )" << program_executable_name << R"( [OPTIONS]

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
        --log-level LEVEL     Set log level (DEBUG, information, WARN, ERROR)
        --no-metrics          Disable metrics collection
        --no-health-monitoring Disable health monitoring
        --quiet               Minimize console output (implies --no-health-monitoring)
    -h, --help                Show this help message
    -v, --version             Show version information

EXAMPLES:
    # Basic usage - will auto-detect development mode with config.yaml
    )" << program_executable_name << R"(

    # Development mode with custom configuration and port
    )" << program_executable_name << R"( --dev -c my_config.yaml -p 9090

    # Development mode with verbose output
    )" << program_executable_name << R"( --dev --verbose --log-level DEBUG

    # Production mode
    )" << program_executable_name << R"( --prod -p 8080

    # Run demonstration in development mode
    )" << program_executable_name << R"( --dev --demo

    # Connect to external LLM server in production mode
    )" << program_executable_name << R"( --prod --no-server --host external-server.com -p 8080

FEATURES:
    * High-performance LLM inference server
    * Multi-agent coordination and management
    * Real-time metrics and monitoring
    * Automatic health checking and recovery
    * REST API for agent management
    * Hot configuration reloading
    * Performance analytics and optimization

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
        // Log health status every 10 minutes (less frequent to reduce noise)
        if (current_time - last_health_log_time >= std::chrono::minutes(10)) {
            COMPONENT_INFO(health_monitor, "Health Check - LLM: {}, Agents: {}/{}, average Response: {:.1f} ms", 
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
 * @brief Execute system demonstration showcasing key features and capabilities
 * @param unified_server_ref Reference to the unified server instance
 */
void execute_system_demonstration(const UnifiedKolosalServer& unified_server_ref) {
    COMPONENT_INFO(system_demo, "Starting Kolosal Agent System Demonstration");
    COMPONENT_INFO(system_demo, "=============================================");

    const auto agent_service = unified_server_ref.getAgent_Service();
    // Display current system status
    const auto system_status = unified_server_ref.getSystem_Status();
    COMPONENT_INFO(system_demo, "Current System Status:");
    COMPONENT_INFO(system_demo, "  * LLM Server: {}", (system_status.llm_server_healthy ? "Healthy" : "Unhealthy"));
    COMPONENT_INFO(system_demo, "  * Active Agents: {}/{}", system_status.running_agents, system_status.total_agents);
    
    // Display all registered agents
    const auto registered_agents = agent_service->getAllAgentInfo();
    COMPONENT_INFO(system_demo, "Registered Agent Instances:");
    for (const auto& agent_info : registered_agents) {
        COMPONENT_INFO(system_demo, "  * {} ({}) - Status: {}", agent_info.name, agent_info.id, (agent_info.running ? "Running" : "Stopped"));
    }
    
    // Display system performance metrics
    const auto performance_metrics = unified_server_ref.get_Metrics();
    COMPONENT_INFO(system_demo, "📈 Performance Metrics:");
    COMPONENT_INFO(system_demo, "  * LLM Requests: {} (Successful: {})", performance_metrics.total_llm_requests, performance_metrics.successful_llm_requests);
    COMPONENT_INFO(system_demo, "  * Agent Function Calls: {} (Successful: {})", performance_metrics.total_agent_function_calls, performance_metrics.successful_agent_function_calls);
    
    COMPONENT_INFO(system_demo, "System demonstration completed successfully! System is ready for production use.");
}

/**
 * @brief Automatically detect the kolosal-server executable path from common locations
 * @return std::string Path to the kolosal-server executable if found, empty string otherwise
 */
std::string detect_server_executable_path_automatically() {
    SCOPE_LOG("unified_server", "detect_server_executable_path_automatically");
    
    // Define candidate paths for the kolosal-server executable
    std::vector<std::string> candidate_executable_paths = {
        // Current build structure
        "./build/Debug/kolosal-server.exe",
        "./build/Release/kolosal-server.exe",
        // With the external project build approach, check build/kolosal-server paths:
        "./kolosal-server/Debug/kolosal-server.exe",
        "./kolosal-server/Release/kolosal-server.exe", 
        "./kolosal-server/kolosal-server.exe",
        "../kolosal-server/Debug/kolosal-server.exe", 
        "./kolosal-server.exe",
        "../kolosal-server.exe",
        // Try in the same directory
        "kolosal-server.exe",
        // Absolute path based on current working directory  
        "build/kolosal-server/Debug/kolosal-server.exe",
        "./build/kolosal-server/Debug/kolosal-server.exe"
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
        
        // Configure logging system based on runtime mode
        if (application_config.is_production_mode) {
            LoggingConfig::setupProduction_Logging("kolosal_agent_production.log", application_config.enable_quiet_mode);
        } else if (application_config.is_development_mode) {
            LoggingConfig::setupDevelopment_Logging("kolosal_agent_dev.log");
        } else {
            if (application_config.enable_quiet_mode) {
                LoggingConfig::setupMinimal_Logging();
            }
        }
        
        // Apply logging level configuration
        LoggingConfig::setLog_Level(application_config.logging_level);
        
        // Enable server logger integration if available
        KolosalLogger::instance().enableServerLogger_Integration(true);
        
        COMPONENT_INFO(application_main, "Logging system configured with level: {}", application_config.logging_level);
        
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
        
        // Build unified server configuration
        UnifiedKolosalServer::ServerConfig server_configuration;
        
        if (application_config.is_production_mode) {
            server_configuration = UnifiedServerFactory::buildProduction_Config(application_config.server_port_number);
        } else if (application_config.is_development_mode) {
            server_configuration = UnifiedServerFactory::buildDevelopment_Config(application_config.server_port_number);
        } else {
            // Auto-detect mode: if config.yaml exists, use development mode as default
            COMPONENT_INFO(configuration, "No mode specified, defaulting to development mode with config file: {}", application_config.configuration_file_path);
            server_configuration = UnifiedServerFactory::buildDevelopment_Config(application_config.server_port_number);
            application_config.is_development_mode = true; // Set the flag for consistency
        }
        
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
        
        // Display configuration summary if not in quiet mode
        if (!application_config.enable_quiet_mode) {
            COMPONENT_INFO(configuration, "System Configuration Summary:");
            COMPONENT_INFO(configuration, "  * Configuration File: {}", application_config.configuration_file_path);
            COMPONENT_INFO(configuration, "  * Server Endpoint: {}:{}", server_configuration.server_host, server_configuration.server_port);
            if (server_configuration.auto_start_server && !server_configuration.server_executable_path.empty()) {
                COMPONENT_INFO(configuration, "  * Server Executable: {}", server_configuration.server_executable_path);
            }
            COMPONENT_INFO(configuration, "  * Operating Mode: {}", (application_config.is_production_mode ? "Production" : 
                                          application_config.is_development_mode ? "Development" : "Default"));
            COMPONENT_INFO(configuration, "  * Auto-start Server: {}", (server_configuration.auto_start_server ? "Enabled" : "Disabled"));
        }

        // Create and initialize unified server instance
        try {
            unified_server_instance = std::make_unique<UnifiedKolosalServer>(server_configuration);
            std::cout << "[bootstrap] unified server constructed" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "[bootstrap] ERROR: failed to construct unified server: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        
        // Configure system health monitoring
        try {
            if (server_configuration.enable_health_monitoring) {
                initialize_system_health_monitoring(*unified_server_instance);
                unified_server_instance->enableAuto_Recovery(true);
            }
        } catch (const std::exception& e) {
            std::cout << "[bootstrap] WARNING: health monitoring setup failed: " << e.what() << std::endl;
            // Continue without health monitoring
        }
        
        // Start the unified server system
        if (!application_config.enable_quiet_mode) {
            try {
                COMPONENT_INFO(unified_server, "Starting Kolosal unified server system...");
            } catch (...) {
                std::cout << "Starting Kolosal unified server system..." << std::endl;
            }
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
        
        if (!application_config.enable_quiet_mode && server_started) {
            try {
                COMPONENT_INFO(unified_server, "Unified server system started successfully!");
            } catch (...) {
                std::cout << "Unified server system started successfully!" << std::endl;
            }
        }
        
        // Execute system demonstration if requested
        if (application_config.enable_system_demonstration) {
            try {
                std::this_thread::sleep_for(std::chrono::seconds(2)); // Allow system to stabilize
                execute_system_demonstration(*unified_server_instance);
            } catch (const std::exception& e) {
                std::cout << "[demo] WARNING: system demonstration failed: " << e.what() << std::endl;
            }
        }
        
        // Main application event loop
        if (!application_config.enable_quiet_mode) {
            try {
                const auto server_config = unified_server_instance ? unified_server_instance->get_Configuration() : server_configuration;
                COMPONENT_INFO(application_main, "🎯 Kolosal Agent System is operational!");
                COMPONENT_INFO(application_main, "   * LLM Inference Server: http://{}:{}", server_config.server_host, server_config.server_port);
                COMPONENT_INFO(application_main, "   * Agent Management API: http://{}:{}/v1/agents", server_config.agent_api_host, server_config.agent_api_port);
                COMPONENT_INFO(application_main, "   * System Status Endpoint: http://{}:{}/v1/system/status", server_config.agent_api_host, server_config.agent_api_port);
                COMPONENT_INFO(application_main, "Press Ctrl+C to initiate graceful shutdown...");
            } catch (const std::exception& e) {
                std::cout << "🎯 Kolosal Agent System is operational!" << std::endl;
                std::cout << "   * LLM Inference Server: http://" << server_configuration.server_host << ":" << server_configuration.server_port << std::endl;
                std::cout << "   * Agent Management API: http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/agents" << std::endl;
                std::cout << "   * System Status Endpoint: http://" << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port << "/v1/system/status" << std::endl;
                std::cout << "Press Ctrl+C to initiate graceful shutdown..." << std::endl;
            }
        } else {
            try {
                const auto server_config = unified_server_instance ? unified_server_instance->get_Configuration() : server_configuration;
                COMPONENT_INFO(application_main, "System operational - LLM: {}:{}, Agent API: {}:{} (Press Ctrl+C to stop)", 
                              server_config.server_host, server_config.server_port,
                              server_config.agent_api_host, server_config.agent_api_port);
            } catch (const std::exception& e) {
                std::cout << "System operational - LLM: " << server_configuration.server_host << ":" << server_configuration.server_port
                          << ", Agent API: " << server_configuration.agent_api_host << ":" << server_configuration.agent_api_port 
                          << " (Press Ctrl+C to stop)" << std::endl;
            }
        }
        
        // Event loop with periodic status monitoring and responsive signal handling
    auto last_status_update_time = std::chrono::system_clock::now();
    while (system_running.load()) {
            // Use shorter sleep intervals to improve signal responsiveness
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            // Periodic status updates in verbose mode
            if (application_config.enable_verbose_logging) {
                auto current_time = std::chrono::system_clock::now();
                if (current_time - last_status_update_time >= std::chrono::minutes(5)) {
            const auto current_system_status = unified_server_instance ? unified_server_instance->getSystem_Status() : UnifiedKolosalServer::SystemStatus{};
                    COMPONENT_INFO(application_main, "📊 System Status - Agents: {}/{}, Response Time: {:.1f} ms", 
                                  current_system_status.running_agents, current_system_status.total_agents,
                                  current_system_status.average_response_time_ms);
                    last_status_update_time = current_time;
                }
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
        return EXIT_SUCCESS;
        
    } catch (const std::exception& exception) {
        std::cerr << "Fatal System Error: " << exception.what() << std::endl;
        
        // Ensure cleanup on exception
        if (unified_server_instance) {
            unified_server_instance->stop();
            unified_server_instance.reset();
        }
        
        return EXIT_FAILURE;
    }
}
