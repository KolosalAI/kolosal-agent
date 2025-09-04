# Testing Guide

Comprehensive guide for testing the Kolosal Agent System v2.0.

## 🧪 Testing Overview

The Kolosal Agent System includes a comprehensive testing framework with multiple test categories:

- **Unit Tests**: Test individual components in isolation
- **Integration Tests**: Test system interactions and API endpoints
- **Performance Tests**: Validate system performance and scalability
- **End-to-End Tests**: Test complete workflows and user scenarios

## 🚀 Quick Testing

### Run All Tests
```bash
# Build with tests enabled
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build . --config Debug

# Run all tests
ctest --output-on-failure

# Run tests in parallel
ctest -j$(nproc) --output-on-failure
```

### Test Categories
```bash
# Run specific test categories
ctest -L unit                # Unit tests only
ctest -L integration         # Integration tests only
ctest -L performance         # Performance tests only

# Run specific test
ctest -R AgentTest
ctest -R HttpServerTest
```

## 🏗️ Test Structure

### Directory Layout
```
tests/
├── unit/                    # Unit tests
│   ├── test_agent.cpp
│   ├── test_agent_manager.cpp
│   ├── test_config.cpp
│   └── ...
├── integration/             # Integration tests
│   ├── test_api_endpoints.cpp
│   ├── test_agent_workflow.cpp
│   └── ...
├── performance/             # Performance tests
│   ├── test_load.cpp
│   ├── test_memory.cpp
│   └── ...
├── fixtures/                # Test data and fixtures
│   ├── sample_configs/
│   ├── test_data/
│   └── ...
└── CMakeLists.txt          # Test configuration
```

### Test Framework

The project uses **Google Test (gtest)** and **Google Mock (gmock)** for testing:

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Test fixture for Agent tests
class AgentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code for each test
    }
    
    void TearDown() override {
        // Cleanup code for each test
    }
    
    // Test data and helper methods
};

// Test case
TEST_F(AgentTest, ShouldInitializeCorrectly) {
    // Test implementation
}
```

## 🔧 Unit Tests

### Agent Core Tests

#### Test Agent Creation and Initialization
```cpp
// tests/unit/test_agent.cpp
#include <gtest/gtest.h>
#include "agent.hpp"

class AgentTest : public ::testing::Test {
protected:
    void SetUp() override {
        agent_ = std::make_unique<Agent>("test_agent");
    }
    
    std::unique_ptr<Agent> agent_;
};

TEST_F(AgentTest, ShouldInitializeWithName) {
    EXPECT_EQ(agent_->get_name(), "test_agent");
    EXPECT_FALSE(agent_->get_id().empty());
    EXPECT_TRUE(agent_->get_capabilities().empty());
}

TEST_F(AgentTest, ShouldAddCapabilities) {
    agent_->add_capability("chat");
    agent_->add_capability("analysis");
    
    auto capabilities = agent_->get_capabilities();
    EXPECT_EQ(capabilities.size(), 2);
    EXPECT_TRUE(agent_->has_capability("chat"));
    EXPECT_TRUE(agent_->has_capability("analysis"));
    EXPECT_FALSE(agent_->has_capability("unknown"));
}

TEST_F(AgentTest, ShouldExecuteFunction) {
    agent_->add_capability("status");
    
    AgentData input;
    auto result = agent_->execute_function("status", input);
    
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.execution_time_ms, 0);
}

TEST_F(AgentTest, ShouldFailOnUnknownFunction) {
    AgentData input;
    auto result = agent_->execute_function("unknown_function", input);
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}
```

#### Test Agent Manager
```cpp
// tests/unit/test_agent_manager.cpp
#include <gtest/gtest.h>
#include "agent_manager.hpp"
#include "agent_config.hpp"

class AgentManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager_ = std::make_unique<AgentManager>();
    }
    
    std::unique_ptr<AgentManager> manager_;
};

TEST_F(AgentManagerTest, ShouldCreateAgent) {
    std::string agent_id = manager_->create_agent("test_agent", {"chat"});
    
    EXPECT_FALSE(agent_id.empty());
    EXPECT_TRUE(manager_->agent_exists(agent_id));
    
    auto agent_info = manager_->get_agent_info(agent_id);
    EXPECT_EQ(agent_info.name, "test_agent");
    EXPECT_EQ(agent_info.capabilities.size(), 1);
    EXPECT_EQ(agent_info.capabilities[0], "chat");
}

TEST_F(AgentManagerTest, ShouldDeleteAgent) {
    std::string agent_id = manager_->create_agent("temp_agent");
    EXPECT_TRUE(manager_->agent_exists(agent_id));
    
    bool deleted = manager_->delete_agent(agent_id);
    EXPECT_TRUE(deleted);
    EXPECT_FALSE(manager_->agent_exists(agent_id));
}

TEST_F(AgentManagerTest, ShouldLoadConfiguration) {
    // Create test configuration file
    std::string test_config = "tests/fixtures/test_agent.yaml";
    
    bool loaded = manager_->load_configuration(test_config);
    EXPECT_TRUE(loaded);
    
    // Verify agents were created from config
    auto agents = manager_->get_all_agents();
    EXPECT_GT(agents.size(), 0);
}
```

### Configuration Tests

#### Test Configuration Loading
```cpp
// tests/unit/test_config.cpp
#include <gtest/gtest.h>
#include "agent_config.hpp"
#include <fstream>

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = std::make_unique<AgentConfigManager>();
        
        // Create temporary test config file
        test_config_file_ = "test_config.yaml";
        create_test_config();
    }
    
    void TearDown() override {
        std::remove(test_config_file_.c_str());
    }
    
    void create_test_config() {
        std::ofstream file(test_config_file_);
        file << R"(
system:
  name: "Test System"
  port: 8080

agents:
  - name: "TestAgent"
    capabilities: ["chat", "analysis"]
    auto_start: true

functions:
  chat:
    description: "Test chat function"
    timeout: 30000
)";
    }
    
    std::unique_ptr<AgentConfigManager> config_manager_;
    std::string test_config_file_;
};

TEST_F(ConfigTest, ShouldLoadValidConfiguration) {
    bool loaded = config_manager_->load_from_file(test_config_file_);
    EXPECT_TRUE(loaded);
    
    const auto& config = config_manager_->get_config();
    EXPECT_EQ(config.system.name, "Test System");
    EXPECT_EQ(config.system.port, 8080);
    EXPECT_EQ(config.agents.size(), 1);
    EXPECT_EQ(config.agents[0].name, "TestAgent");
    EXPECT_EQ(config.agents[0].capabilities.size(), 2);
}

TEST_F(ConfigTest, ShouldFailOnInvalidFile) {
    bool loaded = config_manager_->load_from_file("nonexistent.yaml");
    EXPECT_FALSE(loaded);
}

TEST_F(ConfigTest, ShouldValidateConfiguration) {
    config_manager_->load_from_file(test_config_file_);
    
    auto validation_result = config_manager_->validate();
    EXPECT_TRUE(validation_result.valid);
    EXPECT_TRUE(validation_result.errors.empty());
}
```

## 🌐 Integration Tests

### API Endpoint Tests

#### Test HTTP Server
```cpp
// tests/integration/test_http_server.cpp
#include <gtest/gtest.h>
#include "server_http.hpp"
#include "agent_manager.hpp"
#include <curl/curl.h>
#include <thread>
#include <chrono>

class HttpServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        agent_manager_ = std::make_shared<AgentManager>();
        server_ = std::make_unique<HTTPServer>(8081, agent_manager_);
        
        // Start server in background thread
        server_thread_ = std::thread([this]() {
            server_->start();
        });
        
        // Wait for server to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    void TearDown() override {
        server_->stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }
    
    std::string make_request(const std::string& endpoint, 
                           const std::string& method = "GET",
                           const std::string& data = "") {
        CURL* curl = curl_easy_init();
        std::string response;
        
        if (curl) {
            std::string url = "http://localhost:8081" + endpoint;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            
            if (method == "POST") {
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                if (!data.empty()) {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
                }
            }
            
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }
        
        return response;
    }
    
    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* data) {
        data->append(static_cast<char*>(contents), size * nmemb);
        return size * nmemb;
    }
    
    std::shared_ptr<AgentManager> agent_manager_;
    std::unique_ptr<HTTPServer> server_;
    std::thread server_thread_;
};

TEST_F(HttpServerTest, ShouldRespondToHealthCheck) {
    std::string response = make_request("/v1/health");
    EXPECT_FALSE(response.empty());
    EXPECT_TRUE(response.find("healthy") != std::string::npos);
}

TEST_F(HttpServerTest, ShouldListAgents) {
    std::string response = make_request("/v1/agents");
    EXPECT_FALSE(response.empty());
    EXPECT_TRUE(response.find("agents") != std::string::npos);
}

TEST_F(HttpServerTest, ShouldCreateAgent) {
    std::string data = R"({
        "name": "TestAgent",
        "capabilities": ["chat"]
    })";
    
    std::string response = make_request("/v1/agents", "POST", data);
    EXPECT_FALSE(response.empty());
    EXPECT_TRUE(response.find("agent_id") != std::string::npos);
}
```

### Agent Workflow Tests

#### Test Multi-Agent Interaction
```cpp
// tests/integration/test_agent_workflow.cpp
#include <gtest/gtest.h>
#include "agent_manager.hpp"
#include "workflow_engine.hpp"

class WorkflowTest : public ::testing::Test {
protected:
    void SetUp() override {
        agent_manager_ = std::make_shared<AgentManager>();
        workflow_engine_ = std::make_unique<WorkflowEngine>(agent_manager_);
        
        // Create test agents
        researcher_id_ = agent_manager_->create_agent("Researcher", {"research"});
        analyst_id_ = agent_manager_->create_agent("Analyst", {"analysis"});
        writer_id_ = agent_manager_->create_agent("Writer", {"writing"});
    }
    
    std::shared_ptr<AgentManager> agent_manager_;
    std::unique_ptr<WorkflowEngine> workflow_engine_;
    std::string researcher_id_;
    std::string analyst_id_;
    std::string writer_id_;
};

TEST_F(WorkflowTest, ShouldExecuteSequentialWorkflow) {
    // Create workflow definition
    WorkflowDefinition workflow;
    workflow.name = "ResearchWorkflow";
    workflow.type = WorkflowType::SEQUENTIAL;
    
    // Add steps
    WorkflowStep research_step;
    research_step.name = "research";
    research_step.agent_id = researcher_id_;
    research_step.function = "research";
    research_step.parameters = {{"query", "AI developments"}};
    workflow.steps.push_back(research_step);
    
    WorkflowStep analysis_step;
    analysis_step.name = "analysis";
    analysis_step.agent_id = analyst_id_;
    analysis_step.function = "analyze";
    analysis_step.depends_on = {"research"};
    workflow.steps.push_back(analysis_step);
    
    WorkflowStep writing_step;
    writing_step.name = "writing";
    writing_step.agent_id = writer_id_;
    writing_step.function = "write";
    writing_step.depends_on = {"analysis"};
    workflow.steps.push_back(writing_step);
    
    // Execute workflow
    std::string execution_id = workflow_engine_->execute_workflow(workflow, {});
    EXPECT_FALSE(execution_id.empty());
    
    // Wait for completion
    auto status = workflow_engine_->wait_for_completion(execution_id, 10000);
    EXPECT_EQ(status, WorkflowStatus::COMPLETED);
    
    // Verify results
    auto result = workflow_engine_->get_execution_result(execution_id);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.completed_steps.size(), 3);
}

TEST_F(WorkflowTest, ShouldExecuteParallelWorkflow) {
    WorkflowDefinition workflow;
    workflow.name = "ParallelWorkflow";
    workflow.type = WorkflowType::PARALLEL;
    
    // Add parallel steps
    for (int i = 0; i < 3; ++i) {
        WorkflowStep step;
        step.name = "parallel_" + std::to_string(i);
        step.agent_id = (i == 0) ? researcher_id_ : 
                       (i == 1) ? analyst_id_ : writer_id_;
        step.function = "status";
        workflow.steps.push_back(step);
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    std::string execution_id = workflow_engine_->execute_workflow(workflow, {});
    auto status = workflow_engine_->wait_for_completion(execution_id, 10000);
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time).count();
    
    EXPECT_EQ(status, WorkflowStatus::COMPLETED);
    // Parallel execution should be faster than sequential
    EXPECT_LT(duration, 1000);  // Should complete quickly for status functions
}
```

## ⚡ Performance Tests

### Load Testing

#### Test Agent Creation Performance
```cpp
// tests/performance/test_load.cpp
#include <gtest/gtest.h>
#include "agent_manager.hpp"
#include <chrono>
#include <vector>
#include <thread>

class LoadTest : public ::testing::Test {
protected:
    void SetUp() override {
        agent_manager_ = std::make_shared<AgentManager>();
    }
    
    std::shared_ptr<AgentManager> agent_manager_;
};

TEST_F(LoadTest, ShouldCreateAgentsQuickly) {
    const int num_agents = 100;
    auto start_time = std::chrono::steady_clock::now();
    
    std::vector<std::string> agent_ids;
    for (int i = 0; i < num_agents; ++i) {
        std::string agent_id = agent_manager_->create_agent(
            "LoadTestAgent_" + std::to_string(i), {"chat"});
        agent_ids.push_back(agent_id);
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time).count();
    
    EXPECT_EQ(agent_ids.size(), num_agents);
    EXPECT_LT(duration, 5000);  // Should create 100 agents in < 5 seconds
    
    double avg_time = static_cast<double>(duration) / num_agents;
    EXPECT_LT(avg_time, 50);  // Average < 50ms per agent
    
    std::cout << "Created " << num_agents << " agents in " << duration 
              << "ms (avg: " << avg_time << "ms per agent)" << std::endl;
}

TEST_F(LoadTest, ShouldHandleConcurrentRequests) {
    const int num_threads = 10;
    const int requests_per_thread = 10;
    
    std::vector<std::thread> threads;
    std::vector<std::vector<std::string>> thread_results(num_threads);
    
    auto start_time = std::chrono::steady_clock::now();
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, requests_per_thread, &thread_results]() {
            for (int i = 0; i < requests_per_thread; ++i) {
                std::string agent_id = agent_manager_->create_agent(
                    "ConcurrentAgent_" + std::to_string(t) + "_" + std::to_string(i),
                    {"chat"});
                thread_results[t].push_back(agent_id);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time).count();
    
    // Verify all agents were created
    int total_agents = 0;
    for (const auto& results : thread_results) {
        total_agents += results.size();
    }
    
    EXPECT_EQ(total_agents, num_threads * requests_per_thread);
    EXPECT_LT(duration, 10000);  // Should complete in < 10 seconds
    
    std::cout << "Created " << total_agents << " agents concurrently in " 
              << duration << "ms" << std::endl;
}
```

### Memory Usage Tests

#### Test Memory Consumption
```cpp
// tests/performance/test_memory.cpp
#include <gtest/gtest.h>
#include "agent_manager.hpp"
#include <fstream>
#include <string>

class MemoryTest : public ::testing::Test {
protected:
    size_t get_memory_usage() {
        std::ifstream file("/proc/self/status");
        std::string line;
        while (std::getline(file, line)) {
            if (line.substr(0, 6) == "VmRSS:") {
                std::string value = line.substr(6);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                return std::stoul(value.substr(0, value.find(' '))) * 1024; // Convert KB to bytes
            }
        }
        return 0;
    }
    
    std::shared_ptr<AgentManager> agent_manager_;
};

TEST_F(MemoryTest, ShouldNotLeakMemoryOnAgentCreation) {
    agent_manager_ = std::make_shared<AgentManager>();
    
    size_t initial_memory = get_memory_usage();
    std::vector<std::string> agent_ids;
    
    // Create agents
    for (int i = 0; i < 50; ++i) {
        std::string agent_id = agent_manager_->create_agent(
            "MemoryTestAgent_" + std::to_string(i), {"chat"});
        agent_ids.push_back(agent_id);
    }
    
    size_t after_creation = get_memory_usage();
    
    // Delete agents
    for (const auto& agent_id : agent_ids) {
        agent_manager_->delete_agent(agent_id);
    }
    
    // Force garbage collection (if applicable)
    agent_manager_.reset();
    agent_manager_ = std::make_shared<AgentManager>();
    
    size_t after_deletion = get_memory_usage();
    
    // Memory should not increase significantly after deletion
    size_t memory_increase = after_deletion - initial_memory;
    size_t memory_per_agent = (after_creation - initial_memory) / 50;
    
    EXPECT_LT(memory_increase, memory_per_agent * 5);  // Allow some overhead
    
    std::cout << "Initial memory: " << initial_memory / 1024 << " KB" << std::endl;
    std::cout << "After creation: " << after_creation / 1024 << " KB" << std::endl;
    std::cout << "After deletion: " << after_deletion / 1024 << " KB" << std::endl;
    std::cout << "Memory per agent: " << memory_per_agent / 1024 << " KB" << std::endl;
}
```

## 🔍 Test Utilities

### Mock Objects

#### Mock Model Interface
```cpp
// tests/mocks/mock_model_interface.hpp
#include <gmock/gmock.h>
#include "model_interface.hpp"

class MockModelInterface : public ModelInterface {
public:
    MOCK_METHOD(bool, load_model, (const ModelConfig& config), (override));
    MOCK_METHOD(InferenceResult, chat_completion, (const ChatRequest& request), (override));
    MOCK_METHOD(EmbeddingResult, generate_embedding, (const std::string& text), (override));
    MOCK_METHOD(bool, is_model_loaded, (), (const, override));
    MOCK_METHOD(ModelInfo, get_model_info, (), (const, override));
};
```

#### Test Data Factory
```cpp
// tests/fixtures/test_data_factory.hpp
#pragma once
#include "agent_data.hpp"
#include "agent_config.hpp"

class TestDataFactory {
public:
    static AgentData create_chat_data(const std::string& message, 
                                    const std::string& model = "test_model") {
        AgentData data;
        data.set("message", message);
        data.set("model", model);
        return data;
    }
    
    static AgentSystemConfig::AgentConfig create_agent_config(
        const std::string& name,
        const std::vector<std::string>& capabilities = {"chat"}) {
        AgentSystemConfig::AgentConfig config;
        config.name = name;
        config.capabilities = capabilities;
        config.auto_start = true;
        return config;
    }
    
    static ExecutionResult create_successful_result(const std::string& response = "Test response") {
        ExecutionResult result;
        result.success = true;
        result.result_data.set("response", response);
        result.execution_time_ms = 100;
        return result;
    }
    
    static ExecutionResult create_failed_result(const std::string& error = "Test error") {
        ExecutionResult result;
        result.success = false;
        result.error_message = error;
        result.execution_time_ms = 50;
        return result;
    }
};
```

### Test Configuration

#### CMake Test Configuration
```cmake
# tests/CMakeLists.txt

# Find required packages
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/tests)

# Test executable
add_executable(kolosal_agent_tests
    # Unit tests
    unit/test_agent.cpp
    unit/test_agent_manager.cpp
    unit/test_config.cpp
    unit/test_function_registry.cpp
    
    # Integration tests
    integration/test_http_server.cpp
    integration/test_agent_workflow.cpp
    integration/test_api_endpoints.cpp
    
    # Performance tests
    performance/test_load.cpp
    performance/test_memory.cpp
    
    # Test utilities
    fixtures/test_data_factory.cpp
    mocks/mock_model_interface.cpp
)

# Link libraries
target_link_libraries(kolosal_agent_tests
    ${PROJECT_NAME}  # Main project library
    GTest::GTest
    GTest::Main
    GMock::GMock
    pthread
)

# Register tests with CTest
add_test(NAME AllTests COMMAND kolosal_agent_tests)

# Add test labels
set_tests_properties(AllTests PROPERTIES
    LABELS "all"
    TIMEOUT 300
)

# Separate test categories
add_test(NAME UnitTests 
         COMMAND kolosal_agent_tests --gtest_filter="*Unit*")
set_tests_properties(UnitTests PROPERTIES LABELS "unit")

add_test(NAME IntegrationTests 
         COMMAND kolosal_agent_tests --gtest_filter="*Integration*")
set_tests_properties(IntegrationTests PROPERTIES LABELS "integration")

add_test(NAME PerformanceTests 
         COMMAND kolosal_agent_tests --gtest_filter="*Performance*")
set_tests_properties(PerformanceTests PROPERTIES LABELS "performance")
```

## 🚀 Running Tests

### Local Testing

#### Basic Test Execution
```bash
# Build with tests
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build . --config Debug

# Run all tests
ctest --output-on-failure

# Run with verbose output
ctest --verbose

# Run tests in parallel
ctest -j$(nproc)
```

#### Advanced Test Options
```bash
# Run specific test patterns
ctest -R "Agent"        # All tests containing "Agent"
ctest -R "Http.*Test"   # Tests matching regex pattern

# Run by label
ctest -L unit
ctest -L integration
ctest -L performance

# Exclude certain tests
ctest -E "Performance"  # Exclude performance tests

# Run with timeout
ctest --timeout 60

# Generate XML output for CI
ctest --output-on-failure -T Test
```

### Continuous Integration

#### GitHub Actions Workflow
```yaml
# .github/workflows/tests.yml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    
    strategy:
      matrix:
        build_type: [Debug, Release]
        compiler: [gcc-11, clang-12]
    
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive
    
    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install -y build-essential cmake libcurl4-openssl-dev
        sudo apt install -y libgtest-dev libgmock-dev
    
    - name: Configure CMake
      run: |
        mkdir build
        cd build
        cmake .. -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} -DBUILD_TESTS=ON
    
    - name: Build
      run: cmake --build build --config ${{ matrix.build_type }}
    
    - name: Run tests
      working-directory: build
      run: |
        ctest --output-on-failure -T Test
    
    - name: Upload test results
      uses: actions/upload-artifact@v3
      if: always()
      with:
        name: test-results-${{ matrix.build_type }}-${{ matrix.compiler }}
        path: build/Testing/
    
    - name: Generate coverage report
      if: matrix.build_type == 'Debug'
      run: |
        # Generate coverage report if enabled
        # gcovr --xml-pretty --exclude-unreachable-branches
```

### Test Coverage

#### Enable Coverage Reporting
```bash
# Build with coverage
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON
cmake --build . --config Debug

# Run tests
ctest

# Generate coverage report
gcovr --html --html-details -o coverage.html
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

## 🔧 Writing New Tests

### Test Writing Guidelines

1. **Test Structure**: Use Arrange-Act-Assert pattern
2. **Test Names**: Descriptive names describing expected behavior
3. **Test Independence**: Each test should be independent
4. **Test Data**: Use test fixtures and factories
5. **Assertions**: Use appropriate assertion macros

### Example Test Template

```cpp
// Test file template
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "component_under_test.hpp"
#include "test_data_factory.hpp"

class ComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test environment
        component_ = std::make_unique<ComponentUnderTest>();
    }
    
    void TearDown() override {
        // Cleanup
        component_.reset();
    }
    
    // Helper methods
    void setup_test_scenario() {
        // Common test setup
    }
    
    std::unique_ptr<ComponentUnderTest> component_;
};

TEST_F(ComponentTest, ShouldBehaviorWhenCondition) {
    // Arrange
    auto input = TestDataFactory::create_test_input();
    auto expected = TestDataFactory::create_expected_output();
    
    // Act
    auto result = component_->process(input);
    
    // Assert
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.data, expected);
}

TEST_F(ComponentTest, ShouldThrowExceptionWhenInvalidInput) {
    // Arrange
    auto invalid_input = TestDataFactory::create_invalid_input();
    
    // Act & Assert
    EXPECT_THROW(component_->process(invalid_input), std::invalid_argument);
}
```

## 📊 Test Metrics and Analysis

### Test Metrics Collection
```bash
# Test execution metrics
ctest --verbose 2>&1 | grep "Test #" | awk '{print $4, $6}'

# Memory usage during tests
valgrind --tool=massif ctest
ms_print massif.out.*

# Performance profiling
perf record ctest
perf report
```

### Test Quality Metrics

Monitor these metrics for test quality:
- **Test Coverage**: Aim for >80% line coverage
- **Test Execution Time**: Keep tests fast (<1s per test)
- **Test Reliability**: <1% flaky test rate
- **Test Maintainability**: Regular refactoring and updates

---

This comprehensive testing guide ensures robust testing practices for the Kolosal Agent System. For additional testing strategies and patterns, refer to the [Developer Guide](DEVELOPER_GUIDE.md).
