#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>
#include <vector>
#include <filesystem>
#include <memory>
#include <fstream>

// Agent system includes
#include "multi_agent_system.hpp"
#include "agent_core.hpp"
#include "yaml_config.hpp"
#include "kolosal_server_client.h"

// Platform-specific includes
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

using namespace kolosal::agents;

// Global flags for graceful shutdown
std::atomic<bool> keep_running{true};
std::atomic<bool> server_started{false};

// Server process information
#ifdef _WIN32
HANDLE server_process_handle = nullptr;
DWORD server_process_id = 0;
#else
pid_t server_process_id = 0;
#endif

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
    keep_running = false;
}

// Function to find kolosal-server executable
std::string find_server_executable() {
    std::filesystem::path current_path = std::filesystem::current_path();
    
    // List of possible server executable names
    std::vector<std::string> server_names = {
#ifdef _WIN32
        "kolosal-server.exe",
        "kolosal_server.exe"
#else
        "kolosal-server",
        "kolosal_server"
#endif
    };
    
    // Search locations relative to the agent executable
    std::vector<std::filesystem::path> search_paths = {
        current_path,                                    // Same directory as agent
        current_path / "kolosal-server",                // kolosal-server subdirectory
        current_path / ".." / "kolosal-server",         // Parent's kolosal-server
        current_path / "bin",                           // bin subdirectory
        current_path / ".." / "bin",                    // Parent's bin
    };
    
    for (const auto& search_path : search_paths) {
        for (const auto& server_name : server_names) {
            auto server_path = search_path / server_name;
            if (std::filesystem::exists(server_path)) {
                std::cout << "Found kolosal-server at: " << server_path << std::endl;
                return server_path.string();
            }
        }
    }
    
    return "";
}

// Function to start kolosal-server process
bool start_server_process(const std::string& server_path, int port = 8080) {
    if (server_path.empty()) {
        std::cerr << "Server executable not found!" << std::endl;
        return false;
    }
    
    std::cout << "Starting kolosal-server process..." << std::endl;
    
#ifdef _WIN32
    // Windows process creation
    std::string command = "\"" + server_path + "\" --port " + std::to_string(port) + " --host 127.0.0.1";
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    // Create the server process
    if (CreateProcessA(
        nullptr,                    // Application name
        const_cast<char*>(command.c_str()),  // Command line
        nullptr,                    // Process security attributes
        nullptr,                    // Thread security attributes
        FALSE,                      // Inherit handles
        0,                          // Creation flags
        nullptr,                    // Environment
        nullptr,                    // Current directory
        &si,                        // Startup info
        &pi                         // Process info
    )) {
        server_process_handle = pi.hProcess;
        server_process_id = pi.dwProcessId;
        CloseHandle(pi.hThread);
        server_started = true;
        std::cout << "Server process started with PID: " << server_process_id << std::endl;
        return true;
    } else {
        std::cerr << "Failed to start server process. Error: " << GetLastError() << std::endl;
        return false;
    }
#else
    // Unix/Linux process creation
    server_process_id = fork();
    if (server_process_id == 0) {
        // Child process - execute the server
        execlp(server_path.c_str(), server_path.c_str(), 
               "--port", std::to_string(port).c_str(),
               "--host", "127.0.0.1",
               nullptr);
        
        // If we get here, execlp failed
        std::cerr << "Failed to execute server: " << strerror(errno) << std::endl;
        exit(1);
    } else if (server_process_id > 0) {
        // Parent process
        server_started = true;
        std::cout << "Server process started with PID: " << server_process_id << std::endl;
        return true;
    } else {
        // Fork failed
        std::cerr << "Failed to fork server process: " << strerror(errno) << std::endl;
        return false;
    }
#endif
}

// Function to stop the server process
void stop_server_process() {
    if (!server_started) {
        return;
    }
    
    std::cout << "Stopping kolosal-server process..." << std::endl;
    
#ifdef _WIN32
    if (server_process_handle && server_process_id) {
        // Try to terminate gracefully first
        if (TerminateProcess(server_process_handle, 0)) {
            WaitForSingleObject(server_process_handle, 5000); // Wait up to 5 seconds
            CloseHandle(server_process_handle);
            std::cout << "Server process terminated." << std::endl;
        } else {
            std::cerr << "Failed to terminate server process." << std::endl;
        }
        server_process_handle = nullptr;
        server_process_id = 0;
    }
#else
    if (server_process_id > 0) {
        // Send SIGTERM for graceful shutdown
        if (kill(server_process_id, SIGTERM) == 0) {
            // Wait for process to terminate
            int status;
            int wait_result = waitpid(server_process_id, &status, WNOHANG);
            
            // If process doesn't terminate within 5 seconds, force kill
            for (int i = 0; i < 50 && wait_result == 0; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                wait_result = waitpid(server_process_id, &status, WNOHANG);
            }
            
            if (wait_result == 0) {
                // Force kill
                std::cout << "Force killing server process..." << std::endl;
                kill(server_process_id, SIGKILL);
                waitpid(server_process_id, &status, 0);
            }
            
            std::cout << "Server process stopped." << std::endl;
        } else {
            std::cerr << "Failed to send SIGTERM to server process." << std::endl;
        }
        server_process_id = 0;
    }
#endif
    
    server_started = false;
}

// Function to create a default agent configuration
void create_default_config_if_missing(const std::string& config_path) {
    if (!std::filesystem::exists(config_path)) {
        std::cout << "Creating default agent configuration at: " << config_path << std::endl;
        
        std::ofstream config_file(config_path);
        config_file << R"(# Kolosal Agent System Configuration
system:
  name: "Kolosal Multi-Agent System"
  version: "1.0.0"
  server:
    host: "127.0.0.1"
    port: 8080
    timeout: 30
  logging:
    level: "INFO"
    file: "agent_system.log"

agents:
  - name: "coordinator"
    id: "coord-001"
    type: "coordinator"
    role: "COORDINATOR" 
    specializations:
      - "TASK_PLANNING"
      - "RESOURCE_MANAGEMENT"
    capabilities:
      - "plan_execution"
      - "task_delegation"
      - "system_monitoring"
    functions:
      - "plan_tasks"
      - "delegate_work"
      - "monitor_progress"
    config:
      priority: 1
      auto_start: true
      max_concurrent_tasks: 5
      
  - name: "analyst"
    id: "analyst-001" 
    type: "specialist"
    role: "ANALYST"
    specializations:
      - "DATA_ANALYSIS"
      - "RESEARCH"
    capabilities:
      - "data_processing"
      - "research_synthesis" 
      - "report_generation"
    functions:
      - "analyze_data"
      - "research_topic"
      - "generate_report"
    config:
      priority: 2
      auto_start: true
      max_concurrent_tasks: 3

  - name: "executor"
    id: "exec-001"
    type: "worker" 
    role: "EXECUTOR"
    specializations:
      - "TASK_EXECUTION"
      - "TOOL_USAGE"
    capabilities:
      - "execute_commands"
      - "use_tools"
      - "file_operations"
    functions:
      - "execute_task"
      - "use_tool"
      - "process_files"
    config:
      priority: 3
      auto_start: true
      max_concurrent_tasks: 10

functions:
  - name: "plan_tasks"
    type: "builtin"
    description: "Create execution plans for complex tasks"
    parameters:
      - name: "goal"
        type: "string"
        required: true
      - name: "context"
        type: "string"
        required: false
    
  - name: "analyze_data"
    type: "builtin" 
    description: "Analyze structured and unstructured data"
    parameters:
      - name: "data_source"
        type: "string"
        required: true
      - name: "analysis_type"
        type: "string"
        required: false
        
  - name: "execute_task"
    type: "builtin"
    description: "Execute specific tasks with given parameters"
    parameters:
      - name: "task_definition"
        type: "object"
        required: true
      - name: "priority"
        type: "integer"
        required: false
)";
        config_file.close();
        std::cout << "Default configuration created successfully." << std::endl;
    }
}

// Function to print usage information
void print_usage(const char* program_name) {
    std::cout << "Kolosal Agent System v1.0.0" << std::endl;
    std::cout << "Usage: " << program_name << " [OPTIONS]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -c, --config FILE     Use custom configuration file (default: config.yaml)" << std::endl;
    std::cout << "  -p, --port PORT       Server port (default: 8080)" << std::endl;
    std::cout << "  -s, --server PATH     Path to kolosal-server executable (auto-detect if not specified)" << std::endl;
    std::cout << "  --no-server           Don't start server (assume it's already running)" << std::endl;
    std::cout << "  --demo                Run system demonstration" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
    std::cout << "  -v, --version         Show version information" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << "                           # Use default configuration" << std::endl;
    std::cout << "  " << program_name << " -c my_config.yaml        # Use custom configuration" << std::endl;
    std::cout << "  " << program_name << " -p 9090                  # Use custom port" << std::endl;
    std::cout << "  " << program_name << " --demo                   # Run demonstration" << std::endl;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string config_file = "config.yaml";
    std::string server_path = "";
    int server_port = 8080;
    bool start_server = true;
    bool run_demo = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "Kolosal Agent System v1.0.0" << std::endl;
            return 0;
        } else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                std::cerr << "Error: --config requires a file path" << std::endl;
                return 1;
            }
        } else if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                try {
                    server_port = std::stoi(argv[++i]);
                    if (server_port < 1 || server_port > 65535) {
                        std::cerr << "Error: Port must be between 1 and 65535" << std::endl;
                        return 1;
                    }
                } catch (const std::exception&) {
                    std::cerr << "Error: Invalid port number" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: --port requires a port number" << std::endl;
                return 1;
            }
        } else if (arg == "-s" || arg == "--server") {
            if (i + 1 < argc) {
                server_path = argv[++i];
            } else {
                std::cerr << "Error: --server requires a file path" << std::endl;
                return 1;
            }
        } else if (arg == "--no-server") {
            start_server = false;
        } else if (arg == "--demo") {
            run_demo = true;
        } else {
            std::cerr << "Error: Unknown argument '" << arg << "'" << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Set up signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
#ifdef _WIN32
    std::signal(SIGBREAK, signal_handler);
#endif

    // Print startup banner
    std::cout << "=== Kolosal Agent System v1.0.0 ===" << std::endl;
    std::cout << "Configuration: " << config_file << std::endl;
    std::cout << "Server Port: " << server_port << std::endl;
    std::cout << "Start Server: " << (start_server ? "Yes" : "No") << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
    
    // Create default configuration if it doesn't exist
    create_default_config_if_missing(config_file);
    
    // Start kolosal-server if requested
    std::unique_ptr<KolosalServerClient> server_client;
    if (start_server) {
        if (server_path.empty()) {
            server_path = find_server_executable();
        }
        
        if (!start_server_process(server_path, server_port)) {
            std::cerr << "Failed to start kolosal-server. Exiting." << std::endl;
            return 1;
        }
        
        // Wait a moment for server to initialize
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    // Initialize server client
    std::string server_url = "http://127.0.0.1:" + std::to_string(server_port);
    server_client = std::make_unique<KolosalServerClient>(server_url);
    
    // Wait for server to be ready
    std::cout << "Waiting for kolosal-server to be ready..." << std::endl;
    if (!server_client->waitForServerReady(30)) {
        std::cerr << "Kolosal-server did not become ready in time. Exiting." << std::endl;
        if (start_server) {
            stop_server_process();
        }
        return 1;
    }
    std::cout << "Kolosal-server is ready!" << std::endl;
    
    // Initialize the multi-agent system
    std::cout << "Initializing multi-agent system..." << std::endl;
    YAMLConfigurableAgentManager agent_manager;
    
    // Load configuration
    if (!agent_manager.load_configuration(config_file)) {
        std::cerr << "Failed to load agent configuration from: " << config_file << std::endl;
        if (start_server) {
            stop_server_process();
        }
        return 1;
    }
    
    // Start the agent system
    try {
        agent_manager.start();
        std::cout << "Multi-agent system started successfully!" << std::endl;
        std::cout << std::endl;
        
        // Print system status
        std::cout << agent_manager.get_system_status() << std::endl;
        
        // Run demonstration if requested
        if (run_demo) {
            std::cout << std::endl;
            std::cout << "=== Running System Demonstration ===" << std::endl;
            agent_manager.demonstrate_system();
            std::cout << "=== Demonstration Complete ===" << std::endl;
            std::cout << std::endl;
        }
        
        std::cout << "System is running. Press Ctrl+C to stop..." << std::endl;
        
        // Main event loop
        while (keep_running && agent_manager.is_running()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Periodic health check
            static int health_check_counter = 0;
            if (++health_check_counter >= 100) { // Check every 10 seconds
                if (!server_client->isServerHealthy()) {
                    std::cout << "Warning: Kolosal-server health check failed!" << std::endl;
                }
                health_check_counter = 0;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error running agent system: " << e.what() << std::endl;
    }
    
    // Graceful shutdown
    std::cout << std::endl;
    std::cout << "Shutting down multi-agent system..." << std::endl;
    agent_manager.stop();
    
    // Shutdown server if we started it
    if (start_server) {
        std::cout << "Shutting down kolosal-server..." << std::endl;
        if (server_client) {
            // Try graceful shutdown via API first
            if (!server_client->shutdownServer()) {
                std::cout << "API shutdown failed, terminating process..." << std::endl;
            }
        }
        stop_server_process();
    }
    
    std::cout << "Kolosal Agent System shutdown complete." << std::endl;
    return 0;
}
