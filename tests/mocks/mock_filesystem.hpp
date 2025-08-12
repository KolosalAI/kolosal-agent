/**
 * @file mock_filesystem.hpp
 * @brief Mock filesystem operations for testing
 */

#pragma once

#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace kolosal::agents::test {

/**
 * @brief Mock filesystem interface for testing file operations
 */
class MockFilesystem {
public:
    MOCK_METHOD(bool, file_exists, (const std::string& path), (const));
    MOCK_METHOD(bool, directory_exists, (const std::string& path), (const));
    MOCK_METHOD(bool, create_directory, (const std::string& path), ());
    MOCK_METHOD(bool, remove_file, (const std::string& path), ());
    MOCK_METHOD(bool, remove_directory, (const std::string& path), ());
    MOCK_METHOD(std::string, read_file, (const std::string& path), (const));
    MOCK_METHOD(bool, write_file, (const std::string& path, const std::string& content), ());
    MOCK_METHOD(std::vector<std::string>, list_directory, (const std::string& path), (const));
    MOCK_METHOD(size_t, file_size, (const std::string& path), (const));
    
    // Helper methods for setting up filesystem state
    void addFile(const std::string& path, const std::string& content) {
        files_[path] = content;
        ON_CALL(*this, file_exists(path))
            .WillByDefault(testing::Return(true));
        ON_CALL(*this, read_file(path))
            .WillByDefault(testing::Return(content));
        ON_CALL(*this, file_size(path))
            .WillByDefault(testing::Return(content.size()));
    }
    
    void addDirectory(const std::string& path, const std::vector<std::string>& contents = {}) {
        directories_[path] = contents;
        ON_CALL(*this, directory_exists(path))
            .WillByDefault(testing::Return(true));
        ON_CALL(*this, list_directory(path))
            .WillByDefault(testing::Return(contents));
    }
    
    void removeFile(const std::string& path) {
        files_.erase(path);
        ON_CALL(*this, file_exists(path))
            .WillByDefault(testing::Return(false));
    }
    
    void removeDirectory(const std::string& path) {
        directories_.erase(path);
        ON_CALL(*this, directory_exists(path))
            .WillByDefault(testing::Return(false));
    }
    
    void clear() {
        files_.clear();
        directories_.clear();
    }

private:
    std::map<std::string, std::string> files_;
    std::map<std::string, std::vector<std::string>> directories_;
};

} // namespace kolosal::agents::test
