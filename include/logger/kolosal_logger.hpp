/**
 * @file kolosal_logger.hpp
 * @brief Advanced logging system for the Kolosal Agent System
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * This file provides a comprehensive logging framework with support for multiple
 * appenders, formatters, log levels, and thread-safe operations. It integrates
 * with the existing server logging infrastructure.
 */

#pragma once

#ifndef KOLOSAL_AGENT_LOGGER_HPP
#define KOLOSAL_AGENT_LOGGER_HPP

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

#include "export.hpp"
#include <string>
#include <memory>
#include <mutex>
#include <fstream>
#include <vector>
#include <functional>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace kolosal {

/**
 * @brief Log levels for the Kolosal Agent System
 */
enum class LogLevel {
    TRACE = 0,   // Most verbose - detailed execution flow
    DEBUG = 1,   // Debug information
    INFO = 2,    // General information
    WARN = 3,    // Warning messages
    ERROR = 4,   // Error messages
    FATAL = 5,   // Fatal errors that may cause application termination
    OFF = 6      // Logging disabled
};

/**
 * @brief Log entry structure
 */
struct LogEntry {
    LogLevel level;
    std::chrono::system_clock::time_point timestamp;
    std::string component;
    std::string message;
    std::thread::id thread_id;
};

/**
 * @brief Log formatter interface
 */
class LogFormatter {
public:
    virtual ~LogFormatter() = default;
    virtual std::string format(const LogEntry& entry) = 0;
};

/**
 * @brief Default log formatter with customizable formatting
 */
class DefaultLogFormatter : public LogFormatter {
public:
    DefaultLogFormatter(bool include_timestamp = true, 
                       bool include_level = true, 
                       bool include_component = true,
                       bool include_thread_id = false);
    
    std::string format(const LogEntry& entry) override;
    
private:
    bool include_timestamp_;
    bool include_level_;
    bool include_component_;
    bool include_thread_id_;
    
    std::string levelToString(LogLevel level) const;
    std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp) const;
};

/**
 * @brief Log appender interface for different output destinations
 */
class LogAppender {
public:
    virtual ~LogAppender() = default;
    virtual void append(const LogEntry& entry, const std::string& formatted_message) = 0;
    virtual void flush() = 0;
};

/**
 * @brief Console appender for logging to stdout/stderr
 */
class ConsoleAppender : public LogAppender {
public:
    ConsoleAppender(bool use_colors = true, bool errors_to_stderr = true);
    
    void append(const LogEntry& entry, const std::string& formatted_message) override;
    void flush() override;
    
private:
    bool use_colors_;
    bool errors_to_stderr_;
    std::mutex console_mutex_;
    
    std::string getColorCode(LogLevel level) const;
    std::string getResetCode() const;
};

/**
 * @brief File appender for logging to files with rotation support
 */
class FileAppender : public LogAppender {
public:
    FileAppender(const std::string& filename, 
                size_t max_file_size_mb = 10, 
                size_t max_backup_files = 5);
    
    ~FileAppender();
    
    void append(const LogEntry& entry, const std::string& formatted_message) override;
    void flush() override;
    
    bool isOpen() const;
    
private:
    std::string filename_;
    size_t max_file_size_bytes_;
    size_t max_backup_files_;
    std::ofstream file_stream_;
    size_t current_file_size_;
    mutable std::mutex file_mutex_;
    
    void rotateFile();
    void openFile();
};

/**
 * @brief Main logger class for the Kolosal Agent System
 */
class KOLOSAL_AGENT_API KolosalLogger {
public:
    static KolosalLogger& instance();
    
    // Non-copyable, non-movable
    KolosalLogger(const KolosalLogger&) = delete;
    KolosalLogger& operator=(const KolosalLogger&) = delete;
    KolosalLogger(KolosalLogger&&) = delete;
    KolosalLogger& operator=(KolosalLogger&&) = delete;
    
    // Configuration methods
    void setLevel(LogLevel level);
    LogLevel getLevel() const;
    
    void setComponent(const std::string& component);
    std::string getComponent() const;
    
    void addAppender(std::unique_ptr<LogAppender> appender);
    void clearAppenders();
    
    void setFormatter(std::unique_ptr<LogFormatter> formatter);
    
    // Logging methods
    void log(LogLevel level, const std::string& message);
    void log(LogLevel level, const std::string& component, const std::string& message);
    
    template<typename... Args>
    void log(LogLevel level, const std::string& format, Args&&... args);
    
    template<typename... Args>
    void log(LogLevel level, const std::string& component, const std::string& format, Args&&... args);
    
    // Convenience methods
    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
    
    template<typename... Args>
    void trace(const std::string& format, Args&&... args);
    
    template<typename... Args>
    void debug(const std::string& format, Args&&... args);
    
    template<typename... Args>
    void info(const std::string& format, Args&&... args);
    
    template<typename... Args>
    void warn(const std::string& format, Args&&... args);
    
    template<typename... Args>
    void error(const std::string& format, Args&&... args);
    
    template<typename... Args>
    void fatal(const std::string& format, Args&&... args);
    
    // Component-specific logging
    template<typename... Args>
    void trace(const std::string& component, const std::string& format, Args&&... args);
    
    template<typename... Args>
    void debug(const std::string& component, const std::string& format, Args&&... args);
    
    template<typename... Args>
    void info(const std::string& component, const std::string& format, Args&&... args);
    
    template<typename... Args>
    void warn(const std::string& component, const std::string& format, Args&&... args);
    
    template<typename... Args>
    void error(const std::string& component, const std::string& format, Args&&... args);
    
    template<typename... Args>
    void fatal(const std::string& component, const std::string& format, Args&&... args);
    
    // Utility methods
    void flush();
    void shutdown();
    
    // Integration with existing ServerLogger
    void enableServerLoggerIntegration(bool enable = true);
    bool isServerLoggerIntegrationEnabled() const;
    
    // Get log entries (for inspection/testing)
    std::vector<LogEntry> getRecentEntries(size_t max_entries = 100) const;
    
private:
    KolosalLogger();
    ~KolosalLogger();
    
    void logInternal(LogLevel level, const std::string& component, const std::string& message);
    
    template<typename... Args>
    std::string formatString(const std::string& format, Args&&... args);
    
    LogLevel current_level_;
    std::string default_component_;
    std::vector<std::unique_ptr<LogAppender>> appenders_;
    std::unique_ptr<LogFormatter> formatter_;
    mutable std::mutex logger_mutex_;
    bool server_logger_integration_;
    
    // Ring buffer for recent entries
    std::vector<LogEntry> recent_entries_;
    size_t recent_entries_index_;
    size_t max_recent_entries_;
};

// Template implementations
template<typename... Args>
void KolosalLogger::log(LogLevel level, const std::string& format, Args&&... args) {
    if (level >= current_level_) {
        logInternal(level, default_component_, formatString(format, std::forward<Args>(args)...));
    }
}

template<typename... Args>
void KolosalLogger::log(LogLevel level, const std::string& component, const std::string& format, Args&&... args) {
    if (level >= current_level_) {
        logInternal(level, component, formatString(format, std::forward<Args>(args)...));
    }
}

template<typename... Args>
void KolosalLogger::trace(const std::string& format, Args&&... args) {
    log(LogLevel::TRACE, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::debug(const std::string& format, Args&&... args) {
    log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::info(const std::string& format, Args&&... args) {
    log(LogLevel::INFO, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::warn(const std::string& format, Args&&... args) {
    log(LogLevel::WARN, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::error(const std::string& format, Args&&... args) {
    log(LogLevel::ERROR, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::fatal(const std::string& format, Args&&... args) {
    log(LogLevel::FATAL, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::trace(const std::string& component, const std::string& format, Args&&... args) {
    log(LogLevel::TRACE, component, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::debug(const std::string& component, const std::string& format, Args&&... args) {
    log(LogLevel::DEBUG, component, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::info(const std::string& component, const std::string& format, Args&&... args) {
    log(LogLevel::INFO, component, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::warn(const std::string& component, const std::string& format, Args&&... args) {
    log(LogLevel::WARN, component, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::error(const std::string& component, const std::string& format, Args&&... args) {
    log(LogLevel::ERROR, component, format, std::forward<Args>(args)...);
}

template<typename... Args>
void KolosalLogger::fatal(const std::string& component, const std::string& format, Args&&... args) {
    log(LogLevel::FATAL, component, format, std::forward<Args>(args)...);
}

template<typename... Args>
std::string KolosalLogger::formatString(const std::string& format, Args&&... args) {
    // Simple string formatting - can be enhanced with fmt library if available
    std::ostringstream oss;
    std::string result = format;
    
    // This is a simplified implementation
    // In a real implementation, you'd want to use a proper formatting library like fmt
    auto format_recursive = [&](auto&& self, const std::string& fmt, auto&& first, auto&&... rest) -> std::string {
        size_t pos = fmt.find("{}");
        if (pos != std::string::npos) {
            std::ostringstream temp;
            temp << first;
            std::string partial = fmt.substr(0, pos) + temp.str() + fmt.substr(pos + 2);
            if constexpr (sizeof...(rest) > 0) {
                return self(self, partial, std::forward<decltype(rest)>(rest)...);
            } else {
                return partial;
            }
        }
        return fmt;
    };
    
    if constexpr (sizeof...(args) > 0) {
        return format_recursive(format_recursive, format, std::forward<Args>(args)...);
    } else {
        return format;
    }
}

// Convenience macros
#define KOLOSAL_LOG(level, ...) kolosal::KolosalLogger::instance().log(level, __VA_ARGS__)
#define KOLOSAL_TRACE(...) kolosal::KolosalLogger::instance().trace(__VA_ARGS__)
#define KOLOSAL_DEBUG(...) kolosal::KolosalLogger::instance().debug(__VA_ARGS__)
#define KOLOSAL_INFO(...) kolosal::KolosalLogger::instance().info(__VA_ARGS__)
#define KOLOSAL_WARN(...) kolosal::KolosalLogger::instance().warn(__VA_ARGS__)
#define KOLOSAL_ERROR(...) kolosal::KolosalLogger::instance().error(__VA_ARGS__)
#define KOLOSAL_FATAL(...) kolosal::KolosalLogger::instance().fatal(__VA_ARGS__)

} // namespace kolosal

#endif // KOLOSAL_AGENT_LOGGER_HPP
