#include "../../include/logger_advanced.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <thread>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace kolosal {

// LogEntry implementation
json LogEntry::to_json() const {
    json j;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()).count();
    j["level"] = static_cast<int>(level);
    j["message"] = message;
    j["component"] = context.component;
    j["operation"] = context.operation;
    j["request_id"] = context.request_id;
    j["user_id"] = context.user_id;
    j["session_id"] = context.session_id;
    j["thread_id"] = thread_id;
    j["source_location"] = source_location;
    j["structured_data"] = structured_data;
    j["custom_fields"] = json(context.custom_fields);
    return j;
}

std::string LogEntry::to_string() const {
    std::ostringstream oss;
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    std::string level_str;
    switch (level) {
        case LogLevel::TRACE: level_str = "TRACE"; break;
        case LogLevel::DEBUG: level_str = "DEBUG"; break;
        case LogLevel::INFO: level_str = "INFO"; break;
        case LogLevel::WARNING: level_str = "WARNING"; break;
        case LogLevel::ERROR: level_str = "ERROR"; break;
        case LogLevel::FATAL: level_str = "FATAL"; break;
    }
    
    oss << " [" << level_str << "] ";
    if (!context.component.empty()) {
        oss << "[" << context.component << "] ";
    }
    oss << message;
    return oss.str();
}

// ConsoleLogOutput implementation
ConsoleLogOutput::ConsoleLogOutput(bool use_colors) : use_colors_(use_colors) {}

void ConsoleLogOutput::write_log(const LogEntry& entry) {
    std::string output = entry.to_string();
    
    if (use_colors_) {
        std::cout << get_color_code(entry.level) << output << "\033[0m" << std::endl;
    } else {
        std::cout << output << std::endl;
    }
}

void ConsoleLogOutput::flush() {
    std::cout.flush();
}

std::string ConsoleLogOutput::get_color_code(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return "\033[90m"; // Gray
        case LogLevel::DEBUG: return "\033[36m"; // Cyan
        case LogLevel::INFO: return "\033[37m";  // White
        case LogLevel::WARNING: return "\033[33m"; // Yellow
        case LogLevel::ERROR: return "\033[31m"; // Red
        case LogLevel::FATAL: return "\033[35m"; // Magenta
        default: return "\033[37m";
    }
}

// FileLogOutput implementation
FileLogOutput::FileLogOutput(const std::string& file_path, size_t max_file_size_mb, int max_files)
    : file_path_(file_path), max_file_size_bytes_(max_file_size_mb * 1024 * 1024), max_files_(max_files) {
    
    // Create directory if it doesn't exist
    fs::path path(file_path_);
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }
    
    current_file_.open(file_path_, std::ios::app);
    if (!current_file_.is_open()) {
        throw std::runtime_error("Failed to open log file: " + file_path_);
    }
    
    // Get current file size
    if (fs::exists(file_path_)) {
        current_file_size_ = fs::file_size(file_path_);
    }
}

FileLogOutput::~FileLogOutput() {
    if (current_file_.is_open()) {
        current_file_.close();
    }
}

void FileLogOutput::write_log(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(file_mutex_);
    
    rotate_logs_if_needed();
    
    std::string log_line = entry.to_string() + "\n";
    current_file_ << log_line;
    current_file_.flush();
    current_file_size_ += log_line.length();
}

void FileLogOutput::flush() {
    std::lock_guard<std::mutex> lock(file_mutex_);
    if (current_file_.is_open()) {
        current_file_.flush();
    }
}

void FileLogOutput::rotate_logs_if_needed() {
    if (current_file_size_ < max_file_size_bytes_) {
        return;
    }
    
    current_file_.close();
    
    // Rotate existing files
    for (int i = max_files_ - 1; i > 0; --i) {
        std::string old_file = get_rotated_filename(i - 1);
        std::string new_file = get_rotated_filename(i);
        
        if (fs::exists(old_file)) {
            if (fs::exists(new_file)) {
                fs::remove(new_file);
            }
            fs::rename(old_file, new_file);
        }
    }
    
    // Move current file to .1
    std::string rotated_file = get_rotated_filename(1);
    if (fs::exists(file_path_)) {
        if (fs::exists(rotated_file)) {
            fs::remove(rotated_file);
        }
        fs::rename(file_path_, rotated_file);
    }
    
    // Open new current file
    current_file_.open(file_path_, std::ios::out | std::ios::trunc);
    current_file_size_ = 0;
}

std::string FileLogOutput::get_rotated_filename(int index) const {
    return file_path_ + "." + std::to_string(index);
}

// JsonLogOutput implementation
JsonLogOutput::JsonLogOutput(const std::string& file_path) : file_path_(file_path) {
    fs::path path(file_path_);
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }
    
    file_.open(file_path_, std::ios::app);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to open JSON log file: " + file_path_);
    }
}

JsonLogOutput::~JsonLogOutput() {
    if (file_.is_open()) {
        file_.close();
    }
}

void JsonLogOutput::write_log(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(file_mutex_);
    file_ << entry.to_json().dump() << std::endl;
}

void JsonLogOutput::flush() {
    std::lock_guard<std::mutex> lock(file_mutex_);
    if (file_.is_open()) {
        file_.flush();
    }
}

// AdvancedLogger implementation
AdvancedLogger& AdvancedLogger::instance() {
    static AdvancedLogger instance;
    return instance;
}

AdvancedLogger::AdvancedLogger() : start_time_(std::chrono::system_clock::now()) {
    // Initialize level counts
    for (int i = 0; i <= static_cast<int>(LogLevel::FATAL); ++i) {
        level_counts_[static_cast<LogLevel>(i)] = 0;
    }
}

AdvancedLogger::~AdvancedLogger() {
    shutdown();
}

void AdvancedLogger::set_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    current_level_ = level;
}

void AdvancedLogger::set_context(const LogContext& context) {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    default_context_ = context;
}

void AdvancedLogger::add_output(std::unique_ptr<ILogOutput> output) {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    outputs_.push_back(std::move(output));
}

void AdvancedLogger::remove_all_outputs() {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    outputs_.clear();
}

void AdvancedLogger::log(LogLevel level, const std::string& message, 
                        const LogContext& context, const json& structured_data) {
    if (level < current_level_) return;
    
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = level;
    entry.message = message;
    entry.context = context;
    entry.structured_data = structured_data;
    
    // Add thread ID
    std::ostringstream thread_id_stream;
    thread_id_stream << std::this_thread::get_id();
    entry.thread_id = thread_id_stream.str();
    
    // Merge with default context
    if (entry.context.component.empty() && !default_context_.component.empty()) {
        entry.context.component = default_context_.component;
    }
    if (entry.context.operation.empty() && !default_context_.operation.empty()) {
        entry.context.operation = default_context_.operation;
    }
    
    std::lock_guard<std::mutex> lock(logger_mutex_);
    
    // Write to all outputs
    for (auto& output : outputs_) {
        output->write_log(entry);
    }
    
    // Update statistics
    total_logs_++;
    level_counts_[level]++;
    
    // Add to buffer for querying
    add_to_buffer(entry);
}

void AdvancedLogger::trace(const std::string& message, const LogContext& context, const json& data) {
    log(LogLevel::TRACE, message, context, data);
}

void AdvancedLogger::debug(const std::string& message, const LogContext& context, const json& data) {
    log(LogLevel::DEBUG, message, context, data);
}

void AdvancedLogger::info(const std::string& message, const LogContext& context, const json& data) {
    log(LogLevel::INFO, message, context, data);
}

void AdvancedLogger::warning(const std::string& message, const LogContext& context, const json& data) {
    log(LogLevel::WARNING, message, context, data);
}

void AdvancedLogger::error(const std::string& message, const LogContext& context, const json& data) {
    log(LogLevel::ERROR, message, context, data);
}

void AdvancedLogger::fatal(const std::string& message, const LogContext& context, const json& data) {
    log(LogLevel::FATAL, message, context, data);
}

void AdvancedLogger::flush_all() {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    for (auto& output : outputs_) {
        output->flush();
    }
}

void AdvancedLogger::shutdown() {
    flush_all();
    remove_all_outputs();
}

void AdvancedLogger::add_to_buffer(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    log_buffer_.push_back(entry);
    
    if (log_buffer_.size() > max_buffer_size_) {
        log_buffer_.erase(log_buffer_.begin());
    }
}

json AdvancedLogger::get_log_statistics() const {
    std::lock_guard<std::mutex> lock(logger_mutex_);
    
    json stats;
    stats["total_logs"] = total_logs_.load();
    stats["uptime_seconds"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now() - start_time_).count();
    
    json level_stats;
    for (const auto& [level, count] : level_counts_) {
        std::string level_name;
        switch (level) {
            case LogLevel::TRACE: level_name = "trace"; break;
            case LogLevel::DEBUG: level_name = "debug"; break;
            case LogLevel::INFO: level_name = "info"; break;
            case LogLevel::WARNING: level_name = "warning"; break;
            case LogLevel::ERROR: level_name = "error"; break;
            case LogLevel::FATAL: level_name = "fatal"; break;
        }
        level_stats[level_name] = count.load();
    }
    stats["level_counts"] = level_stats;
    
    return stats;
}

// StructuredException implementation
StructuredException::StructuredException(const ErrorInfo& error_info) : error_info_(error_info) {}

StructuredException::StructuredException(ErrorType type, const std::string& message, const std::string& details)
    : error_info_(type, message) {
    error_info_.details = details;
}

const char* StructuredException::what() const noexcept {
    if (what_string_.empty()) {
        what_string_ = error_info_.message + (error_info_.details.empty() ? "" : ": " + error_info_.details);
    }
    return what_string_.c_str();
}

const ErrorInfo& StructuredException::get_error_info() const {
    return error_info_;
}

json StructuredException::to_json() const {
    return error_info_.to_json();
}

// ErrorInfo implementation
json ErrorInfo::to_json() const {
    json j;
    j["type"] = static_cast<int>(type);
    j["code"] = code;
    j["message"] = message;
    j["details"] = details;
    j["context"] = context;
    j["trace_id"] = trace_id;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()).count();
    j["stack_trace"] = stack_trace;
    return j;
}

std::string ErrorInfo::to_string() const {
    return message + (details.empty() ? "" : ": " + details);
}

} // namespace kolosal
