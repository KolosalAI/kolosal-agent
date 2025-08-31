#include "kolosal_server_launcher.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#include <tlhelp32.h>
#pragma comment(lib, "winhttp.lib")
// Undefine Windows macros that conflict with our enum values
#ifdef ERROR
#undef ERROR
#endif
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <curl/curl.h>
#endif

namespace fs = std::filesystem;

// Forward declaration for helper function
std::string get_status_string_for(KolosalServerLauncher::Status status);

KolosalServerLauncher::KolosalServerLauncher(const ServerConfig& config)
    : config_(config), status_(KolosalServerLauncher::Status::STOPPED), should_stop_(false)
#ifdef _WIN32
    , process_handle_(nullptr), thread_handle_(nullptr)
#else
    , process_id_(0)
#endif
{
    TRACE_FUNCTION();
    LOG_INFO_F("KolosalServerLauncher created with host: %s, port: %d", config_.host.c_str(), config_.port);
    
    // Auto-detect executable path if not provided
    if (config_.executable_path.empty()) {
        config_.executable_path = find_server_executable();
        if (config_.executable_path.empty()) {
            LOG_ERROR("Could not find kolosal-server executable");
            update_status(ERROR, "Executable not found");
        } else {
            LOG_INFO_F("Auto-detected server executable: %s", config_.executable_path.c_str());
        }
    }
}

KolosalServerLauncher::~KolosalServerLauncher() {
    TRACE_FUNCTION();
    
    if (is_running()) {
        LOG_INFO("Stopping server during destructor");
        stop();
    }
}

bool KolosalServerLauncher::start() {
    TRACE_FUNCTION();
    SCOPED_TIMER("server_start");
    
    if (status_ == Status::RUNNING) {
        LOG_DEBUG("Server is already running");
        return true;
    }
    
    if (status_ == Status::STARTING) {
        LOG_DEBUG("Server is already starting, waiting...");
        return wait_for_ready(config_.timeout);
    }
    
    if (config_.executable_path.empty()) {
        LOG_ERROR("No executable path configured");
        update_status(ERROR, "No executable path");
        return false;
    }
    
    if (!fs::exists(config_.executable_path)) {
        LOG_ERROR_F("Server executable not found: %s", config_.executable_path.c_str());
        update_status(ERROR, "Executable not found");
        return false;
    }
    
    LOG_INFO_F("Starting Kolosal Server: %s", config_.executable_path.c_str());
    update_status(STARTING, "Launching server process");
    
    // Launch the server process
    if (!launch_process()) {
        LOG_ERROR("Failed to launch server process");
        update_status(ERROR, "Process launch failed");
        return false;
    }
    
    // Start monitoring thread
    should_stop_ = false;
    monitor_thread_ = std::make_unique<std::thread>(&KolosalServerLauncher::monitor_process, this);
    
    // Wait for server to be ready
    bool ready = wait_for_ready(config_.timeout);
    if (ready) {
        update_status(RUNNING, "Server ready");
        LOG_INFO_F("Kolosal Server started successfully on %s", get_server_url().c_str());
    } else {
        LOG_ERROR("Server failed to become ready within timeout");
        stop();
        return false;
    }
    
    return true;
}

bool KolosalServerLauncher::stop() {
    TRACE_FUNCTION();
    SCOPED_TIMER("server_stop");
    
    if (status_ == Status::STOPPED) {
        LOG_DEBUG("Server is already stopped");
        return true;
    }
    
    LOG_INFO("Stopping Kolosal Server");
    update_status(STOPPING, "Stopping server");
    
    // Signal threads to stop
    should_stop_ = true;
    
    // Terminate the process
    bool success = terminate_process();
    
    // Wait for monitor thread to finish
    if (monitor_thread_ && monitor_thread_->joinable()) {
        monitor_thread_->join();
        monitor_thread_.reset();
    }
    
    update_status(STOPPED, success ? "Server stopped" : "Stop failed");
    
    if (success) {
        LOG_INFO("Kolosal Server stopped successfully");
    } else {
        LOG_ERROR("Failed to stop Kolosal Server cleanly");
    }
    
    return success;
}

bool KolosalServerLauncher::is_running() const {
    KolosalServerLauncher::Status current_status = status_.load();
    return current_status == RUNNING || current_status == STARTING;
}

KolosalServerLauncher::Status KolosalServerLauncher::get_status() const {
    return status_.load();
}

std::string KolosalServerLauncher::get_status_string() const {
    switch (status_.load()) {
        case STOPPED: return "STOPPED";
        case STARTING: return "STARTING";
        case RUNNING: return "RUNNING";
        case STOPPING: return "STOPPING";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

bool KolosalServerLauncher::wait_for_ready(int timeout_seconds) {
    TRACE_FUNCTION();
    
    auto start_time = std::chrono::steady_clock::now();
    auto timeout_duration = std::chrono::seconds(timeout_seconds);
    
    LOG_DEBUG_F("Waiting for server to be ready (timeout: %d seconds)", timeout_seconds);
    
    while (std::chrono::steady_clock::now() - start_time < timeout_duration) {
        if (should_stop_) {
            LOG_DEBUG("Stop requested while waiting for server");
            return false;
        }
        
        if (!is_process_running()) {
            LOG_ERROR("Server process terminated while waiting for ready");
            return false;
        }
        
        if (is_healthy()) {
            LOG_DEBUG("Server is ready and healthy");
            return true;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    LOG_ERROR_F("Server did not become ready within %d seconds", timeout_seconds);
    return false;
}

bool KolosalServerLauncher::is_healthy() const {
    return check_server_endpoint("/health");
}

std::string KolosalServerLauncher::get_server_url() const {
    return "http://" + config_.host + ":" + std::to_string(config_.port);
}

void KolosalServerLauncher::update_config(const ServerConfig& config) {
    TRACE_FUNCTION();
    
    bool needs_restart = false;
    
    // Check if critical settings changed
    if (config.executable_path != config_.executable_path ||
        config.host != config_.host ||
        config.port != config_.port ||
        config.config_file != config_.config_file ||
        config.working_directory != config_.working_directory) {
        needs_restart = true;
    }
    
    config_ = config;
    
    if (needs_restart && is_running()) {
        LOG_INFO("Server configuration changed, restart required");
        // Note: keeping current status since this is just an informational update
    }
}

void KolosalServerLauncher::set_status_callback(std::function<void(KolosalServerLauncher::Status, const std::string&)> callback) {
    status_callback_ = std::move(callback);
}

bool KolosalServerLauncher::launch_process() {
    TRACE_FUNCTION();
    
    auto args = build_command_args();
    
    // Log the command being executed
    std::string cmd_str = config_.executable_path;
    for (size_t i = 1; i < args.size(); ++i) {
        cmd_str += " " + args[i];
    }
    LOG_DEBUG_F("Launching command: %s", cmd_str.c_str());
    
#ifdef _WIN32
    // Build command line string
    std::string cmdline;
    for (const auto& arg : args) {
        if (!cmdline.empty()) cmdline += " ";
        
        // Quote arguments that contain spaces
        if (arg.find(' ') != std::string::npos) {
            cmdline += "\"" + arg + "\"";
        } else {
            cmdline += arg;
        }
    }
    
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    
    // Set working directory if specified
    const char* working_dir = nullptr;
    if (!config_.working_directory.empty()) {
        working_dir = config_.working_directory.c_str();
    }
    
    BOOL success = CreateProcessA(
        nullptr,  // Let Windows find the executable from command line
        const_cast<char*>(cmdline.c_str()),
        nullptr,  // Process security attributes
        nullptr,  // Thread security attributes
        FALSE,    // Inherit handles
        0,        // Creation flags
        nullptr,  // Environment
        working_dir,
        &si,
        &pi
    );
    
    if (!success) {
        DWORD error = GetLastError();
        LOG_ERROR_F("CreateProcess failed with error %lu", error);
        return false;
    }
    
    process_handle_ = pi.hProcess;
    thread_handle_ = pi.hThread;
    
    LOG_DEBUG_F("Process launched successfully, PID: %lu", pi.dwProcessId);
    return true;
    
#else
    // Fork and exec for Unix-like systems
    process_id_ = fork();
    
    if (process_id_ == -1) {
        LOG_ERROR("Fork failed");
        return false;
    }
    
    if (process_id_ == 0) {
        // Child process
        
        // Change working directory if specified
        if (!config_.working_directory.empty()) {
            if (chdir(config_.working_directory.c_str()) != 0) {
                std::cerr << "Failed to change working directory" << std::endl;
                _exit(1);
            }
        }
        
        // Convert args to char* array
        std::vector<char*> argv;
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        
        // Execute the server
        execv(config_.executable_path.c_str(), argv.data());
        
        // If we get here, exec failed
        std::cerr << "execv failed" << std::endl;
        _exit(1);
    }
    
    // Parent process
    LOG_DEBUG_F("Process launched successfully, PID: %d", process_id_);
    return true;
#endif
}

bool KolosalServerLauncher::terminate_process() {
    TRACE_FUNCTION();
    
#ifdef _WIN32
    if (!process_handle_) {
        return true;  // Nothing to terminate
    }
    
    // Try graceful termination first
    if (!TerminateProcess(process_handle_, 0)) {
        DWORD error = GetLastError();
        LOG_ERROR_F("TerminateProcess failed with error %lu", error);
        return false;
    }
    
    // Wait for process to exit
    DWORD wait_result = WaitForSingleObject(process_handle_, 5000);  // 5 second timeout
    if (wait_result != WAIT_OBJECT_0) {
        LOG_WARN("Process did not exit gracefully within timeout");
    }
    
    CloseHandle(process_handle_);
    CloseHandle(thread_handle_);
    process_handle_ = nullptr;
    thread_handle_ = nullptr;
    
    return true;
    
#else
    if (process_id_ <= 0) {
        return true;  // Nothing to terminate
    }
    
    // Try graceful termination first
    if (kill(process_id_, SIGTERM) != 0) {
        LOG_ERROR_F("Failed to send SIGTERM to process %d", process_id_);
        return false;
    }
    
    // Wait for process to exit
    int status;
    int wait_count = 0;
    while (wait_count < 50) {  // 5 second timeout
        pid_t result = waitpid(process_id_, &status, WNOHANG);
        if (result == process_id_) {
            LOG_DEBUG("Process exited gracefully");
            process_id_ = 0;
            return true;
        } else if (result == -1) {
            LOG_ERROR("waitpid failed");
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        wait_count++;
    }
    
    // Force kill if still running
    LOG_WARN("Process did not exit gracefully, sending SIGKILL");
    if (kill(process_id_, SIGKILL) == 0) {
        waitpid(process_id_, &status, 0);
        process_id_ = 0;
        return true;
    }
    
    return false;
#endif
}

void KolosalServerLauncher::monitor_process() {
    TRACE_FUNCTION();
    
    LOG_DEBUG("Starting process monitor thread");
    
    while (!should_stop_) {
        if (!is_process_running()) {
            LOG_ERROR("Server process has terminated unexpectedly");
            update_status(ERROR, "Process terminated");
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    LOG_DEBUG("Process monitor thread stopping");
}

bool KolosalServerLauncher::is_process_running() const {
#ifdef _WIN32
    if (!process_handle_) {
        return false;
    }
    
    DWORD exit_code;
    if (!GetExitCodeProcess(process_handle_, &exit_code)) {
        return false;
    }
    
    return exit_code == STILL_ACTIVE;
    
#else
    if (process_id_ <= 0) {
        return false;
    }
    
    // Send signal 0 to check if process exists
    return kill(process_id_, 0) == 0;
#endif
}

bool KolosalServerLauncher::check_server_endpoint(const std::string& endpoint) const {
    std::string url = get_server_url() + endpoint;
    
#ifdef _WIN32
    // Use WinHTTP for Windows
    HINTERNET hSession = WinHttpOpen(L"KolosalAgent/1.0", 
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME, 
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        return false;
    }
    
    std::wstring w_host(config_.host.begin(), config_.host.end());
    HINTERNET hConnect = WinHttpConnect(hSession, w_host.c_str(), config_.port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }
    
    std::wstring w_endpoint(endpoint.begin(), endpoint.end());
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", w_endpoint.c_str(),
                                           NULL, WINHTTP_NO_REFERER, 
                                           WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }
    
    // Set short timeout for health checks
    DWORD timeout = 2000;  // 2 seconds
    WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
    
    BOOL result = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                    WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    
    bool success = false;
    if (result && WinHttpReceiveResponse(hRequest, NULL)) {
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                               WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX)) {
            success = (statusCode == 200);
        }
    }
    
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
    return success;
    
#else
    // Use libcurl for Unix-like systems
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);  // HEAD request
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L); // 2 second timeout
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void*, size_t, size_t, void*) { return 0; });
    
    CURLcode res = curl_easy_perform(curl);
    
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK && response_code == 200;
#endif
}

// Helper function for status string conversion
std::string get_status_string_for(KolosalServerLauncher::Status status) {
    switch (status) {
        case KolosalServerLauncher::STOPPED: return "STOPPED";
        case KolosalServerLauncher::STARTING: return "STARTING";
        case KolosalServerLauncher::RUNNING: return "RUNNING";
        case KolosalServerLauncher::STOPPING: return "STOPPING";
        case KolosalServerLauncher::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void KolosalServerLauncher::update_status(KolosalServerLauncher::Status new_status, const std::string& message) {
    KolosalServerLauncher::Status old_status = status_.exchange(new_status);
    
    if (old_status != new_status) {
        LOG_DEBUG_F("Server status changed: %s -> %s (%s)", 
                   get_status_string_for(old_status).c_str(),
                   get_status_string_for(new_status).c_str(),
                   message.c_str());
        
        if (status_callback_) {
            status_callback_(new_status, message);
        }
    }
}

std::vector<std::string> KolosalServerLauncher::build_command_args() const {
    std::vector<std::string> args;
    
    args.push_back(config_.executable_path);
    
    // Host and port
    args.push_back("--host");
    args.push_back(config_.host);
    args.push_back("--port");
    args.push_back(std::to_string(config_.port));
    
    // Config file
    if (!config_.config_file.empty()) {
        args.push_back("--config");
        args.push_back(config_.config_file);
    }
    
    // Log level
    args.push_back("--log-level");
    args.push_back(config_.log_level);
    
    // Note: --quiet flag is not supported by kolosal-server
    // The quiet_mode setting is handled by the launcher internally
    
    // Public access
    if (config_.public_access) {
        args.push_back("--public");
    }
    
    // Internet access
    if (config_.internet_access) {
        args.push_back("--internet");
    }
    
    return args;
}

std::string KolosalServerLauncher::find_server_executable() const {
    // List of possible paths to check
    std::vector<std::string> search_paths = {
        // Current directory
        "./kolosal-server",
        "./kolosal-server.exe",
        
        // Build directories
        "./build/kolosal-server/kolosal-server",
        "./build/kolosal-server/kolosal-server.exe",
        "./build/Debug/kolosal-server.exe",
        "./build/Release/kolosal-server.exe",
        "./build/kolosal-server/Debug/kolosal-server.exe",
        "./build/kolosal-server/Release/kolosal-server.exe",
        
        // Test build directories
        "./build_tests/kolosal-server/kolosal-server",
        "./build_tests/kolosal-server/kolosal-server.exe",
        "./build_tests/Debug/kolosal-server.exe",
        "./build_tests/Release/kolosal-server.exe",
        "./build_tests/kolosal-server/Debug/kolosal-server.exe",
        "./build_tests/kolosal-server/Release/kolosal-server.exe",
        
        // Minimal test build directories
        "./build_minimal_tests/kolosal-server/kolosal-server",
        "./build_minimal_tests/kolosal-server/kolosal-server.exe",
        "./build_minimal_tests/Debug/kolosal-server.exe",
        "./build_minimal_tests/Release/kolosal-server.exe",
        "./build_minimal_tests/kolosal-server/Debug/kolosal-server.exe",
        "./build_minimal_tests/kolosal-server/Release/kolosal-server.exe",
        
        // Basic test build directories
        "./build_basic_tests/kolosal-server/kolosal-server",
        "./build_basic_tests/kolosal-server/kolosal-server.exe",
        "./build_basic_tests/Debug/kolosal-server.exe",
        "./build_basic_tests/Release/kolosal-server.exe",
        "./build_basic_tests/kolosal-server/Debug/kolosal-server.exe",
        "./build_basic_tests/kolosal-server/Release/kolosal-server.exe",
        
        // Relative to current working directory
        "../kolosal-server/build/kolosal-server",
        "../kolosal-server/build/kolosal-server.exe",
        "../kolosal-server/build/Debug/kolosal-server.exe",
        "../kolosal-server/build/Release/kolosal-server.exe",
        
        // Standard installation paths
#ifdef _WIN32
        "C:/Program Files/Kolosal/kolosal-server.exe",
        "C:/Program Files (x86)/Kolosal/kolosal-server.exe"
#else
        "/usr/local/bin/kolosal-server",
        "/usr/bin/kolosal-server"
#endif
    };
    
    // Check each path
    for (const auto& path : search_paths) {
        if (fs::exists(path) && fs::is_regular_file(path)) {
            auto absolute_path = fs::absolute(path);
            LOG_DEBUG_F("Found server executable: %s", absolute_path.string().c_str());
            return absolute_path.string();
        }
    }
    
    LOG_WARN("Could not find kolosal-server executable in any of the standard locations");
    return "";
}

KolosalServerLauncher::ServerConfig create_default_server_config(const std::string& workspace_path) {
    KolosalServerLauncher::ServerConfig config;
    
    // Set default values
    config.host = "127.0.0.1";
    config.port = 8081;
    config.log_level = "INFO";
    config.quiet_mode = false;
    config.public_access = false;
    config.internet_access = false;
    config.timeout = 30;
    
    // Set working directory if provided
    if (!workspace_path.empty()) {
        config.working_directory = workspace_path;
        
        // Look for config file in workspace
        std::string config_file = workspace_path + "/config.yaml";
        if (fs::exists(config_file)) {
            config.config_file = config_file;
        }
    }
    
    return config;
}
