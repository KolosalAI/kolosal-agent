/**
 * @file logger_system.cpp
 * @brief Advanced logging system with multiple appenders
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "logger_system.hpp"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <thread>
#include <filesystem>
#include <mutex>

// Try to include ServerLogger if available
#ifdef ENABLE_SERVER_LOGGER_INTEGRATION
#include "kolosal/logger.hpp"
#endif

namespace kolosal {

// DefaultLogFormatter implementation
DefaultLogFormatter::DefaultLogFormatter(bool include_timestamp, bool include_level,
                                       bool include_component, bool include_thread_id)
    /**
     * @brief Perform include timestamp  operation
     * @return : Description of return value
     */
    : include_timestamp_(include_timestamp)
    , include_level_(include_level)
    , include_component_(include_component)
    , include_thread_id_(include_thread_id) {}

std::string DefaultLogFormatter::format(const LogEntry& entry) {
    std::ostringstream oss;
    
    if (include_timestamp_) {
        oss << "[" << format_Timestamp(entry.timestamp) << "] ";
    }
    
    if (include_level_) {
        oss << "[" << levelTo_String(entry.level) << "] ";
    }
    
    if (include_component_ && !entry.component.empty()) {
        oss << "[" << entry.component << "] ";
    }
    
    if (include_thread_id_) {
        oss << "[" << entry.thread_id << "] ";
    }
    
    oss << entry.message;
    
    return oss.str();
}

std::string DefaultLogFormatter::levelTo_String(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string DefaultLogFormatter::format_Timestamp(const std::chrono::system_clock::time_point& timestamp) const {
    const auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()) % 1000;
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

// ConsoleAppender implementation
ConsoleAppender::ConsoleAppender(bool use_colors, bool errors_to_stderr)
    /**
     * @brief Perform use colors  operation
     * @return : Description of return value
     */
    : use_colors_(use_colors)
    , errors_to_stderr_(errors_to_stderr) {}

void ConsoleAppender::append(const LogEntry& entry, const std::string& formatted_message) {
    std::lock_guard<std::mutex> lock(console_mutex_);
    
    const bool use_stderr = errors_to_stderr_ && (entry.level >= LogLevel::ERROR);
    std::ostream& stream = use_stderr ? std::cerr : std::cout;
    
    if (use_colors_) {
        stream << getColor_Code(entry.level) << formatted_message << getReset_Code() << std::endl;
    } else {
        stream << formatted_message << std::endl;
    }
}

void ConsoleAppender::flush() {
    std::lock_guard<std::mutex> lock(console_mutex_);
    std::cout.flush();
    std::cerr.flush();
}

std::string ConsoleAppender::getColor_Code(LogLevel level) const {
    if (!use_colors_) return "";
    
    switch (level) {
        case LogLevel::TRACE: return "\033[90m";  // Dark gray
        case LogLevel::DEBUG: return "\033[36m";  // Cyan
    case LogLevel::INFO:  return "\033[32m";  // Green
        case LogLevel::WARN:  return "\033[33m";  // Yellow
        case LogLevel::ERROR: return "\033[31m";  // Red
        case LogLevel::FATAL: return "\033[91m";  // Bright red
        default: return "";
    }
}

std::string ConsoleAppender::getReset_Code() const {
    return use_colors_ ? "\033[0m" : "";
}

// FileAppender implementation
FileAppender::FileAppender(const std::string& filename, size_t max_file_size_mb, size_t max_backup_files)
    /**
     * @brief Perform filename  operation
     * @return : Description of return value
     */
    : filename_(filename)
    , max_file_size_bytes_(max_file_size_mb * 1024 * 1024)
    , max_backup_files_(max_backup_files)
    , current_file_size_(0) {
    
    open_File();
}

FileAppender::~FileAppender() {
    std::lock_guard<std::mutex> lock(file_mutex_);
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
}

void FileAppender::append(const LogEntry& entry, const std::string& formatted_message) {
    std::lock_guard<std::mutex> lock(file_mutex_);
    
    if (!file_stream_.is_open()) {
        open_File();
    }
    
    if (file_stream_.is_open()) {
        file_stream_ << formatted_message << std::endl;
        current_file_size_ += formatted_message.length() + 1; // +1 for newline
        
        // Check if rotation is needed
        if (current_file_size_ > max_file_size_bytes_) {
            rotate_File();
        }
    }
}

void FileAppender::flush() {
    std::lock_guard<std::mutex> lock(file_mutex_);
    if (file_stream_.is_open()) {
        file_stream_.flush();
    }
}

bool FileAppender::is_Open() const {
    std::lock_guard<std::mutex> lock(file_mutex_);
    return file_stream_.is_open();
}

void FileAppender::rotate_File() {
    if (max_backup_files_ == 0) return;
    
    file_stream_.close();
    
    // Rotate existing backup files
    for (int i = static_cast<int>(max_backup_files_) - 1; i > 0; --i) {
    std::string old_backup = filename_ + "." + std::to_string(i);
    std::string new_backup = filename_ + "." + std::to_string(i + 1);
        if (std::filesystem::exists(old_backup)) {
            std::error_code ec;
            std::filesystem::rename(old_backup, new_backup, ec);
            // Ignore errors during rotation
        }
    }
    
    // Move current file to .1
    if (std::filesystem::exists(filename_)) {
    std::string first_backup = filename_ + ".1";
        std::error_code ec;
        std::filesystem::rename(filename_, first_backup, ec);
        // Ignore errors during rotation
    }
    
    open_File();
}

void FileAppender::open_File() {
    file_stream_.open(filename_, std::ios::app);
    if (file_stream_.is_open()) {
        // Get current file size
        file_stream_.seekp(0, std::ios::end);
        current_file_size_ = file_stream_.tellp();
        if (current_file_size_ == std::streampos(-1)) {
            current_file_size_ = 0;
        }
    }
}

// KolosalLogger implementation
KolosalLogger::KolosalLogger()
    /**
     * @brief Perform current level  operation
     * @return : Description of return value
     */
    : current_level_(LogLevel::INFO)
    , default_component_("MAIN")
    , formatter_(std::make_unique<DefaultLogFormatter>())
    , server_logger_integration_(false)
    , recent_entries_index_(0)
    , max_recent_entries_(100) {
    
    recent_entries_.resize(max_recent_entries_);
    
    // Add default console appender
    add_Appender(std::make_unique<ConsoleAppender>());
}

KolosalLogger::~KolosalLogger() {
    shutdown();
}

KolosalLogger& KolosalLogger::instance() {
    static KolosalLogger instance;
    return instance;
}

void KolosalLogger::set_Level(LogLevel level) {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    current_level_ = level;
}

LogLevel KolosalLogger::get_Level() const {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    return current_level_;
}

void KolosalLogger::set_Component(const std::string& component) {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    default_component_ = component;
}

std::string KolosalLogger::get_Component() const {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    return default_component_;
}

void KolosalLogger::add_Appender(std::unique_ptr<LogAppender> appender) {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    appenders_.push_back(std::move(appender));
}

void KolosalLogger::clear_Appenders() {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    appenders_.clear();
}

void KolosalLogger::set_Formatter(std::unique_ptr<LogFormatter> formatter) {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    formatter_ = std::move(formatter);
}

void KolosalLogger::log(LogLevel level, const std::string& message) {
    log_Internal(level, default_component_, message);
}

void KolosalLogger::log(LogLevel level, const std::string& component, const std::string& message) {
    log_Internal(level, component, message);
}

void KolosalLogger::trace(const std::string& message) {
    log_Internal(LogLevel::TRACE, default_component_, message);
}

void KolosalLogger::debug(const std::string& message) {
    log_Internal(LogLevel::DEBUG, default_component_, message);
}

void KolosalLogger::info(const std::string& message) {
    log_Internal(LogLevel::INFO, default_component_, message);
}

void KolosalLogger::warn(const std::string& message) {
    log_Internal(LogLevel::WARN, default_component_, message);
}

void KolosalLogger::error(const std::string& message) {
    log_Internal(LogLevel::ERROR, default_component_, message);
}

void KolosalLogger::fatal(const std::string& message) {
    log_Internal(LogLevel::FATAL, default_component_, message);
}

void KolosalLogger::flush() {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    for (auto& appender : appenders_) {
        appender->flush();
    }
}

void KolosalLogger::shutdown() {
    flush();
    clear_Appenders();
}

void KolosalLogger::enableServerLogger_Integration(bool enable) {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    server_logger_integration_ = enable;
}

bool KolosalLogger::isServerLoggerIntegration_Enabled() const {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    return server_logger_integration_;
}

std::vector<LogEntry> KolosalLogger::getRecent_Entries(size_t max_entries) const {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    std::vector<LogEntry> result;
    result.reserve(std::min(max_entries, max_recent_entries_));
    
    // Collect entries starting from the oldest
    size_t count = 0;
    for (size_t i = recent_entries_index_; count < max_entries && count < max_recent_entries_; ++count) {
        const size_t index = (i + count) % max_recent_entries_;
        if (recent_entries_[index].level != LogLevel::OFF) { // OFF means uninitialized
            result.push_back(recent_entries_[index]);
        }
    }
    
    return result;
}

void KolosalLogger::log_Internal(LogLevel level, const std::string& component, const std::string& message) {
    if (level < current_level_) return;
    
    // Create log entry
    LogEntry entry;
    entry.level = level;
    entry.timestamp = std::chrono::system_clock::now();
    entry.component = component;
    entry.message = message;
    entry.thread_id = std::this_thread::get_id();
    
    std::lock_guard<std::mutex> lock(logger_mutex_);
    
    // Store in recent entries ring buffer
    recent_entries_[recent_entries_index_] = entry;
    recent_entries_index_ = (recent_entries_index_ + 1) % max_recent_entries_;
    
    // Format the message
    std::string formatted_message;
    if (formatter_) {
        formatted_message = formatter_->format(entry);
    } else {
        formatted_message = message;
    }
    
    // Send to all appenders
    for (auto& appender : appenders_) {
        appender->append(entry, formatted_message);
    }
    
    // Integrate with ServerLogger if enabled and available
#ifdef ENABLE_SERVER_LOGGER_INTEGRATION
    if (server_logger_integration_) {
        ServerLogger& serverLogger = ServerLogger::instance();
        switch (level) {
            case LogLevel::TRACE:
            case LogLevel::DEBUG:
                serverLogger.debug("[%s] %s", component.c_str(), message.c_str());
                break;
            case LogLevel::information:
                serverLogger.info("[%s] %s", component.c_str(), message.c_str());
                break;
            case LogLevel::WARN:
                serverLogger.warning("[%s] %s", component.c_str(), message.c_str());
                break;
            case LogLevel::ERROR:
            case LogLevel::FATAL:
                serverLogger.error("[%s] %s", component.c_str(), message.c_str());
                break;
            default:
                break;
        }
    }
#endif
}

} // namespace kolosal
