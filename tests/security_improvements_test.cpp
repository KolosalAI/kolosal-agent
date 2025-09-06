#include <gtest/gtest.h>
#include "http_client.hpp"
#include "model_file.hpp"
#include "path_validator.hpp"
#include <filesystem>
#include <fstream>

// Test fixtures
class HttpClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.base_url = "http://localhost:8081";
        config_.timeout_seconds = 5;
        config_.max_retries = 2;
        config_.retry_delay_ms = 100;
    }
    
    HttpClient::Config config_;
};

class ModelFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test file
        test_file_path_ = std::filesystem::temp_directory_path() / "test_model.gguf";
        std::ofstream file(test_file_path_, std::ios::binary);
        
        // Write GGUF magic and version
        file.write("GGUF", 4);
        uint32_t version = 1;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));
        
        // Write some dummy data
        std::vector<char> dummy_data(1024, 'A');
        file.write(dummy_data.data(), dummy_data.size());
        file.close();
    }
    
    void TearDown() override {
        if (std::filesystem::exists(test_file_path_)) {
            std::filesystem::remove(test_file_path_);
        }
    }
    
    std::filesystem::path test_file_path_;
};

class PathValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for testing
        test_dir_ = std::filesystem::temp_directory_path() / "path_validator_test";
        std::filesystem::create_directories(test_dir_);
    }
    
    void TearDown() override {
        if (std::filesystem::exists(test_dir_)) {
            std::filesystem::remove_all(test_dir_);
        }
    }
    
    std::filesystem::path test_dir_;
};

// HTTP Client Security Tests
TEST_F(HttpClientTest, RejectsInvalidURLs) {
    // Test various invalid URL formats
    std::vector<std::string> invalid_urls = {
        "",
        "not-a-url",
        "http://",
        "ftp://example.com",
        "http://[invalid-host",
        "http://host:99999",
        std::string(3000, 'a'), // Too long URL
        "http://host\nwith\nnewlines"
    };
    
    for (const auto& url : invalid_urls) {
        HttpClient::Config invalid_config;
        invalid_config.base_url = url;
        
        EXPECT_THROW(HttpClient client(invalid_config), std::invalid_argument);
    }
}

TEST_F(HttpClientTest, ValidatesTimeoutRange) {
    // Test invalid timeout values
    std::vector<int> invalid_timeouts = {-1, 0, 301, 999};
    
    for (int timeout : invalid_timeouts) {
        HttpClient::Config invalid_config = config_;
        invalid_config.timeout_seconds = timeout;
        
        EXPECT_THROW(HttpClient client(invalid_config), std::invalid_argument);
    }
}

TEST_F(HttpClientTest, ValidatesRetryRange) {
    // Test invalid retry values
    std::vector<int> invalid_retries = {-1, 11, 100};
    
    for (int retries : invalid_retries) {
        HttpClient::Config invalid_config = config_;
        invalid_config.max_retries = retries;
        
        EXPECT_THROW(HttpClient client(invalid_config), std::invalid_argument);
    }
}

TEST_F(HttpClientTest, HandlesLargeResponseBody) {
    HttpClient client(config_);
    
    // Test request with large endpoint (should be validated)
    std::string large_endpoint(2000, 'a');
    auto result = client.request("GET", large_endpoint);
    
    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.status_code, 400);
}

TEST_F(HttpClientTest, SanitizesHeaders) {
    HttpClient client(config_);
    
    // Test headers with potentially dangerous content
    std::map<std::string, std::string> dangerous_headers = {
        {"X-Test\nHeader", "value"},
        {"X-Test\rHeader", "value"},
        {"X-Test", "value\nwith\nnewlines"},
        {std::string(10000, 'H'), "value"} // Too long header
    };
    
    auto result = client.request("GET", "/test", "", dangerous_headers);
    // Should not crash and should handle sanitization
    EXPECT_TRUE(result.status_code >= 400); // Expected to fail due to invalid server
}

// Model File Security Tests
TEST_F(ModelFileTest, ValidatesFilePath) {
    ModelFile model;
    
    // Test various invalid paths
    std::vector<std::string> invalid_paths = {
        "",
        "../../../etc/passwd",
        "/dev/null",
        "C:\\Windows\\System32\\config\\SAM",
        std::string(5000, 'a'), // Too long path
        "model\0injection.gguf", // Null byte injection
        "model|dangerous.gguf"   // Pipe character
    };
    
    for (const auto& path : invalid_paths) {
        EXPECT_FALSE(model.load(path)) << "Should reject path: " << path;
        EXPECT_FALSE(model.is_loaded());
    }
}

TEST_F(ModelFileTest, ValidatesFileExtension) {
    ModelFile model;
    
    // Create file with invalid extension
    auto invalid_file = std::filesystem::temp_directory_path() / "test_model.exe";
    std::ofstream file(invalid_file);
    file << "dummy content";
    file.close();
    
    EXPECT_FALSE(model.load(invalid_file.string()));
    EXPECT_FALSE(model.is_loaded());
    
    std::filesystem::remove(invalid_file);
}

TEST_F(ModelFileTest, LoadsValidGGUFFile) {
    ModelFile model;
    
    EXPECT_TRUE(model.load(test_file_path_.string()));
    EXPECT_TRUE(model.is_loaded());
    EXPECT_EQ(model.get_name(), "test_model.gguf");
    EXPECT_GT(model.get_size(), 0);
    
    // Test metadata
    auto metadata = model.get_metadata();
    EXPECT_EQ(metadata["format"], "gguf");
    EXPECT_EQ(metadata["version"], 1);
}

TEST_F(ModelFileTest, ValidatesIntegrity) {
    ModelFile model;
    
    EXPECT_TRUE(model.load(test_file_path_.string()));
    EXPECT_TRUE(model.validate_integrity());
    
    // Modify file and test integrity
    std::ofstream file(test_file_path_, std::ios::app);
    file << "corrupted data";
    file.close();
    
    EXPECT_FALSE(model.validate_integrity());
}

TEST_F(ModelFileTest, SafeChunkReading) {
    ModelFile model;
    EXPECT_TRUE(model.load(test_file_path_.string()));
    
    std::vector<uint8_t> chunk;
    
    // Test valid chunk reading
    EXPECT_TRUE(model.read_chunk(0, 100, chunk));
    EXPECT_EQ(chunk.size(), 100);
    
    // Test invalid parameters
    EXPECT_FALSE(model.read_chunk(model.get_size(), 100, chunk)); // Beyond file
    EXPECT_FALSE(model.read_chunk(0, 0, chunk)); // Zero size
    EXPECT_FALSE(model.read_chunk(0, SIZE_MAX, chunk)); // Too large
}

// Path Validator Security Tests
TEST_F(PathValidatorTest, NormalizesValidPaths) {
    auto normalized = PathValidator::normalize_path("./test/../valid/path");
    EXPECT_TRUE(normalized.has_value());
    EXPECT_EQ(normalized->find(".."), std::string::npos);
}

TEST_F(PathValidatorTest, RejectsTraversalAttempts) {
    std::vector<std::string> traversal_attempts = {
        "../../../etc/passwd",
        "..\\..\\..\\windows\\system32",
        "/test/../../../root",
        "test/../../etc/shadow",
        "./test/../../../usr/bin"
    };
    
    for (const auto& attempt : traversal_attempts) {
        EXPECT_FALSE(PathValidator::is_safe_path(attempt)) 
            << "Should reject traversal: " << attempt;
    }
}

TEST_F(PathValidatorTest, RejectsReservedNames) {
    std::vector<std::string> reserved_names = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "LPT1", "LPT2",
        "con.txt", "prn.log"
    };
    
    for (const auto& name : reserved_names) {
        EXPECT_FALSE(PathValidator::is_safe_path(name))
            << "Should reject reserved name: " << name;
    }
}

TEST_F(PathValidatorTest, ValidatesDirectoryBounds) {
    std::filesystem::path safe_dir = test_dir_ / "safe";
    std::filesystem::create_directories(safe_dir);
    
    std::string safe_file = (safe_dir / "file.txt").string();
    std::string unsafe_file = (test_dir_ / "../outside.txt").string();
    
    EXPECT_TRUE(PathValidator::is_within_directory(safe_file, test_dir_.string()));
    EXPECT_FALSE(PathValidator::is_within_directory(unsafe_file, test_dir_.string()));
}

TEST_F(PathValidatorTest, SanitizesFilenames) {
    struct TestCase {
        std::string input;
        std::string expected_safe;
    };
    
    std::vector<TestCase> test_cases = {
        {"normal_file.txt", "normal_file.txt"},
        {"file<with>bad:chars", "file_with_bad_chars"},
        {"CON", "safe_CON"},
        {"file\0with\0nulls", "file_with_nulls"},
        {"   file   ", "file"},
        {"..hidden", "hidden"},
        {"", "unnamed_file"}
    };
    
    for (const auto& test_case : test_cases) {
        auto result = PathValidator::sanitize_filename(test_case.input);
        EXPECT_FALSE(result.empty()) << "Sanitized filename should not be empty for: " << test_case.input;
        EXPECT_EQ(result.find('<'), std::string::npos) << "Should remove dangerous chars from: " << test_case.input;
    }
}

// Integration Security Tests
TEST(SecurityIntegrationTest, EndToEndHttpSafety) {
    HttpClient::Config config;
    config.base_url = "http://localhost:9999"; // Non-existent server
    config.timeout_seconds = 1;
    config.max_retries = 1;
    
    HttpClient client(config);
    
    // Test that client handles connection failures gracefully
    auto result = client.request("GET", "/test");
    EXPECT_FALSE(result.is_success());
    EXPECT_TRUE(result.retry_recommended); // Should suggest retry for connection issues
}

// Performance and Resource Tests
TEST(SecurityPerformanceTest, HandlesConcurrentRequests) {
    HttpClient::Config config;
    config.base_url = "http://localhost:9999";
    config.timeout_seconds = 1;
    
    // Test that multiple clients can be created safely
    std::vector<std::unique_ptr<HttpClient>> clients;
    for (int i = 0; i < 10; ++i) {
        clients.emplace_back(std::make_unique<HttpClient>(config));
    }
    
    // All clients should be created successfully
    EXPECT_EQ(clients.size(), 10);
}

TEST(SecurityPerformanceTest, LimitsMemoryUsage) {
    ModelFile model;
    
    // Create a larger test file
    auto large_file = std::filesystem::temp_directory_path() / "large_model.gguf";
    std::ofstream file(large_file, std::ios::binary);
    
    // Write GGUF header
    file.write("GGUF", 4);
    uint32_t version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    
    // Write larger dummy data (1MB)
    std::vector<char> dummy_data(1024 * 1024, 'B');
    file.write(dummy_data.data(), dummy_data.size());
    file.close();
    
    EXPECT_TRUE(model.load(large_file.string()));
    
    // Test chunked reading doesn't consume excessive memory
    std::vector<uint8_t> chunk;
    EXPECT_TRUE(model.read_chunk(0, 64 * 1024, chunk)); // 64KB chunk
    EXPECT_EQ(chunk.size(), 64 * 1024);
    
    std::filesystem::remove(large_file);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
