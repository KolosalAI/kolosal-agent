# Developer Guide

Complete guide for developers contributing to or extending the Kolosal Agent System v2.0.

## 🎯 Getting Started

### Development Environment Setup

#### Prerequisites
- **Git** with submodule support
- **C++17** compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- **CMake** 3.14+
- **IDE/Editor** (VS Code, Visual Studio, CLion recommended)

#### Clone and Setup
```bash
# Clone with submodules
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# Create development build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build . --config Debug

# Verify development build
ctest --output-on-failure
```

#### Recommended VS Code Extensions
```json
{
  "recommendations": [
    "ms-vscode.cpptools",
    "ms-vscode.cmake-tools",
    "twxs.cmake",
    "ms-vscode.cpptools-extension-pack",
    "streetsidesoftware.code-spell-checker",
    "github.copilot"
  ]
}
```

## 🏗️ Project Structure

### Directory Layout
```
kolosal-agent/
├── docs/                    # Documentation
├── include/                 # Public header files
│   ├── agent.hpp
│   ├── agent_manager.hpp
│   ├── agent_config.hpp
│   └── ...
├── src/                     # Source code
│   ├── core/               # Core components
│   │   ├── agent.cpp
│   │   ├── agent_manager.cpp
│   │   └── ...
│   └── api/                # API implementation
│       ├── routes/
│       └── handlers/
├── tests/                   # Test files
│   ├── unit/               # Unit tests
│   ├── integration/        # Integration tests
│   └── performance/        # Performance tests
├── external/               # External dependencies
├── cmake/                  # CMake modules
├── config.yaml            # Default configuration
├── agent.yaml             # Agent configuration
└── CMakeLists.txt         # Main CMake file
```

### Code Organization

#### Core Components
- **`include/`**: Public API headers
- **`src/core/`**: Core system implementation
- **`src/api/`**: REST API implementation

#### Key Classes
```cpp
// Core agent functionality
class Agent;
class AgentManager;
class AgentConfigManager;

// HTTP and API
class HTTPServer;
class AgentManagementRoute;

// Model integration
class ModelInterface;
class RetrievalManager;
```

## 🔧 Development Workflow

### 1. Setting Up Development Build

#### Debug Build with All Features
```bash
mkdir build-dev && cd build-dev
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic"
```

#### Development CMake Presets
```json
// CMakePresets.json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "dev-debug",
      "displayName": "Development Debug",
      "generator": "Unix Makefiles",
      "binaryDir": "${sourceDir}/build-dev",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "BUILD_TESTS": "ON",
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
        "CMAKE_CXX_FLAGS": "-Wall -Wextra -Wpedantic -g"
      }
    },
    {
      "name": "dev-release",
      "displayName": "Development Release",
      "generator": "Unix Makefiles",
      "binaryDir": "${sourceDir}/build-release",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "BUILD_TESTS": "ON"
      }
    }
  ]
}
```

### 2. Code Style and Standards

#### Formatting Configuration
```yaml
# .clang-format
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
NamespaceIndentation: None
AccessModifierOffset: -2
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
```

#### Naming Conventions
```cpp
// Classes: PascalCase
class AgentManager;
class HTTPServer;

// Functions and variables: snake_case
void create_agent();
std::string agent_id;

// Constants: UPPER_SNAKE_CASE
const int MAX_AGENTS = 100;

// Private members: trailing underscore
class Agent {
private:
    std::string name_;
    std::vector<std::string> capabilities_;
};
```

#### Header Guards and Includes
```cpp
#pragma once

// System includes first
#include <memory>
#include <string>
#include <vector>

// Third-party includes
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

// Project includes
#include "agent_config.hpp"
#include "model_interface.hpp"
```

### 3. Writing Tests

#### Unit Test Structure
```cpp
// tests/unit/test_agent.cpp
#include <gtest/gtest.h>
#include "agent.hpp"

class AgentTest : public ::testing::Test {
protected:
    void SetUp() override {
        agent_ = std::make_unique<Agent>("test_agent");
    }
    
    void TearDown() override {
        agent_.reset();
    }
    
    std::unique_ptr<Agent> agent_;
};

TEST_F(AgentTest, InitializationTest) {
    ASSERT_TRUE(agent_->initialize());
    EXPECT_EQ(agent_->get_name(), "test_agent");
    EXPECT_TRUE(agent_->is_initialized());
}

TEST_F(AgentTest, CapabilityManagement) {
    agent_->add_capability("chat");
    agent_->add_capability("analysis");
    
    auto capabilities = agent_->get_capabilities();
    EXPECT_EQ(capabilities.size(), 2);
    EXPECT_TRUE(agent_->has_capability("chat"));
    EXPECT_TRUE(agent_->has_capability("analysis"));
    EXPECT_FALSE(agent_->has_capability("unknown"));
}
```

#### Integration Test Example
```cpp
// tests/integration/test_agent_api.cpp
#include <gtest/gtest.h>
#include <curl/curl.h>
#include "server_http.hpp"
#include "agent_manager.hpp"

class AgentAPITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Start HTTP server
        server_ = std::make_unique<HTTPServer>(8081);
        server_->start();
        
        // Wait for server to be ready
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    void TearDown() override {
        server_->stop();
    }
    
    std::string make_request(const std::string& endpoint, const std::string& method = "GET") {
        // Implementation using libcurl
        // ...
    }
    
    std::unique_ptr<HTTPServer> server_;
};

TEST_F(AgentAPITest, CreateAndListAgents) {
    // Create agent via API
    std::string create_payload = R"({
        "name": "test_agent",
        "capabilities": ["chat", "analysis"]
    })";
    
    std::string response = make_request("/v1/agents", "POST", create_payload);
    EXPECT_TRUE(response.find("agent_id") != std::string::npos);
    
    // List agents
    response = make_request("/v1/agents");
    EXPECT_TRUE(response.find("test_agent") != std::string::npos);
}
```

#### Running Tests
```bash
# Run all tests
ctest --output-on-failure

# Run specific test category
ctest -L unit
ctest -L integration

# Run specific test
ctest -R AgentTest

# Run tests with verbose output
ctest --verbose

# Run tests in parallel
ctest -j$(nproc)
```

### 4. Debugging

#### Debug Build Configuration
```bash
# Debug build with symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g -O0"

# Build with AddressSanitizer
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address -g"

# Build with ThreadSanitizer
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=thread -g"
```

#### GDB Debugging
```bash
# Run with GDB
gdb ./kolosal-agent

# GDB commands
(gdb) set args --config ../config.yaml
(gdb) run
(gdb) bt        # backtrace on crash
(gdb) info threads
```

#### Valgrind Memory Check
```bash
# Check for memory leaks
valgrind --leak-check=full --show-leak-kinds=all ./kolosal-agent

# Check for thread issues
valgrind --tool=helgrind ./kolosal-agent
```

## 🔧 Adding New Features

### 1. Adding a New Agent Function

#### Step 1: Define Function Interface
```cpp
// include/functions/custom_function.hpp
#pragma once
#include "function_base.hpp"

class CustomFunction : public FunctionBase {
public:
    explicit CustomFunction(const std::string& name);
    
    ExecutionResult execute(const AgentData& input) override;
    
    std::vector<Parameter> get_parameters() const override;
    std::string get_description() const override;

private:
    void validate_input(const AgentData& input) const;
    AgentData process_data(const AgentData& input) const;
};
```

#### Step 2: Implement Function
```cpp
// src/functions/custom_function.cpp
#include "functions/custom_function.hpp"
#include <stdexcept>

CustomFunction::CustomFunction(const std::string& name) 
    : FunctionBase(name) {}

ExecutionResult CustomFunction::execute(const AgentData& input) {
    try {
        validate_input(input);
        AgentData result = process_data(input);
        
        return ExecutionResult{
            .success = true,
            .result_data = result,
            .execution_time_ms = 0,  // Will be set by caller
            .error_message = ""
        };
    } catch (const std::exception& e) {
        return ExecutionResult{
            .success = false,
            .result_data = AgentData{},
            .execution_time_ms = 0,
            .error_message = e.what()
        };
    }
}

std::vector<Parameter> CustomFunction::get_parameters() const {
    return {
        {"input_text", "string", true, "Text to process"},
        {"mode", "string", false, "Processing mode (default: basic)"}
    };
}

void CustomFunction::validate_input(const AgentData& input) const {
    if (!input.has("input_text")) {
        throw std::invalid_argument("Missing required parameter: input_text");
    }
    
    if (input.get_string("input_text").empty()) {
        throw std::invalid_argument("input_text cannot be empty");
    }
}
```

#### Step 3: Register Function
```cpp
// src/core/function_registry.cpp
#include "functions/custom_function.hpp"

void FunctionRegistry::register_default_functions() {
    // Existing functions...
    
    // Register new function
    register_function(std::make_unique<CustomFunction>("custom_process"));
}
```

### 2. Adding a New API Endpoint

#### Step 1: Define Route Handler
```cpp
// src/api/routes/custom_route.hpp
#pragma once
#include "route_base.hpp"

class CustomRoute : public RouteBase {
public:
    explicit CustomRoute(std::shared_ptr<AgentManager> agent_manager);
    
    void register_routes(HTTPServer& server) override;

private:
    void handle_custom_operation(const Request& req, Response& res);
    void handle_custom_batch(const Request& req, Response& res);
    
    std::shared_ptr<AgentManager> agent_manager_;
};
```

#### Step 2: Implement Route Handler
```cpp
// src/api/routes/custom_route.cpp
#include "api/routes/custom_route.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

CustomRoute::CustomRoute(std::shared_ptr<AgentManager> agent_manager)
    : agent_manager_(agent_manager) {}

void CustomRoute::register_routes(HTTPServer& server) {
    server.Post("/v1/custom/operation", 
                [this](const Request& req, Response& res) {
                    handle_custom_operation(req, res);
                });
    
    server.Post("/v1/custom/batch",
                [this](const Request& req, Response& res) {
                    handle_custom_batch(req, res);
                });
}

void CustomRoute::handle_custom_operation(const Request& req, Response& res) {
    try {
        json request_body = json::parse(req.body);
        
        // Validate request
        if (!request_body.contains("operation_type")) {
            res.status = 400;
            res.set_content(R"({"error": "Missing operation_type"})", "application/json");
            return;
        }
        
        std::string operation_type = request_body["operation_type"];
        
        // Process operation
        json result = process_custom_operation(operation_type, request_body);
        
        res.set_content(result.dump(), "application/json");
        
    } catch (const std::exception& e) {
        res.status = 500;
        json error_response = {
            {"error", "Internal server error"},
            {"message", e.what()}
        };
        res.set_content(error_response.dump(), "application/json");
    }
}
```

#### Step 3: Register Route
```cpp
// src/api/http_server.cpp
#include "api/routes/custom_route.hpp"

void HTTPServer::setup_routes() {
    // Existing routes...
    
    // Register custom route
    auto custom_route = std::make_shared<CustomRoute>(agent_manager_);
    custom_route->register_routes(*this);
}
```

### 3. Adding Configuration Options

#### Step 1: Update Configuration Schema
```cpp
// include/agent_config.hpp
struct CustomConfig {
    bool enabled = false;
    int max_items = 100;
    std::string mode = "default";
    std::vector<std::string> allowed_types;
};

struct AgentSystemConfig {
    // Existing config...
    CustomConfig custom;
};
```

#### Step 2: Implement Configuration Parsing
```cpp
// src/core/agent_config.cpp
void AgentConfigManager::parse_custom_config(const YAML::Node& node) {
    if (node["custom"]) {
        const auto& custom_node = node["custom"];
        
        config_.custom.enabled = custom_node["enabled"].as<bool>(false);
        config_.custom.max_items = custom_node["max_items"].as<int>(100);
        config_.custom.mode = custom_node["mode"].as<std::string>("default");
        
        if (custom_node["allowed_types"]) {
            for (const auto& type : custom_node["allowed_types"]) {
                config_.custom.allowed_types.push_back(type.as<std::string>());
            }
        }
    }
}
```

#### Step 3: Use Configuration
```cpp
// Usage in components
void SomeComponent::initialize() {
    const auto& config = config_manager_->get_config();
    
    if (config.custom.enabled) {
        max_items_ = config.custom.max_items;
        mode_ = config.custom.mode;
        // ...
    }
}
```

## 🧪 Testing Guidelines

### Test Categories

#### 1. Unit Tests
- Test individual classes and functions in isolation
- Use mocks for dependencies
- Fast execution (< 1ms per test)

#### 2. Integration Tests
- Test component interactions
- Use real dependencies where possible
- Test API endpoints end-to-end

#### 3. Performance Tests
- Measure execution times
- Test memory usage
- Validate throughput requirements

### Testing Best Practices

#### Test Structure
```cpp
// Arrange, Act, Assert pattern
TEST_F(ComponentTest, MethodShouldBehaveProperly) {
    // Arrange
    auto input = create_test_input();
    auto expected = create_expected_output();
    
    // Act
    auto result = component_->process(input);
    
    // Assert
    EXPECT_EQ(result.status, Status::SUCCESS);
    EXPECT_EQ(result.data, expected);
}
```

#### Mock Usage
```cpp
// Using Google Mock
class MockModelInterface : public ModelInterface {
public:
    MOCK_METHOD(InferenceResult, chat_completion, 
                (const ChatRequest& request), (override));
    MOCK_METHOD(bool, load_model, 
                (const ModelConfig& config), (override));
};

TEST_F(AgentTest, ShouldCallModelInterface) {
    auto mock_model = std::make_shared<MockModelInterface>();
    
    EXPECT_CALL(*mock_model, chat_completion(testing::_))
        .Times(1)
        .WillOnce(testing::Return(create_successful_result()));
    
    agent_->set_model_interface(mock_model);
    auto result = agent_->execute_chat("test message");
    
    EXPECT_TRUE(result.success);
}
```

### Continuous Integration

#### GitHub Actions Workflow
```yaml
# .github/workflows/ci.yml
name: CI

on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive
    
    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install -y build-essential cmake libcurl4-openssl-dev
    
    - name: Configure CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
    
    - name: Build
      run: cmake --build build --config Release
    
    - name: Test
      working-directory: build
      run: ctest --output-on-failure
    
    - name: Upload test results
      uses: actions/upload-artifact@v3
      if: failure()
      with:
        name: test-results
        path: build/Testing/
```

## 📝 Documentation Guidelines

### Code Documentation

#### Header Documentation
```cpp
/**
 * @file agent_manager.hpp
 * @brief Agent lifecycle management and coordination
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * This file contains the AgentManager class which is responsible for
 * managing the lifecycle of AI agents, including creation, deletion,
 * and coordination of agent activities.
 */
```

#### Class Documentation
```cpp
/**
 * @brief Manages the lifecycle and coordination of AI agents
 * 
 * The AgentManager class provides a centralized interface for creating,
 * managing, and coordinating multiple AI agents. It handles agent
 * registration, lifecycle management, and inter-agent communication.
 * 
 * @example
 * ```cpp
 * auto manager = std::make_unique<AgentManager>();
 * std::string agent_id = manager->create_agent("assistant", {"chat"});
 * manager->start_agent(agent_id);
 * ```
 */
class AgentManager {
    /**
     * @brief Creates a new agent with specified capabilities
     * @param name The name of the agent
     * @param capabilities List of capabilities for the agent
     * @return Agent ID if successful
     * @throws std::runtime_error if agent creation fails
     */
    std::string create_agent(const std::string& name, 
                           const std::vector<std::string>& capabilities);
};
```

#### Function Documentation
```cpp
/**
 * @brief Executes a function on the specified agent
 * 
 * This method executes a named function on the target agent with the
 * provided parameters. The execution is synchronous and will block
 * until completion or timeout.
 * 
 * @param agent_id Unique identifier of the target agent
 * @param function_name Name of the function to execute
 * @param parameters Input parameters for the function
 * @param timeout_ms Maximum execution time in milliseconds (0 = no timeout)
 * 
 * @return ExecutionResult containing the result and metadata
 * 
 * @throws std::invalid_argument if agent_id or function_name is invalid
 * @throws std::runtime_error if execution fails
 * @throws std::timeout_error if execution exceeds timeout
 * 
 * @example
 * ```cpp
 * AgentData params;
 * params.set("message", "Hello, world!");
 * auto result = manager->execute_function("agent-001", "chat", params);
 * if (result.success) {
 *     std::cout << result.result_data.get_string("response") << std::endl;
 * }
 * ```
 */
ExecutionResult execute_function(const std::string& agent_id,
                               const std::string& function_name,
                               const AgentData& parameters,
                               int timeout_ms = 30000);
```

### API Documentation

Use OpenAPI/Swagger specification:

```yaml
# docs/api/openapi.yml
openapi: 3.0.3
info:
  title: Kolosal Agent System API
  version: 2.0.0
  description: REST API for managing AI agents

paths:
  /v1/agents:
    get:
      summary: List all agents
      operationId: listAgents
      responses:
        '200':
          description: Successful response
          content:
            application/json:
              schema:
                type: object
                properties:
                  agents:
                    type: array
                    items:
                      $ref: '#/components/schemas/Agent'
```

## 🔧 Performance Optimization

### Profiling

#### CPU Profiling with perf
```bash
# Build with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Profile with perf
perf record -g ./kolosal-agent
perf report

# Generate flame graph
perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg
```

#### Memory Profiling with Valgrind
```bash
# Memory usage profiling
valgrind --tool=massif ./kolosal-agent
ms_print massif.out.* > memory_profile.txt

# Cache miss analysis
valgrind --tool=cachegrind ./kolosal-agent
cg_annotate cachegrind.out.* > cache_analysis.txt
```

### Performance Best Practices

#### 1. Memory Management
```cpp
// Use smart pointers
std::shared_ptr<Agent> agent = std::make_shared<Agent>("name");

// Prefer move semantics
AgentData create_data() {
    AgentData data;
    // populate data
    return data;  // Move constructor used
}

// Reserve vector capacity
std::vector<Agent> agents;
agents.reserve(expected_count);

// Use object pools for frequent allocations
class AgentPool {
    std::queue<std::unique_ptr<Agent>> available_agents_;
    
public:
    std::unique_ptr<Agent> acquire() {
        if (available_agents_.empty()) {
            return std::make_unique<Agent>();
        }
        auto agent = std::move(available_agents_.front());
        available_agents_.pop();
        return agent;
    }
    
    void release(std::unique_ptr<Agent> agent) {
        agent->reset();
        available_agents_.push(std::move(agent));
    }
};
```

#### 2. Asynchronous Programming
```cpp
// Use std::async for parallel operations
auto future1 = std::async(std::launch::async, [&]() {
    return agent1->execute_function("analyze", data);
});

auto future2 = std::async(std::launch::async, [&]() {
    return agent2->execute_function("summarize", data);
});

auto result1 = future1.get();
auto result2 = future2.get();

// Use thread pool for many small tasks
class ThreadPool {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;

public:
    template<class F>
    auto enqueue(F&& f) -> std::future<typename std::result_of<F()>::type> {
        using return_type = typename std::result_of<F()>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::forward<F>(f)
        );
        
        std::future<return_type> res = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks_.emplace([task](){ (*task)(); });
        }
        
        condition_.notify_one();
        return res;
    }
};
```

## 🤝 Contributing Guidelines

### Contribution Process

1. **Fork the repository**
2. **Create feature branch**: `git checkout -b feature/new-feature`
3. **Make changes** following coding standards
4. **Add tests** for new functionality
5. **Update documentation** as needed
6. **Run tests**: `ctest --output-on-failure`
7. **Submit pull request**

### Pull Request Template

```markdown
## Description
Brief description of changes made.

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Added unit tests
- [ ] Added integration tests
- [ ] All tests pass locally
- [ ] Manual testing completed

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] No breaking changes (or marked as such)
```

### Code Review Guidelines

#### For Authors
- Keep changes focused and atomic
- Write clear commit messages
- Add comprehensive tests
- Update documentation

#### For Reviewers
- Check for code clarity and maintainability
- Verify test coverage
- Look for potential performance issues
- Ensure documentation is updated

## 🔧 Advanced Topics

### Plugin Development

#### Creating Custom Plugins
```cpp
// include/plugins/plugin_interface.hpp
class PluginInterface {
public:
    virtual ~PluginInterface() = default;
    virtual bool initialize(const PluginConfig& config) = 0;
    virtual void shutdown() = 0;
    virtual std::string get_name() const = 0;
    virtual std::string get_version() const = 0;
};

// Custom plugin implementation
class CustomPlugin : public PluginInterface {
public:
    bool initialize(const PluginConfig& config) override {
        // Plugin initialization
        return true;
    }
    
    void shutdown() override {
        // Cleanup
    }
    
    std::string get_name() const override { return "CustomPlugin"; }
    std::string get_version() const override { return "1.0.0"; }
};

// Plugin registration
extern "C" {
    PluginInterface* create_plugin() {
        return new CustomPlugin();
    }
    
    void destroy_plugin(PluginInterface* plugin) {
        delete plugin;
    }
}
```

### Custom Model Integration

#### Model Interface Implementation
```cpp
class CustomModelInterface : public ModelInterface {
public:
    bool load_model(const ModelConfig& config) override {
        // Load your custom model
        model_path_ = config.path;
        // Implementation specific to your model
        return true;
    }
    
    InferenceResult chat_completion(const ChatRequest& request) override {
        // Implement chat completion using your model
        InferenceResult result;
        result.success = true;
        result.response = generate_response(request.message);
        return result;
    }
    
private:
    std::string model_path_;
    // Your model-specific members
};
```

---

This developer guide provides comprehensive information for contributing to and extending the Kolosal Agent System. For specific questions or advanced topics, please refer to the [API Reference](API_REFERENCE.md) or open an issue on GitHub.
