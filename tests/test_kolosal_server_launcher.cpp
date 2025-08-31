#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "kolosal_server_launcher.hpp"
#include <filesystem>
#include <thread>
#include <chrono>

class KolosalServerLauncherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test configuration
        config.executable_path = "mock_kolosal_server"; // Mock executable for testing
        config.host = "127.0.0.1";
        config.port = 8082; // Use different port for testing
        config.log_level = "DEBUG";
        config.quiet_mode = true;
        config.public_access = false;
        config.internet_access = false;
        config.timeout = 10; // Shorter timeout for testing
        config.working_directory = std::filesystem::current_path().string();
        
        launcher = std::make_unique<KolosalServerLauncher>(config);
    }

    void TearDown() override {
        if (launcher && launcher->is_running()) {
            launcher->stop();
        }
        launcher.reset();
    }

    KolosalServerLauncher::ServerConfig config;
    std::unique_ptr<KolosalServerLauncher> launcher;
};

TEST_F(KolosalServerLauncherTest, ConstructorWithConfig) {
    EXPECT_NE(launcher, nullptr);
    EXPECT_EQ(launcher->get_status(), KolosalServerLauncher::Status::STOPPED);
    EXPECT_FALSE(launcher->is_running());
}

TEST_F(KolosalServerLauncherTest, GetConfiguration) {
    const auto& launcher_config = launcher->get_config();
    
    EXPECT_EQ(launcher_config.host, config.host);
    EXPECT_EQ(launcher_config.port, config.port);
    EXPECT_EQ(launcher_config.log_level, config.log_level);
    EXPECT_EQ(launcher_config.quiet_mode, config.quiet_mode);
}

TEST_F(KolosalServerLauncherTest, UpdateConfiguration) {
    KolosalServerLauncher::ServerConfig new_config = config;
    new_config.host = "0.0.0.0";
    new_config.port = 9090;
    new_config.log_level = "ERROR";
    
    launcher->update_config(new_config);
    
    const auto& updated_config = launcher->get_config();
    EXPECT_EQ(updated_config.host, "0.0.0.0");
    EXPECT_EQ(updated_config.port, 9090);
    EXPECT_EQ(updated_config.log_level, "ERROR");
}

TEST_F(KolosalServerLauncherTest, GetServerUrl) {
    std::string url = launcher->get_server_url();
    EXPECT_EQ(url, "http://127.0.0.1:8082");
}

TEST_F(KolosalServerLauncherTest, GetStatusString) {
    std::string status_str = launcher->get_status_string();
    EXPECT_FALSE(status_str.empty());
    EXPECT_EQ(status_str, "STOPPED"); // Initial status
}

TEST_F(KolosalServerLauncherTest, StatusValues) {
    // Test all status values
    EXPECT_EQ(static_cast<int>(KolosalServerLauncher::Status::STOPPED), 0);
    EXPECT_NE(static_cast<int>(KolosalServerLauncher::Status::STARTING), 
              static_cast<int>(KolosalServerLauncher::Status::STOPPED));
}

// Note: The following tests are for interface validation since we don't have a real executable

TEST_F(KolosalServerLauncherTest, StartWithNonExistentExecutable) {
    // Should handle non-existent executable gracefully
    bool started = launcher->start();
    EXPECT_FALSE(started); // Should fail to start non-existent executable
    EXPECT_EQ(launcher->get_status(), KolosalServerLauncher::Status::ERROR);
}

TEST_F(KolosalServerLauncherTest, StopWhenNotRunning) {
    // Should handle stop when not running gracefully
    bool stopped = launcher->stop();
    EXPECT_TRUE(stopped); // Should succeed stopping a non-running server
}

TEST_F(KolosalServerLauncherTest, IsHealthyWhenNotRunning) {
    bool healthy = launcher->is_healthy();
    EXPECT_FALSE(healthy); // Should not be healthy when not running
}

TEST_F(KolosalServerLauncherTest, WaitForReadyWhenNotRunning) {
    bool ready = launcher->wait_for_ready(1); // Short timeout
    EXPECT_FALSE(ready); // Should timeout when server is not running
}

TEST_F(KolosalServerLauncherTest, StatusCallbackSetup) {
    bool callback_called = false;
    std::string callback_message;
    KolosalServerLauncher::Status callback_status;
    
    launcher->set_status_callback([&](KolosalServerLauncher::Status status, const std::string& message) {
        callback_called = true;
        callback_status = status;
        callback_message = message;
    });
    
    // Try to start (will fail but should trigger callback)
    launcher->start();
    
    // Give time for callback
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(callback_called);
}

TEST_F(KolosalServerLauncherTest, CreateDefaultServerConfig) {
    auto default_config = create_default_server_config();
    
    EXPECT_FALSE(default_config.executable_path.empty());
    EXPECT_EQ(default_config.host, "127.0.0.1");
    EXPECT_GT(default_config.port, 0);
    EXPECT_FALSE(default_config.log_level.empty());
}

TEST_F(KolosalServerLauncherTest, CreateDefaultServerConfigWithWorkspace) {
    std::string workspace_path = "/test/workspace";
    auto config_with_workspace = create_default_server_config(workspace_path);
    
    EXPECT_FALSE(config_with_workspace.executable_path.empty());
    // Working directory might be set based on workspace
}

TEST_F(KolosalServerLauncherTest, ConfigurationEdgeCases) {
    KolosalServerLauncher::ServerConfig edge_config;
    
    // Test with edge case values
    edge_config.executable_path = "";
    edge_config.host = "";
    edge_config.port = 0;
    edge_config.timeout = 0;
    
    auto edge_launcher = std::make_unique<KolosalServerLauncher>(edge_config);
    EXPECT_NE(edge_launcher, nullptr);
    
    // Should handle edge cases gracefully
    bool started = edge_launcher->start();
    EXPECT_FALSE(started);
}

TEST_F(KolosalServerLauncherTest, PortRangeValidation) {
    // Test with different port ranges
    std::vector<int> test_ports = {80, 443, 8080, 8081, 8082, 9000, 65535};
    
    for (int port : test_ports) {
        KolosalServerLauncher::ServerConfig port_config = config;
        port_config.port = port;
        
        auto port_launcher = std::make_unique<KolosalServerLauncher>(port_config);
        EXPECT_EQ(port_launcher->get_config().port, port);
        
        std::string expected_url = "http://127.0.0.1:" + std::to_string(port);
        EXPECT_EQ(port_launcher->get_server_url(), expected_url);
    }
}

TEST_F(KolosalServerLauncherTest, HostVariations) {
    std::vector<std::string> test_hosts = {
        "localhost",
        "0.0.0.0",
        "127.0.0.1",
        "192.168.1.100"
    };
    
    for (const auto& host : test_hosts) {
        KolosalServerLauncher::ServerConfig host_config = config;
        host_config.host = host;
        
        auto host_launcher = std::make_unique<KolosalServerLauncher>(host_config);
        EXPECT_EQ(host_launcher->get_config().host, host);
        
        std::string expected_url = "http://" + host + ":" + std::to_string(config.port);
        EXPECT_EQ(host_launcher->get_server_url(), expected_url);
    }
}

TEST_F(KolosalServerLauncherTest, LogLevelConfiguration) {
    std::vector<std::string> log_levels = {"ERROR", "WARN", "INFO", "DEBUG"};
    
    for (const auto& level : log_levels) {
        KolosalServerLauncher::ServerConfig level_config = config;
        level_config.log_level = level;
        
        auto level_launcher = std::make_unique<KolosalServerLauncher>(level_config);
        EXPECT_EQ(level_launcher->get_config().log_level, level);
    }
}

TEST_F(KolosalServerLauncherTest, AccessModeConfiguration) {
    // Test public access mode
    KolosalServerLauncher::ServerConfig public_config = config;
    public_config.public_access = true;
    public_config.internet_access = true;
    
    auto public_launcher = std::make_unique<KolosalServerLauncher>(public_config);
    EXPECT_TRUE(public_launcher->get_config().public_access);
    EXPECT_TRUE(public_launcher->get_config().internet_access);
    
    // Test private access mode
    KolosalServerLauncher::ServerConfig private_config = config;
    private_config.public_access = false;
    private_config.internet_access = false;
    
    auto private_launcher = std::make_unique<KolosalServerLauncher>(private_config);
    EXPECT_FALSE(private_launcher->get_config().public_access);
    EXPECT_FALSE(private_launcher->get_config().internet_access);
}

TEST_F(KolosalServerLauncherTest, TimeoutConfiguration) {
    // Test various timeout values
    std::vector<int> timeouts = {1, 5, 10, 30, 60, 120};
    
    for (int timeout : timeouts) {
        KolosalServerLauncher::ServerConfig timeout_config = config;
        timeout_config.timeout = timeout;
        
        auto timeout_launcher = std::make_unique<KolosalServerLauncher>(timeout_config);
        EXPECT_EQ(timeout_launcher->get_config().timeout, timeout);
    }
}

TEST_F(KolosalServerLauncherTest, WorkingDirectoryConfiguration) {
    std::string test_dir = std::filesystem::current_path().string();
    
    KolosalServerLauncher::ServerConfig dir_config = config;
    dir_config.working_directory = test_dir;
    
    auto dir_launcher = std::make_unique<KolosalServerLauncher>(dir_config);
    EXPECT_EQ(dir_launcher->get_config().working_directory, test_dir);
}

TEST_F(KolosalServerLauncherTest, ConfigFileConfiguration) {
    std::string config_file_path = "test_server_config.yaml";
    
    KolosalServerLauncher::ServerConfig file_config = config;
    file_config.config_file = config_file_path;
    
    auto file_launcher = std::make_unique<KolosalServerLauncher>(file_config);
    EXPECT_EQ(file_launcher->get_config().config_file, config_file_path);
}

TEST_F(KolosalServerLauncherTest, QuietModeConfiguration) {
    KolosalServerLauncher::ServerConfig quiet_config = config;
    quiet_config.quiet_mode = true;
    
    auto quiet_launcher = std::make_unique<KolosalServerLauncher>(quiet_config);
    EXPECT_TRUE(quiet_launcher->get_config().quiet_mode);
    
    KolosalServerLauncher::ServerConfig verbose_config = config;
    verbose_config.quiet_mode = false;
    
    auto verbose_launcher = std::make_unique<KolosalServerLauncher>(verbose_config);
    EXPECT_FALSE(verbose_launcher->get_config().quiet_mode);
}
