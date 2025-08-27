/**
 * @file minimal_test_demo.cpp
 * @brief Minimal test demonstration without external dependencies
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 */

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <algorithm>

// Simple test framework
class SimpleTest {
private:
    static int total_tests;
    static int passed_tests;
    static int failed_tests;
    
public:
    static void assert_true(bool condition, const std::string& message) {
        total_tests++;
        if (condition) {
            passed_tests++;
            std::cout << "[PASS] " << message << std::endl;
        } else {
            failed_tests++;
            std::cout << "[FAIL] " << message << std::endl;
        }
    }
    
    static void assert_equals(size_t expected, size_t actual, const std::string& message) {
        assert_true(expected == actual, message + " (expected: " + std::to_string(expected) + ", got: " + std::to_string(actual) + ")");
    }
    
    static void assert_equals(const std::string& expected, const std::string& actual, const std::string& message) {
        assert_true(expected == actual, message + " (expected: '" + expected + "', got: '" + actual + "')");
    }
    
    static void print_summary() {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "TEST SUMMARY" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        std::cout << "Total Tests: " << total_tests << std::endl;
        std::cout << "Passed: " << passed_tests << std::endl;
        std::cout << "Failed: " << failed_tests << std::endl;
        std::cout << "Success Rate: " << (total_tests > 0 ? (passed_tests * 100 / total_tests) : 0) << "%" << std::endl;
        
        if (failed_tests == 0) {
            std::cout << "All tests passed!" << std::endl;
        } else {
            std::cout << "Some tests failed." << std::endl;
        }
    }
    
    static bool all_passed() {
        return failed_tests == 0;
    }
};

int SimpleTest::total_tests = 0;
int SimpleTest::passed_tests = 0;
int SimpleTest::failed_tests = 0;

// Mock classes for demonstration
class MockAgent {
private:
    std::string id_;
    std::string name_;
    std::vector<std::string> capabilities_;
    bool running_ = false;
    
public:
    MockAgent(const std::string& name) : name_(name) {
        // Generate simple ID
        id_ = "agent_" + std::to_string(std::hash<std::string>{}(name) % 10000);
    }
    
    const std::string& get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    const std::vector<std::string>& get_capabilities() const { return capabilities_; }
    
    void add_capability(const std::string& capability) {
        capabilities_.push_back(capability);
    }
    
    bool start() {
        if (running_) return false;
        running_ = true;
        return true;
    }
    
    void stop() {
        running_ = false;
    }
    
    bool is_running() const { return running_; }
    
    std::string execute_task(const std::string& task) {
        if (!running_) return "ERROR: Agent not running";
        return "Completed: " + task;
    }
};

class MockAgentManager {
private:
    std::vector<std::shared_ptr<MockAgent>> agents_;
    
public:
    std::shared_ptr<MockAgent> create_agent(const std::string& name) {
        auto agent = std::make_shared<MockAgent>(name);
        agents_.push_back(agent);
        return agent;
    }
    
    size_t get_agent_count() const {
        return agents_.size();
    }
    
    std::shared_ptr<MockAgent> get_agent(const std::string& id) {
        for (auto& agent : agents_) {
            if (agent->get_id() == id) {
                return agent;
            }
        }
        return nullptr;
    }
    
    bool remove_agent(const std::string& id) {
        auto it = std::remove_if(agents_.begin(), agents_.end(),
            [&id](const std::shared_ptr<MockAgent>& agent) {
                return agent->get_id() == id;
            });
        
        if (it != agents_.end()) {
            agents_.erase(it, agents_.end());
            return true;
        }
        return false;
    }
    
    void stop_all_agents() {
        for (auto& agent : agents_) {
            agent->stop();
        }
    }
};

// Test functions
void test_agent_creation() {
    std::cout << "\n--- Testing Agent Creation ---" << std::endl;
    
    MockAgent agent("TestAgent");
    
    SimpleTest::assert_true(!agent.get_id().empty(), "Agent should have an ID");
    SimpleTest::assert_equals("TestAgent", agent.get_name(), "Agent name should be set correctly");
    SimpleTest::assert_true(!agent.is_running(), "Agent should not be running initially");
    SimpleTest::assert_true(agent.get_capabilities().empty(), "Agent should have no capabilities initially");
}

void test_agent_lifecycle() {
    std::cout << "\n--- Testing Agent Lifecycle ---" << std::endl;
    
    MockAgent agent("LifecycleAgent");
    
    // Test starting
    SimpleTest::assert_true(agent.start(), "Agent should start successfully");
    SimpleTest::assert_true(agent.is_running(), "Agent should be running after start");
    
    // Test double start (should fail)
    SimpleTest::assert_true(!agent.start(), "Agent should not start when already running");
    
    // Test stopping
    agent.stop();
    SimpleTest::assert_true(!agent.is_running(), "Agent should not be running after stop");
}

void test_agent_capabilities() {
    std::cout << "\n--- Testing Agent Capabilities ---" << std::endl;
    
    MockAgent agent("CapabilityAgent");
    
    agent.add_capability("text_processing");
    agent.add_capability("data_analysis");
    
    SimpleTest::assert_equals(agent.get_capabilities().size(), size_t(2), "Agent should have 2 capabilities");
    SimpleTest::assert_equals("text_processing", agent.get_capabilities()[0], "First capability should be text_processing");
    SimpleTest::assert_equals("data_analysis", agent.get_capabilities()[1], "Second capability should be data_analysis");
}

void test_agent_execution() {
    std::cout << "\n--- Testing Agent Execution ---" << std::endl;
    
    MockAgent agent("ExecutionAgent");
    
    // Test execution without running
    std::string result = agent.execute_task("test_task");
    SimpleTest::assert_true(result.find("ERROR") != std::string::npos, "Execution should fail when agent not running");
    
    // Test execution while running
    agent.start();
    result = agent.execute_task("test_task");
    SimpleTest::assert_true(result.find("Completed") != std::string::npos, "Execution should succeed when agent running");
}

void test_agent_manager() {
    std::cout << "\n--- Testing Agent Manager ---" << std::endl;
    
    MockAgentManager manager;
    
    // Test initial state
    SimpleTest::assert_equals(manager.get_agent_count(), size_t(0), "Manager should start with no agents");
    
    // Test creating agents
    auto agent1 = manager.create_agent("Agent1");
    auto agent2 = manager.create_agent("Agent2");
    
    SimpleTest::assert_true(agent1 != nullptr, "Agent1 should be created successfully");
    SimpleTest::assert_true(agent2 != nullptr, "Agent2 should be created successfully");
    SimpleTest::assert_equals(manager.get_agent_count(), size_t(2), "Manager should have 2 agents");
    
    // Test getting agents by ID
    auto found_agent = manager.get_agent(agent1->get_id());
    SimpleTest::assert_true(found_agent != nullptr, "Should find agent by ID");
    SimpleTest::assert_equals(agent1->get_id(), found_agent->get_id(), "Found agent should have correct ID");
    
    // Test removing agents
    bool removed = manager.remove_agent(agent1->get_id());
    SimpleTest::assert_true(removed, "Agent should be removed successfully");
    SimpleTest::assert_equals(manager.get_agent_count(), size_t(1), "Manager should have 1 agent after removal");
    
    // Test stop all agents
    agent2->start();
    SimpleTest::assert_true(agent2->is_running(), "Agent2 should be running");
    manager.stop_all_agents();
    SimpleTest::assert_true(!agent2->is_running(), "Agent2 should be stopped after stop_all_agents");
}

void test_performance_metrics() {
    std::cout << "\n--- Testing Performance Metrics ---" << std::endl;
    
    auto start_time = std::chrono::steady_clock::now();
    
    MockAgentManager manager;
    
    // Create multiple agents
    const int num_agents = 100;
    for (int i = 0; i < num_agents; ++i) {
        manager.create_agent("Agent" + std::to_string(i));
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    SimpleTest::assert_equals(manager.get_agent_count(), size_t(num_agents), "Should create all agents");
    SimpleTest::assert_true(duration.count() < 1000, "Agent creation should be fast (< 1 second)");
    
    std::cout << "Created " << num_agents << " agents in " << duration.count() << "ms" << std::endl;
}

void test_error_handling() {
    std::cout << "\n--- Testing Error Handling ---" << std::endl;
    
    MockAgentManager manager;
    
    // Test getting non-existent agent
    auto agent = manager.get_agent("non_existent_id");
    SimpleTest::assert_true(agent == nullptr, "Should return nullptr for non-existent agent");
    
    // Test removing non-existent agent
    bool removed = manager.remove_agent("non_existent_id");
    SimpleTest::assert_true(!removed, "Should return false when removing non-existent agent");
}

int main() {
    std::cout << "Kolosal Agent System - Test Demonstration" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // Run all tests
    test_agent_creation();
    test_agent_lifecycle();
    test_agent_capabilities();
    test_agent_execution();
    test_agent_manager();
    test_performance_metrics();
    test_error_handling();
    
    // Print final summary
    SimpleTest::print_summary();
    
    return SimpleTest::all_passed() ? 0 : 1;
}
