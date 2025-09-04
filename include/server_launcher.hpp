#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <functional>
#include "logger.hpp"

/**
 * @brief Manages the lifecycle of the Kolosal Server process
 * 
 * This class handles launching, monitoring, and stopping the Kolosal Server
 * as a separate process. It ensures the server is available for model inference
 * requests from the agents.
 */
class KolosalServerLauncher {
public:
    /**
     * @brief Configuration for the Kolosal Server
     */
    struct ServerConfig {
        std::string executable_path;      // Path to kolosal-server executable
        std::string host = "127.0.0.1";   // Server host
        int port = 8081;                  // Server port
        std::string config_file;          // Optional config file path
        std::string log_level = "INFO";   // Log level (ERROR, WARN, INFO, DEBUG)
        bool quiet_mode = false;          // Quiet mode
        bool public_access = false;       // Allow public access
        bool internet_access = false;    // Allow internet access
        int timeout = 30;                 // Startup timeout in seconds
        std::string working_directory;    // Working directory for the server
    };

    /**
     * @brief Server status enumeration
     */
    enum Status {
        STOPPED,
        STARTING,
        RUNNING,
        STOPPING,
        ERROR
    };

    /**
     * @brief Constructor
     * @param config Server configuration
     */
    explicit KolosalServerLauncher(const ServerConfig& config);

    /**
     * @brief Destructor - ensures server is stopped
     */
    ~KolosalServerLauncher();

    /**
     * @brief Start the Kolosal Server
     * @return true if server started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Stop the Kolosal Server
     * @return true if server stopped successfully, false otherwise
     */
    bool stop();

    /**
     * @brief Check if the server is running
     * @return true if server is running, false otherwise
     */
    bool is_running() const;

    /**
     * @brief Get current server status
     * @return Current status
     */
    KolosalServerLauncher::Status get_status() const;

    /**
     * @brief Get server status as string
     * @return Status string
     */
    std::string get_status_string() const;

    /**
     * @brief Wait for server to be ready for requests
     * @param timeout_seconds Maximum time to wait in seconds
     * @return true if server is ready, false if timeout or error
     */
    bool wait_for_ready(int timeout_seconds = 30);

    /**
     * @brief Check if server is responding to health checks
     * @return true if server is healthy, false otherwise
     */
    bool is_healthy() const;

    /**
     * @brief Get the server URL
     * @return Full server URL (e.g., "http://127.0.0.1:8081")
     */
    std::string get_server_url() const;

    /**
     * @brief Get the server configuration
     * @return Current server configuration
     */
    const ServerConfig& get_config() const { return config_; }

    /**
     * @brief Update server configuration (requires restart to take effect)
     * @param config New configuration
     */
    void update_config(const ServerConfig& config);

    /**
     * @brief Set callback for status changes
     * @param callback Function to call when status changes
     */
    void set_status_callback(std::function<void(KolosalServerLauncher::Status, const std::string&)> callback);

private:
    ServerConfig config_;
    std::atomic<KolosalServerLauncher::Status> status_;
    std::unique_ptr<std::thread> monitor_thread_;
    std::atomic<bool> should_stop_;
    std::function<void(KolosalServerLauncher::Status, const std::string&)> status_callback_;

#ifdef _WIN32
    void* process_handle_;  // HANDLE for Windows
    void* thread_handle_;   // HANDLE for Windows
#else
    pid_t process_id_;      // Process ID for Unix-like systems
#endif

    /**
     * @brief Launch the server process
     * @return true if process launched successfully, false otherwise
     */
    bool launch_process();

    /**
     * @brief Terminate the server process
     * @return true if process terminated successfully, false otherwise
     */
    bool terminate_process();

    /**
     * @brief Monitor the server process
     */
    void monitor_process();

    /**
     * @brief Check if process is still running
     * @return true if process is running, false otherwise
     */
    bool is_process_running() const;

    /**
     * @brief Make HTTP request to check server health
     * @param endpoint The endpoint to check (default: "/health")
     * @return true if request successful, false otherwise
     */
    bool check_server_endpoint(const std::string& endpoint = "/health") const;

    /**
     * @brief Update status and notify callback if set
     * @param new_status New status
     * @param message Optional message
     */
    void update_status(KolosalServerLauncher::Status new_status, const std::string& message = "");

    /**
     * @brief Build command line arguments for the server
     * @return Vector of command line arguments
     */
    std::vector<std::string> build_command_args() const;

    /**
     * @brief Find the kolosal-server executable
     * @return Path to executable or empty string if not found
     */
    std::string find_server_executable() const;
};

/**
 * @brief Helper function to create default server configuration
 * @param workspace_path Path to the workspace directory
 * @return Default server configuration
 */
KolosalServerLauncher::ServerConfig create_default_server_config(const std::string& workspace_path = "");
