/**
 * @file server_client_interface.cpp
 * @brief Core functionality for server client interface
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "server/server_client_interface.h"
#include "api/http_client.hpp"
#include "utils/loading_animation_utils.hpp"
#include "kolosal/logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <regex>
#include <fstream>
#include <filesystem>
#include <vector>
#include <json.hpp>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <cstdlib>  // for _dupenv_s
#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <mach-o/dyld.h>
#include <libproc.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#endif

using json = nlohmann::json;
// Cross-platform helper functions
#ifdef _WIN32
    #define PATH_SEPARATOR "\\"
    #define EXECUTABLE_EXTENSION ".exe"
#else
    #define PATH_SEPARATOR "/"
    #define EXECUTABLE_EXTENSION ""
#endif

// Cross-platform process ID type
#ifdef _WIN32
typedef DWORD ProcessId;
#else
typedef pid_t ProcessId;
#endif

// Helper function to get executable path
static std::string getExecutable_Path() {
#ifdef _WIN32
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    return std::string(exePath);
#elif defined(__APPLE__)
    // macOS: Use _NSGetExecutablePath
    char exePath[1024];
        uint32_t size = sizeof(exePath);
        if (_NSGetExecutablePath(exePath, &size) == 0) {
        // Resolve symbolic links if any
        char resolvedPath[1024];
        if (realpath(exePath, resolvedPath) != nullptr) {
                return std::string(resolvedPath);
        } else {
            return std::string(exePath);
        }
    }
    return "";
#else
    // Linux: Use /proc/self/exe
    char exePath[1024];
    const ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len != -1) {
        exePath[len] = '\0';
        return std::string(exePath);
    }
    return "";
#endif
}

// Helper function to check if file exists
/**
 * @brief Perform file exists operation
 * @return bool Description of return value
 */
static bool file_Exists(const std::string& path) {
    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

// Helper function to find server process ID
/**
 * @brief Perform findserver process operation
 * @return ProcessId Description of return value
 */
static ProcessId findServer_Process() {
#ifdef _WIN32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return 0;
    }

    ProcessId serverPid = 0;
    do {
        if (_wcsicmp(pe32.szExeFile, L"kolosal-server.exe") == 0) {
            serverPid = pe32.th32ProcessID;
            break;
        }
    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return serverPid;
#elif defined(__APPLE__)
    // On macOS, use proc_listpids and proc_pidpath to find the process
    pid_t pids[1024];
    int numberOfPids = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
    numberOfPids = numberOfPids / sizeof(pid_t);
    
    for (int i = 0; i < numberOfPids; ++i) {
        if (pids[i] == 0) continue;
        
        char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
        const int ret = proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer));
        if (ret > 0) {
            std::string process_Path(pathBuffer);
            // Check if this process is kolosal-server
            if (processPath.find("kolosal-server") != std::string::npos) {
                return pids[i];
            }
        }
    }
    return 0;
#else
    // On Linux, search through /proc for the process
    DIR* procDir = opendir("/proc");
    if (!procDir) return 0;

    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        // Check if directory name is a number (PID)
        if (strspn(entry->d_name, "0123456789") == strlen(entry->d_name)) {
            std::string cmdlinePath = std::string("/proc/") + entry->d_name + "/cmdline";
            std::ifstream cmdlineFile(cmdlinePath);
            if (cmdlineFile.is_open()) {
                std::string cmdline;
                std::getline(cmdlineFile, cmdline);
                cmdlineFile.close();
                
                // Check if this process is kolosal-server
                if (cmdline.find("kolosal-server") != std::string::npos) {
                    closedir(procDir);
                    return std::stoi(entry->d_name);
                }
            }
        }
    }
    closedir(procDir);
    return 0;
#endif
}

// Helper function to terminate process
/**
 * @brief Perform terminate process operation
 * @return bool Description of return value
 */
static bool terminate_Process(ProcessId pid) {
#ifdef _WIN32
    const HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess != nullptr) {
        const bool success = TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
        return success;
    }
    return false;
#else
    return kill(pid, SIGTERM) == 0;
#endif
}

KolosalServerClient::KolosalServerClient(const std::string &baseUrl, const std::string &apiKey)
    /**
     * @brief Perform member base url operation
     * @return : Description of return value
     */
    : member_baseUrl(baseUrl), member_apiKey(apiKey)
{}

KolosalServerClient::~KolosalServerClient() = default;

bool KolosalServerClient::start_Server(const std::string &serverPath, const int port)
{
    if (isServer_Healthy())
    {
        return true;
    }

    std::string actualServerPath = serverPath;
    if (actualServerPath.empty())
    {
    const std::string exePath = getExecutable_Path();
        if (exePath.empty()) {
            std::cerr << "Error: Could not determine executable path" << std::endl;
            return false;
        }
        
    const std::string exeDir = exePath.substr(0, exePath.find_last_of(PATH_SEPARATOR));
        // Priority 1: Try same directory as CLI (highest priority for Windows)
    std::string sameDirPath = exeDir + PATH_SEPARATOR + "kolosal-server" + EXECUTABLE_EXTENSION;
        if (file_Exists(sameDirPath))
        {
            actualServerPath = sameDirPath;
        }
        else
        {
            // Priority 2: Try kolosal-server subdirectory (common for extracted packages)
            std::string subDirPath = exeDir + PATH_SEPARATOR + "kolosal-server" + PATH_SEPARATOR + "kolosal-server" + EXECUTABLE_EXTENSION;
            if (file_Exists(subDirPath))
            {
                actualServerPath = subDirPath;
            }
            else
            {
                // Priority 3: Try parent/server-bin directory (Windows package structure)
                std::string parentDir = exeDir.substr(0, exeDir.find_last_of(PATH_SEPARATOR));
                std::string serverBinPath = parentDir + PATH_SEPARATOR + "server-bin" + PATH_SEPARATOR + "kolosal-server" + EXECUTABLE_EXTENSION;
                if (file_Exists(serverBinPath))
                {
                    actualServerPath = serverBinPath;
                }
                else
                {
                    // Priority 4: Try build directory (development environment)
                    std::string buildPath = parentDir + PATH_SEPARATOR + "build" + PATH_SEPARATOR + "kolosal-server" + PATH_SEPARATOR + "kolosal-server" + EXECUTABLE_EXTENSION;
                    std::cout << "Checking build directory: " << buildPath << std::endl;
                    if (file_Exists(buildPath))
                    {
                        actualServerPath = buildPath;
                    }
                    else
                    {
                        // Last resort: Fall back to system PATH
                        actualServerPath = std::string("kolosal-server") + std::string(EXECUTABLE_EXTENSION);
                    }
                }
            }
        }
    } 
    else 
    {
        std::cout << "Using provided server path: " << actualServerPath << std::endl;
    }
    
    // Verify the server path exists before attemporaryoraryoraryoraryoraryting to start
    if (!actualServerPath.empty() && actualServerPath != "kolosal-server" + std::string(EXECUTABLE_EXTENSION))
    {
        if (!file_Exists(actualServerPath))
        {
            std::cerr << "Error: Server executable not found at: " << actualServerPath << std::endl;
            return false;
        }
    } 
    
#ifdef _WIN32
    // Build command line using the actual server path
    std::string commandLine;
    if (actualServerPath == "kolosal-server.exe") {
        commandLine = "kolosal-server.exe";
    } else {
        commandLine = "\"" + actualServerPath + "\"";
    }
    
    // Use the same directory as the CLI executable for configurationurationurationurationurationurationurationurationurationuration.yaml access
    std::string workingDir;
    
    // Priority 1: Use the same directory as the CLI executable (for config.yaml access)
    std::string exePath = getExecutable_Path();
    if (!exePath.empty()) {
        std::string exeDir = exePath.substr(0, exePath.find_last_of(PATH_SEPARATOR));
        // Check if we can write to the exe directory (for logs, temp files, etc.)
    std::string testFile = exeDir + PATH_SEPARATOR + "test_write.tmp";
        std::ofstream testWrite(testFile);
        if (testWrite.is_open()) {
            testWrite.close();
            std::filesystem::remove(testFile);
            workingDir = exeDir;
        }
    }
    
    // Fallback: Use user profile or temp if exe directory is not writable
    if (workingDir.empty()) {
        char* userProfile = nullptr;
        if (_dupenv_s(&userProfile, nullptr, "USERPROFILE") == 0 && userProfile != nullptr) {
            workingDir = std::string(userProfile);
            free(userProfile);
            std::cout << "Using user profile as working directory: " << workingDir << std::endl;
        } else {
            // Final fallback to temp directory
            char* tempDir = nullptr;
            if (_dupenv_s(&tempDir, nullptr, "TEMP") == 0 && tempDir != nullptr) {
                workingDir = std::string(tempDir);
                free(tempDir);
            } else {
                workingDir = "C:\\temp";
            }
            std::cout << "Using temp directory as working directory: " << workingDir << std::endl;
        }
    }
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));
    
    const BOOL result = CreateProcessA(
        (actualServerPath == "kolosal-server.exe") ? NULL : actualServerPath.c_str(),
        const_cast<char *>(commandLine.c_str()),
        NULL,
        NULL,
        FALSE,
        DETACHED_PROCESS,
        NULL,
        workingDir.c_str(),
        &si,
        &pi
    );
    if (!result)
    {
    const DWORD error = GetLastError();
        switch (error)
        {
        case ERROR_FILE_NOT_FOUND:
            std::cerr << "Error: Server executable not found. Please ensure kolosal-server.exe is available." << std::endl;
            break;
        case ERROR_ACCESS_DENIED:
            std::cerr << "Error: Access denied. Please run as administrator if necessary." << std::endl;
            break;
        default:
            std::cerr << "Error: Failed to start server (code: " << error << ")" << std::endl;
            break;
        }
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    // Check if the file is executable
    if (access(actualServerPath.c_str(), X_OK) != 0) {
        std::cerr << "Error: Server binary is not executable: " << actualServerPath << std::endl;
        std::cerr << "Try running: chmod +x " << actualServerPath << std::endl;
        return false;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Change working directory to prioritize config.yaml access
        std::string workingDir;
        
        // Priority 1: Use the same directory as the CLI executable (for config.yaml access)
        std::string exePath = getExecutable_Path();
        if (!exePath.empty()) {
            std::string exeDir = exePath.substr(0, exePath.find_last_of(PATH_SEPARATOR));
            // Check if we can write to the exe directory
            if (access(exeDir.c_str(), W_OK) == 0) {
                workingDir = exeDir;
            }
        }
        
        // Fallback: Use system directories if exe directory is not writable
        if (workingDir.empty()) {
            // Try to use /var/lib/kolosal if it exists and is writable
            if (access("/var/lib/kolosal", W_OK) == 0) {
                workingDir = "/var/lib/kolosal";
                std::cerr << "Using /var/lib/kolosal as working directory: " << workingDir << std::endl;
            }
            // Otherwise use user's home directory
            else {
            const char* homeDir = getenv("HOME");
                if (homeDir) {
                    workingDir = std::string(homeDir);
                    std::cerr << "Using home directory as working directory: " << workingDir << std::endl;
                } else {
                    workingDir = "/tmp";  // Last resort
                    std::cerr << "Using /tmp as working directory: " << workingDir << std::endl;
                }
            }
        }
        
        if (chdir(workingDir.c_str()) != 0) {
            std::cerr << "Warning: Could not change to chosen directory: " << workingDir << std::endl;
            // Try /tmp as absolute last resort
            if (chdir("/tmp") != 0) {
                std::cerr << "Error: Could not change to any writable directory" << std::endl;
            } else {
                std::cerr << "Using /tmp as fallback working directory" << std::endl;
            }
        }
        
        // Execute the server in background (detached)
    setsid(); // Create new session
        
        // For debugging, redirect to log files instead of /dev/null temporarily
        // This will help us see what's happening with the server startup
    std::string logPath = "/tmp/kolosal-server.log";
        freopen(logPath.c_str(), "w", stdout);
        freopen(logPath.c_str(), "w", stderr);
        
        // Execute with config parameter if available
        execl(actualServerPath.c_str(), "kolosal-server", nullptr);
        
        // If execl fails, log the error and exit child process
        std::cerr << "Failed to execute server: " << actualServerPath << std::endl;
        std::cerr << "Error: " << strerror(errno) << std::endl;
        _exit(1);
    } else if (pid < 0) {
        // Fork failed
        std::cerr << "Error: Failed to start server process: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Give the server a moment to start before returning
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
#endif

    return true;
}

bool KolosalServerClient::isServer_Healthy()
{
    std::string response;
    if (!makeGet_Request("/v1/health", response))
    {
        return false;
    }

    try
    {
    const json healthJson = json::parse(response);
    std::string status = healthJson.value("status", "");
    return status == "healthy";
    }
    catch (const std::exception &)
    {
        return false;
    }
}

bool KolosalServerClient::waitForServer_Ready(int timeoutSeconds)
{
    const auto startTime = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(timeoutSeconds);
    LoadingAnimation loading("Waiting for server to start");
    loading.start();

    while (std::chrono::steady_clock::now() - startTime < timeout)
    {
        if (isServer_Healthy())
        {
            loading.complete("Server started successfully");
            return true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    loading.stop();
    return false;
}

bool KolosalServerClient::get_Engines(std::vector<std::string>& engines)
{
    std::string response;
    
    // Use the unified models endpoint
    if (!makeGet_Request("/models", response))
    {
        return false;
    }

    try
    {
        const json modelsJson = json::parse(response);
        engines.clear();
        
        // Handle the unified models response format
    if (modelsJson.contains("models") && modelsJson["models"].is_array())
        {
            for (const auto& model : modelsJson["models"])
            {
                if (model.contains("model_id"))
                {
                    engines.push_back(model["model_id"].get<std::string>());
                }
            }
        }
        
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool KolosalServerClient::engine_Exists(const std::string& engineId)
{
    std::vector<std::string> engines;
    if (!get_Engines(engines))
    {
        return false;
    }
    
    for (const auto& engine : engines)
    {
        if (engine == engineId)
        {
            return true;
        }
    }
    
    return false;
}

bool KolosalServerClient::add_Engine(const std::string &engineId, const std::string &modelUrl,
                                    const std::string &modelPath)
{
    try
    {
        // No loading animation for cleaner output - just show final result
        json payload;
        payload["model_id"] = engineId;
        payload["model_path"] = modelUrl; // For download, we pass URL as model_path
        payload["load_immediately"] = false;
        payload["main_gpu_id"] = 0;
        
        // Set comprehensive loading parameters
        json loadParams;
        loadParams["n_ctx"] = 4096;
        loadParams["n_keep"] = 2048;
        loadParams["use_mmap"] = true;
        loadParams["use_mlock"] = true;
        loadParams["n_parallel"] = 4;
        loadParams["cont_batching"] = true;
        loadParams["warmup"] = false;
        loadParams["n_gpu_layers"] = 50;
        loadParams["n_batch"] = 2048;
        loadParams["n_ubatch"] = 512;
        
        payload["loading_parameters"] = loadParams;

        std::string response;
        if (!makePost_Request("/models", payload.dump(), response))  // Using new model-based API endpoint
        {
            return false;
        }
        
        // Parse response to check if it was successful
        try {
            const json responseJson = json::parse(response);
            // Check for error responses first
            if (responseJson.contains("error")) {
                const auto error = responseJson["error"];
                std::string errorMessage = error.value("message", "Unknown error");
                std::string errorCode = error.value("code", "unknown");
                // Only treat "model_already_loaded" as a success case
                if (errorCode == "model_already_loaded") {
                    return true;
                }
                
                // For other errors, don't update config and return false
                std::cerr << "Server error: " << errorMessage << std::endl;
                return false;
            }
            
            // Check for success status codes
            if (responseJson.contains("status")) {
                std::string status = responseJson["status"];
                if (status == "loaded" || status == "created" || status == "downloading") {
                    // Server handles config updates automatically now
                    return true;
                }
            }
            
            // If no clear status, assume success for backward compatibility
            // Server handles config updates automatically now
            return true;
        } catch (const std::exception&) {
            // If we can't parse the response, still consider it successful if we got here
            // Server handles config updates automatically now
            return true;
        }
    }
    catch (const std::exception &)
    {
        return false;
    }
}

bool KolosalServerClient::getDownload_Progress(const std::string &modelId, long long &downloadedBytes,
                                              long long &totalBytes, double &percentage, std::string &status)
{
    std::string response;
    std::string endpoint = "/v1/downloads/" + modelId;
    // Try v1 endpoint first, then fallback to non-v1
    if (!makeGet_Request(endpoint, response))
    {
        endpoint = "/downloads/" + modelId;
        if (!makeGet_Request(endpoint, response))
        {
            // Check if response contains error indicating download not found
            if (!response.empty())
            {
                try
                {
                const json errorJson = json::parse(response);
                if (errorJson.contains("error") && errorJson["error"].contains("code"))
                {
                    std::string errorCode = errorJson["error"]["code"];
                    if (errorCode == "download_not_found")
                    {
                        status = "not_found";
                        downloadedBytes = 0;
                        totalBytes = 0;
                        percentage = 0.0;
                        return true; // Return true to indicate we successfully determined there's no download
                    }
                }
            }
            catch (const std::exception &)
            {
                // If we can't parse the error response, fall through to return false
            }
            }
            return false;
        }
    }

    try
    {
        const json progressJson = json::parse(response);
        status = progressJson.value("status", "unknown");

        if (progressJson.contains("progress"))
        {
            const auto progress = progressJson["progress"];
            downloadedBytes = progress.value("downloaded_bytes", 0LL);
            totalBytes = progress.value("total_bytes", 0LL);
            percentage = progress.value("percentage", 0.0);
        }
        else
        {
            downloadedBytes = 0;
            totalBytes = 0;
            percentage = 0.0;
        }
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

bool KolosalServerClient::monitorDownload_Progress(const std::string &modelId,
                                                  std::function<void(double, const std::string &, long long, long long)> progressCallback,
                                                  int checkIntervalMs)
{
    auto startTime = std::chrono::steady_clock::now();
    auto maxDuration = std::chrono::minutes(30); // 30 minute timeout
    
    // Variables to track 100% completion status
    auto lastHundredPercentTime = std::chrono::steady_clock::time_point {};
    auto hundredPercentTimeout = std::chrono::seconds(30); // 30 seconds timeout at 100%
    bool hasReachedHundred = false;
    while (true)
    {
        // Check for timeout
        if (std::chrono::steady_clock::now() - startTime > maxDuration) {
            return false;
        }
        
        long long downloadedBytes, totalBytes;
        double percentage;
        std::string status;

        if (!getDownload_Progress(modelId, downloadedBytes, totalBytes, percentage, status))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
            continue;
        }

    progressCallback(percentage, status, downloadedBytes, totalBytes);

        // Success statuses
        if (status == "completed" || status == "creating_engine" || status == "engine_created")
        {
            return true;
        }
        // Failure statuses
        /**
         * @brief Perform if operation
         * @return else Description of return value
         */
        else if (status == "failed" || status == "cancelled" || status == "engine_creation_failed")
        {
            return false;
        }
        // No download found - this happens when a model file already exists and no download was needed
        /**
         * @brief Perform if operation
         * @return else Description of return value
         */
        else if (status == "not_found")
        {
            return true; // This is actually a success case - no download was needed
        }
        // Handle stuck at 100% case - download is complete but status hasn't transitioned yet
        /**
         * @brief Perform if operation
         * @return else Description of return value
         */
        else if (status == "downloading" && percentage >= 100.0)
        {
            if (!hasReachedHundred)
            {
                hasReachedHundred = true;
                lastHundredPercentTime = std::chrono::steady_clock::now();
                // Trigger status callback to show progress transition
                progressCallback(percentage, "completing", downloadedBytes, totalBytes);
            }
            else
            {
                auto timeSinceHundred = std::chrono::steady_clock::now() - lastHundredPercentTime;
                // Check if we've been stuck at 100% for a reasonable amount of time
                if (timeSinceHundred > hundredPercentTimeout)
                {
                    // First, check if the engine already exists (quick check)
                    if (engine_Exists(modelId))
                    {
                        // Engine exists, so download and engine creation was successful
                        progressCallback(100.0, "engine_created", downloadedBytes, totalBytes);
                        return true;
                    }
                    
                    // If more than 2 minutes at 100%, assume something is wrong
                    if (timeSinceHundred > std::chrono::minutes(2))
                    {
                        // Do one final check for engine existence before giving up
                        if (engine_Exists(modelId))
                        {
                            progressCallback(100.0, "engine_created", downloadedBytes, totalBytes);
                            return true;
                        }
                        return false; // Stuck too long, consider it failed
                    }
                    
                    // Between 30 seconds and 2 minutes, show "processing" status
                    if (timeSinceHundred > std::chrono::seconds(30))
                    {
                        progressCallback(percentage, "processing", downloadedBytes, totalBytes);
                    }
                }
            }
        }
        else
        {
            // Reset the 100% timer if we're not at 100%
            hasReachedHundred = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
    }
}

bool KolosalServerClient::makeGet_Request(const std::string &endpoint, std::string &response)
{
    std::string url = member_baseUrl + endpoint;
    std::vector<std::string> headers;
    if (!member_apiKey.empty())
    {
        headers.push_back("X-API-Key: " + member_apiKey);
    }

    HttpClient client;
    return client.get(url, response, headers);
}

bool KolosalServerClient::makePost_Request(const std::string &endpoint, const std::string &payload, std::string &response)
{
    std::string url = member_baseUrl + endpoint;
    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json");
    if (!member_apiKey.empty())
    {
        headers.push_back("X-API-Key: " + member_apiKey);
    }

    HttpClient client;
    return client.post(url, payload, response, headers);
}

bool KolosalServerClient::makeDelete_Request(const std::string &endpoint, const std::string &payload, std::string &response)
{
    std::string url = member_baseUrl + endpoint;
    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json");
    if (!member_apiKey.empty())
    {
        headers.push_back("X-API-Key: " + member_apiKey);
    }

    HttpClient client;
    return client.delete_Request(url, headers);
}

bool KolosalServerClient::makePut_Request(const std::string &endpoint, const std::string &payload, std::string &response)
{
    std::string url = member_baseUrl + endpoint;
    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json");
    if (!member_apiKey.empty())
    {
        headers.push_back("X-API-Key: " + member_apiKey);
    }

    HttpClient client;
    return client.put(url, payload, response, headers);
}

bool KolosalServerClient::parseJson_Value(const std::string &jsonString, const std::string &key, std::string &value)
{
    try
    {
    const json j = json::parse(jsonString);
        if (j.contains(key))
        {
            value = j[key].get<std::string>();
            return true;
        }
    }
    catch (const std::exception &)
    {
    }
    return false;
}

bool KolosalServerClient::parseJson_Number(const std::string &jsonString, const std::string &key, double &value)
{
    try
    {
        json j = json::parse(jsonString);
        if (j.contains(key))
        {
            value = j[key].get<double>();
            return true;
        }
    }
    catch (const std::exception &)
    {
    }
    return false;
}

bool KolosalServerClient::shutdown_Server()
{
    // Find the kolosal-server process
    ProcessId serverPid = findServer_Process();
    if (serverPid == 0) {
        // No server process found, consider it already stopped
        return true;
    }

    LoadingAnimation loading("Shutting down server");
    loading.start();

    // Terminate the process
    if (terminate_Process(serverPid)) {
        loading.complete("Server shutdown successfully");
        return true;
    } else {
        loading.stop();
        std::cerr << "Error: Failed to terminate server process" << std::endl;
        return false;
    }
}

bool KolosalServerClient::cancel_Download(const std::string &modelId)
{
    LoadingAnimation loading("Cancelling download");
    loading.start();

    std::string response;
    std::string endpoint = "/v1/downloads/" + modelId + "/cancel";
    // Try v1 endpoint first, then fallback to non-v1
    if (!makePost_Request(endpoint, "{}", response))
    {
        endpoint = "/downloads/" + modelId + "/cancel";
        if (!makePost_Request(endpoint, "{}", response))
        {
            loading.stop();
            return false;
        }
    }

    try
    {
    const json cancelJson = json::parse(response);
        const bool success = cancelJson.value("success", false);
        if (success) {
            loading.complete("Download cancelled");
            // Server handles config updates automatically now
        } else {
            loading.stop();
        }
        return success;
    }
    catch (const std::exception &)
    {
        loading.stop();
        return false;
    }
}

bool KolosalServerClient::cancelAll_Downloads()
{
    LoadingAnimation loading("Cancelling all downloads");
    loading.start();

    std::string response;
    std::string endpoint = "/v1/downloads/cancel";
    // Try v1 endpoint first, then fallback to non-v1
    if (!makePost_Request(endpoint, "{}", response))
    {
        endpoint = "/downloads/cancel";
        if (!makePost_Request(endpoint, "{}", response))
        {
            loading.stop();
            return false;
        }
    }

    try
    {
        json cancelJson = json::parse(response);
        const int cancelledCount = cancelJson.value("cancelled_count", 0);
        loading.complete("All downloads cancelled");
        return true;
    }
    catch (const std::exception &)
    {
        loading.stop();
        return false;
    }
}

bool KolosalServerClient::pause_Download(const std::string &modelId)
{
    LoadingAnimation loading("Pausing download");
    loading.start();

    std::string response;
    std::string endpoint = "/v1/downloads/" + modelId + "/pause";
    // Try v1 endpoint first, then fallback to non-v1
    if (!makePost_Request(endpoint, "{}", response))
    {
        endpoint = "/downloads/" + modelId + "/pause";
        if (!makePost_Request(endpoint, "{}", response))
        {
            loading.stop();
            return false;
        }
    }

    try
    {
        const json pauseJson = json::parse(response);
        const bool success = pauseJson.value("success", false);
        if (success) {
            loading.complete("Download paused");
        } else {
            loading.stop();
        }
        return success;
    }
    catch (const std::exception &)
    {
        loading.stop();
        return false;
    }
}

bool KolosalServerClient::resume_Download(const std::string &modelId)
{
    LoadingAnimation loading("Resuming download");
    loading.start();

    std::string response;
    std::string endpoint = "/v1/downloads/" + modelId + "/resume";
    // Try v1 endpoint first, then fallback to non-v1
    if (!makePost_Request(endpoint, "{}", response))
    {
        endpoint = "/downloads/" + modelId + "/resume";
        if (!makePost_Request(endpoint, "{}", response))
        {
            loading.stop();
            return false;
        }
    }

    try
    {
        const json resumeJson = json::parse(response);
        const bool success = resumeJson.value("success", false);
        if (success) {
            loading.complete("Download resumed");
        } else {
            loading.stop();
        }
        return success;
    }
    catch (const std::exception &)
    {
        loading.stop();
        return false;
    }
}

bool KolosalServerClient::getAll_Downloads(std::vector<std::tuple<std::string, std::string, double, long long, long long>>& downloads)
{
    std::string response;
    std::string endpoint = "/v1/downloads";
    // Try v1 endpoint first, then fallback to non-v1
    if (!makeGet_Request(endpoint, response))
    {
        endpoint = "/downloads";
        if (!makeGet_Request(endpoint, response))
        {
            return false;
        }
    }

    try
    {
        const json downloadsJson = json::parse(response);
    if (downloadsJson.contains("downloads") && downloadsJson["downloads"].is_array())
        {
            downloads.clear();
            for (const auto& download : downloadsJson["downloads"])
            {
                std::string modelId = download.value("model_id", "");
                std::string status = download.value("status", "");
                const double percentage = download.value("percentage", 0.0);
                long long downloadedBytes = download.value("downloaded_bytes", 0LL);
                long long totalBytes = download.value("total_bytes", 0LL);
                downloads.emplace_back(modelId, status, percentage, downloadedBytes, totalBytes);
            }
            return true;
        }
        
        return false;
    }
    catch (const std::exception &)
    {
        return false;
    }
}



bool KolosalServerClient::chat_Completion(const std::string& engineId, const std::string& message, std::string& response)
{
    try {
        const json requestBody = {
            {"model", engineId},
            {"messages", json::array({
                {{"role", "user"}, {"content", message}}
            })},
            {"streaming", false},
            {"maxNewTokens", 2048},
            {"temperature", 0.7},
            {"topP", 0.9}
        };
        std::string jsonResponse;
        if (!makePost_Request("/v1/inference/chat/completions", requestBody.dump(), jsonResponse)) {
            return false;
        }

        // Parse the response - using inference format
        const json responseJson = json::parse(jsonResponse);
        if (responseJson.contains("text")) {
            response = responseJson["text"].get<std::string>();
            return true;
        }

        return false;
    } catch (const std::exception&) {
        return false;
    }
}

bool KolosalServerClient::streamingChat_Completion(const std::string& engineId, const std::string& message, 
                                                std::function<void(const std::string&, double, double)> responseCallback)
{
    try {
        const json requestBody = {
            {"model", engineId},
            {"messages", json::array({
                {{"role", "user"}, {"content", message}}
            })},
            {"streaming", true},
            {"maxNewTokens", 2048},
            {"temperature", 0.7},
            {"topP", 0.9}
        };
        // For streaming, we need to make a custom HTTP request to handle Server-Sent Events
    std::string url = member_baseUrl + "/v1/inference/chat/completions";
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/json";
        headers["Accept"] = "text/event-stream";
        headers["Cache-Control"] = "no-cache";
        if (!member_apiKey.empty()) {
            headers["Authorization"] = "Bearer " + member_apiKey;
        }

        // Use a basic streaming implementation
        std::string buffer;
    bool receivedContent = false;
    bool streamComplete = false;
        const bool httpSuccess = HttpClient::get_Instance().makeStreaming_Request(url, requestBody.dump(), headers, 
            [&](const std::string& jsonData) -> bool {
                try {
                    json chunkJson = json::parse(jsonData);
                    // Handle inference chat completion response format
                    if (chunkJson.contains("text")) {
                        std::string content = chunkJson["text"].get<std::string>();
                        const double tps = chunkJson.value("tps", 0.0);
                        const double ttft = chunkJson.value("ttft", 0.0);
                        if (!content.empty()) {
                            responseCallback(content, tps, ttft);
                            receivedContent = true;
                        }
                    }
                    
                    // Check for completion - final chunk has partial = false
                    if (chunkJson.contains("partial") && !chunkJson["partial"].get<bool>()) {
                        streamComplete = true;
                        return false; // Stop streaming
                    }
                    
                    // Check for error
                    if (chunkJson.contains("error")) {
                        streamComplete = true;
                        return false; // Stop streaming
                    }
                    
                    return true; // Continue streaming
                } catch (const nlohmann::json::parse_error& e) {
                    // Log JSON parse errors but continue streaming for malformed chunks
                    ServerLogger::logDebug("JSON parse error in streaming chunk (continuing): %s", e.what());
                    return true; // Continue streaming even on parse errors
                } catch (const std::exception& e) {
                    // Log other exceptions during chunk processing
                    ServerLogger::logWarning("Exception processing streaming chunk: %s", e.what());
                    return true; // Continue streaming
                }
            });

        // Consider streaming successful if we received any content or completed stream, regardless of HTTP status
        return receivedContent || streamComplete;
    } catch (const std::exception&) {
        return false;
    }
}

bool KolosalServerClient::get_Logs(std::vector<std::tuple<std::string, std::string, std::string>>& logs) {
    try {
        std::string response;
    if (!makeGet_Request("/v1/logs", response) && !makeGet_Request("/logs", response)) {
            return false;
        }
        
        const json jsonResponse = json::parse(response);
    if (!jsonResponse.contains("logs") || !jsonResponse["logs"].is_array()) {
            return false;
        }
        
        logs.clear();
        for (const auto& logEntry : jsonResponse["logs"]) {
            if (logEntry.contains("level") && logEntry.contains("timestamp") && logEntry.contains("message")) {
                logs.emplace_back(
                    logEntry["level"].get<std::string>(),
                    logEntry["timestamp"].get<std::string>(),
                    logEntry["message"].get<std::string>()
                );
            }
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool KolosalServerClient::getInference_Engines(std::vector<std::tuple<std::string, std::string, std::string, std::string, bool>>& engines)
{
    std::string response;
    
    // Try both /v1/inference-engines and /inference-engines endpoints
    if (!makeGet_Request("/v1/inference-engines", response))
    {
        if (!makeGet_Request("/inference-engines", response))
        {
            return false;
        }
    }

    try
    {
        const json enginesJson = json::parse(response);
        engines.clear();
        
        // Handle inference engines response format
    if (enginesJson.contains("inference_engines") && enginesJson["inference_engines"].is_array())
        {
            for (const auto& engine : enginesJson["inference_engines"])
            {
                std::string name = engine.value("name", "");
                std::string version = engine.value("version", "");
                std::string description = engine.value("description", "");
                std::string library_path = engine.value("library_path", "");
                const bool is_loaded = engine.value("is_loaded", false);
                engines.emplace_back(name, version, description, library_path, is_loaded);
            }
        }
        
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool KolosalServerClient::addInference_Engine(const std::string& name, const std::string& libraryPath, bool loadOnStartup)
{
    try {
        // Prepare JSON payload for the POST request
        json payload;
        payload["name"] = name;
        payload["library_path"] = libraryPath;
        payload["load_on_startup"] = loadOnStartup;
        
    const std::string requestBody = payload.dump();
        std::string response;
        
        // Try both /v1/engines and /engines endpoints
        if (!makePost_Request("/v1/inference-engines", requestBody, response))
        {
            if (!makePost_Request("/inference-engines", requestBody, response))
            {
                return false;
            }
        }
        
        // Parse the response to check if addition was successful
        const json responseJson = json::parse(response);
        // Check if the response indicates success
        if (responseJson.contains("status") && responseJson["status"] == "success")
        {
            return true;
        }
        
        // Also accept if we get a success message
        if (responseJson.contains("message"))
        {
            std::string message = responseJson["message"].get<std::string>();
            return message.find("successfully") != std::string::npos ||
                   message.find("added") != std::string::npos;
        }
        
        return false;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool KolosalServerClient::remove_Model(const std::string& modelId)
{
    try {
        std::string response;
        // Use DELETE request to the RESTful endpoint /models/{id}
    std::string endpoint = "/models/" + modelId;
        if (!makeDelete_Request(endpoint, "", response)) {
            return false;
        }

        // Parse the response to check if removal was successful
        json responseJson = json::parse(response);
        return responseJson.contains("status") && responseJson["status"] == "removed";
    }
    catch (const std::exception&) {
        return false;
    }
}

bool KolosalServerClient::getModel_Status(const std::string& modelId, std::string& status, std::string& message)
{
    try {
        std::string response;
        // Use GET request to the RESTful status endpoint /models/{id}/status
    std::string endpoint = "/models/" + modelId + "/status";
        if (!makeGet_Request(endpoint, response)) {
            return false;
        }

        // Parse the response
        json responseJson = json::parse(response);
        status = responseJson.value("status", "unknown");
        message = responseJson.value("message", "");
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool KolosalServerClient::setDefaultInference_Engine(const std::string& engineName)
{
    try {
        // Prepare JSON payload for the PUT request
        json payload;
        payload["engine_name"] = engineName;
        
        std::string requestBody = payload.dump();
        std::string response;
        
        // Try both /v1/engines and /engines endpoints
        if (!makePut_Request("/v1/inference-engines/default", requestBody, response))
        {
            if (!makePut_Request("/inference-engines/default", requestBody, response))
            {
                return false;
            }
        }
        
        // Parse the response to check if setting was successful
        const json responseJson = json::parse(response);
        // Check if the response indicates success
        if (responseJson.contains("message"))
        {
            std::string message = responseJson["message"].get<std::string>();
            return message.find("successfully") != std::string::npos ||
                   message.find("set") != std::string::npos;
        }
        
        return false;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool KolosalServerClient::getDefaultInference_Engine(std::string& defaultEngine)
{
    std::string response;
    
    // Try both /v1/engines and /engines endpoints
    if (!makeGet_Request("/v1/inference-engines/default", response))
    {
        if (!makeGet_Request("/inference-engines/default", response))
        {
            return false;
        }
    }

    try
    {
        const json enginesJson = json::parse(response);
        // Extract the default engine from the response
        if (enginesJson.contains("default_engine"))
        {
            defaultEngine = enginesJson["default_engine"].get<std::string>();
            return true;
        }
        
        return false;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

