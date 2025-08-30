#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <memory>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <unordered_map>

// Prevent Windows ERROR macro conflicts
#ifdef ERROR
#undef ERROR
#endif

namespace KolosalAgent {

enum class LogLevel {
    DEBUG_LVL = 0,
    INFO_LVL = 1,
    WARN_LVL = 2,
    ERROR_LVL = 3,
    FATAL_LVL = 4
};

class Logger {
public:
    static Logger& instance();
    
    // Configure logger
    void set_level(LogLevel level);
    void set_console_output(bool enabled);
    void set_file_output(const std::string& filename);
    void enable_timestamps(bool enabled);
    void enable_thread_id(bool enabled);
    void enable_function_tracing(bool enabled);
    
    // Logging methods
    void debug(const std::string& message, const std::string& function = "", const std::string& file = "", int line = 0);
    void info(const std::string& message, const std::string& function = "", const std::string& file = "", int line = 0);
    void warn(const std::string& message, const std::string& function = "", const std::string& file = "", int line = 0);
    void error(const std::string& message, const std::string& function = "", const std::string& file = "", int line = 0);
    void fatal(const std::string& message, const std::string& function = "", const std::string& file = "", int line = 0);
    
    // Variadic template logging
    template<typename... Args>
    void debug(const std::string& format, Args&&... args) {
        if (should_log(LogLevel::DEBUG_LVL)) {
            log(LogLevel::DEBUG_LVL, format_string(format, std::forward<Args>(args)...), "", "", 0);
        }
    }
    
    template<typename... Args>
    void info(const std::string& format, Args&&... args) {
        if (should_log(LogLevel::INFO_LVL)) {
            log(LogLevel::INFO_LVL, format_string(format, std::forward<Args>(args)...), "", "", 0);
        }
    }
    
    template<typename... Args>
    void warn(const std::string& format, Args&&... args) {
        if (should_log(LogLevel::WARN_LVL)) {
            log(LogLevel::WARN_LVL, format_string(format, std::forward<Args>(args)...), "", "", 0);
        }
    }
    
    template<typename... Args>
    void error(const std::string& format, Args&&... args) {
        if (should_log(LogLevel::ERROR_LVL)) {
            log(LogLevel::ERROR_LVL, format_string(format, std::forward<Args>(args)...), "", "", 0);
        }
    }
    
    template<typename... Args>
    void fatal(const std::string& format, Args&&... args) {
        if (should_log(LogLevel::FATAL_LVL)) {
            log(LogLevel::FATAL_LVL, format_string(format, std::forward<Args>(args)...), "", "", 0);
        }
    }
    
    // Function tracing for debug builds
    void trace_function_entry(const std::string& function, const std::string& file, int line);
    void trace_function_exit(const std::string& function, const std::string& file, int line);
    
    // Performance timing
    void start_timer(const std::string& timer_name);
    void end_timer(const std::string& timer_name);
    
    LogLevel get_level() const { return current_level_; }
    bool should_log(LogLevel level) const { return level >= current_level_; }
    
private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void log(LogLevel level, const std::string& message, const std::string& function, const std::string& file, int line);
    std::string get_timestamp() const;
    std::string get_thread_id() const;
    std::string level_to_string(LogLevel level) const;
    std::string level_to_color(LogLevel level) const;
    
    template<typename... Args>
    std::string format_string(const std::string& format, Args&&... args) {
        size_t size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1);
    }
    
    LogLevel current_level_;
    bool console_output_enabled_;
    bool file_output_enabled_;
    bool timestamps_enabled_;
    bool thread_id_enabled_;
    bool function_tracing_enabled_;
    bool colors_enabled_;
    
    std::string log_filename_;
    std::ofstream log_file_;
    std::mutex log_mutex_;
    
    // Performance timing
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> timers_;
    std::mutex timer_mutex_;
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
    std::chrono::steady_clock::time_point start_time_;
};

// Timer RAII class
class ScopedTimer {
public:
    ScopedTimer(const std::string& name);
    ~ScopedTimer();
    
private:
    std::string timer_name_;
};

} // namespace KolosalAgent

// Convenience macros that automatically include debug information
#ifdef DEBUG_BUILD
    #define LOG_DEBUG(message) KolosalAgent::Logger::instance().debug(message, __FUNCTION__, __FILE__, __LINE__)
    #define LOG_INFO(message) KolosalAgent::Logger::instance().info(message, __FUNCTION__, __FILE__, __LINE__)
    #define LOG_WARN(message) KolosalAgent::Logger::instance().warn(message, __FUNCTION__, __FILE__, __LINE__)
    #define LOG_ERROR(message) KolosalAgent::Logger::instance().error(message, __FUNCTION__, __FILE__, __LINE__)
    #define LOG_FATAL(message) KolosalAgent::Logger::instance().fatal(message, __FUNCTION__, __FILE__, __LINE__)
    
    #define LOG_DEBUG_F(format, ...) KolosalAgent::Logger::instance().debug(format, ##__VA_ARGS__)
    #define LOG_INFO_F(format, ...) KolosalAgent::Logger::instance().info(format, ##__VA_ARGS__)
    #define LOG_WARN_F(format, ...) KolosalAgent::Logger::instance().warn(format, ##__VA_ARGS__)
    #define LOG_ERROR_F(format, ...) KolosalAgent::Logger::instance().error(format, ##__VA_ARGS__)
    #define LOG_FATAL_F(format, ...) KolosalAgent::Logger::instance().fatal(format, ##__VA_ARGS__)
    
    #define TRACE_FUNCTION() KolosalAgent::FunctionTracer _tracer(__FUNCTION__, __FILE__, __LINE__)
    #define SCOPED_TIMER(name) KolosalAgent::ScopedTimer _timer(name)
#else
    #define LOG_DEBUG(message) KolosalAgent::Logger::instance().debug(message)
    #define LOG_INFO(message) KolosalAgent::Logger::instance().info(message)
    #define LOG_WARN(message) KolosalAgent::Logger::instance().warn(message)
    #define LOG_ERROR(message) KolosalAgent::Logger::instance().error(message)
    #define LOG_FATAL(message) KolosalAgent::Logger::instance().fatal(message)
    
    #define LOG_DEBUG_F(format, ...) KolosalAgent::Logger::instance().debug(format, ##__VA_ARGS__)
    #define LOG_INFO_F(format, ...) KolosalAgent::Logger::instance().info(format, ##__VA_ARGS__)
    #define LOG_WARN_F(format, ...) KolosalAgent::Logger::instance().warn(format, ##__VA_ARGS__)
    #define LOG_ERROR_F(format, ...) KolosalAgent::Logger::instance().error(format, ##__VA_ARGS__)
    #define LOG_FATAL_F(format, ...) KolosalAgent::Logger::instance().fatal(format, ##__VA_ARGS__)
    
    #define TRACE_FUNCTION() // No-op in release builds
    #define SCOPED_TIMER(name) // No-op in release builds
#endif

// Simplified macros for basic logging without debug info
#define SIMPLE_LOG_DEBUG(message) if (KolosalAgent::Logger::instance().should_log(KolosalAgent::LogLevel::DEBUG)) { KolosalAgent::Logger::instance().debug(message); }
#define SIMPLE_LOG_INFO(message) if (KolosalAgent::Logger::instance().should_log(KolosalAgent::LogLevel::INFO)) { KolosalAgent::Logger::instance().info(message); }
#define SIMPLE_LOG_WARN(message) if (KolosalAgent::Logger::instance().should_log(KolosalAgent::LogLevel::WARN)) { KolosalAgent::Logger::instance().warn(message); }
#define SIMPLE_LOG_ERROR(message) if (KolosalAgent::Logger::instance().should_log(KolosalAgent::LogLevel::ERROR)) { KolosalAgent::Logger::instance().error(message); }
#define SIMPLE_LOG_FATAL(message) if (KolosalAgent::Logger::instance().should_log(KolosalAgent::LogLevel::FATAL)) { KolosalAgent::Logger::instance().fatal(message); }
