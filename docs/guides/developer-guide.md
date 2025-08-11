# Developer Guide - Kolosal Agent System v2.0

This guide provides comprehensive information for developers who want to extend, modify, or contribute to the Kolosal Agent System v2.0.

## Table of Contents

- [Development Environment Setup](#development-environment-setup)
- [Architecture Overview](#architecture-overview)
- [Code Structure](#code-structure)
- [Extending the System](#extending-the-system)
- [Custom Agent Development](#custom-agent-development)
- [Function Development](#function-development)
- [Tool Integration](#tool-integration)
- [API Development](#api-development)
- [Testing Guidelines](#testing-guidelines)
- [Contributing Guidelines](#contributing-guidelines)
- [Build System](#build-system)
- [Debugging and Profiling](#debugging-and-profiling)

## Development Environment Setup

### Prerequisites

**Required Tools:**
- **CMake 3.14+**: Build system
- **C++17 Compiler**: MSVC 2019+, GCC 9+, or Clang 10+
- **Git**: Version control with submodule support
- **Python 3.8+**: For build scripts and tools
- **Node.js 16+**: For web interface development (optional)

**Development Dependencies:**
- **Doxygen**: Documentation generation
- **clang-format**: Code formatting
- **cppcheck**: Static analysis
- **valgrind**: Memory debugging (Linux/macOS)
- **gdb**: Debugging (Linux/macOS)
- **Visual Studio Debugger**: Debugging (Windows)

### Environment Setup

```bash
# Clone the repository with submodules
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# Set up development environment
export KOLOSAL_DEV_MODE=1
export KOLOSAL_LOG_LEVEL=DEBUG

# Install pre-commit hooks (optional but recommended)
pip install pre-commit
pre-commit install
```

### Build Configuration for Development

```bash
# Debug build with all development features
mkdir build-dev && cd build-dev
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DBUILD_EXAMPLES=ON \
  -DBUILD_DOCS=ON \
  -DENABLE_ASAN=ON \
  -DENABLE_UBSAN=ON \
  -DMCP_PROTOCOL_ENABLED=ON \
  -DUSE_PODOFO=ON

# Build with parallel processing
cmake --build . --parallel

# Run tests
ctest --output-on-failure

# Generate documentation
cmake --build . --target docs
```

### IDE Configuration

#### Visual Studio Code

```json
// .vscode/settings.json
{
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "C_Cpp.default.cppStandard": "c++17",
    "C_Cpp.default.intelliSenseMode": "gcc-x64",
    "cmake.buildDirectory": "${workspaceFolder}/build-dev",
    "files.associations": {
        "*.hpp": "cpp",
        "*.cpp": "cpp",
        "*.yaml": "yaml",
        "*.json": "json"
    },
    "editor.formatOnSave": true,
    "C_Cpp.clang_format_style": "file"
}
```

```json
// .vscode/tasks.json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake Configure",
            "type": "shell",
            "command": "cmake",
            "args": ["..", "-DCMAKE_BUILD_TYPE=Debug", "-DBUILD_TESTS=ON"],
            "group": "build",
            "options": {
                "cwd": "${workspaceFolder}/build-dev"
            }
        },
        {
            "label": "Build Debug",
            "type": "shell",
            "command": "cmake",
            "args": ["--build", ".", "--parallel"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "options": {
                "cwd": "${workspaceFolder}/build-dev"
            }
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "ctest",
            "args": ["--output-on-failure"],
            "group": "test",
            "options": {
                "cwd": "${workspaceFolder}/build-dev"
            }
        }
    ]
}
```

## Architecture Overview

### Core Components

```cpp
// Core system architecture
namespace kolosal {
    namespace agents {
        // Agent core functionality
        class AgentCore;                    // Individual agent implementation
        class YAMLConfigurableAgentManager; // Agent lifecycle management
        class AgentOrchestrator;            // Multi-agent coordination
        class WorkflowEngine;               // Complex workflow execution
        class SequentialWorkflowExecutor;   // Sequential workflows
        
        // Memory system
        class MemorySystem;                 // Unified memory interface
        class EpisodicMemory;               // Experience storage
        class SemanticMemory;               // Knowledge base
        class WorkingMemory;                // Active context
        
        // Function system
        class FunctionManager;              // Function registry
        class BuiltinFunctionRegistry;      // Built-in functions
        class ToolManager;                  // External tool integration
    }
    
    namespace services {
        // Service layer
        class AgentService;                 // High-level operations
        class WorkflowAgentService;         // Workflow services
        class DocumentAgentService;         // Document processing
        
        #ifdef MCP_PROTOCOL_ENABLED
        class MCPAgentAdapter;              // MCP protocol integration
        #endif
    }
    
    namespace integration {
        // Integration layer
        class UnifiedKolosalServer;         // Main server orchestrator
        class UnifiedServerFactory;         // Server factory
    }
    
    namespace api {
        // API layer
        class SimpleHttpServer;             // HTTP server
        class AgentManagementRoute;         // REST API routes
        class IRoute;                       // Route interface
    }
}
```

### Design Patterns Used

1. **Factory Pattern**: `UnifiedServerFactory`, `AgentFactory`
2. **Observer Pattern**: Event notifications, health monitoring
3. **Strategy Pattern**: Different agent types, workflow patterns
4. **Command Pattern**: Function execution, tool invocation
5. **Singleton Pattern**: Logger, configuration manager
6. **Bridge Pattern**: MCP protocol integration
7. **Composite Pattern**: Complex workflows, nested agents
8. **Template Method Pattern**: Base classes with specializations

## Code Structure

```
kolosal-agent/
├── include/                    # Header files
│   ├── agent/                  # Agent system headers
│   │   ├── core/              # Core agent functionality
│   │   ├── services/          # Service layer
│   │   └── utils/             # Agent utilities
│   ├── api/                   # REST API headers
│   ├── config/                # Configuration management
│   ├── server/                # Server integration
│   ├── tools/                 # Tool system
│   ├── utils/                 # Common utilities
│   ├── workflow/              # Workflow system
│   └── export.hpp             # Symbol export macros
│
├── src/                       # Source files
│   ├── agent/                 # Agent implementation
│   ├── api/                   # REST API implementation
│   ├── config/                # Configuration parsing
│   ├── server/                # Server implementation
│   ├── tools/                 # Tool implementations
│   ├── utils/                 # Utility implementations
│   └── workflow/              # Workflow implementation
│
├── kolosal-server/            # LLM server component
│   ├── include/kolosal/       # LLM server headers
│   └── src/                   # LLM server implementation
│
├── external/                  # Third-party dependencies
│   ├── curl/                  # HTTP client library
│   ├── nlohmann/              # JSON library
│   ├── yaml-cpp/              # YAML parsing
│   └── mcp-cpp/               # MCP protocol (if enabled)
│
├── examples/                  # Example code
├── tests/                     # Unit and integration tests
├── docs/                      # Documentation
├── cmake/                     # CMake modules
└── scripts/                   # Build and utility scripts
```

## Extending the System

### Adding New Agent Types

```cpp
// include/agent/core/custom_agent.hpp
#pragma once

#include "agent_core.hpp"

namespace kolosal::agents {

/**
 * @brief Custom agent type for specialized functionality
 */
class KOLOSAL_AGENT_API CustomAgent : public AgentCore {
public:
    CustomAgent(const std::string& name, const AgentConfig& config);
    virtual ~CustomAgent() = default;
    
    // Override virtual methods
    bool initialize() override;
    bool start() override;
    bool stop() override;
    
    // Custom functionality
    FunctionResult processCustomData(const AgentData& data);
    
private:
    // Custom implementation details
    std::unique_ptr<CustomProcessor> processor_;
    CustomConfig custom_config_;
};

}
```

```cpp
// src/agent/core/custom_agent.cpp
#include "agent/core/custom_agent.hpp"
#include "tools/custom_tools.hpp"

namespace kolosal::agents {

CustomAgent::CustomAgent(const std::string& name, const AgentConfig& config)
    : AgentCore(name, "custom", AgentRole::SPECIALIST) {
    
    // Initialize custom configuration
    custom_config_.load_from(config);
    
    // Register custom functions
    auto custom_function = std::make_unique<CustomFunction>(custom_config_);
    function_manager_->register_function(std::move(custom_function));
    
    // Initialize custom processor
    processor_ = std::make_unique<CustomProcessor>(custom_config_);
}

bool CustomAgent::initialize() {
    if (!AgentCore::initialize()) {
        return false;
    }
    
    // Custom initialization
    if (!processor_->initialize()) {
        ServerLogger::logError("Failed to initialize custom processor");
        return false;
    }
    
    return true;
}

FunctionResult CustomAgent::processCustomData(const AgentData& data) {
    FunctionResult result;
    
    try {
        // Custom processing logic
        auto processed_data = processor_->process(data);
        
        result.set_success(true);
        result.set_result(processed_data.to_json());
        result.add_metadata("processing_time", processor_->getLastExecutionTime());
        
    } catch (const std::exception& e) {
        result.set_success(false);
        result.set_error_message(e.what());
        ServerLogger::logError("Custom processing failed: %s", e.what());
    }
    
    return result;
}

}
```

### Registering Custom Agent Types

```cpp
// Agent factory registration
namespace kolosal::agents {

// In agent manager or factory
void registerCustomAgentTypes() {
    AgentFactory::registerAgentType("custom", [](const std::string& name, const AgentConfig& config) {
        return std::make_unique<CustomAgent>(name, config);
    });
}

}
```

## Custom Agent Development

### Agent Lifecycle

```cpp
// Custom agent lifecycle management
class MyCustomAgent : public AgentCore {
public:
    // Constructor
    MyCustomAgent(const std::string& name, const AgentConfig& config);
    
    // Lifecycle methods (override as needed)
    bool initialize() override;
    bool start() override;
    bool stop() override;
    bool pause() override;
    bool resume() override;
    
    // State management
    AgentState get_state() const override;
    bool set_state(AgentState state) override;
    
    // Function execution (override for custom behavior)
    FunctionResult execute_function(const std::string& function_name, 
                                   const AgentData& params) override;
    
    // Memory management (override for custom memory handling)
    bool store_memory(const std::string& key, const AgentData& data) override;
    std::optional<AgentData> retrieve_memory(const std::string& key) override;
    
    // Health monitoring
    HealthStatus get_health_status() const override;
    
private:
    // Custom state
    std::unique_ptr<CustomState> custom_state_;
    
    // Custom components
    std::unique_ptr<CustomMemoryManager> custom_memory_;
    std::unique_ptr<CustomFunctionExecutor> custom_executor_;
};
```

### Agent Configuration Schema

```cpp
// Custom configuration structure
struct CustomAgentConfig {
    // Standard agent config
    std::string name;
    std::string type;
    AgentRole role;
    
    // Custom configuration
    struct CustomSettings {
        std::string processing_mode;
        int batch_size = 10;
        bool enable_caching = true;
        std::chrono::milliseconds timeout{30000};
        
        // Validation
        bool validate() const {
            return batch_size > 0 && timeout.count() > 0;
        }
    } custom;
    
    // Load from configuration
    void load_from_yaml(const YAML::Node& node) {
        name = node["name"].as<std::string>();
        type = node["type"].as<std::string>();
        
        if (node["custom"]) {
            auto custom_node = node["custom"];
            custom.processing_mode = custom_node["processing_mode"].as<std::string>("standard");
            custom.batch_size = custom_node["batch_size"].as<int>(10);
            custom.enable_caching = custom_node["enable_caching"].as<bool>(true);
            
            auto timeout_ms = custom_node["timeout_ms"].as<int>(30000);
            custom.timeout = std::chrono::milliseconds(timeout_ms);
        }
    }
};
```

## Function Development

### Custom Function Template

```cpp
// include/tools/custom_function.hpp
#pragma once

#include "agent/interfaces/function_interface.hpp"

namespace kolosal::agents {

/**
 * @brief Custom function implementation
 */
class KOLOSAL_AGENT_API CustomFunction : public FunctionInterface {
public:
    CustomFunction();
    virtual ~CustomFunction() = default;
    
    // Function metadata
    std::string get_name() const override { return "custom_process"; }
    std::string get_description() const override { return "Custom processing function"; }
    std::string get_category() const override { return "custom"; }
    
    // Parameter schema
    std::vector<ParameterInfo> get_parameters() const override;
    
    // Execution
    FunctionResult execute(const AgentData& params) override;
    
private:
    // Custom implementation
    std::unique_ptr<CustomProcessor> processor_;
    
    // Helper methods
    bool validate_parameters(const AgentData& params);
    AgentData process_data(const AgentData& input);
};

}
```

```cpp
// src/tools/custom_function.cpp
#include "tools/custom_function.hpp"
#include "kolosal/logger.hpp"

namespace kolosal::agents {

CustomFunction::CustomFunction() 
    : processor_(std::make_unique<CustomProcessor>()) {
}

std::vector<ParameterInfo> CustomFunction::get_parameters() const {
    return {
        {
            .name = "input_data",
            .type = "object",
            .description = "Data to process",
            .required = true
        },
        {
            .name = "processing_mode",
            .type = "string",
            .description = "Processing mode (fast|thorough|custom)",
            .required = false,
            .default_value = "fast"
        },
        {
            .name = "options",
            .type = "object", 
            .description = "Additional processing options",
            .required = false
        }
    };
}

FunctionResult CustomFunction::execute(const AgentData& params) {
    auto start_time = std::chrono::high_resolution_clock::now();
    FunctionResult result;
    
    try {
        // Validate parameters
        if (!validate_parameters(params)) {
            result.set_success(false);
            result.set_error_message("Invalid parameters");
            return result;
        }
        
        ServerLogger::logInfo("Executing custom function with mode: %s", 
                             params.get_string("processing_mode", "fast").c_str());
        
        // Process the data
        auto processed_data = process_data(params.get_object("input_data"));
        
        // Set result
        result.set_success(true);
        result.set_result(processed_data.to_json());
        
        // Add metadata
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);
        result.add_metadata("execution_time_ms", duration.count());
        result.add_metadata("function_name", get_name());
        
        ServerLogger::logInfo("Custom function completed in %lld ms", duration.count());
        
    } catch (const std::exception& e) {
        result.set_success(false);
        result.set_error_message(e.what());
        ServerLogger::logError("Custom function failed: %s", e.what());
    }
    
    return result;
}

bool CustomFunction::validate_parameters(const AgentData& params) {
    // Required parameter validation
    if (!params.has_key("input_data")) {
        return false;
    }
    
    // Optional parameter validation
    if (params.has_key("processing_mode")) {
        std::string mode = params.get_string("processing_mode");
        if (mode != "fast" && mode != "thorough" && mode != "custom") {
            return false;
        }
    }
    
    return true;
}

AgentData CustomFunction::process_data(const AgentData& input) {
    // Custom processing logic
    AgentData result;
    
    // Example: Process each field in the input
    for (const auto& [key, value] : input.get_data()) {
        // Apply custom processing
        auto processed_value = processor_->process_value(value);
        result.set(key + "_processed", processed_value);
    }
    
    // Add processing metadata
    result.set("processing_timestamp", std::time(nullptr));
    result.set("processor_version", "1.0");
    
    return result;
}

}
```

### Function Registration

```cpp
// Register custom functions
void register_custom_functions(FunctionManager& manager) {
    // Register individual functions
    manager.register_function(std::make_unique<CustomFunction>());
    manager.register_function(std::make_unique<AnotherCustomFunction>());
    
    // Register function categories
    manager.register_category("custom", "Custom processing functions");
}
```

## Tool Integration

### External Tool Wrapper

```cpp
// include/tools/external_tool.hpp
#pragma once

#include "agent/interfaces/tool_interface.hpp"

namespace kolosal::tools {

/**
 * @brief Wrapper for external tools/APIs
 */
class KOLOSAL_AGENT_API ExternalToolWrapper : public ToolInterface {
public:
    explicit ExternalToolWrapper(const ToolConfig& config);
    virtual ~ExternalToolWrapper() = default;
    
    // Tool interface
    std::string get_name() const override { return config_.name; }
    std::string get_version() const override { return config_.version; }
    bool is_available() const override;
    
    // Execution
    ToolResult execute(const std::string& operation, 
                      const nlohmann::json& parameters) override;
    
    // Configuration
    bool configure(const ToolConfig& config) override;
    ToolConfig get_config() const override { return config_; }
    
private:
    ToolConfig config_;
    std::unique_ptr<HttpClient> http_client_;
    
    // Helper methods
    ToolResult call_api(const std::string& endpoint, 
                       const nlohmann::json& payload);
    bool validate_response(const nlohmann::json& response);
};

}
```

### HTTP-based Tool Integration

```cpp
// Example: Web search tool integration
class WebSearchTool : public ExternalToolWrapper {
public:
    WebSearchTool() {
        ToolConfig config;
        config.name = "web_search";
        config.base_url = "https://api.search.com/v1";
        config.api_key = std::getenv("SEARCH_API_KEY");
        config.timeout_ms = 30000;
        configure(config);
    }
    
    ToolResult search(const std::string& query, int max_results = 10) {
        nlohmann::json params = {
            {"query", query},
            {"max_results", max_results},
            {"include_snippets", true}
        };
        
        return execute("search", params);
    }
};
```

### Database Tool Integration

```cpp
// Example: Database query tool
class DatabaseTool : public ToolInterface {
public:
    explicit DatabaseTool(const std::string& connection_string) 
        : connection_(connection_string) {
    }
    
    ToolResult execute_query(const std::string& query) {
        ToolResult result;
        
        try {
            auto db_result = connection_.execute(query);
            
            nlohmann::json json_result = nlohmann::json::array();
            for (const auto& row : db_result) {
                nlohmann::json json_row;
                for (const auto& [column, value] : row) {
                    json_row[column] = value;
                }
                json_result.push_back(json_row);
            }
            
            result.success = true;
            result.data = json_result;
            result.metadata["row_count"] = db_result.size();
            
        } catch (const std::exception& e) {
            result.success = false;
            result.error_message = e.what();
        }
        
        return result;
    }
    
private:
    DatabaseConnection connection_;
};
```

## API Development

### Custom REST API Routes

```cpp
// include/api/custom_route.hpp
#pragma once

#include "api/route_interface.hpp"

namespace kolosal::api {

/**
 * @brief Custom API route for specialized functionality
 */
class KOLOSAL_AGENT_API CustomRoute : public IRoute {
public:
    explicit CustomRoute(std::shared_ptr<CustomService> service);
    
    // Route interface
    bool match(const std::string& method, const std::string& path) override;
    void handle(SocketType sock, const std::string& body) override;
    
private:
    std::shared_ptr<CustomService> service_;
    std::string matched_method_;
    std::string matched_path_;
    
    // Route handlers
    void handle_custom_operation(SocketType sock, const std::string& body);
    void handle_custom_query(SocketType sock, const std::string& query_params);
    void handle_custom_status(SocketType sock);
    
    // Helper methods
    void send_json_response(SocketType sock, int status_code, 
                           const nlohmann::json& data);
    std::map<std::string, std::string> parse_query_params(const std::string& query);
};

}
```

### WebSocket Support

```cpp
// WebSocket integration example
class WebSocketRoute : public IRoute {
public:
    bool match(const std::string& method, const std::string& path) override {
        return method == "GET" && path == "/v1/websocket";
    }
    
    void handle(SocketType sock, const std::string& body) override {
        // Upgrade to WebSocket
        if (is_websocket_upgrade_request(sock)) {
            upgrade_to_websocket(sock);
            handle_websocket_connection(sock);
        }
    }
    
private:
    void handle_websocket_connection(SocketType sock) {
        while (true) {
            auto message = receive_websocket_message(sock);
            if (message.empty()) break;
            
            // Process message
            auto response = process_message(message);
            send_websocket_message(sock, response);
        }
    }
};
```

## Testing Guidelines

### Unit Testing Framework

```cpp
// tests/agent/test_custom_agent.cpp
#include <gtest/gtest.h>
#include "agent/core/custom_agent.hpp"
#include "test_helpers/mock_agent_manager.hpp"

namespace kolosal::agents::test {

class CustomAgentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test environment
        agent_config_.name = "test_agent";
        agent_config_.type = "custom";
        agent_config_.role = AgentRole::SPECIALIST;
        
        agent_ = std::make_unique<CustomAgent>("test_agent", agent_config_);
        
        // Initialize with mock dependencies
        mock_manager_ = std::make_shared<MockAgentManager>();
    }
    
    void TearDown() override {
        agent_.reset();
    }
    
    AgentConfig agent_config_;
    std::unique_ptr<CustomAgent> agent_;
    std::shared_ptr<MockAgentManager> mock_manager_;
};

TEST_F(CustomAgentTest, InitializationSuccess) {
    EXPECT_TRUE(agent_->initialize());
    EXPECT_EQ(agent_->get_state(), AgentState::INITIALIZED);
}

TEST_F(CustomAgentTest, StartAndStop) {
    ASSERT_TRUE(agent_->initialize());
    
    EXPECT_TRUE(agent_->start());
    EXPECT_EQ(agent_->get_state(), AgentState::RUNNING);
    
    EXPECT_TRUE(agent_->stop());
    EXPECT_EQ(agent_->get_state(), AgentState::STOPPED);
}

TEST_F(CustomAgentTest, FunctionExecution) {
    ASSERT_TRUE(agent_->initialize());
    ASSERT_TRUE(agent_->start());
    
    AgentData params;
    params.set("input_data", "test_value");
    params.set("processing_mode", "fast");
    
    auto result = agent_->execute_function("custom_process", params);
    
    EXPECT_TRUE(result.is_success());
    EXPECT_FALSE(result.get_result().empty());
    EXPECT_TRUE(result.has_metadata("execution_time_ms"));
}

}
```

### Integration Testing

```cpp
// tests/integration/test_agent_workflow.cpp
#include <gtest/gtest.h>
#include "server/unified_server.hpp"
#include "test_helpers/test_server.hpp"

namespace kolosal::integration::test {

class AgentWorkflowIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Start test server
        server_config_.server_port = 8080;
        server_config_.agent_api_port = 8081;
        server_config_.enable_agent_api = true;
        
        server_ = std::make_unique<UnifiedKolosalServer>(server_config_);
        ASSERT_TRUE(server_->start());
        
        // Wait for server to be ready
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    void TearDown() override {
        server_->stop();
    }
    
    UnifiedKolosalServer::ServerConfig server_config_;
    std::unique_ptr<UnifiedKolosalServer> server_;
};

TEST_F(AgentWorkflowIntegrationTest, CreateAndExecuteAgent) {
    // Create agent via API
    nlohmann::json agent_config = {
        {"name", "test_integration_agent"},
        {"type", "specialist"},
        {"role", 2},
        {"capabilities", {"data_processing"}}
    };
    
    auto response = http_client_->post("http://localhost:8081/v1/agents", 
                                      agent_config.dump());
    
    ASSERT_EQ(response.status_code, 201);
    
    auto response_data = nlohmann::json::parse(response.body);
    std::string agent_id = response_data["agent_id"];
    
    // Execute function via API
    nlohmann::json exec_request = {
        {"function", "analyze_data"},
        {"parameters", {
            {"data_source", "test_data.csv"},
            {"analysis_type", "basic"}
        }}
    };
    
    auto exec_response = http_client_->post(
        "http://localhost:8081/v1/agents/" + agent_id + "/execute",
        exec_request.dump());
    
    ASSERT_EQ(exec_response.status_code, 200);
    
    auto exec_data = nlohmann::json::parse(exec_response.body);
    EXPECT_TRUE(exec_data["success"].get<bool>());
}

}
```

### Performance Testing

```cpp
// tests/performance/benchmark_agent_execution.cpp
#include <benchmark/benchmark.h>
#include "agent/core/agent_core.hpp"

namespace kolosal::agents::benchmark {

// Benchmark agent function execution
static void BM_AgentFunctionExecution(benchmark::State& state) {
    // Setup
    AgentConfig config;
    config.name = "benchmark_agent";
    config.type = "specialist";
    
    auto agent = std::make_unique<AgentCore>("benchmark_agent", "specialist", 
                                           AgentRole::SPECIALIST);
    agent->initialize();
    agent->start();
    
    AgentData params;
    params.set("test_data", "benchmark_data");
    
    // Benchmark loop
    for (auto _ : state) {
        auto result = agent->execute_function("analyze_data", params);
        benchmark::DoNotOptimize(result);
    }
    
    // Metrics
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_AgentFunctionExecution)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1000)
    ->Threads(1);

// Benchmark concurrent agent execution
static void BM_ConcurrentAgentExecution(benchmark::State& state) {
    const int num_agents = state.range(0);
    
    // Setup multiple agents
    std::vector<std::unique_ptr<AgentCore>> agents;
    for (int i = 0; i < num_agents; ++i) {
        auto agent = std::make_unique<AgentCore>(
            "benchmark_agent_" + std::to_string(i), 
            "specialist", AgentRole::SPECIALIST);
        agent->initialize();
        agent->start();
        agents.push_back(std::move(agent));
    }
    
    AgentData params;
    params.set("test_data", "benchmark_data");
    
    // Benchmark concurrent execution
    for (auto _ : state) {
        std::vector<std::future<FunctionResult>> futures;
        
        for (auto& agent : agents) {
            futures.push_back(std::async(std::launch::async, 
                [&agent, &params]() {
                    return agent->execute_function("analyze_data", params);
                }));
        }
        
        for (auto& future : futures) {
            auto result = future.get();
            benchmark::DoNotOptimize(result);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * num_agents);
}

BENCHMARK(BM_ConcurrentAgentExecution)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(16)
    ->Unit(benchmark::kMillisecond);

}

BENCHMARK_MAIN();
```

## Contributing Guidelines

### Code Style

```cpp
// C++ coding standards
namespace kolosal::agents {

// Class naming: PascalCase
class AgentCore {
public:
    // Public methods: snake_case
    bool initialize();
    void start_execution();
    
    // Getters/setters: get_/set_ prefix
    std::string get_name() const;
    void set_name(const std::string& name);
    
private:
    // Private members: trailing underscore
    std::string agent_name_;
    std::unique_ptr<FunctionManager> function_manager_;
    
    // Private methods: snake_case
    bool validate_configuration();
    void cleanup_resources();
};

// Constants: SCREAMING_SNAKE_CASE
const int MAX_AGENTS = 100;
const std::chrono::seconds DEFAULT_TIMEOUT{30};

// Enums: PascalCase with SCREAMING_SNAKE_CASE values
enum class AgentState {
    UNINITIALIZED,
    INITIALIZED,
    RUNNING,
    PAUSED,
    STOPPED,
    ERROR
};

}
```

### Code Formatting

```bash
# Use clang-format with provided configuration
clang-format -i src/**/*.cpp include/**/*.hpp

# Pre-commit hook for formatting
#!/bin/bash
# .git/hooks/pre-commit
files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|hpp)$')
if [ ! -z "$files" ]; then
    clang-format -i $files
    git add $files
fi
```

### Documentation Standards

```cpp
/**
 * @file agent_core.hpp
 * @brief Core agent functionality
 * @version 2.0.0
 * @author Your Name
 * @date 2025
 * 
 * Detailed description of the file's purpose and functionality.
 * This file contains the core agent implementation for the
 * Kolosal Agent System v2.0.
 */

namespace kolosal::agents {

/**
 * @brief Core agent implementation
 * 
 * The AgentCore class provides the fundamental functionality for
 * all agent types in the system. It handles initialization, 
 * lifecycle management, function execution, and memory operations.
 * 
 * @example Basic Usage:
 * @code
 * auto agent = std::make_unique<AgentCore>("test_agent", "specialist", 
 *                                         AgentRole::SPECIALIST);
 * agent->initialize();
 * agent->start();
 * 
 * AgentData params;
 * params.set("input", "test_data");
 * auto result = agent->execute_function("process_data", params);
 * @endcode
 * 
 * @see AgentManager for agent lifecycle management
 * @see FunctionManager for function registration
 */
class KOLOSAL_AGENT_API AgentCore {
public:
    /**
     * @brief Construct a new Agent Core object
     * 
     * @param name The name of the agent (must be unique within the system)
     * @param type The type of the agent (e.g., "coordinator", "analyst")
     * @param role The role of the agent in the system
     * 
     * @throws std::invalid_argument if name is empty or role is invalid
     */
    AgentCore(const std::string& name, const std::string& type, AgentRole role);
    
    /**
     * @brief Initialize the agent
     * 
     * Performs all necessary initialization steps including:
     * - Configuration validation
     * - Memory system setup
     * - Function registry initialization
     * - Resource allocation
     * 
     * @return true if initialization was successful
     * @return false if initialization failed
     * 
     * @note This method must be called before start()
     */
    virtual bool initialize();
};

}
```

### Pull Request Process

1. **Fork and Branch**:
   ```bash
   git fork https://github.com/kolosalai/kolosal-agent.git
   git checkout -b feature/your-feature-name
   ```

2. **Development**:
   - Follow coding standards
   - Add comprehensive tests
   - Update documentation
   - Ensure all tests pass

3. **Testing**:
   ```bash
   # Run all tests
   ctest --output-on-failure
   
   # Run specific test categories
   ctest -R "unit_tests"
   ctest -R "integration_tests"
   
   # Run performance benchmarks
   ./build-dev/benchmark_tests
   ```

4. **Submit PR**:
   - Clear title and description
   - Link to related issues
   - Include test results
   - Request appropriate reviewers

## Build System

### CMake Configuration

```cmake
# Custom CMake configuration for extensions
# CMakeLists.txt for custom components

cmake_minimum_required(VERSION 3.14)
project(KolosalAgentCustom VERSION 2.0.0)

# Find required packages
find_package(kolosal-agent REQUIRED)

# Custom agent library
add_library(custom-agents SHARED
    src/custom_agent.cpp
    src/custom_functions.cpp
    src/custom_tools.cpp
)

target_link_libraries(custom-agents
    kolosal::agent-core
    kolosal::agent-services
    kolosal::tools
)

# Custom tests
if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    
    add_executable(custom-agent-tests
        tests/test_custom_agent.cpp
        tests/test_custom_functions.cpp
    )
    
    target_link_libraries(custom-agent-tests
        custom-agents
        GTest::gtest_main
    )
    
    add_test(NAME CustomAgentTests COMMAND custom-agent-tests)
endif()

# Installation
install(TARGETS custom-agents
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)
```

### Build Scripts

```bash
#!/bin/bash
# scripts/build-custom.sh - Custom component build script

set -e

PROJECT_ROOT=$(dirname "$(dirname "$(realpath "$0")")")
BUILD_DIR="$PROJECT_ROOT/build-custom"

echo "Building custom Kolosal Agent components..."

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=ON \
    -DKOLOSAL_ROOT="$PROJECT_ROOT"

# Build
cmake --build . --parallel

# Test
if [ "$RUN_TESTS" = "1" ]; then
    echo "Running tests..."
    ctest --output-on-failure
fi

# Install
if [ "$INSTALL" = "1" ]; then
    echo "Installing..."
    cmake --install .
fi

echo "Build completed successfully!"
```

## Debugging and Profiling

### Debug Build Configuration

```bash
# Debug build with sanitizers
mkdir build-debug && cd build-debug
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_ASAN=ON \
    -DENABLE_UBSAN=ON \
    -DENABLE_MSAN=ON \
    -DCMAKE_CXX_FLAGS="-g -O0 -fno-omit-frame-pointer"

# Build with debug info
cmake --build . --parallel
```

### Debugging Tools

```bash
# GDB debugging
gdb --args ./kolosal-agent-unified --config config.yaml --debug

# Valgrind memory analysis
valgrind --tool=memcheck \
         --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./kolosal-agent-unified

# AddressSanitizer
export ASAN_OPTIONS=abort_on_error=1:fast_unwind_on_malloc=0
./kolosal-agent-unified

# Thread sanitizer
export TSAN_OPTIONS=halt_on_error=1
./kolosal-agent-unified
```

### Performance Profiling

```bash
# CPU profiling with perf (Linux)
perf record -g ./kolosal-agent-unified
perf report

# Memory profiling with massif
valgrind --tool=massif ./kolosal-agent-unified
massif-visualizer massif.out.*

# Custom profiling points in code
#include <chrono>

class PerformanceProfiler {
public:
    static void start_timer(const std::string& name) {
        timers_[name] = std::chrono::high_resolution_clock::now();
    }
    
    static void end_timer(const std::string& name) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - timers_[name]);
        
        ServerLogger::logInfo("Timer %s: %lld μs", name.c_str(), duration.count());
    }
    
private:
    static std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> timers_;
};

// Usage in performance-critical code
PerformanceProfiler::start_timer("agent_execution");
auto result = agent->execute_function(function_name, params);
PerformanceProfiler::end_timer("agent_execution");
```

This developer guide provides comprehensive information for extending, modifying, and contributing to the Kolosal Agent System v2.0. Use it as a reference for implementing custom functionality and following best practices.
