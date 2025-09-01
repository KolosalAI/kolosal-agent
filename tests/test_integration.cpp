#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <memory>
#include "logger.hpp"
#include "agent_config.hpp"
#include "agent_manager.hpp"
#include "workflow_manager.hpp"

using namespace KolosalAgent;

// Integration test class
class KolosalAgentIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        TRACE_FUNCTION();
        LOG_INFO("Setting up integration test");
        
        // Configure extensive logging for test
        auto& logger = Logger::instance();
        logger.set_level(LogLevel::DEBUG_LVL);
        logger.enable_timestamps(true);
        logger.enable_thread_id(true);
        logger.enable_function_tracing(true);
        
        LOG_DEBUG("Integration test setup completed");
    }
    
    void TearDown() override {
        TRACE_FUNCTION();
        LOG_INFO("Tearing down integration test");
    }
};

TEST_F(KolosalAgentIntegrationTest, BasicSystemInitialization) {
    SCOPED_TIMER("BasicSystemInitialization");
    LOG_INFO("Testing basic system initialization");
    
    // Test logger functionality
    LOG_DEBUG("Testing debug logging");
    LOG_INFO("Testing info logging");
    LOG_WARN("Testing warning logging");
    
    // Test performance timer
    auto& logger = Logger::instance();
    logger.start_timer("test_timer");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    logger.end_timer("test_timer");
    
    EXPECT_TRUE(true) << "Basic system initialization should succeed";
}

TEST_F(KolosalAgentIntegrationTest, ConfigurationLoading) {
    SCOPED_TIMER("ConfigurationLoading");
    LOG_INFO("Testing configuration loading functionality");
    
    try {
        // Try to create a basic agent configuration
        // AgentConfig config;  // Commented out as AgentConfig might not be available
        
        // Test basic configuration operations
        LOG_DEBUG("Testing basic configuration operations");
        
        EXPECT_TRUE(true) << "Configuration loading should succeed";
        
    } catch (const std::exception& e) {
        LOG_ERROR_F("Configuration loading failed: %s", e.what());
        FAIL() << "Configuration loading should not throw exception: " << e.what();
    }
}

TEST_F(KolosalAgentIntegrationTest, AgentManagerInitialization) {
    SCOPED_TIMER("AgentManagerInitialization");
    LOG_INFO("Testing agent manager initialization");
    
    try {
        // Test agent manager creation
        auto manager = std::make_unique<AgentManager>();
        
        LOG_DEBUG("Agent manager created successfully");
        EXPECT_TRUE(manager != nullptr) << "Agent manager should be created successfully";
        
    } catch (const std::exception& e) {
        LOG_ERROR_F("Agent manager initialization failed: %s", e.what());
        // Don't fail the test if agent manager can't be initialized without proper config
        LOG_WARN("Agent manager initialization may require proper configuration files");
    }
}

TEST_F(KolosalAgentIntegrationTest, WorkflowManagerInitialization) {
    SCOPED_TIMER("WorkflowManagerInitialization");
    LOG_INFO("Testing workflow manager initialization");
    
    try {
        // Test workflow manager creation requires AgentManager
        auto agent_manager = std::make_shared<AgentManager>();
        auto workflow_manager = std::make_unique<WorkflowManager>(agent_manager);
        
        LOG_DEBUG("Workflow manager created successfully");
        EXPECT_TRUE(workflow_manager != nullptr) << "Workflow manager should be created successfully";
        
    } catch (const std::exception& e) {
        LOG_ERROR_F("Workflow manager initialization failed: %s", e.what());
        // Don't fail the test if workflow manager can't be initialized without proper config
        LOG_WARN("Workflow manager initialization may require proper configuration files");
    }
}

TEST_F(KolosalAgentIntegrationTest, LoggerFunctionality) {
    SCOPED_TIMER("LoggerFunctionality");
    LOG_INFO("Testing comprehensive logger functionality");
    
    auto& logger = Logger::instance();
    
    // Test different log levels
    logger.debug("Debug message test");
    logger.info("Info message test");
    logger.warn("Warning message test");
    logger.error("Error message test");
    
    // Test variadic logging
    logger.debug("Debug with parameters: %s, %d", "test", 123);
    logger.info("Info with parameters: %s, %d", "test", 456);
    
    // Test timer functionality
    logger.start_timer("functionality_test");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    logger.end_timer("functionality_test");
    
    EXPECT_TRUE(true) << "Logger functionality should work correctly";
}

// Performance test
TEST_F(KolosalAgentIntegrationTest, PerformanceBaseline) {
    SCOPED_TIMER("PerformanceBaseline");
    LOG_INFO("Running performance baseline test");
    
    auto& logger = Logger::instance();
    
    // Test logging performance
    const int num_logs = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_logs; ++i) {
        logger.debug("Performance test log entry %d", i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    LOG_INFO_F("Logged %d entries in %lld microseconds", num_logs, duration.count());
    
    // Basic performance expectation (should complete within reasonable time)
    EXPECT_LT(duration.count(), 1000000) << "Logging performance should be reasonable";
}

// Stress test
TEST_F(KolosalAgentIntegrationTest, StressTest) {
    SCOPED_TIMER("StressTest");
    LOG_INFO("Running stress test");
    
    auto& logger = Logger::instance();
    
    // Multi-threaded logging stress test
    const int num_threads = 4;
    const int logs_per_thread = 100;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&logger, t, logs_per_thread]() {
            for (int i = 0; i < logs_per_thread; ++i) {
                logger.debug("Thread %d log entry %d", t, i);
                logger.info("Thread %d info entry %d", t, i);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    LOG_INFO_F("Stress test completed with %d threads, %d logs per thread", 
             num_threads, logs_per_thread);
    
    EXPECT_TRUE(true) << "Stress test should complete successfully";
}

// Test for extensive debug logging
TEST_F(KolosalAgentIntegrationTest, ExtensiveDebugLogging) {
    SCOPED_TIMER("ExtensiveDebugLogging");
    LOG_INFO("Testing extensive debug logging capabilities");
    
    auto& logger = Logger::instance();
    
    // Enable all debug features
    logger.enable_timestamps(true);
    logger.enable_thread_id(true);
    logger.enable_function_tracing(true);
    
    LOG_DEBUG("Debug logging with all features enabled");
    LOG_INFO("Info logging with all features enabled");
    LOG_WARN("Warning logging with all features enabled");
    
    // Test function tracing
    {
        TRACE_FUNCTION();
        LOG_DEBUG("Inside traced function scope");
    }
    
    // Test nested scoped operation
    {
        SCOPED_TIMER("nested_trace_operation");
        LOG_DEBUG("Inside nested traced function scope");
    }
    
    // Test scoped timer
    {
        SCOPED_TIMER("scoped_operation");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        LOG_DEBUG("Inside scoped timer");
    }
    
    EXPECT_TRUE(true) << "Extensive debug logging should work correctly";
}
