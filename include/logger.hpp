#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <mutex>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <sstream>
#include <cstdio>

namespace KolosalAgent {

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
    FATAL = 5
};

// Convenience constants for backward compatibility
const LogLevel TRACE_LVL = LogLevel::TRACE;
const LogLevel DEBUG_LVL = LogLevel::DEBUG;
const LogLevel INFO_LVL = LogLevel::INFO;
const LogLevel WARNING_LVL = LogLevel::WARNING;
const LogLevel ERROR_LVL = LogLevel::ERROR;
const LogLevel FATAL_LVL = LogLevel::FATAL;

// Legacy enum for backward compatibility
enum LogLevelLegacy {
    INFO_LVL_LEGACY = 2
};

class Logger {
public:
    static Logger& instance();
    
    // For backward compatibility
    static Logger& get_instance();
    
    // Configuration
    void set_level(LogLevel level);
    void set_level(int level);
    void set_log_level(const std::string& level);
    void set_console_output(bool enabled);
    void enable_timestamps(bool enabled);
    void set_log_file(const std::string& filename);
    
    // Core logging methods
    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
    
    // Formatted logging methods
    void trace_f(const std::string& format);
    void debug_f(const std::string& format);
    void info_f(const std::string& format);
    void warn_f(const std::string& format);
    void error_f(const std::string& format);
    void fatal_f(const std::string& format);
    
    // Legacy methods with explicit parameters for backward compatibility  
    void debug(const std::string& message, const std::string& function, const std::string& file, int line);
    void info(const std::string& message, const std::string& function, const std::string& file, int line);
    void warn(const std::string& message, const std::string& function, const std::string& file, int line);
    void error(const std::string& message, const std::string& function, const std::string& file, int line);
    void fatal(const std::string& message, const std::string& function, const std::string& file, int line);
    
    // Utility methods
    bool should_log(LogLevel level) const;
    
    // Control
    void shutdown();
    
private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Internal logging method
    void log_message(LogLevel level, const std::string& message);
    std::string level_to_string(LogLevel level) const;
    
    LogLevel current_level_;
    bool console_output_enabled_;
    bool file_output_enabled_;
    bool timestamps_enabled_;
    bool thread_id_enabled_;
    bool function_tracing_enabled_;
    bool colors_enabled_;
    
    std::string filename_;
    std::ofstream file_stream_;
    std::mutex log_mutex_;
};

// RAII class for function tracing
class FunctionTracer {
public:
    FunctionTracer(const std::string& function, const std::string& file, int line);
    ~FunctionTracer();
    
private:
    std::string function_name_;
    std::string file_name_;
    int line_number_;
};

// Timer RAII class
class ScopedTimer {
public:
    ScopedTimer(const std::string& name);
    ~ScopedTimer();
    
private:
    std::string timer_name_;
};

// Convenience macros that automatically include debug information
#ifdef DEBUG_BUILD
    #define LOG_DEBUG(message) KolosalAgent::Logger::instance().debug(message, __FUNCTION__, __FILE__, __LINE__)
    #define LOG_INFO(message) KolosalAgent::Logger::instance().info(message, __FUNCTION__, __FILE__, __LINE__)
    #define LOG_WARN(message) KolosalAgent::Logger::instance().warn(message, __FUNCTION__, __FILE__, __LINE__)
    #define LOG_ERROR(message) KolosalAgent::Logger::instance().error(message, __FUNCTION__, __FILE__, __LINE__)
    #define LOG_FATAL(message) KolosalAgent::Logger::instance().fatal(message, __FUNCTION__, __FILE__, __LINE__)
    
    #define LOG_DEBUG_F(format, ...) { \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        KolosalAgent::Logger::instance().debug(std::string(buffer), __FUNCTION__, __FILE__, __LINE__); \
    }
    #define LOG_INFO_F(format, ...) { \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        KolosalAgent::Logger::instance().info(std::string(buffer), __FUNCTION__, __FILE__, __LINE__); \
    }
    #define LOG_WARN_F(format, ...) { \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        KolosalAgent::Logger::instance().warn(std::string(buffer), __FUNCTION__, __FILE__, __LINE__); \
    }
    #define LOG_ERROR_F(format, ...) { \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        KolosalAgent::Logger::instance().error(std::string(buffer), __FUNCTION__, __FILE__, __LINE__); \
    }
    #define LOG_FATAL_F(format, ...) { \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        KolosalAgent::Logger::instance().fatal(std::string(buffer), __FUNCTION__, __FILE__, __LINE__); \
    }
    
    #define TRACE_FUNCTION() KolosalAgent::FunctionTracer _tracer(__FUNCTION__, __FILE__, __LINE__)
    #define SCOPED_TIMER(name) KolosalAgent::ScopedTimer _timer(name)
#else
    #define LOG_DEBUG(message) KolosalAgent::Logger::instance().debug(message)
    #define LOG_INFO(message) KolosalAgent::Logger::instance().info(message)
    #define LOG_WARN(message) KolosalAgent::Logger::instance().warn(message)
    #define LOG_ERROR(message) KolosalAgent::Logger::instance().error(message)
    #define LOG_FATAL(message) KolosalAgent::Logger::instance().fatal(message)
    
    #define LOG_DEBUG_F(format, ...) { \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        KolosalAgent::Logger::instance().debug(std::string(buffer)); \
    }
    #define LOG_INFO_F(format, ...) { \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        KolosalAgent::Logger::instance().info(std::string(buffer)); \
    }
    #define LOG_WARN_F(format, ...) { \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        KolosalAgent::Logger::instance().warn(std::string(buffer)); \
    }
    #define LOG_ERROR_F(format, ...) { \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        KolosalAgent::Logger::instance().error(std::string(buffer)); \
    }
    #define LOG_FATAL_F(format, ...) { \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        KolosalAgent::Logger::instance().fatal(std::string(buffer)); \
    }
    
    #define TRACE_FUNCTION() // No-op in release builds
    #define SCOPED_TIMER(name) // No-op in release builds
#endif

} // namespace KolosalAgent
