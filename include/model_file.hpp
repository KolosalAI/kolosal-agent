#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <json.hpp>

/**
 * @brief Safe model file handler with buffer overflow protection
 * 
 * Features:
 * - Path validation and normalization
 * - File size and extension validation
 * - Safe buffer operations with bounds checking
 * - Integrity validation
 * - Secure metadata loading
 */
class ModelFile {
public:
    /**
     * @brief Constructor
     */
    ModelFile();

    /**
     * @brief Destructor - securely clears data
     */
    ~ModelFile();

    /**
     * @brief Load model file with safety checks
     * @param path Path to model file (validated and normalized)
     * @return true if loaded successfully, false otherwise
     */
    bool load(const std::string& path);

    /**
     * @brief Unload model and clear data securely
     */
    void unload();

    /**
     * @brief Check if model is loaded
     * @return true if model is loaded
     */
    bool is_loaded() const;

    /**
     * @brief Get model file path
     * @return Model file path (empty if not loaded)
     */
    std::string get_path() const;

    /**
     * @brief Get model name (filename)
     * @return Model name (empty if not loaded)
     */
    std::string get_name() const;

    /**
     * @brief Get model file size
     * @return File size in bytes (0 if not loaded)
     */
    size_t get_size() const;

    /**
     * @brief Get model metadata
     * @return JSON metadata object (empty if not loaded)
     */
    nlohmann::json get_metadata() const;

    /**
     * @brief Validate model file integrity
     * @return true if file is intact and accessible
     */
    bool validate_integrity() const;

    /**
     * @brief Read chunk of model file safely
     * @param offset Offset in bytes from start of file
     * @param size Number of bytes to read
     * @param output Output buffer (resized appropriately)
     * @return true if read successfully
     */
    bool read_chunk(size_t offset, size_t size, std::vector<uint8_t>& output) const;

private:
    bool loaded_;
    char model_path_[4096];  // Fixed-size buffer for security
    char model_name_[256];   // Fixed-size buffer for security
    size_t file_size_;
    nlohmann::json metadata_;

    /**
     * @brief Load metadata from accompanying files or headers
     * @param model_path Path to model file
     * @return true if metadata loaded (or defaults set)
     */
    bool load_metadata(const std::string& model_path);

    /**
     * @brief Load metadata from GGUF file header
     * @param model_path Path to GGUF model file
     * @return true if GGUF metadata parsed successfully
     */
    bool load_gguf_metadata(const std::string& model_path);
};
