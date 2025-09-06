#include "path_validator.hpp"
#include "logger.hpp"
#include <filesystem>
#include <algorithm>
#include <regex>

namespace {
    // Dangerous path patterns
    const std::vector<std::string> DANGEROUS_PATTERNS = {
        "..", "~", "$", "|", "&", ";", "`", "!", "<", ">", "*", "?"
    };
    
    // Dangerous filenames (Windows reserved names)
    const std::vector<std::string> RESERVED_NAMES = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };
    
    // Maximum path length across platforms
    constexpr size_t MAX_PATH_LENGTH = 4096;
    constexpr size_t MAX_FILENAME_LENGTH = 255;
}

std::optional<std::string> PathValidator::normalize_path(const std::string& path) {
    if (path.empty() || path.length() > MAX_PATH_LENGTH) {
        return std::nullopt;
    }
    
    try {
        // Use filesystem to normalize the path
        std::filesystem::path fs_path(path);
        std::error_code ec;
        
        // Convert to absolute path to resolve relative components
        auto canonical_path = std::filesystem::weakly_canonical(fs_path, ec);
        if (ec) {
            LOG_DEBUG_F("Failed to canonicalize path: %s", path.c_str());
            return std::nullopt;
        }
        
        std::string normalized = canonical_path.string();
        
        // Additional validation
        if (normalized.length() > MAX_PATH_LENGTH) {
            return std::nullopt;
        }
        
        // Check for dangerous components
        if (has_dangerous_components(normalized)) {
            return std::nullopt;
        }
        
        return normalized;
        
    } catch (const std::exception& e) {
        LOG_DEBUG_F("Path normalization failed: %s", e.what());
        return std::nullopt;
    }
}

bool PathValidator::is_safe_path(const std::string& path) {
    if (path.empty() || path.length() > MAX_PATH_LENGTH) {
        return false;
    }
    
    // Check for null bytes
    if (path.find('\0') != std::string::npos) {
        return false;
    }
    
    // Check for dangerous components
    if (has_dangerous_components(path)) {
        return false;
    }
    
    // Check for reserved names (Windows)
    std::filesystem::path fs_path(path);
    std::string filename = fs_path.filename().string();
    
    // Convert to uppercase for comparison
    std::string upper_filename = filename;
    std::transform(upper_filename.begin(), upper_filename.end(), upper_filename.begin(), ::toupper);
    
    // Remove extension for reserved name check
    size_t dot_pos = upper_filename.find('.');
    if (dot_pos != std::string::npos) {
        upper_filename = upper_filename.substr(0, dot_pos);
    }
    
    for (const auto& reserved : RESERVED_NAMES) {
        if (upper_filename == reserved) {
            return false;
        }
    }
    
    return true;
}

bool PathValidator::is_within_directory(const std::string& path, const std::string& allowed_dir) {
    auto normalized_path = normalize_path(path);
    auto normalized_allowed = normalize_path(allowed_dir);
    
    if (!normalized_path || !normalized_allowed) {
        return false;
    }
    
    try {
        std::filesystem::path target_path(*normalized_path);
        std::filesystem::path base_path(*normalized_allowed);
        
        // Convert to absolute paths
        std::error_code ec;
        target_path = std::filesystem::absolute(target_path, ec);
        if (ec) return false;
        
        base_path = std::filesystem::absolute(base_path, ec);
        if (ec) return false;
        
        // Check if target path starts with base path
        auto target_it = target_path.begin();
        auto base_it = base_path.begin();
        
        for (; base_it != base_path.end() && target_it != target_path.end(); ++base_it, ++target_it) {
            if (*base_it != *target_it) {
                return false;
            }
        }
        
        // Ensure we've consumed all of base path
        return base_it == base_path.end();
        
    } catch (const std::exception& e) {
        LOG_DEBUG_F("Directory validation failed: %s", e.what());
        return false;
    }
}

std::string PathValidator::sanitize_filename(const std::string& filename) {
    if (filename.empty() || filename.length() > MAX_FILENAME_LENGTH) {
        return "";
    }
    
    std::string sanitized = filename;
    
    // Remove or replace dangerous characters
    static const std::string dangerous_chars = R"(<>:"/\|?*)" "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
                                                "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f";
    
    for (char& c : sanitized) {
        if (dangerous_chars.find(c) != std::string::npos) {
            c = '_';
        }
    }
    
    // Remove leading/trailing spaces and dots
    while (!sanitized.empty() && (sanitized.front() == ' ' || sanitized.front() == '.')) {
        sanitized.erase(0, 1);
    }
    
    while (!sanitized.empty() && (sanitized.back() == ' ' || sanitized.back() == '.')) {
        sanitized.pop_back();
    }
    
    // Check against reserved names
    std::string upper_sanitized = sanitized;
    std::transform(upper_sanitized.begin(), upper_sanitized.end(), upper_sanitized.begin(), ::toupper);
    
    size_t dot_pos = upper_sanitized.find('.');
    std::string name_part = (dot_pos != std::string::npos) ? upper_sanitized.substr(0, dot_pos) : upper_sanitized;
    
    for (const auto& reserved : RESERVED_NAMES) {
        if (name_part == reserved) {
            sanitized = "safe_" + sanitized;
            break;
        }
    }
    
    // Ensure filename is not empty after sanitization
    if (sanitized.empty()) {
        sanitized = "unnamed_file";
    }
    
    return sanitized;
}

bool PathValidator::has_dangerous_components(const std::string& path) {
    // Check for dangerous patterns
    for (const auto& pattern : DANGEROUS_PATTERNS) {
        if (path.find(pattern) != std::string::npos) {
            // Special case for ".." - allow it only in normalized form
            if (pattern == ".." && path.find("..") != std::string::npos) {
                // Check if it's part of a directory traversal attempt
                if (path.find("../") != std::string::npos || 
                    path.find("..\\") != std::string::npos ||
                    path.find("/..") != std::string::npos ||
                    path.find("\\..") != std::string::npos) {
                    return true;
                }
            } else if (pattern != "..") {
                return true;
            }
        }
    }
    
    // Check for suspicious sequences
    static const std::vector<std::string> suspicious = {
        "//", "\\\\", "./", ".\\", "~/"
    };
    
    for (const auto& seq : suspicious) {
        if (path.find(seq) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}
