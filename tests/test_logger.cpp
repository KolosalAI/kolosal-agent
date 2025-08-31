#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "logger.hpp"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace KolosalAgent;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_log_file = "test_logger.log";
        
        // Reset logger to default state
        Logger::instance().set_level(LogLevel::INFO_LVL);
        Logger::instance().set_console_output(true);
        Logger::instance().enable_timestamps(true);
        Logger::instance().enable_thread_id(false);
        Logger::instance().enable_function_tracing(false);
    }

    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(test_log_file)) {
            std::filesystem::remove(test_log_file);
        }
    }

    std::string test_log_file;
};

TEST_F(LoggerTest, SingletonInstance) {
    Logger& logger1 = Logger::instance();
    Logger& logger2 = Logger::instance();
    
    EXPECT_EQ(&logger1, &logger2);
}

TEST_F(LoggerTest, LogLevelManagement) {
    Logger& logger = Logger::instance();
    
    logger.set_level(LogLevel::DEBUG_LVL);
    EXPECT_EQ(logger.get_level(), LogLevel::DEBUG_LVL);
    EXPECT_TRUE(logger.should_log(LogLevel::DEBUG_LVL));
    EXPECT_TRUE(logger.should_log(LogLevel::INFO_LVL));
    EXPECT_TRUE(logger.should_log(LogLevel::ERROR_LVL));
    
    logger.set_level(LogLevel::ERROR_LVL);
    EXPECT_EQ(logger.get_level(), LogLevel::ERROR_LVL);
    EXPECT_FALSE(logger.should_log(LogLevel::DEBUG_LVL));
    EXPECT_FALSE(logger.should_log(LogLevel::INFO_LVL));
    EXPECT_TRUE(logger.should_log(LogLevel::ERROR_LVL));
}

TEST_F(LoggerTest, BasicLogging) {
    Logger& logger = Logger::instance();
    logger.set_level(LogLevel::DEBUG_LVL);
    
    // These should not throw
    EXPECT_NO_THROW(logger.debug("Debug message"));
    EXPECT_NO_THROW(logger.info("Info message"));
    EXPECT_NO_THROW(logger.warn("Warning message"));
    EXPECT_NO_THROW(logger.error("Error message"));
    EXPECT_NO_THROW(logger.fatal("Fatal message"));
}

TEST_F(LoggerTest, LoggingWithDebugInfo) {
    Logger& logger = Logger::instance();
    logger.set_level(LogLevel::DEBUG_LVL);
    
    // Test logging with function, file, and line info
    EXPECT_NO_THROW(logger.debug("Debug message", "test_function", "test_file.cpp", 42));
    EXPECT_NO_THROW(logger.info("Info message", "test_function", "test_file.cpp", 43));
    EXPECT_NO_THROW(logger.warn("Warning message", "test_function", "test_file.cpp", 44));
    EXPECT_NO_THROW(logger.error("Error message", "test_function", "test_file.cpp", 45));
    EXPECT_NO_THROW(logger.fatal("Fatal message", "test_function", "test_file.cpp", 46));
}

TEST_F(LoggerTest, VariadicTemplateLogging) {
    Logger& logger = Logger::instance();
    logger.set_level(LogLevel::DEBUG_LVL);
    
    // Test template logging with format strings
    EXPECT_NO_THROW(logger.debug("Debug: %s %d", "test", 42));
    EXPECT_NO_THROW(logger.info("Info: %s %d", "test", 42));
    EXPECT_NO_THROW(logger.warn("Warning: %s %d", "test", 42));
    EXPECT_NO_THROW(logger.error("Error: %s %d", "test", 42));
    EXPECT_NO_THROW(logger.fatal("Fatal: %s %d", "test", 42));
}

TEST_F(LoggerTest, FileLogging) {
    Logger& logger = Logger::instance();
    logger.set_file_output(test_log_file);
    logger.set_level(LogLevel::INFO_LVL);
    
    logger.info("Test file logging message");
    
    // Give some time for file writing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(std::filesystem::exists(test_log_file));
    
    std::ifstream file(test_log_file);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    EXPECT_THAT(content, ::testing::HasSubstr("Test file logging message"));
}

TEST_F(LoggerTest, ConsoleOutputToggle) {
    Logger& logger = Logger::instance();
    
    logger.set_console_output(true);
    EXPECT_NO_THROW(logger.info("Console enabled message"));
    
    logger.set_console_output(false);
    EXPECT_NO_THROW(logger.info("Console disabled message"));
}

TEST_F(LoggerTest, TimestampToggle) {
    Logger& logger = Logger::instance();
    
    logger.enable_timestamps(true);
    EXPECT_NO_THROW(logger.info("Message with timestamp"));
    
    logger.enable_timestamps(false);
    EXPECT_NO_THROW(logger.info("Message without timestamp"));
}

TEST_F(LoggerTest, ThreadIdToggle) {
    Logger& logger = Logger::instance();
    
    logger.enable_thread_id(true);
    EXPECT_NO_THROW(logger.info("Message with thread ID"));
    
    logger.enable_thread_id(false);
    EXPECT_NO_THROW(logger.info("Message without thread ID"));
}

TEST_F(LoggerTest, FunctionTracingToggle) {
    Logger& logger = Logger::instance();
    
    logger.enable_function_tracing(true);
    EXPECT_NO_THROW(logger.trace_function_entry("test_function", "test_file.cpp", 100));
    EXPECT_NO_THROW(logger.trace_function_exit("test_function", "test_file.cpp", 100));
    
    logger.enable_function_tracing(false);
    EXPECT_NO_THROW(logger.trace_function_entry("test_function", "test_file.cpp", 100));
    EXPECT_NO_THROW(logger.trace_function_exit("test_function", "test_file.cpp", 100));
}

TEST_F(LoggerTest, PerformanceTimers) {
    Logger& logger = Logger::instance();
    
    std::string timer_name = "test_timer";
    
    EXPECT_NO_THROW(logger.start_timer(timer_name));
    
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_NO_THROW(logger.end_timer(timer_name));
}

TEST_F(LoggerTest, FunctionTracerRAII) {
    Logger& logger = Logger::instance();
    logger.enable_function_tracing(true);
    
    {
        FunctionTracer tracer("test_function", "test_file.cpp", 200);
        // Tracer should log entry here
        
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        
        // Tracer should log exit when destroyed
    }
    
    // Should not throw
    EXPECT_TRUE(true);
}

TEST_F(LoggerTest, ScopedTimerRAII) {
    Logger& logger = Logger::instance();
    
    {
        ScopedTimer timer("test_scoped_timer");
        // Timer should start here
        
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        
        // Timer should end when destroyed
    }
    
    // Should not throw
    EXPECT_TRUE(true);
}

TEST_F(LoggerTest, ConcurrentLogging) {
    Logger& logger = Logger::instance();
    logger.set_level(LogLevel::DEBUG_LVL);
    
    const int num_threads = 10;
    const int messages_per_thread = 100;
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&logger, t, messages_per_thread]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                logger.info("Thread %d message %d", t, i);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Should complete without crashing
    EXPECT_TRUE(true);
}

TEST_F(LoggerTest, LogLevelFiltering) {
    Logger& logger = Logger::instance();
    logger.set_file_output(test_log_file);
    logger.set_level(LogLevel::WARN_LVL);
    
    logger.debug("This debug message should not appear");
    logger.info("This info message should not appear");
    logger.warn("This warning message should appear");
    logger.error("This error message should appear");
    
    // Give some time for file writing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::ifstream file(test_log_file);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    EXPECT_THAT(content, ::testing::Not(::testing::HasSubstr("debug message")));
    EXPECT_THAT(content, ::testing::Not(::testing::HasSubstr("info message")));
    EXPECT_THAT(content, ::testing::HasSubstr("warning message"));
    EXPECT_THAT(content, ::testing::HasSubstr("error message"));
}

TEST_F(LoggerTest, MacroLogging) {
    Logger& logger = Logger::instance();
    logger.set_level(LogLevel::DEBUG_LVL);
    
    // Test logging macros (should not throw)
    EXPECT_NO_THROW(LOG_DEBUG("Debug macro test"));
    EXPECT_NO_THROW(LOG_INFO("Info macro test"));
    EXPECT_NO_THROW(LOG_WARN("Warning macro test"));
    EXPECT_NO_THROW(LOG_ERROR("Error macro test"));
    EXPECT_NO_THROW(LOG_FATAL("Fatal macro test"));
}

TEST_F(LoggerTest, FormattedMacroLogging) {
    Logger& logger = Logger::instance();
    logger.set_level(LogLevel::DEBUG_LVL);
    
    // Test formatted logging macros
    EXPECT_NO_THROW(LOG_DEBUG_F("Debug formatted: %s %d", "test", 42));
    EXPECT_NO_THROW(LOG_INFO_F("Info formatted: %s %d", "test", 42));
    EXPECT_NO_THROW(LOG_WARN_F("Warning formatted: %s %d", "test", 42));
    EXPECT_NO_THROW(LOG_ERROR_F("Error formatted: %s %d", "test", 42));
    EXPECT_NO_THROW(LOG_FATAL_F("Fatal formatted: %s %d", "test", 42));
}

TEST_F(LoggerTest, SimplifiedMacroLogging) {
    Logger& logger = Logger::instance();
    logger.set_level(LogLevel::DEBUG_LVL);
    
    // Test simplified logging macros
    EXPECT_NO_THROW(SIMPLE_LOG_DEBUG("Simple debug test"));
    EXPECT_NO_THROW(SIMPLE_LOG_INFO("Simple info test"));
    EXPECT_NO_THROW(SIMPLE_LOG_WARN("Simple warning test"));
    EXPECT_NO_THROW(SIMPLE_LOG_ERROR("Simple error test"));
    EXPECT_NO_THROW(SIMPLE_LOG_FATAL("Simple fatal test"));
}
