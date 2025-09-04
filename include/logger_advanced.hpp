#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <mutex>
#include <chrono>
#include <atomic>
#include <json.hpp>

using json = nlohmann::json;

/**
 * @brief Log levels for structured logging
 */
enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
    FATAL = 5
};

/**
 * @brief Log context information
 */
struct LogContext {
    std::string component;
    std::string operation;
    std::string request_id;
    std::string user_id;
    std::string session_id;
    std::unordered_map<std::string, json> custom_fields;
    
    LogContext(const std::string& comp = "", const std::string& op = "")
        : component(comp), operation(op) {}
    
    LogContext& with_request_id(const std::string& id) { request_id = id; return *this; }
    LogContext& with_user_id(const std::string& id) { user_id = id; return *this; }
    LogContext& with_session_id(const std::string& id) { session_id = id; return *this; }
    LogContext& with_field(const std::string& key, const json& value) { 
        custom_fields[key] = value; return *this; 
    }
};

/**
 * @brief Structured log entry
 */
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string message;
    LogContext context;
    json structured_data;
    std::string thread_id;
    std::string source_location;
    
    LogEntry() : timestamp(std::chrono::system_clock::now()) {}
    
    json to_json() const;
    std::string to_string() const;
};

/**
 * @brief Log output destination interface
 */
class ILogOutput {
public:
    virtual ~ILogOutput() = default;
    virtual void write_log(const LogEntry& entry) = 0;
    virtual void flush() = 0;
};

/**
 * @brief Console log output
 */
class ConsoleLogOutput : public ILogOutput {
public:
    ConsoleLogOutput(bool use_colors = true);
    void write_log(const LogEntry& entry) override;
    void flush() override;
    
private:
    bool use_colors_;
    std::string get_color_code(LogLevel level) const;
};

/**
 * @brief File log output with rotation
 */
class FileLogOutput : public ILogOutput {
public:
    FileLogOutput(const std::string& file_path, 
                  size_t max_file_size_mb = 100,
                  int max_files = 10);
    ~FileLogOutput();
    
    void write_log(const LogEntry& entry) override;
    void flush() override;
    
private:
    std::string file_path_;
    size_t max_file_size_bytes_;
    int max_files_;
    std::ofstream current_file_;
    std::atomic<size_t> current_file_size_{0};
    mutable std::mutex file_mutex_;
    
    void rotate_logs_if_needed();
    std::string get_rotated_filename(int index) const;
};

/**
 * @brief JSON structured log output
 */
class JsonLogOutput : public ILogOutput {
public:
    JsonLogOutput(const std::string& file_path);
    ~JsonLogOutput();
    
    void write_log(const LogEntry& entry) override;
    void flush() override;
    
private:
    std::string file_path_;
    std::ofstream file_;
    mutable std::mutex file_mutex_;
};

/**
 * @brief Remote log output (e.g., to ELK stack, Loki, etc.)
 */
class RemoteLogOutput : public ILogOutput {
public:
    RemoteLogOutput(const std::string& endpoint, 
                    const std::string& api_key = "",
                    const std::string& format = "json");
    ~RemoteLogOutput();
    
    void write_log(const LogEntry& entry) override;
    void flush() override;
    
private:
    std::string endpoint_;
    std::string api_key_;
    std::string format_;
    std::vector<LogEntry> buffer_;
    mutable std::mutex buffer_mutex_;
    std::thread sender_thread_;
    std::atomic<bool> running_{false};
    
    void sender_loop();
    void send_logs(const std::vector<LogEntry>& logs);
};

/**
 * @brief Advanced structured logger
 */
class AdvancedLogger {
public:
    static AdvancedLogger& instance();
    
    // Configuration
    void set_level(LogLevel level);
    void set_context(const LogContext& context);
    void add_output(std::unique_ptr<ILogOutput> output);
    void remove_all_outputs();
    
    // Logging methods
    void log(LogLevel level, const std::string& message, 
             const LogContext& context = LogContext{},
             const json& structured_data = json{});
    
    void trace(const std::string& message, const LogContext& context = LogContext{}, const json& data = json{});
    void debug(const std::string& message, const LogContext& context = LogContext{}, const json& data = json{});
    void info(const std::string& message, const LogContext& context = LogContext{}, const json& data = json{});
    void warning(const std::string& message, const LogContext& context = LogContext{}, const json& data = json{});
    void error(const std::string& message, const LogContext& context = LogContext{}, const json& data = json{});
    void fatal(const std::string& message, const LogContext& context = LogContext{}, const json& data = json{});
    
    // Formatted logging
    template<typename... Args>
    void logf(LogLevel level, const std::string& format, const LogContext& context, Args... args);
    
    template<typename... Args>
    void infof(const std::string& format, Args... args);
    
    template<typename... Args>
    void errorf(const std::string& format, Args... args);
    
    // Request logging
    void log_request_start(const std::string& request_id, 
                          const std::string& endpoint,
                          const json& parameters = json{});
    void log_request_end(const std::string& request_id,
                        int status_code,
                        double duration_ms,
                        const json& response_metadata = json{});
    
    // Performance logging
    void log_performance(const std::string& operation,
                        double duration_ms,
                        const json& metrics = json{});
    
    // Error logging with stack traces
    void log_exception(const std::exception& e, 
                      const LogContext& context = LogContext{});
    
    // Security and audit logging
    void log_security_event(const std::string& event_type,
                           const std::string& user_id,
                           const json& details = json{});
    void log_audit_event(const std::string& action,
                        const std::string& resource,
                        const std::string& user_id,
                        const json& details = json{});
    
    // Batch logging
    void log_batch(const std::vector<LogEntry>& entries);
    
    // Log analysis and querying
    std::vector<LogEntry> query_logs(LogLevel min_level = LogLevel::INFO,
                                   const std::chrono::system_clock::time_point& start_time = {},
                                   const std::chrono::system_clock::time_point& end_time = {},
                                   const std::string& component = "",
                                   int limit = 100) const;
    
    // Statistics
    json get_log_statistics() const;
    void reset_statistics();
    
    // Control
    void flush_all();
    void shutdown();
    
private:
    AdvancedLogger();
    ~AdvancedLogger();
    
    LogLevel current_level_{LogLevel::INFO};
    LogContext default_context_;
    std::vector<std::unique_ptr<ILogOutput>> outputs_;
    mutable std::mutex logger_mutex_;
    
    // Statistics
    std::atomic<uint64_t> total_logs_{0};
    std::unordered_map<LogLevel, std::atomic<uint64_t>> level_counts_;
    std::chrono::system_clock::time_point start_time_;
    
    // Log buffer for querying
    std::vector<LogEntry> log_buffer_;
    mutable std::mutex buffer_mutex_;
    size_t max_buffer_size_ = 10000;
    
    void add_to_buffer(const LogEntry& entry);
    std::string format_message(const std::string& format, ...) const;
};

/**
 * @brief Error types for structured error handling
 */
enum class ErrorType {
    VALIDATION_ERROR,
    AUTHENTICATION_ERROR,
    AUTHORIZATION_ERROR,
    RESOURCE_NOT_FOUND,
    RESOURCE_CONFLICT,
    RATE_LIMIT_EXCEEDED,
    SERVICE_UNAVAILABLE,
    EXTERNAL_SERVICE_ERROR,
    CONFIGURATION_ERROR,
    NETWORK_ERROR,
    TIMEOUT_ERROR,
    INTERNAL_ERROR,
    UNKNOWN_ERROR
};

/**
 * @brief Structured error information
 */
struct ErrorInfo {
    ErrorType type;
    std::string code;
    std::string message;
    std::string details;
    json context;
    std::string trace_id;
    std::chrono::system_clock::time_point timestamp;
    std::vector<std::string> stack_trace;
    
    ErrorInfo(ErrorType t = ErrorType::UNKNOWN_ERROR, 
              const std::string& msg = "Unknown error")
        : type(t), message(msg), timestamp(std::chrono::system_clock::now()) {}
    
    json to_json() const;
    std::string to_string() const;
};

/**
 * @brief Base class for structured exceptions
 */
class StructuredException : public std::exception {
public:
    StructuredException(const ErrorInfo& error_info);
    StructuredException(ErrorType type, const std::string& message, const std::string& details = "");
    
    const char* what() const noexcept override;
    const ErrorInfo& get_error_info() const;
    json to_json() const;
    
private:
    ErrorInfo error_info_;
    mutable std::string what_string_;
};

/**
 * @brief Specific exception types
 */
class ValidationException : public StructuredException {
public:
    ValidationException(const std::string& message, const std::string& field = "");
};

class AuthenticationException : public StructuredException {
public:
    AuthenticationException(const std::string& message = "Authentication failed");
};

class AuthorizationException : public StructuredException {
public:
    AuthorizationException(const std::string& message = "Access denied");
};

class ResourceNotFoundException : public StructuredException {
public:
    ResourceNotFoundException(const std::string& resource_type, const std::string& resource_id);
};

class ServiceUnavailableException : public StructuredException {
public:
    ServiceUnavailableException(const std::string& service_name, const std::string& reason = "");
};

class TimeoutException : public StructuredException {
public:
    TimeoutException(const std::string& operation, int timeout_seconds);
};

/**
 * @brief Error handler and recovery system
 */
class ErrorHandler {
public:
    static ErrorHandler& instance();
    
    // Error handling configuration
    void set_error_callback(std::function<void(const ErrorInfo&)> callback);
    void set_recovery_strategy(ErrorType type, std::function<json(const ErrorInfo&)> strategy);
    
    // Error handling
    json handle_error(const std::exception& e, const LogContext& context = LogContext{});
    json handle_error(const ErrorInfo& error_info);
    
    // Recovery attempts
    json attempt_recovery(const ErrorInfo& error_info);
    bool register_recovery_strategy(ErrorType type, std::function<json(const ErrorInfo&)> strategy);
    
    // Error reporting
    json create_error_response(const ErrorInfo& error_info) const;
    json create_error_response(int http_status, const std::string& message, 
                              const std::string& details = "") const;
    
    // Error analytics
    json get_error_statistics() const;
    std::vector<ErrorInfo> get_recent_errors(int limit = 100) const;
    json analyze_error_patterns() const;
    
private:
    ErrorHandler();
    
    std::function<void(const ErrorInfo&)> error_callback_;
    std::unordered_map<ErrorType, std::function<json(const ErrorInfo&)>> recovery_strategies_;
    mutable std::mutex handler_mutex_;
    
    // Error tracking
    std::vector<ErrorInfo> recent_errors_;
    std::unordered_map<ErrorType, std::atomic<uint64_t>> error_counts_;
    mutable std::mutex errors_mutex_;
    
    void track_error(const ErrorInfo& error_info);
    ErrorType classify_exception(const std::exception& e);
};

/**
 * @brief RAII logging context manager
 */
class LogContextManager {
public:
    LogContextManager(const LogContext& context);
    ~LogContextManager();
    
    LogContextManager& add_field(const std::string& key, const json& value);
    
private:
    LogContext previous_context_;
};

/**
 * @brief Performance monitoring logger
 */
class PerformanceLogger {
public:
    PerformanceLogger(const std::string& operation, const LogContext& context = LogContext{});
    ~PerformanceLogger();
    
    void add_metric(const std::string& name, const json& value);
    void set_success(bool success);
    
private:
    std::string operation_;
    LogContext context_;
    std::chrono::steady_clock::time_point start_time_;
    json metrics_;
    bool success_ = true;
};

// Convenient macros for logging
#define LOG_TRACE(msg, ...) AdvancedLogger::instance().trace(msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) AdvancedLogger::instance().debug(msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) AdvancedLogger::instance().info(msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) AdvancedLogger::instance().warning(msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) AdvancedLogger::instance().error(msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) AdvancedLogger::instance().fatal(msg, ##__VA_ARGS__)

#define LOG_PERFORMANCE(operation) PerformanceLogger _perf_logger(operation)
#define LOG_CONTEXT(context) LogContextManager _context_manager(context)
