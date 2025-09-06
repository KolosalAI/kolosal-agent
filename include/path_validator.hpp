#pragma once

#include <string>
#include <optional>

/**
 * @brief Path validation and normalization utilities
 * 
 * Provides secure path handling to prevent directory traversal attacks
 * and ensure paths are within allowed boundaries.
 */
class PathValidator {
public:
    /**
     * @brief Normalize path and resolve relative components
     * @param path Path to normalize
     * @return Normalized path or nullopt if invalid
     */
    static std::optional<std::string> normalize_path(const std::string& path);

    /**
     * @brief Check if path is safe (no traversal attacks)
     * @param path Path to check
     * @return true if path is safe
     */
    static bool is_safe_path(const std::string& path);

    /**
     * @brief Check if path is within allowed directory
     * @param path Path to check
     * @param allowed_dir Allowed base directory
     * @return true if path is within allowed directory
     */
    static bool is_within_directory(const std::string& path, const std::string& allowed_dir);

    /**
     * @brief Sanitize filename (remove dangerous characters)
     * @param filename Filename to sanitize
     * @return Sanitized filename
     */
    static std::string sanitize_filename(const std::string& filename);

private:
    /**
     * @brief Check for dangerous path components
     * @param path Path to check
     * @return true if path contains dangerous components
     */
    static bool has_dangerous_components(const std::string& path);
};
