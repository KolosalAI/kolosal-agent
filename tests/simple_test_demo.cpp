/**
 * @file simple_test_demo.cpp
 * @brief Simple demonstration test without external dependencies
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 */

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <chrono>
#include <memory>

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
            std::cout << "✓ PASS: " << message << std::endl;
        } else {
            failed_tests++;
            std::cout << "✗ FAIL: " << message << std::endl;
        }
    }
    
    static void assert_equals(const std::string& expected, const std::string& actual, const std::string& message) {
        assert_true(expected == actual, message + " (expected: '" + expected + "', got: '" + actual + "')");
    }
    
    static void run_test(const std::string& test_name, std::function<void()> test_func) {
        std::cout << "\n--- Running: " << test_name << " ---" << std::endl;
        auto start = std::chrono::steady_clock::now();
        
        try {
            test_func();
        } catch (const std::exception& e) {
            failed_tests++;
            std::cout << "✗ EXCEPTION: " << test_name << " - " << e.what() << std::endl;
        }
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Duration: " << duration.count() << "ms" << std::endl;
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
            std::cout << "🎉 All tests passed!" << std::endl;
        } else {
            std::cout << "❌ Some tests failed." << std::endl;
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
    bool is_running() const { return running_; }
    
    void add_capability(const std::string& capability) {
        capabilities_.push_back(capability);
    }
    
    bool start() {
        running_ = true;
        return true;
    }
    
    void stop() {
        running_ = false;
    }
    
    std::string execute_function(const std::string& function_name, const std::string& params) {
        if (function_name == "echo") {
            return "Echo: " + params;
        } else if (function_name == "chat") {
            return "Chat response to: " + params;
        } else if (function_name == "analyze") {
            return "Analysis of: " + params;
        } else {
            throw std::runtime_error("Unknown function: " + function_name);
        }
    }
};

class MockAgentManager {
private:
    std::vector<std::unique_ptr<MockAgent>> agents_;
    
public:
    std::string create_agent(const std::string& name, const std::vector<std::string>& capabilities) {
        auto agent = std::make_unique<MockAgent>(name);
        std::string agent_id = agent->get_id();
        
        for (const auto& capability : capabilities) {
            agent->add_capability(capability);
        }
        
        agents_.push_back(std::move(agent));
        return agent_id;
    }
    
    MockAgent* get_agent(const std::string& agent_id) {
        for (const auto& agent : agents_) {
            if (agent->get_id() == agent_id) {
                return agent.get();
            }
        }
        return nullptr;
    }
    
    bool start_agent(const std::string& agent_id) {
        auto agent = get_agent(agent_id);
        return agent ? agent->start() : false;
    }
    
    void stop_agent(const std::string& agent_id) {
        auto agent = get_agent(agent_id);
        if (agent) {
            agent->stop();
        }
    }
    
    bool delete_agent(const std::string& agent_id) {
        auto it = std::find_if(agents_.begin(), agents_.end(),
            [&agent_id](const std::unique_ptr<MockAgent>& agent) {
                return agent->get_id() == agent_id;
            });
        
        if (it != agents_.end()) {
            agents_.erase(it);
            return true;
        }
        return false;
    }
    
    size_t get_agent_count() const {
        return agents_.size();
    }
    
    size_t get_running_count() const {
        size_t count = 0;
        for (const auto& agent : agents_) {
            if (agent->is_running()) {
                count++;
            }
        }
        return count;
    }
};

// Test functions
void test_agent_creation() {
    MockAgentManager manager;
    
    // Test basic agent creation
    std::string agent_id = manager.create_agent("TestAgent", {"chat", "analysis"});
    SimpleTest::assert_true(!agent_id.empty(), "Agent ID should not be empty");
    
    auto agent = manager.get_agent(agent_id);
    SimpleTest::assert_true(agent != nullptr, "Agent should be retrievable");
    SimpleTest::assert_equals("TestAgent", agent->get_name(), "Agent name should match");
    SimpleTest::assert_true(agent->get_capabilities().size() == 2, "Agent should have 2 capabilities");
    SimpleTest::assert_true(!agent->is_running(), "Agent should not be running initially");
}

void test_agent_lifecycle() {
    MockAgentManager manager;
    
    // Create and start agent
    std::string agent_id = manager.create_agent("LifecycleAgent", {"chat"});
    auto agent = manager.get_agent(agent_id);
    
    SimpleTest::assert_true(manager.start_agent(agent_id), "Agent should start successfully");
    SimpleTest::assert_true(agent->is_running(), "Agent should be running after start");
    
    // Stop agent
    manager.stop_agent(agent_id);
    SimpleTest::assert_true(!agent->is_running(), "Agent should not be running after stop");
    
    // Delete agent
    SimpleTest::assert_true(manager.delete_agent(agent_id), "Agent should be deleted successfully");
    SimpleTest::assert_true(manager.get_agent(agent_id) == nullptr, "Deleted agent should not be retrievable");
}

void test_function_execution() {
    MockAgent agent("FunctionTestAgent");
    agent.add_capability("chat");
    agent.add_capability("analysis");
    agent.start();
    
    // Test echo function
    std::string result = agent.execute_function("echo", "test data");
    SimpleTest::assert_equals("Echo: test data", result, "Echo function should return prefixed input");
    
    // Test chat function
    result = agent.execute_function("chat", "Hello");
    SimpleTest::assert_equals("Chat response to: Hello", result, "Chat function should return response");
    
    // Test analyze function
    result = agent.execute_function("analyze", "sample text");
    SimpleTest::assert_equals("Analysis of: sample text", result, "Analyze function should return analysis");
    
    // Test invalid function
    bool exception_thrown = false;
    try {
        agent.execute_function("invalid_function", "test");
    } catch (const std::exception& e) {
        exception_thrown = true;
        SimpleTest::assert_true(std::string(e.what()).find("Unknown function") != std::string::npos, 
                               "Exception should mention unknown function");
    }
    SimpleTest::assert_true(exception_thrown, "Invalid function should throw exception");
}

void test_multiple_agents() {
    MockAgentManager manager;
    
    // Create multiple agents
    std::vector<std::string> agent_ids;
    for (int i = 0; i < 5; ++i) {
        std::string agent_id = manager.create_agent("Agent" + std::to_string(i), {"chat"});
        agent_ids.push_back(agent_id);
        manager.start_agent(agent_id);
    }
    
    SimpleTest::assert_true(manager.get_agent_count() == 5, "Should have 5 agents");
    SimpleTest::assert_true(manager.get_running_count() == 5, "All 5 agents should be running");
    
    // Stop some agents
    for (size_t i = 0; i < 3; ++i) {
        manager.stop_agent(agent_ids[i]);
    }
    
    SimpleTest::assert_true(manager.get_running_count() == 2, "Should have 2 running agents");
    
    // Delete all agents
    for (const auto& agent_id : agent_ids) {
        manager.delete_agent(agent_id);
    }
    
    SimpleTest::assert_true(manager.get_agent_count() == 0, "Should have no agents after deletion");
}

void test_agent_capabilities() {
    MockAgent agent("CapabilityTestAgent");
    
    SimpleTest::assert_true(agent.get_capabilities().empty(), "Agent should have no capabilities initially");
    
    agent.add_capability("chat");
    agent.add_capability("analysis");
    agent.add_capability("research");
    
    const auto& capabilities = agent.get_capabilities();
    SimpleTest::assert_true(capabilities.size() == 3, "Agent should have 3 capabilities");
    SimpleTest::assert_equals("chat", capabilities[0], "First capability should be chat");
    SimpleTest::assert_equals("analysis", capabilities[1], "Second capability should be analysis");
    SimpleTest::assert_equals("research", capabilities[2], "Third capability should be research");
}

void test_error_handling() {
    MockAgentManager manager;
    
    // Test invalid agent operations
    SimpleTest::assert_true(!manager.start_agent("invalid_id"), "Starting invalid agent should fail");
    SimpleTest::assert_true(manager.get_agent("invalid_id") == nullptr, "Getting invalid agent should return null");
    SimpleTest::assert_true(!manager.delete_agent("invalid_id"), "Deleting invalid agent should fail");
    
    // Create agent and test double deletion
    std::string agent_id = manager.create_agent("ErrorTestAgent", {"chat"});
    SimpleTest::assert_true(manager.delete_agent(agent_id), "First deletion should succeed");
    SimpleTest::assert_true(!manager.delete_agent(agent_id), "Second deletion should fail");
}

void test_performance_basic() {
    MockAgentManager manager;
    
    auto start = std::chrono::steady_clock::now();
    
    // Create many agents
    const int num_agents = 1000;
    std::vector<std::string> agent_ids;
    
    for (int i = 0; i < num_agents; ++i) {
        std::string agent_id = manager.create_agent("PerfAgent" + std::to_string(i), {"chat"});
        agent_ids.push_back(agent_id);
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    SimpleTest::assert_true(manager.get_agent_count() == num_agents, "Should create all agents");
    SimpleTest::assert_true(duration.count() < 5000, "Agent creation should be reasonably fast");
    
    std::cout << "Created " << num_agents << " agents in " << duration.count() << "ms" << std::endl;
    
    // Cleanup
    for (const auto& agent_id : agent_ids) {
        manager.delete_agent(agent_id);
    }
}

int main() {
    std::cout << "Kolosal Agent System - Test Demonstration" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    // Run all tests
    SimpleTest::run_test("Agent Creation", test_agent_creation);
    SimpleTest::run_test("Agent Lifecycle", test_agent_lifecycle);
    SimpleTest::run_test("Function Execution", test_function_execution);
    SimpleTest::run_test("Multiple Agents", test_multiple_agents);
    SimpleTest::run_test("Agent Capabilities", test_agent_capabilities);
    SimpleTest::run_test("Error Handling", test_error_handling);
    SimpleTest::run_test("Basic Performance", test_performance_basic);
    
    // Print summary
    SimpleTest::print_summary();
    
    return SimpleTest::all_passed() ? 0 : 1;
}
