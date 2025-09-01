#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include "logger.hpp"
#include "agent_config.hpp"
#include "agent_manager.hpp"
#include "workflow_manager.hpp"

using namespace KolosalAgent;

class TestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        // Configure extensive debug logging
        auto& logger = Logger::instance();
        logger.set_level(LogLevel::DEBUG_LVL);
        logger.set_console_output(true);
        logger.set_file_output("kolosal_agent_test_debug.log");
        logger.enable_timestamps(true);
        logger.enable_thread_id(true);
        logger.enable_function_tracing(true);
        
        LOG_INFO("=== Kolosal Agent Test Suite Starting ===");
        LOG_INFO("Test Environment Setup Complete");
        LOG_DEBUG("Debug logging enabled for comprehensive testing");
        
        // Initialize system components
        try {
            LOG_INFO("Initializing test environment components...");
            
            // Test configuration loading
            LOG_DEBUG("Testing configuration system...");
            
            // Test basic agent manager
            LOG_DEBUG("Testing agent manager initialization...");
            
            LOG_INFO("Test environment initialization complete");
        } catch (const std::exception& e) {
            LOG_ERROR_F("Failed to initialize test environment: %s", e.what());
            throw;
        }
    }
    
    void TearDown() override {
        LOG_INFO("=== Kolosal Agent Test Suite Completed ===");
        LOG_INFO("Test Environment Cleanup Complete");
    }
};

// Test statistics structure
struct TestStats {
    std::string test_case;
    std::string test_name;
    bool passed;
    long long duration_ms;
    std::string failure_message;
};

// Custom test listener for detailed logging and statistics collection
class DetailedTestListener : public ::testing::EmptyTestEventListener {
public:
    DetailedTestListener() : total_duration_ms_(0) {}
    
    void OnTestStart(const ::testing::TestInfo& test_info) override {
        LOG_INFO_F("Starting Test: %s.%s", test_info.test_case_name(), test_info.name());
        test_start_time_ = std::chrono::high_resolution_clock::now();
        suite_start_time_ = std::chrono::high_resolution_clock::now();
    }
    
    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - test_start_time_);
        total_duration_ms_ += duration.count();
        
        TestStats stats;
        stats.test_case = test_info.test_case_name();
        stats.test_name = test_info.name();
        stats.passed = test_info.result()->Passed();
        stats.duration_ms = duration.count();
        
        if (test_info.result()->Passed()) {
            LOG_INFO_F("Test PASSED: %s.%s (Duration: %lldms)", 
                    test_info.test_case_name(), test_info.name(), duration.count());
            passed_tests_.push_back(stats);
        } else {
            // Get failure message if available
            if (test_info.result()->total_part_count() > 0) {
                const auto& part = test_info.result()->GetTestPartResult(0);
                stats.failure_message = part.message();
            }
            
            LOG_ERROR_F("Test FAILED: %s.%s (Duration: %lldms)", 
                     test_info.test_case_name(), test_info.name(), duration.count());
            failed_tests_.push_back(stats);
        }
        all_tests_.push_back(stats);
    }
    
    void OnTestCaseStart(const ::testing::TestCase& test_case) override {
        LOG_INFO_F("=== Starting Test Case: %s ===", test_case.name());
        case_start_times_[test_case.name()] = std::chrono::high_resolution_clock::now();
    }
    
    void OnTestCaseEnd(const ::testing::TestCase& test_case) override {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto it = case_start_times_.find(test_case.name());
        if (it != case_start_times_.end()) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - it->second);
            case_durations_[test_case.name()] = duration.count();
        }
        
        LOG_INFO_F("=== Completed Test Case: %s (Tests: %d, Failures: %d) ===", 
                test_case.name(), test_case.total_test_count(), test_case.failed_test_count());
    }
    
    void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
        PrintTestSummary(unit_test);
    }
    
    void PrintTestSummary(const ::testing::UnitTest& unit_test) {
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "                    KOLOSAL AGENT TEST SUMMARY" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        // Overall statistics
        int total_tests = unit_test.total_test_count();
        int passed_tests = unit_test.successful_test_count();
        int failed_tests = unit_test.failed_test_count();
        int disabled_tests = unit_test.disabled_test_count();
        int skipped_tests = unit_test.skipped_test_count();
        
        std::cout << "Overall Results:" << std::endl;
        std::cout << "  Total Tests:    " << total_tests << std::endl;
        std::cout << "  Passed:         " << passed_tests << " (" << 
                     (total_tests > 0 ? (passed_tests * 100 / total_tests) : 0) << "%)" << std::endl;
        std::cout << "  Failed:         " << failed_tests << " (" << 
                     (total_tests > 0 ? (failed_tests * 100 / total_tests) : 0) << "%)" << std::endl;
        std::cout << "  Disabled:       " << disabled_tests << std::endl;
        std::cout << "  Skipped:        " << skipped_tests << std::endl;
        std::cout << "  Total Duration: " << total_duration_ms_ << "ms (" << 
                     (total_duration_ms_ / 1000.0) << "s)" << std::endl;
        
        // Test case breakdown
        std::cout << "\nTest Case Breakdown:" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        
        std::map<std::string, std::pair<int, int>> case_stats; // passed, failed
        for (const auto& test : all_tests_) {
            if (case_stats.find(test.test_case) == case_stats.end()) {
                case_stats[test.test_case] = {0, 0};
            }
            if (test.passed) {
                case_stats[test.test_case].first++;
            } else {
                case_stats[test.test_case].second++;
            }
        }
        
        for (const auto& case_stat : case_stats) {
            int passed = case_stat.second.first;
            int failed = case_stat.second.second;
            int total = passed + failed;
            auto duration_it = case_durations_.find(case_stat.first);
            long long duration = (duration_it != case_durations_.end()) ? duration_it->second : 0;
            
            std::cout << "  " << std::setw(25) << std::left << case_stat.first 
                      << " | P:" << std::setw(3) << passed 
                      << " F:" << std::setw(3) << failed 
                      << " T:" << std::setw(3) << total
                      << " | " << std::setw(6) << duration << "ms" << std::endl;
        }
        
        // Failed tests details
        if (!failed_tests_.empty()) {
            std::cout << "\nFailed Tests Details:" << std::endl;
            std::cout << std::string(60, '-') << std::endl;
            for (size_t i = 0; i < failed_tests_.size() && i < 10; ++i) {
                const auto& test = failed_tests_[i];
                std::cout << "  " << (i + 1) << ". " << test.test_case << "." << test.test_name 
                          << " (" << test.duration_ms << "ms)" << std::endl;
                if (!test.failure_message.empty()) {
                    std::string msg = test.failure_message;
                    if (msg.length() > 100) msg = msg.substr(0, 100) + "...";
                    std::cout << "     Error: " << msg << std::endl;
                }
            }
            if (failed_tests_.size() > 10) {
                std::cout << "  ... and " << (failed_tests_.size() - 10) << " more failed tests." << std::endl;
            }
        }
        
        // Performance analysis
        std::cout << "\nPerformance Analysis:" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        
        if (!all_tests_.empty()) {
            // Find slowest tests
            std::vector<TestStats> sorted_tests = all_tests_;
            std::sort(sorted_tests.begin(), sorted_tests.end(), 
                     [](const TestStats& a, const TestStats& b) {
                         return a.duration_ms > b.duration_ms;
                     });
            
            std::cout << "  Slowest Tests (Top 5):" << std::endl;
            for (size_t i = 0; i < std::min(size_t(5), sorted_tests.size()); ++i) {
                const auto& test = sorted_tests[i];
                std::cout << "    " << (i + 1) << ". " << test.test_case << "." << test.test_name 
                          << " - " << test.duration_ms << "ms" << std::endl;
            }
            
            // Calculate average duration
            long long avg_duration = total_duration_ms_ / all_tests_.size();
            std::cout << "  Average Test Duration: " << avg_duration << "ms" << std::endl;
            
            // Find tests above average
            int above_avg_count = 0;
            for (const auto& test : all_tests_) {
                if (test.duration_ms > avg_duration * 2) {
                    above_avg_count++;
                }
            }
            std::cout << "  Tests >2x Average:     " << above_avg_count << std::endl;
        }
        
        // Final verdict
        std::cout << "\n" << std::string(80, '=') << std::endl;
        if (failed_tests == 0) {
            std::cout << "🎉 ALL TESTS PASSED! Kolosal Agent is ready for deployment." << std::endl;
        } else {
            std::cout << "❌ " << failed_tests << " TEST(S) FAILED. Please review and fix issues." << std::endl;
        }
        std::cout << std::string(80, '=') << std::endl;
        
        // Log summary to file as well
        LOG_INFO_F("TEST SUMMARY: Total=%d, Passed=%d, Failed=%d, Duration=%lldms", 
                  total_tests, passed_tests, failed_tests, total_duration_ms_);
    }

private:
    std::chrono::high_resolution_clock::time_point test_start_time_;
    std::chrono::high_resolution_clock::time_point suite_start_time_;
    std::map<std::string, std::chrono::high_resolution_clock::time_point> case_start_times_;
    std::map<std::string, long long> case_durations_;
    std::vector<TestStats> all_tests_;
    std::vector<TestStats> passed_tests_;
    std::vector<TestStats> failed_tests_;
    long long total_duration_ms_;
};

int main(int argc, char **argv) {
    std::cout << "Kolosal Agent Test Executable (kolosal-agent-test.exe)" << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << "Build Configuration: Debug with Extensive Logging" << std::endl;
    std::cout << "Test Framework: Google Test" << std::endl;
    std::cout << "======================================================" << std::endl;
    
    ::testing::InitGoogleTest(&argc, argv);
    
    // Add custom environment
    ::testing::AddGlobalTestEnvironment(new TestEnvironment);
    
    // Add detailed test listener
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new DetailedTestListener);
    
    // Configure test output
    ::testing::GTEST_FLAG(print_time) = true;
    ::testing::GTEST_FLAG(color) = "yes";
    
    std::cout << "Starting comprehensive test suite..." << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    std::cout << "======================================================" << std::endl;
    std::cout << "Test execution completed with result: " << result << std::endl;
    std::cout << "Check kolosal_agent_test_debug.log for detailed logs" << std::endl;
    std::cout << "======================================================" << std::endl;
    
    return result;
}
