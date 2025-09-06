#include "model_file.hpp"
#include "logger.hpp"
#include "path_validator.hpp"
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <cstring>

namespace {
    // Safe buffer size constants
    constexpr size_t MAX_FILE_SIZE = 50ULL * 1024 * 1024 * 1024; // 50GB
    constexpr size_t READ_BUFFER_SIZE = 64 * 1024; // 64KB chunks
    constexpr size_t MAX_PATH_LENGTH = 4096;
    constexpr size_t MAX_METADATA_SIZE = 1024 * 1024; // 1MB
    
    // Validate file extension for security
    bool is_valid_model_extension(const std::string& path) {
        static const std::vector<std::string> valid_extensions = {
            ".gguf", ".ggml", ".bin", ".safetensors", ".pt", ".pth", ".model", ".onnx"
        };
        
        std::filesystem::path file_path(path);
        std::string ext = file_path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        return std::find(valid_extensions.begin(), valid_extensions.end(), ext) != valid_extensions.end();
    }
    
    // Safe string copy with bounds checking
    void safe_string_copy(char* dest, size_t dest_size, const std::string& src) {
        if (dest == nullptr || dest_size == 0) {
            return;
        }
        
        size_t copy_size = std::min(src.length(), dest_size - 1);
        std::memcpy(dest, src.c_str(), copy_size);
        dest[copy_size] = '\0';
    }
    
    // Validate metadata content
    bool is_valid_metadata(const nlohmann::json& metadata) {
        if (metadata.dump().size() > MAX_METADATA_SIZE) {
            return false;
        }
        
        // Check for potentially dangerous keys
        static const std::vector<std::string> dangerous_keys = {
            "script", "exec", "command", "eval", "__proto__", "constructor"
        };
        
        std::function<bool(const nlohmann::json&)> check_recursive = [&](const nlohmann::json& obj) -> bool {
            if (obj.is_object()) {
                for (auto& [key, value] : obj.items()) {
                    // Check for dangerous keys
                    for (const auto& dangerous_key : dangerous_keys) {
                        if (key.find(dangerous_key) != std::string::npos) {
                            return false;
                        }
                    }
                    
                    if (!check_recursive(value)) {
                        return false;
                    }
                }
            } else if (obj.is_array()) {
                for (const auto& item : obj) {
                    if (!check_recursive(item)) {
                        return false;
                    }
                }
            }
            
            return true;
        };
        
        return check_recursive(metadata);
    }
}

ModelFile::ModelFile() : loaded_(false), file_size_(0) {
    // Initialize with safe defaults
    std::memset(model_path_, 0, sizeof(model_path_));
    std::memset(model_name_, 0, sizeof(model_name_));
}

ModelFile::~ModelFile() {
    if (loaded_) {
        unload();
    }
}

bool ModelFile::load(const std::string& path) {
    if (path.empty() || path.length() > MAX_PATH_LENGTH) {
        LOG_ERROR("Invalid model file path: empty or too long");
        return false;
    }
    
    // Validate and normalize path
    auto normalized_path = PathValidator::normalize_path(path);
    if (!normalized_path) {
        LOG_ERROR_F("Failed to normalize model file path: %s", path.c_str());
        return false;
    }
    
    if (!PathValidator::is_safe_path(*normalized_path)) {
        LOG_ERROR_F("Unsafe model file path detected: %s", path.c_str());
        return false;
    }
    
    // Validate file extension
    if (!is_valid_model_extension(*normalized_path)) {
        LOG_ERROR_F("Invalid model file extension: %s", normalized_path->c_str());
        return false;
    }
    
    // Check if file exists and get size
    std::error_code ec;
    if (!std::filesystem::exists(*normalized_path, ec) || ec) {
        LOG_ERROR_F("Model file does not exist: %s", normalized_path->c_str());
        return false;
    }
    
    auto file_size = std::filesystem::file_size(*normalized_path, ec);
    if (ec || file_size > MAX_FILE_SIZE) {
        LOG_ERROR_F("Model file too large or inaccessible: %s (size: %llu)", 
                   normalized_path->c_str(), static_cast<unsigned long long>(file_size));
        return false;
    }
    
    // Validate file permissions
    auto perms = std::filesystem::status(*normalized_path, ec).permissions();
    if (ec || (perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none) {
        LOG_ERROR_F("Model file not readable: %s", normalized_path->c_str());
        return false;
    }
    
    // Unload previous model if loaded
    if (loaded_) {
        unload();
    }
    
    // Load model metadata safely
    try {
        if (!load_metadata(*normalized_path)) {
            LOG_WARN_F("Failed to load metadata for model: %s", normalized_path->c_str());
            // Continue loading without metadata
        }
        
        // Store validated information
        safe_string_copy(model_path_, sizeof(model_path_), *normalized_path);
        
        std::filesystem::path file_path(*normalized_path);
        std::string filename = file_path.filename().string();
        safe_string_copy(model_name_, sizeof(model_name_), filename);
        
        file_size_ = file_size;
        loaded_ = true;
        
        LOG_INFO_F("Model loaded successfully: %s (size: %llu bytes)", 
                  model_name_, static_cast<unsigned long long>(file_size_));
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to load model %s: %s", normalized_path->c_str(), e.what());
        return false;
    }
}

void ModelFile::unload() {
    if (!loaded_) {
        return;
    }
    
    // Clear sensitive data securely
    std::memset(model_path_, 0, sizeof(model_path_));
    std::memset(model_name_, 0, sizeof(model_name_));
    
    metadata_.clear();
    file_size_ = 0;
    loaded_ = false;
    
    LOG_INFO("Model unloaded successfully");
}

bool ModelFile::is_loaded() const {
    return loaded_;
}

std::string ModelFile::get_path() const {
    if (!loaded_) {
        return "";
    }
    
    return std::string(model_path_);
}

std::string ModelFile::get_name() const {
    if (!loaded_) {
        return "";
    }
    
    return std::string(model_name_);
}

size_t ModelFile::get_size() const {
    return loaded_ ? file_size_ : 0;
}

nlohmann::json ModelFile::get_metadata() const {
    return loaded_ ? metadata_ : nlohmann::json::object();
}

bool ModelFile::validate_integrity() const {
    if (!loaded_) {
        LOG_ERROR("Cannot validate integrity: no model loaded");
        return false;
    }
    
    try {
        // Check if file still exists and has same size
        std::error_code ec;
        if (!std::filesystem::exists(model_path_, ec) || ec) {
            LOG_ERROR_F("Model file no longer exists: %s", model_path_);
            return false;
        }
        
        auto current_size = std::filesystem::file_size(model_path_, ec);
        if (ec || current_size != file_size_) {
            LOG_ERROR_F("Model file size changed: expected %llu, got %llu", 
                       static_cast<unsigned long long>(file_size_),
                       static_cast<unsigned long long>(current_size));
            return false;
        }
        
        // Validate file is still readable
        std::ifstream file(model_path_, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERROR_F("Model file no longer readable: %s", model_path_);
            return false;
        }
        
        // Quick integrity check - read first and last chunks
        std::vector<char> buffer(std::min(static_cast<size_t>(READ_BUFFER_SIZE), file_size_));
        
        file.read(buffer.data(), buffer.size());
        if (file.gcount() != static_cast<std::streamsize>(buffer.size())) {
            LOG_ERROR_F("Failed to read from model file: %s", model_path_);
            return false;
        }
        
        // Check end of file if large enough
        if (file_size_ > READ_BUFFER_SIZE * 2) {
            file.seekg(-static_cast<std::streamoff>(READ_BUFFER_SIZE), std::ios::end);
            file.read(buffer.data(), buffer.size());
            if (file.gcount() != static_cast<std::streamsize>(buffer.size())) {
                LOG_ERROR_F("Failed to read end of model file: %s", model_path_);
                return false;
            }
        }
        
        LOG_DEBUG_F("Model integrity validation passed: %s", model_path_);
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR_F("Model integrity validation failed: %s", e.what());
        return false;
    }
}

bool ModelFile::read_chunk(size_t offset, size_t size, std::vector<uint8_t>& output) const {
    if (!loaded_) {
        LOG_ERROR("Cannot read chunk: no model loaded");
        return false;
    }
    
    if (offset >= file_size_ || size == 0) {
        LOG_ERROR_F("Invalid chunk parameters: offset=%zu, size=%zu, file_size=%zu", 
                   offset, size, file_size_);
        return false;
    }
    
    // Ensure we don't read beyond file bounds
    size_t actual_size = std::min(size, file_size_ - offset);
    if (actual_size > READ_BUFFER_SIZE * 1024) { // Limit to reasonable chunk size
        LOG_ERROR_F("Chunk size too large: requested=%zu, max=%zu", actual_size, READ_BUFFER_SIZE * 1024);
        return false;
    }
    
    try {
        std::ifstream file(model_path_, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERROR_F("Failed to open model file for reading: %s", model_path_);
            return false;
        }
        
        file.seekg(static_cast<std::streamoff>(offset));
        if (file.fail()) {
            LOG_ERROR_F("Failed to seek to offset %zu in model file", offset);
            return false;
        }
        
        // Safely resize output buffer
        try {
            output.resize(actual_size);
        } catch (const std::bad_alloc&) {
            LOG_ERROR_F("Failed to allocate buffer for chunk: size=%zu", actual_size);
            return false;
        }
        
        file.read(reinterpret_cast<char*>(output.data()), static_cast<std::streamsize>(actual_size));
        if (file.gcount() != static_cast<std::streamsize>(actual_size)) {
            LOG_ERROR_F("Failed to read full chunk: expected=%zu, read=%lld", 
                       actual_size, static_cast<long long>(file.gcount()));
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to read chunk from model file: %s", e.what());
        return false;
    }
}

bool ModelFile::load_metadata(const std::string& model_path) {
    try {
        // Try to load metadata from accompanying .json file
        std::filesystem::path meta_path = std::filesystem::path(model_path).replace_extension(".json");
        
        std::error_code ec;
        if (std::filesystem::exists(meta_path, ec) && !ec) {
            std::ifstream meta_file(meta_path);
            if (meta_file.is_open()) {
                // Check file size before reading
                auto meta_size = std::filesystem::file_size(meta_path, ec);
                if (!ec && meta_size <= MAX_METADATA_SIZE) {
                    nlohmann::json temp_metadata;
                    meta_file >> temp_metadata;
                    
                    if (is_valid_metadata(temp_metadata)) {
                        metadata_ = std::move(temp_metadata);
                        LOG_DEBUG_F("Loaded metadata from: %s", meta_path.string().c_str());
                        return true;
                    } else {
                        LOG_WARN_F("Invalid metadata content in: %s", meta_path.string().c_str());
                    }
                } else {
                    LOG_WARN_F("Metadata file too large: %s", meta_path.string().c_str());
                }
            }
        }
        
        // Try to extract metadata from GGUF file header (basic implementation)
        if (model_path.ends_with(".gguf")) {
            return load_gguf_metadata(model_path);
        }
        
        // Set default metadata
        metadata_ = nlohmann::json::object();
        metadata_["format"] = "unknown";
        metadata_["loaded_at"] = std::time(nullptr);
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_WARN_F("Failed to load metadata: %s", e.what());
        metadata_ = nlohmann::json::object();
        return false;
    }
}

bool ModelFile::load_gguf_metadata(const std::string& model_path) {
    try {
        std::ifstream file(model_path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Read GGUF magic number (4 bytes)
        char magic[4];
        file.read(magic, 4);
        if (file.gcount() != 4 || std::string(magic, 4) != "GGUF") {
            LOG_DEBUG("Not a valid GGUF file");
            return false;
        }
        
        // Basic GGUF header parsing with bounds checking
        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (file.gcount() != sizeof(version)) {
            return false;
        }
        
        metadata_ = nlohmann::json::object();
        metadata_["format"] = "gguf";
        metadata_["version"] = version;
        metadata_["loaded_at"] = std::time(nullptr);
        
        LOG_DEBUG_F("Loaded GGUF metadata: version=%u", version);
        return true;
        
    } catch (const std::exception& e) {
        LOG_WARN_F("Failed to parse GGUF metadata: %s", e.what());
        return false;
    }
}
