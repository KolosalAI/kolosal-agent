#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <atomic>
#include <future>
#include "logger.hpp"
#include "agent_config.hpp"
#include "agent_manager.hpp"
#include "workflow_manager.hpp"
#include "kolosal_server_launcher.hpp"
#include "kolosal_client.hpp"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#endif

using namespace KolosalAgent;

// Global test environment for integration testing
class IntegrationTestEnvironment : public ::testing::Environment {
private:
    std::unique_ptr<KolosalServerLauncher> server_launcher_;
    std::unique_ptr<AgentManager> agent_manager_;
    std::atomic<bool> server_ready_{false};
    std::atomic<bool> shutdown_requested_{false};

public:
    void SetUp() override {
        // Configure extensive debug logging
        auto& logger = Logger::instance();
        logger.set_level(LogLevel::DEBUG_LVL);
        logger.set_console_output(true);
        logger.set_file_output("kolosal_agent_integration_test.log");
        logger.enable_timestamps(true);
        logger.enable_thread_id(true);
        logger.enable_function_tracing(true);
        
        LOG_INFO("=== Kolosal Agent Integration Test Environment Starting ===");
        LOG_INFO("Initializing comprehensive test environment with server integration");
        
        try {
            // Step 1: Initialize Kolosal Server
            LOG_INFO("Step 1: Initializing Kolosal Server...");
            InitializeKolosalServer();
            
            // Step 2: Initialize Agent System
            LOG_INFO("Step 2: Initializing Agent System...");
            InitializeAgentSystem();
            
            // Step 3: Wait for system readiness
            LOG_INFO("Step 3: Waiting for system readiness...");
            WaitForSystemReadiness();
            
            LOG_INFO("Integration test environment setup complete!");
            LOG_INFO("🚀 System is ready for comprehensive testing");
            
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize integration test environment: %s", e.what());
            throw;
        }
    }
    
    void TearDown() override {
        LOG_INFO("=== Integration Test Environment Cleanup Starting ===");
        
        shutdown_requested_ = true;
        
        // Stop agent system first
        if (agent_manager_) {
            LOG_INFO("Stopping agent system...");
            try {
                agent_manager_->stop_all_agents();
                agent_manager_.reset();
                LOG_INFO("Agent system stopped successfully");
            } catch (const std::exception& e) {
                LOG_WARN("Error stopping agent system: %s", e.what());
            }
        }
        
        // Stop server
        if (server_launcher_) {
            LOG_INFO("Stopping Kolosal server...");
            try {
                server_launcher_->stop();
                server_launcher_.reset();
                LOG_INFO("Kolosal server stopped successfully");
            } catch (const std::exception& e) {
                LOG_WARN("Error stopping server: %s", e.what());
            }
        }
        
        LOG_INFO("=== Integration Test Environment Cleanup Complete ===");
    }

private:
    void InitializeKolosalServer() {
        try {
            // Create server configuration
            KolosalServerConfig server_config;
            server_config.executable_path = GetServerExecutablePath();
            server_config.host = "127.0.0.1";
            server_config.port = 8081;
            server_config.model_path = GetModelPath();
            server_config.log_level = "info";
            server_config.quiet = false;
            
            LOG_DEBUG("Server config - Host: %s, Port: %d", server_config.host.c_str(), server_config.port);
            LOG_DEBUG("Server executable path: %s", server_config.executable_path.c_str());
            
            // Create and configure server launcher
            server_launcher_ = std::make_unique<KolosalServerLauncher>(server_config);
            
            // Set up status callback
            server_launcher_->set_status_callback([this](const std::string& status) {
                LOG_INFO("Server status update: %s", status.c_str());
                if (status.find("ready") != std::string::npos || 
                    status.find("listening") != std::string::npos) {
                    server_ready_ = true;
                }
            });
            
            LOG_INFO("Starting Kolosal server...");
            if (!server_launcher_->start()) {
                throw std::runtime_error("Failed to start Kolosal server");
            }
            
            LOG_INFO("Kolosal server startup initiated");
            
        } catch (const std::exception& e) {
            LOG_ERROR("Server initialization failed: %s", e.what());
            throw;
        }
    }
    
    void InitializeAgentSystem() {
        try {
            // Load configuration
            auto config = std::make_shared<AgentConfig>();
            
            // Try to load from config file, or use defaults
            try {
                config->load_from_file("config.yaml");
                LOG_INFO("Loaded configuration from config.yaml");
            } catch (const std::exception& e) {
                LOG_WARN("Could not load config.yaml, using default configuration: %s", e.what());
                CreateDefaultConfig(config);
            }
            
            // Create agent manager with config
            agent_manager_ = std::make_unique<AgentManager>(config);
            
            LOG_INFO("Agent manager created successfully");
            
            // Initialize default agents if they don't exist
            InitializeDefaultAgents();
            
        } catch (const std::exception& e) {
            LOG_ERROR("Agent system initialization failed: %s", e.what());
            throw;
        }
    }
    
    void InitializeDefaultAgents() {
        try {
            LOG_INFO("Initializing default agents for testing...");
            
            // Create Assistant agent
            if (!agent_manager_->agent_exists("Assistant")) {
                agent_manager_->create_agent("Assistant", "chat");
                LOG_INFO("Created Assistant agent");
            }
            
            // Create RetrievalAgent if retrieval capabilities are available
            #ifdef BUILD_WITH_RETRIEVAL
            if (!agent_manager_->agent_exists("RetrievalAgent")) {
                std::vector<std::string> capabilities = {"chat", "retrieval"};
                agent_manager_->create_agent("RetrievalAgent", capabilities);
                LOG_INFO("Created RetrievalAgent with retrieval capabilities");
            }
            #endif
            
            // Start all agents
            for (const auto& agent_name : agent_manager_->list_agents()) {
                if (!agent_manager_->start_agent(agent_name)) {
                    LOG_WARN("Failed to start agent: %s", agent_name.c_str());
                } else {
                    LOG_INFO("Started agent: %s", agent_name.c_str());
                }
            }
            
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize default agents: %s", e.what());
            throw;
        }
    }
    
    void WaitForSystemReadiness() {
        LOG_INFO("Waiting for system components to be ready...");
        
        const int max_wait_seconds = 60;
        const int check_interval_ms = 500;
        int elapsed_ms = 0;
        
        while (elapsed_ms < max_wait_seconds * 1000 && !shutdown_requested_) {
            bool all_ready = true;
            
            // Check server readiness
            if (!server_ready_) {
                all_ready = false;
                LOG_DEBUG("Waiting for server to be ready...");
            }
            
            // Check if we can connect to server
            if (server_ready_ && !TestServerConnectivity()) {
                all_ready = false;
                LOG_DEBUG("Server not yet accepting connections...");
            }
            
            if (all_ready) {
                LOG_INFO("System is ready! (took %dms)", elapsed_ms);
                return;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(check_interval_ms));
            elapsed_ms += check_interval_ms;
        }
        
        if (shutdown_requested_) {
            throw std::runtime_error("System readiness check interrupted by shutdown");
        } else {
            throw std::runtime_error("System failed to become ready within timeout period");
        }
    }
    
    bool TestServerConnectivity() {
        try {
            KolosalClient client("http://127.0.0.1:8081");
            return client.is_server_healthy();
        } catch (const std::exception& e) {
            LOG_DEBUG("Server connectivity test failed: %s", e.what());
            return false;
        }
    }
    
    std::string GetServerExecutablePath() {
        // Try to find the server executable in common locations
        std::vector<std::string> possible_paths = {
            "build/kolosal-server/kolosal-server-inference.exe",
            "build/kolosal-server/Debug/kolosal-server-inference.exe",
            "kolosal-server/build/kolosal-server-inference.exe",
            "kolosal-server/build/Debug/kolosal-server-inference.exe",
            "./kolosal-server-inference.exe",
            "../kolosal-server/build/kolosal-server-inference.exe"
        };
        
        for (const auto& path : possible_paths) {
            #ifdef _WIN32
            if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                LOG_INFO("Found server executable at: %s", path.c_str());
                return path;
            }
            #else
            if (access(path.c_str(), F_OK) == 0) {
                LOG_INFO("Found server executable at: %s", path.c_str());
                return path;
            }
            #endif
        }
        
        LOG_WARN("Could not find server executable, using default path");
        return "kolosal-server/build/Debug/kolosal-server-inference.exe";
    }
    
    std::string GetModelPath() {
        // Try to find model directory
        std::vector<std::string> possible_paths = {
            "models",
            "build/models",
            "../models",
            "./kolosal-server/models"
        };
        
        for (const auto& path : possible_paths) {
            #ifdef _WIN32
            if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                LOG_INFO("Found models directory at: %s", path.c_str());
                return path;
            }
            #else
            struct stat info;
            if (stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode)) {
                LOG_INFO("Found models directory at: %s", path.c_str());
                return path;
            }
            #endif
        }
        
        LOG_WARN("Could not find models directory, using default");
        return "models";
    }
    
    void CreateDefaultConfig(std::shared_ptr<AgentConfig> config) {
        LOG_INFO("Creating default configuration for testing");
        
        // This would normally be loaded from a config file
        // For now, we'll use minimal defaults that work with the test environment
        
        // Set reasonable defaults
        config->system_instruction = "You are a helpful AI assistant designed for testing purposes.";
        config->host = "localhost";
        config->port = 8080;
        
        LOG_INFO("Default configuration created");
    }
    
public:
    AgentManager* GetAgentManager() const { return agent_manager_.get(); }
    KolosalServerLauncher* GetServerLauncher() const { return server_launcher_.get(); }
    bool IsReady() const { return server_ready_ && agent_manager_ != nullptr; }
};

// Global environment instance
static IntegrationTestEnvironment* g_test_env = nullptr;

// Custom test listener for integration testing
class IntegrationTestListener : public ::testing::EmptyTestEventListener {
public:
    void OnTestStart(const ::testing::TestInfo& test_info) override {
        LOG_INFO("🧪 Starting Integration Test: %s.%s", 
                test_info.test_case_name(), test_info.name());
        test_start_time_ = std::chrono::high_resolution_clock::now();
    }
    
    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - test_start_time_);
        
        if (test_info.result()->Passed()) {
            LOG_INFO("✅ Integration Test PASSED: %s.%s (Duration: %lldms)", 
                    test_info.test_case_name(), test_info.name(), duration.count());
        } else {
            LOG_ERROR("❌ Integration Test FAILED: %s.%s (Duration: %lldms)", 
                     test_info.test_case_name(), test_info.name(), duration.count());
        }
    }
    
    void OnTestCaseStart(const ::testing::TestCase& test_case) override {
        LOG_INFO("🚀 Starting Integration Test Suite: %s", test_case.name());
    }
    
    void OnTestCaseEnd(const ::testing::TestCase& test_case) override {
        LOG_INFO("🏁 Completed Integration Test Suite: %s (Tests: %d, Failures: %d)", 
                test_case.name(), test_case.total_test_count(), test_case.failed_test_count());
    }

private:
    std::chrono::high_resolution_clock::time_point test_start_time_;
};

int main(int argc, char **argv) {
    std::cout << "🧪 Kolosal Agent Integration Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Build Configuration: Debug with Full Integration" << std::endl;
    std::cout << "Test Framework: Google Test" << std::endl;
    std::cout << "Features: Server Auto-Start, Full System Testing" << std::endl;
    std::cout << "========================================" << std::endl;
    
    ::testing::InitGoogleTest(&argc, argv);
    
    // Create and add the integration test environment
    g_test_env = new IntegrationTestEnvironment();
    ::testing::AddGlobalTestEnvironment(g_test_env);
    
    // Add detailed test listener
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new IntegrationTestListener);
    
    // Configure test output
    ::testing::GTEST_FLAG(print_time) = true;
    ::testing::GTEST_FLAG(color) = "yes";
    
    std::cout << "🚀 Starting comprehensive integration test suite..." << std::endl;
    std::cout << "   • Kolosal Server will be automatically started" << std::endl;
    std::cout << "   • Agent system will be initialized" << std::endl;
    std::cout << "   • Full end-to-end testing will be performed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    std::cout << "========================================" << std::endl;
    std::cout << "🏁 Integration test execution completed with result: " << result << std::endl;
    std::cout << "📄 Check kolosal_agent_integration_test.log for detailed logs" << std::endl;
    
    if (result == 0) {
        std::cout << "✅ ALL INTEGRATION TESTS PASSED!" << std::endl;
        std::cout << "🎉 Kolosal Agent system is fully operational!" << std::endl;
    } else {
        std::cout << "❌ Some integration tests failed." << std::endl;
        std::cout << "🔍 Review logs for detailed error information." << std::endl;
    }
    
    std::cout << "========================================" << std::endl;
    
    return result;
}
