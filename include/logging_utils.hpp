/**
 * @file logging_utils.hpp
 * @brief Enhanced logging utilities and component-based loggers for the Kolosal Agent System
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * This file provides comprehensive logging utilities including component-based loggers,
 * performance monitoring, and configuration management for the entire system.
 */

#pragma once

#ifndef KOLOSAL_LOGGING_UTILS_HPP
#define KOLOSAL_LOGGING_UTILS_HPP

// Prevent Windows macros from interfering with our LogLevel enum
#ifdef _WIN32
#ifdef ERROR
#undef ERROR
#endif
#ifdef WARN
#undef WARN
#endif
#ifdef DEBUG
#undef DEBUG
#endif
#ifdef FATAL
#undef FATAL
#endif
#endif

#include "kolosal_logger.hpp"
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace kolosal::logging {

/**
 * @brief Component-based logger wrapper for easier usage
 */
class ComponentLogger {
public:
    ComponentLogger(const std::string& component_name) 
        : component_name_(component_name) {}
    
    template<typename... Args>
    void trace(const std::string& format, Args&&... args) {
        kolosal::KolosalLogger::instance().trace(component_name_, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void debug(const std::string& format, Args&&... args) {
        kolosal::KolosalLogger::instance().debug(component_name_, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(const std::string& format, Args&&... args) {
        kolosal::KolosalLogger::instance().info(component_name_, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warn(const std::string& format, Args&&... args) {
        kolosal::KolosalLogger::instance().warn(component_name_, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(const std::string& format, Args&&... args) {
        kolosal::KolosalLogger::instance().error(component_name_, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void fatal(const std::string& format, Args&&... args) {
        kolosal::KolosalLogger::instance().fatal(component_name_, format, std::forward<Args>(args)...);
    }
    
    // Simple string versions
    void trace(const std::string& message) {
        kolosal::KolosalLogger::instance().trace(component_name_, message);
    }
    
    void debug(const std::string& message) {
        kolosal::KolosalLogger::instance().debug(component_name_, message);
    }
    
    void info(const std::string& message) {
        kolosal::KolosalLogger::instance().info(component_name_, message);
    }
    
    void warn(const std::string& message) {
        kolosal::KolosalLogger::instance().warn(component_name_, message);
    }
    
    void error(const std::string& message) {
        kolosal::KolosalLogger::instance().error(component_name_, message);
    }
    
    void fatal(const std::string& message) {
        kolosal::KolosalLogger::instance().fatal(component_name_, message);
    }
    
private:
    std::string component_name_;
};

/**
 * @brief Utility functions for setting up logging
 */
class LoggingConfig {
public:
    /**
     * @brief Configure logging based on string level
     */
    static void setLogLevel(const std::string& level) {
        kolosal::LogLevel log_level = kolosal::LogLevel::INFO; // default
        
        std::string upper_level = level;
        std::transform(upper_level.begin(), upper_level.end(), upper_level.begin(), ::toupper);
        
        if (upper_level == "TRACE") {
            log_level = kolosal::LogLevel::TRACE;
        } else if (upper_level == "DEBUG") {
            log_level = kolosal::LogLevel::DEBUG;
        } else if (upper_level == "INFO") {
            log_level = kolosal::LogLevel::INFO;
        } else if (upper_level == "WARN" || upper_level == "WARNING") {
            log_level = kolosal::LogLevel::WARN;
        } else if (upper_level == "ERROR") {
            log_level = kolosal::LogLevel::ERROR;
        } else if (upper_level == "FATAL") {
            log_level = kolosal::LogLevel::FATAL;
        } else if (upper_level == "OFF") {
            log_level = kolosal::LogLevel::OFF;
        }
        
        kolosal::KolosalLogger::instance().setLevel(log_level);
    }
    
    /**
     * @brief Add file logging with optional rotation
     */
    static bool addFileLogging(const std::string& filename, 
                              size_t max_file_size_mb = 10, 
                              size_t max_backup_files = 5) {
        auto file_appender = std::make_unique<kolosal::FileAppender>(filename, max_file_size_mb, max_backup_files);
        bool success = file_appender->isOpen();
        
        if (success) {
            kolosal::KolosalLogger::instance().addAppender(std::move(file_appender));
        }
        
        return success;
    }
    
    /**
     * @brief Configure console logging
     */
    static void configureConsoleLogging(bool use_colors = true, bool errors_to_stderr = true) {
        // Clear existing appenders and add new console appender
        kolosal::KolosalLogger::instance().clearAppenders();
        kolosal::KolosalLogger::instance().addAppender(
            std::make_unique<kolosal::ConsoleAppender>(use_colors, errors_to_stderr)
        );
    }
    
    /**
     * @brief Setup default logging configuration for production
     */
    static void setupProductionLogging(const std::string& log_file = "", bool quiet_console = false) {
        setLogLevel("INFO");
        
        if (!quiet_console) {
            configureConsoleLogging(false, true); // No colors in production
        } else {
            kolosal::KolosalLogger::instance().clearAppenders();
        }
        
        if (!log_file.empty()) {
            addFileLogging(log_file, 50, 10); // 50MB files, keep 10 backups
        }
    }
    
    /**
     * @brief Setup logging for development
     */
    static void setupDevelopmentLogging(const std::string& log_file = "") {
        setLogLevel("DEBUG");
        configureConsoleLogging(true, true); // Colors enabled
        
        if (!log_file.empty()) {
            addFileLogging(log_file, 10, 3); // Smaller files for dev
        }
    }
    
    /**
     * @brief Setup minimal logging (errors only to stderr)
     */
    static void setupMinimalLogging() {
        setLogLevel("ERROR");
        kolosal::KolosalLogger::instance().clearAppenders();
        kolosal::KolosalLogger::instance().addAppender(
            std::make_unique<kolosal::ConsoleAppender>(false, true)
        );
    }
};

/**
 * @brief Performance logging utility
 */
class PerformanceLogger {
public:
    PerformanceLogger(const std::string& component, const std::string& operation)
        : component_(component)
        , operation_(operation)
        , start_time_(std::chrono::high_resolution_clock::now()) {
        
        kolosal::KolosalLogger::instance().debug(component_, "Started: {}", operation_);
    }
    
    ~PerformanceLogger() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
        
        kolosal::KolosalLogger::instance().info(component_, "Completed: {} (took {}ms)", operation_, duration.count());
    }
    
private:
    std::string component_;
    std::string operation_;
    std::chrono::high_resolution_clock::time_point start_time_;
};

/**
 * @brief RAII-based logging scope
 */
class LoggingScope {
public:
    LoggingScope(const std::string& component, const std::string& scope_name)
        : logger_(component), scope_name_(scope_name) {
        logger_.trace("Entering scope: {}", scope_name_);
    }
    
    ~LoggingScope() {
        logger_.trace("Exiting scope: {}", scope_name_);
    }
    
private:
    ComponentLogger logger_;
    std::string scope_name_;
};

// Convenience macros for component loggers
#define DECLARE_COMPONENT_LOGGER(component_name) \
    static kolosal::logging::ComponentLogger component_name##_logger(#component_name)

#define COMPONENT_TRACE(component, ...) component##_logger.trace(__VA_ARGS__)
#define COMPONENT_DEBUG(component, ...) component##_logger.debug(__VA_ARGS__)
#define COMPONENT_INFO(component, ...) component##_logger.info(__VA_ARGS__)
#define COMPONENT_WARN(component, ...) component##_logger.warn(__VA_ARGS__)
#define COMPONENT_ERROR(component, ...) component##_logger.error(__VA_ARGS__)
#define COMPONENT_FATAL(component, ...) component##_logger.fatal(__VA_ARGS__)

// Performance monitoring macros
#define PERF_LOG_UNIQUE_NAME(name, line) perf_log_##name##_##line
#define PERF_LOG_MAKE_UNIQUE(name, line) PERF_LOG_UNIQUE_NAME(name, line)

#define PERF_LOG(component, operation) \
    kolosal::logging::PerformanceLogger PERF_LOG_MAKE_UNIQUE(__COUNTER__, __LINE__)(component, operation)

#define SCOPE_LOG_UNIQUE_NAME(name, line) scope_log_##name##_##line
#define SCOPE_LOG_MAKE_UNIQUE(name, line) SCOPE_LOG_UNIQUE_NAME(name, line)

#define SCOPE_LOG(component, scope_name) \
    kolosal::logging::LoggingScope SCOPE_LOG_MAKE_UNIQUE(__COUNTER__, __LINE__)(component, scope_name)

} // namespace kolosal::logging

#endif // KOLOSAL_LOGGING_UTILS_HPP
