#include "logger.hpp"
#include <iomanip>
#include <sstream>
#include <iostream>
#include <mutex>
#include <chrono>

namespace KolosalAgent {

Logger& Logger::instance() {
    static Logger instance_;
    return instance_;
}

Logger& Logger::get_instance() {
    return instance();
}

Logger::Logger() : current_level_(LogLevel::INFO) {
    console_output_enabled_ = true;
    file_output_enabled_ = false;
    timestamps_enabled_ = true;
    thread_id_enabled_ = false;
    function_tracing_enabled_ = false;
    colors_enabled_ = true;
}

Logger::~Logger() {
    try {
        shutdown();
    } catch (...) {
        // Suppress exceptions in destructor to prevent abort()
    }
}

std::string Logger::level_to_string(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

void Logger::log_message(LogLevel level, const std::string& message) {
    if (!should_log(level)) return;
    
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    std::string formatted_message;
    
    // Add timestamp if enabled
    if (timestamps_enabled_) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        formatted_message = "[" + ss.str() + "] ";
    }
    
    formatted_message += "[" + level_to_string(level) + "] " + message;
    
    // Output to console
    if (console_output_enabled_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << formatted_message << std::endl;
        } else {
            std::cout << formatted_message << std::endl;
        }
    }
    
    // Output to file
    if (file_output_enabled_ && file_stream_.is_open()) {
        file_stream_ << formatted_message << std::endl;
        file_stream_.flush();
    }
}

void Logger::set_level(LogLevel level) {
    current_level_ = level;
}

void Logger::set_level(int level) {
    current_level_ = static_cast<LogLevel>(level);
}

void Logger::set_log_level(const std::string& level) {
    if (level == "TRACE") current_level_ = LogLevel::TRACE;
    else if (level == "DEBUG") current_level_ = LogLevel::DEBUG;
    else if (level == "INFO") current_level_ = LogLevel::INFO;
    else if (level == "WARN" || level == "WARNING") current_level_ = LogLevel::WARNING;
    else if (level == "ERROR") current_level_ = LogLevel::ERROR;
    else if (level == "FATAL") current_level_ = LogLevel::FATAL;
}

void Logger::set_log_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    filename_ = filename;
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
    if (!filename.empty()) {
        file_stream_.open(filename, std::ios::app);
        file_output_enabled_ = true;
    } else {
        file_output_enabled_ = false;
    }
}

void Logger::trace(const std::string& message) {
    log_message(LogLevel::TRACE, message);
}

void Logger::debug(const std::string& message) {
    log_message(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log_message(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) {
    log_message(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log_message(LogLevel::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log_message(LogLevel::FATAL, message);
}

// Formatted logging methods (simple implementation)
void Logger::trace_f(const std::string& format) {
    trace(format);
}

void Logger::debug_f(const std::string& format) {
    debug(format);
}

void Logger::info_f(const std::string& format) {
    info(format);
}

void Logger::warn_f(const std::string& format) {
    warn(format);
}

void Logger::error_f(const std::string& format) {
    error(format);
}

void Logger::fatal_f(const std::string& format) {
    fatal(format);
}

void Logger::debug(const std::string& message, const std::string& function, const std::string& file, int line) {
    if (should_log(LogLevel::DEBUG)) {
        std::string enhanced_message = function + "(" + file + ":" + std::to_string(line) + ") " + message;
        log_message(LogLevel::DEBUG, enhanced_message);
    }
}

void Logger::info(const std::string& message, const std::string& function, const std::string& file, int line) {
    if (should_log(LogLevel::INFO)) {
        std::string enhanced_message = function + "(" + file + ":" + std::to_string(line) + ") " + message;
        log_message(LogLevel::INFO, enhanced_message);
    }
}

void Logger::warn(const std::string& message, const std::string& function, const std::string& file, int line) {
    if (should_log(LogLevel::WARNING)) {
        std::string enhanced_message = function + "(" + file + ":" + std::to_string(line) + ") " + message;
        log_message(LogLevel::WARNING, enhanced_message);
    }
}

void Logger::error(const std::string& message, const std::string& function, const std::string& file, int line) {
    if (should_log(LogLevel::ERROR)) {
        std::string enhanced_message = function + "(" + file + ":" + std::to_string(line) + ") " + message;
        log_message(LogLevel::ERROR, enhanced_message);
    }
}

void Logger::fatal(const std::string& message, const std::string& function, const std::string& file, int line) {
    if (should_log(LogLevel::FATAL)) {
        std::string enhanced_message = function + "(" + file + ":" + std::to_string(line) + ") " + message;
        log_message(LogLevel::FATAL, enhanced_message);
    }
}

bool Logger::should_log(LogLevel level) const {
    return level >= current_level_;
}

void Logger::shutdown() {
    try {
        std::lock_guard<std::mutex> lock(log_mutex_);
        if (file_stream_.is_open()) {
            file_stream_.close();
        }
    } catch (...) {
        // Suppress exceptions during shutdown
    }
}

void Logger::set_console_output(bool enabled) {
    console_output_enabled_ = enabled;
}

void Logger::enable_timestamps(bool enabled) {
    timestamps_enabled_ = enabled;
}

// FunctionTracer implementation
FunctionTracer::FunctionTracer(const std::string& function, const std::string& file, int line) 
    : function_name_(function), file_name_(file), line_number_(line) {
}

FunctionTracer::~FunctionTracer() {
}

// ScopedTimer implementation
ScopedTimer::ScopedTimer(const std::string& name) : timer_name_(name) {
}

ScopedTimer::~ScopedTimer() {
}

} // namespace KolosalAgent