#include "logger.hpp"
#include <thread>
#include <ctime>
#include <unordered_map>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

namespace KolosalAgent {

std::string Logger::level_to_string(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG_LVL: return "DEBUG";
        case LogLevel::INFO_LVL:  return "INFO ";
        case LogLevel::WARN_LVL:  return "WARN ";
        case LogLevel::ERROR_LVL: return "ERROR";
        case LogLevel::FATAL_LVL: return "FATAL";
        default: return "UNKNO";
    }
}

std::string Logger::level_to_color(LogLevel level) const {
    if (!colors_enabled_) return "";
    
    switch (level) {
        case LogLevel::DEBUG_LVL: return "\033[36m";    // Cyan
        case LogLevel::INFO_LVL:  return "\033[32m";    // Green
        case LogLevel::WARN_LVL:  return "\033[33m";    // Yellow
        case LogLevel::ERROR_LVL: return "\033[31m";    // Red
        case LogLevel::FATAL_LVL: return "\033[35m";    // Magenta
        default: return "";
    }
}

Logger::Logger() : current_level_(LogLevel::INFO_LVL),
      console_output_enabled_(true),
      file_output_enabled_(false),
      timestamps_enabled_(true),
      thread_id_enabled_(false),
      function_tracing_enabled_(false),
      colors_enabled_(true) {
    
    // Detect if we're in a debug build and adjust defaults accordingly
#ifdef DEBUG_BUILD
    current_level_ = LogLevel::DEBUG_LVL;
    thread_id_enabled_ = true;
    function_tracing_enabled_ = true;
    
    // Check if we have console color support
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        colors_enabled_ = SetConsoleMode(hOut, dwMode) != FALSE;
    } else {
        colors_enabled_ = false;
    }
#else
    colors_enabled_ = isatty(STDOUT_FILENO);
#endif
#endif
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::set_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    current_level_ = level;
}

void Logger::set_console_output(bool enabled) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    console_output_enabled_ = enabled;
}

void Logger::set_file_output(const std::string& filename) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    if (!filename.empty()) {
        log_filename_ = filename;
        log_file_.open(filename, std::ios::app);
        file_output_enabled_ = log_file_.is_open();
        
        if (file_output_enabled_) {
            log_file_ << "\n=== Logger Session Started at " << get_timestamp() << " ===" << std::endl;
        }
    } else {
        file_output_enabled_ = false;
    }
}

void Logger::enable_timestamps(bool enabled) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    timestamps_enabled_ = enabled;
}

void Logger::enable_thread_id(bool enabled) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    thread_id_enabled_ = enabled;
}

void Logger::enable_function_tracing(bool enabled) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    function_tracing_enabled_ = enabled;
}

void Logger::debug(const std::string& message, const std::string& function, const std::string& file, int line) {
    if (should_log(LogLevel::DEBUG_LVL)) {
        log(LogLevel::DEBUG_LVL, message, function, file, line);
    }
}

void Logger::info(const std::string& message, const std::string& function, const std::string& file, int line) {
    if (should_log(LogLevel::INFO_LVL)) {
        log(LogLevel::INFO_LVL, message, function, file, line);
    }
}

void Logger::warn(const std::string& message, const std::string& function, const std::string& file, int line) {
    if (should_log(LogLevel::WARN_LVL)) {
        log(LogLevel::WARN_LVL, message, function, file, line);
    }
}

void Logger::error(const std::string& message, const std::string& function, const std::string& file, int line) {
    if (should_log(LogLevel::ERROR_LVL)) {
        log(LogLevel::ERROR_LVL, message, function, file, line);
    }
}

void Logger::fatal(const std::string& message, const std::string& function, const std::string& file, int line) {
    if (should_log(LogLevel::FATAL_LVL)) {
        log(LogLevel::FATAL_LVL, message, function, file, line);
    }
}

void Logger::trace_function_entry(const std::string& function, const std::string& file, int line) {
    if (function_tracing_enabled_ && should_log(LogLevel::DEBUG_LVL)) {
        std::string trace_msg = "ENTER -> " + function;
        log(LogLevel::DEBUG_LVL, trace_msg, function, file, line);
    }
}

void Logger::trace_function_exit(const std::string& function, const std::string& file, int line) {
    if (function_tracing_enabled_ && should_log(LogLevel::DEBUG_LVL)) {
        std::string trace_msg = "EXIT  <- " + function;
        log(LogLevel::DEBUG_LVL, trace_msg, function, file, line);
    }
}

void Logger::start_timer(const std::string& timer_name) {
    if (should_log(LogLevel::DEBUG_LVL)) {
        std::lock_guard<std::mutex> lock(timer_mutex_);
        timers_[timer_name] = std::chrono::steady_clock::now();
        debug("Timer started: " + timer_name);
    }
}

void Logger::end_timer(const std::string& timer_name) {
    if (should_log(LogLevel::DEBUG_LVL)) {
        std::lock_guard<std::mutex> lock(timer_mutex_);
        auto it = timers_.find(timer_name);
        if (it != timers_.end()) {
            auto duration = std::chrono::steady_clock::now() - it->second;
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            debug("Timer finished: " + timer_name + " took " + std::to_string(ms) + "ms");
            timers_.erase(it);
        }
    }
}

void Logger::log(LogLevel level, const std::string& message, const std::string& function, const std::string& file, int line) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    std::ostringstream log_stream;
    
    // Add timestamp
    if (timestamps_enabled_) {
        log_stream << "[" << get_timestamp() << "]";
    }
    
    // Add thread ID in debug mode
    if (thread_id_enabled_) {
        log_stream << "[" << get_thread_id() << "]";
    }
    
    // Add log level
    log_stream << "[" << level_to_string(level) << "]";
    
    // Add function information in debug builds
    if (!function.empty() && should_log(LogLevel::DEBUG_LVL)) {
        std::string filename = file;
        if (!filename.empty()) {
            size_t pos = filename.find_last_of("/\\");
            if (pos != std::string::npos) {
                filename = filename.substr(pos + 1);
            }
        }
        
        if (!filename.empty() && line > 0) {
            log_stream << "[" << function << "() " << filename << ":" << line << "]";
        } else if (!function.empty()) {
            log_stream << "[" << function << "()]";
        }
    }
    
    log_stream << " " << message;
    
    std::string log_line = log_stream.str();
    
    // Console output with colors
    if (console_output_enabled_) {
        if (colors_enabled_) {
            std::cout << level_to_color(level) << log_line << "\033[0m" << std::endl;
        } else {
            std::cout << log_line << std::endl;
        }
    }
    
    // File output (no colors)
    if (file_output_enabled_ && log_file_.is_open()) {
        log_file_ << log_line << std::endl;
        log_file_.flush();
    }
}

std::string Logger::get_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::get_thread_id() const {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

// FunctionTracer implementation
FunctionTracer::FunctionTracer(const std::string& function, const std::string& file, int line)
    : function_name_(function), file_name_(file), line_number_(line),
      start_time_(std::chrono::steady_clock::now()) {
    Logger::instance().trace_function_entry(function_name_, file_name_, line_number_);
}

FunctionTracer::~FunctionTracer() {
    Logger::instance().trace_function_exit(function_name_, file_name_, line_number_);
    
    // Log execution time if debug level is enabled
    if (Logger::instance().should_log(LogLevel::DEBUG_LVL)) {
        auto duration = std::chrono::steady_clock::now() - start_time_;
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        
        std::string timing_msg = function_name_ + "() execution time: ";
        if (us >= 1000) {
            timing_msg += std::to_string(us / 1000) + "ms";
        } else {
            timing_msg += std::to_string(us) + "μs";
        }
        
        Logger::instance().debug(timing_msg, function_name_, file_name_, line_number_);
    }
}

// ScopedTimer implementation
ScopedTimer::ScopedTimer(const std::string& name) : timer_name_(name) {
    Logger::instance().start_timer(timer_name_);
}

ScopedTimer::~ScopedTimer() {
    Logger::instance().end_timer(timer_name_);
}

} // namespace KolosalAgent
