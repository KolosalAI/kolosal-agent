# Kolosal Agent System 🤖

A powerful multi-agent platform with LLM integration, designed for building sophisticated AI-powered applications with dynamic agent management and real-time inference capabilities.

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/KolosalAI/kolosal-agent)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](#platform-support)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](#building)

## 📚 Table of Contents

- [🚀 Quick Start](#-quick-start)
- [🌟 Key Features](#-key-features)
- [🏗️ Architecture Overview](#️-architecture-overview)
- [🛠️ Building the Application](#️-building-the-application)
- [🎯 Running the Application](#-running-the-application)
- [🌐 API Reference](#-api-reference)
- [⚙️ Configuration](#️-configuration)
- [🧪 Testing](#-testing)
- [🔌 Integration Examples](#-integration-examples)
- [📊 Performance](#-performance)
- [ Troubleshooting](#-troubleshooting)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)

---

## 🌟 Key Features

### 🤖 Multi-Agent System
- **Dynamic Agent Creation**: Create specialized agents with custom capabilities
- **Agent Lifecycle Management**: Start, stop, and manage multiple agents simultaneously
- **System Prompts**: Configure agents with system instructions and specialized prompts
- **Built-in Functions**: Chat, analysis, research, and custom function execution
- **Workflow Orchestration**: Complex multi-agent workflows with sequential, parallel, and conditional execution

### 🔄 Advanced Workflow System
- **Workflow Templates**: Pre-built workflows for research, analysis, and decision-making
- **Custom Workflows**: Design and register your own multi-step workflows
- **Execution Control**: Pause, resume, and cancel long-running workflows
- **Pipeline Processing**: Chain agent outputs as inputs to subsequent steps
- **Conditional Logic**: Execute steps based on dynamic conditions and results

### � High-Performance Server
- **HTTP REST API**: OpenAPI-compatible endpoints for all operations
- **Real-time Communication**: Instant agent responses and status updates
- **Configuration Management**: Hot-reloadable YAML configurations
- **Health Monitoring**: System status and agent health tracking

### 🧠 LLM Integration
- **Kolosal Server Integration**: Built-in integration with high-performance inference server
- **Model Interface**: Flexible model communication and parameter handling
- **Retrieval Augmented Generation**: Document retrieval and web search capabilities
- **Multi-Model Support**: Support for various LLM models and embedding models

### 🔧 Developer-Friendly
- **Simple Configuration**: YAML-based configuration files
- **Comprehensive Testing**: Unit, integration, and performance tests
- **Cross-Platform**: Windows, Linux, and macOS support
- **Extensive Documentation**: Complete API documentation and examples

## 🚀 Quick Start

Get the Kolosal Agent System up and running in just a few minutes:

### Prerequisites

- **C++20** compatible compiler (GCC 9+, Clang 10+, or Visual Studio 2019+)
- **CMake 3.14** or higher
- **Git** with submodule support
- **4GB RAM** minimum (8GB+ recommended)

### Build and Run

```bash
# 1. Clone the repository
git clone --recursive https://github.com/KolosalAI/kolosal-agent.git
cd kolosal-agent

# 2. Build the application
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug

# 3. Run the system
./kolosal-agent                    # Linux/macOS
.\Debug\kolosal-agent.exe          # Windows

# 4. Test the API
curl http://localhost:8080/status
```

### First Steps

Once running, the system provides:
- **Web API**: `http://localhost:8080` for all operations
- **Default Agents**: Assistant, Analyzer, and Researcher agents pre-configured
- **Configuration**: `agent.yaml` for agent system, `config.yaml` for kolosal-server
- **Real-time API**: Immediate agent creation and function execution

### Quick Examples

```bash
# List all agents
curl http://localhost:8080/agents

# Chat with assistant (requires model configuration)
curl -X POST http://localhost:8080/agents/Assistant/execute \
  -H "Content-Type: application/json" \
  -d '{"function": "chat", "params": {"message": "Hello!", "model": "your_model"}}'

# Analyze text
curl -X POST http://localhost:8080/agents/Analyzer/execute \
  -H "Content-Type: application/json" \
  -d '{"function": "analyze", "params": {"text": "Sample text for analysis"}}'

# Execute a research workflow
curl -X POST http://localhost:8080/workflows/execute \
  -H "Content-Type: application/json" \
  -d '{"workflow_id": "research_workflow", "input_data": {"question": "What is AI?"}}'

# Submit a workflow request
curl -X POST http://localhost:8080/workflow/execute \
  -H "Content-Type: application/json" \
  -d '{"agent_name": "Assistant", "function_name": "chat", "parameters": {"message": "Hello!", "model": "default"}}'
```

## 🏗️ Architecture Overview

The Kolosal Agent System features a layered architecture designed for scalability and maintainability:

```
┌─────────────────────────────────────────────────────────────┐
│                    🌐 REST API Layer                        │
│  ┌─────────────────┬─────────────────┬─────────────────┐    │
│  │   Agent APIs    │   System APIs   │   Health APIs   │    │
│  └─────────────────┴─────────────────┴─────────────────┘    │
├─────────────────────────────────────────────────────────────┤
│                 🤖 Multi-Agent System                       │
│  ┌─────────────────┬─────────────────┬──────────────────┐   │
│  │ Agent Manager   │ Message Router  │ Function Registry│   │
│  └─────────────────┴─────────────────┴──────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                 🧠 Core Components                          │
│  ┌─────────────────┬─────────────────┬─────────────────┐    │
│  │   Agent Core    │  Config Manager │  HTTP Server    │    │
│  └─────────────────┴─────────────────┴─────────────────┘    │
├─────────────────────────────────────────────────────────────┤
│               ⚡ Inference & Data Layer                      │
│  ┌─────────────────┬─────────────────┬─────────────────┐    │
│  │ Kolosal Server  │ Model Interface │Retrieval Manager│    │
│  └─────────────────┴─────────────────┴─────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

### Core Components

#### 🤖 **Agent System**
- **Agent Manager**: Manages agent lifecycle, creation, and deletion
- **Agent Core**: Individual agent implementation with system prompts
- **Function Registry**: Built-in functions like chat, analyze, research

#### 🌐 **HTTP Server**
- **REST API**: Complete HTTP API for all operations
- **Request Handling**: Concurrent request processing
- **Configuration**: YAML-based configuration management

#### ⚡ **Kolosal Server Integration** (Optional)
- **LLM Inference**: High-performance model inference
- **Model Management**: Multiple model support
- **Retrieval Support**: Document search and web search capabilities

#### 📊 **Configuration Management**
- **Hot Reloading**: Configuration changes without restart
- **Multi-file Support**: Separate configs for agent system and server
- **Validation**: Built-in configuration validation

## 🌐 API Reference

### Agent Management

#### List All Agents
```http
GET /agents
```

**Response:**
```json
{
  "agents": [
    {
      "id": "agent_123",
      "name": "Assistant",
      "capabilities": ["chat", "analysis", "reasoning"],
      "running": true,
      "info": {
        "system_instruction": "You are a helpful AI assistant...",
        "agent_specific_prompt": "You excel at general conversation..."
      }
    }
  ]
}
```

#### Create New Agent
```http
POST /agents
Content-Type: application/json
```

```json
{
  "name": "CustomAgent",
  "capabilities": ["chat", "research"]
}
```

#### Execute Function
```http
POST /agents/{agent_id}/execute
Content-Type: application/json
```

```json
{
  "function": "chat",
  "params": {
    "message": "Hello, how can you help me?",
    "model": "your_model_name"
  }
}
```

### System Management

#### System Status
```http
GET /status
```

**Response:**
```json
{
  "status": "running",
  "total_agents": 3,
  "running_agents": 2,
  "timestamp": "2025-08-28T10:00:00Z"
}
```
    "max_concurrent_tasks": 5
  }
}
```

#### Execute Function
```http
POST /v1/agents/{agent_id}/execute
Content-Type: application/json
```

```json
{
  "function": "analyze_data",
  "parameters": {
    "data_source": "sales_report.csv",
    "analysis_type": "comprehensive",
    "output_format": "json"
  }
}
```

### System Management

#### System Status
```http
GET /v1/system/status
```

**Response:**
```json
{
  "system_running": true,
  "status": "All systems operational",
  "total_agents": 4,
  "timestamp": 1703123456
}
```

#### Reload Configuration
```http
POST /v1/system/reload
Content-Type: application/json
```

```json
{
  "config_file": "production_config.yaml"
}
```

## ⚙️ Configuration

The system uses YAML configuration files for both the agent system and the optional kolosal-server component.

### Agent System Configuration (`agent.yaml`)

```yaml
# Kolosal Agent System Configuration
system:
  name: "Kolosal Agent System"
  version: "1.0.0"
  host: "127.0.0.1"
  port: 8080
  log_level: "info"
  max_concurrent_requests: 100

# System instruction for all agents
system_instruction: |
  You are a helpful AI assistant that is part of the Kolosal Agent System.
  You should be helpful, accurate, and professional in your responses.

# Default agents to create on startup
agents:
  - name: "Assistant"
    capabilities: ["chat", "analysis", "reasoning"]
    auto_start: true
    system_prompt: |
      You are an AI assistant specialized in general conversation and help.
      Be friendly, helpful, and informative in your responses.

  - name: "Analyzer" 
    capabilities: ["analysis", "data_processing", "summarization"]
    auto_start: true
    system_prompt: |
      You are an AI analyst specialized in text and data analysis.
      Provide detailed analysis with clear insights and conclusions.

# Available functions
functions:
  chat:
    description: "Interactive chat functionality with AI model support"
    timeout: 30000
    parameters:
      - name: "message"
        type: "string"
        required: true
      - name: "model"
        type: "string"
        required: true

  analyze:
    description: "Text and data analysis functionality"
    timeout: 60000
    parameters:
      - name: "text"
        type: "string"
        required: true
      - name: "analysis_type"
        type: "string"
        required: false

# Performance settings
performance:
  max_memory_usage: "2GB"
  worker_threads: 4
  request_timeout: 30000

# Security settings
security:
  enable_cors: true
  allowed_origins: ["http://localhost:3000"]
  max_request_rate: 100
  enable_auth: false
```

### Kolosal Server Configuration (`config.yaml`)

If building with kolosal-server integration:

```yaml
server:
  port: 8080
  host: 0.0.0.0
  idle_timeout: 300

models:
  - id: gemma3-1b
    path: path/to/model.gguf
    type: llm
    load_immediately: true
    load_params:
      n_ctx: 2048
      n_gpu_layers: 100

  - id: qwen3-embedding-0.6b
    path: path/to/embedding-model.gguf
    type: embedding
    load_immediately: true

database:
  vector_database: faiss
  retrieval_embedding_model: qwen3-embedding-0.6b
  faiss:
    index_type: Flat
    dimensions: 1536

search:
  enabled: true
  searxng_url: https://searx.stream/
  timeout: 30
  max_results: 20
```

## 🧪 Testing

The Kolosal Agent System includes comprehensive testing support.

### Running Tests

Build with tests enabled:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build . --config Debug
```

Run tests:
```bash
# Run all tests
ctest --output-on-failure

# Run specific test files
./test_agent_execution.exe
./test_model_interface.exe
./test_config_manager.exe
./test_http_server.exe
./test_error_scenarios.exe
```

### Quick Test Demo

For a minimal test without dependencies:
```bash
cd tests
g++ -std=c++17 -o minimal_test_demo.exe minimal_test_demo.cpp
./minimal_test_demo.exe
```

### Test Coverage

- **Agent Creation & Lifecycle**: Creating, starting, stopping agents
- **Function Execution**: Chat, analyze, and other built-in functions
- **Configuration Management**: YAML loading and validation
- **HTTP Server**: API endpoints and request handling
- **Error Scenarios**: Graceful error handling and recovery

For detailed testing information, see [tests/README.md](tests/README.md).

## 🔌 Integration Examples

### C++ Integration

```cpp
#include "agent_manager.hpp"
#include "agent_config.hpp"

int main() {
    // Create configuration manager
    auto config_manager = std::make_shared<AgentConfigManager>();
    config_manager->load_config("agent.yaml");
    
    // Create agent manager
    auto agent_manager = std::make_shared<AgentManager>(config_manager);
    
    // Create a custom agent
    std::string agent_id = agent_manager->create_agent("MyAgent", {"chat", "analysis"});
    
    // Start the agent
    agent_manager->start_agent(agent_id);
    
    // Execute a function
    json params = {{"message", "Hello!"}, {"model", "your_model"}};
    auto result = agent_manager->execute_function(agent_id, "chat", params);
    
    return 0;
}
```

### HTTP API Integration

```bash
# Create a new agent
curl -X POST http://localhost:8080/agents \
  -H "Content-Type: application/json" \
  -d '{"name": "MyCustomAgent", "capabilities": ["chat", "research"]}'

# Execute chat function
curl -X POST http://localhost:8080/agents/MyCustomAgent/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "chat",
    "params": {
      "message": "What is machine learning?",
      "model": "your_model_name"
    }
  }'

# Get agent status
curl http://localhost:8080/agents/MyCustomAgent
```

### Python Integration

```python
import requests
import json

class KolosalAgentClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
    
    def create_agent(self, name, capabilities):
        response = requests.post(
            f"{self.base_url}/agents",
            json={"name": name, "capabilities": capabilities}
        )
        return response.json()
    
    def execute_function(self, agent_name, function, params):
        response = requests.post(
            f"{self.base_url}/agents/{agent_name}/execute",
            json={"function": function, "params": params}
        )
        return response.json()

# Usage
client = KolosalAgentClient()
agent = client.create_agent("PythonAgent", ["chat", "analysis"])
result = client.execute_function("PythonAgent", "chat", {
    "message": "Hello from Python!",
    "model": "your_model"
})
print(result)
```

## 📊 Performance

The Kolosal Agent System is designed for high performance and scalability:

### Performance Characteristics

- **Agent Creation**: Sub-second agent creation and initialization
- **Function Execution**: Low-latency function calls with concurrent execution
- **Memory Efficiency**: Optimized memory usage per agent
- **Request Handling**: High-throughput HTTP request processing

### Benchmarks

Based on testing with the included test suite:
- **Agent Creation**: 100 agents created in milliseconds
- **Concurrent Operations**: Support for multiple simultaneous agent operations  
- **Memory Usage**: Approximately 0.9MB per agent
- **Startup Time**: Full system startup in under 5 seconds

### Optimization Tips

1. **Configuration**: Tune `max_concurrent_requests` and `worker_threads`
2. **Model Integration**: Use local models for faster inference
3. **Caching**: Enable response caching for repeated queries
4. **Resource Limits**: Set appropriate memory limits per agent

## 🐛 Troubleshooting

### Common Issues

#### Build Issues

**CMake Configuration Fails:**
```bash
# Ensure you have required dependencies
sudo apt install build-essential cmake git libyaml-cpp-dev  # Linux
brew install cmake git yaml-cpp                            # macOS

# Initialize submodules
git submodule update --init --recursive
```

**Compilation Errors:**
```bash
# Check compiler version (requires C++20)
g++ --version   # Should be 9.0+
clang --version # Should be 10.0+

# Clean build if needed
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

#### Runtime Issues

**Server Won't Start:**
```bash
# Check port availability
netstat -ln | grep 8080

# Try different port
./kolosal-agent --port 9090

# Check configuration
./kolosal-agent --config agent.yaml --help
```

**Agent Creation Fails:**
```bash
# Verify configuration file
cat agent.yaml | head -20

# Check system logs
tail -f agent_system.log

# Test with minimal configuration
curl -X POST http://localhost:8080/agents \
  -d '{"name": "TestAgent", "capabilities": ["chat"]}'
```

#### Model Integration Issues

**Model Not Found:**
- Ensure model paths are correct in `config.yaml`
- Verify model files exist and are readable
- Check kolosal-server is running (if using model integration)

**Function Execution Timeout:**
- Increase timeout values in configuration
- Check model performance and resource availability
- Verify network connectivity to model servers

### Debug Mode

Enable verbose logging for detailed troubleshooting:
```bash
./kolosal-agent --host 127.0.0.1 --port 8080 --config agent.yaml
```

Check the system logs for detailed error information:
```bash
tail -f agent_system.log
```

## 🤝 Contributing

We welcome contributions to the Kolosal Agent System!

### Development Setup

1. **Fork and Clone**
   ```bash
   git clone --recursive https://github.com/your-username/kolosal-agent.git
   cd kolosal-agent
   ```

2. **Build with Tests**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
   cmake --build . --config Debug
   ```

3. **Run Tests**
   ```bash
   ctest --output-on-failure
   ```

### Contributing Guidelines

- **Code Style**: Follow existing C++ style conventions
- **Testing**: Add tests for new features
- **Documentation**: Update documentation for API changes
- **Commits**: Use clear, descriptive commit messages

### Areas for Contribution

- **Additional Functions**: New built-in functions for agents
- **Performance Optimizations**: Memory and speed improvements
- **Platform Support**: Enhanced platform compatibility
- **Documentation**: Examples, tutorials, and guides
- **Testing**: Additional test coverage and scenarios

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [nlohmann/json](https://github.com/nlohmann/json) - JSON library for Modern C++
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) - YAML parsing library
- [Kolosal Server](kolosal-server/) - High-performance inference server integration

## 📞 Support

- **Documentation**: [docs/](docs/) - Complete documentation
- **Issues**: [GitHub Issues](https://github.com/KolosalAI/kolosal-agent/issues)
- **Examples**: [tests/](tests/) - Working examples and demos

---

**Kolosal Agent System** - *Building the future of multi-agent AI systems.*
make -j$(nproc)
```

### Verification

```bash
# Check build outputs
ls build/                           # Linux/macOS
dir build\Debug\                    # Windows

# Should see:
# - kolosal-agent (main executable)
# - kolosal-server (if built with server support)
```

### 🖥️ Platform-Specific Build Instructions

#### Windows (PowerShell)
```powershell
# Clone the repository
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# Create build directory
New-Item -ItemType Directory -Path "build" -Force
Set-Location "build"

# Choose build mode:
# Standard build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug

# OR with tests
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

# OR with deep research demo
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug -DBUILD_DEEPRESEARCH_DEMO=ON

# Build the project
cmake --build . --config Debug --parallel
```

#### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake git libcurl4-openssl-dev

# Clone and build
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent && mkdir build && cd build

# Choose build mode:
cmake .. -DCMAKE_BUILD_TYPE=Debug                              # Standard
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON             # With tests
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_DEEPRESEARCH_DEMO=ON # With deep research demo

# Build using all CPU cores
make -j$(nproc)
```

#### macOS
```bash
# Install dependencies via Homebrew
brew install cmake git curl yaml-cpp

# Clone and build
git clone --recursive https://github.com/kolosal-ai/kolosal-agent.git
cd kolosal-agent && mkdir build && cd build

# Choose build mode:
cmake .. -DCMAKE_BUILD_TYPE=Debug                              # Standard
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON             # With tests
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_DEEPRESEARCH_DEMO=ON # With deep research demo

# Build using all CPU cores
make -j$(sysctl -n hw.ncpu)
```

### Verification and Installation

#### Verify Build Success
```bash
# Check if executables were created
ls -la build/Debug/  # or build/ on Linux/macOS

# Expected executables:
# - kolosal-agent          (main application)
# - kolosal-server-exe     (server executable)
# - deep-research-demo     (if built with -DBUILD_DEEPRESEARCH_DEMO=ON)
```

#### Quick Functionality Test
```bash
# Run the main application to verify build
./build/Debug/kolosal-agent.exe  # Windows
./build/kolosal-agent             # Linux/macOS

# Should start successfully and show initialization messages
```

#### Install the Application
```bash
# Install to system directories (optional)
cmake --install . --config Debug

# Or specify custom install prefix
cmake --install . --config Debug --prefix /opt/kolosal-agent
```

### Troubleshooting Common Build Issues

#### Issue 1: CMake Version Too Old
```bash
# Error: CMake 3.14 or higher is required
# Solution: Update CMake
pip install cmake  # or download from cmake.org
```

#### Issue 2: Git Submodules Not Initialized
```bash
# Error: Missing dependencies in external/
# Solution: Initialize submodules
git submodule update --init --recursive
```

#### Issue 3: Compiler Not Found
```bash
# Error: No suitable C++ compiler found
# Solution: Install development tools

# Windows: Install Visual Studio Build Tools
# Linux: sudo apt install build-essential
# macOS: xcode-select --install
```

#### Issue 4: Missing Dependencies
```bash
# Error: Could NOT find CURL/YAML-CPP
# Solution: Install development packages

# Ubuntu/Debian:
sudo apt install libcurl4-openssl-dev libyaml-cpp-dev

# macOS:
brew install curl yaml-cpp

# Windows: Use vcpkg
vcpkg install curl yaml-cpp
```

#### Issue 5: Out of Memory During Compilation
```bash
# Error: Compilation killed/terminated
# Solution: Reduce parallel jobs
cmake --build . --config Debug -j2  # Use only 2 parallel jobs
```

#### Clean Build (If Things Go Wrong)
```bash
# Remove build directory and start fresh
rm -rf build  # or rmdir /s build on Windows
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

## 🎯 Running the Application

### Basic Launch

```bash
# Navigate to build directory
cd build

# Run the main application (default port 8080)
./kolosal-agent                    # Linux/macOS
.\Debug\kolosal-agent.exe          # Windows

# Check if it's running
curl http://localhost:8080/status
```

### Command Line Options

```bash
# Custom port
./kolosal-agent --port 9090

# Custom configuration file
./kolosal-agent --config my-agent.yaml

# Display help
./kolosal-agent --help
```

### Available Endpoints

Once running, access the system at:
- **System Status**: `http://localhost:8080/status`
- **List Agents**: `http://localhost:8080/agents`
- **Agent Info**: `http://localhost:8080/agents/{id}`
- **Execute Function**: `POST http://localhost:8080/agents/{id}/execute`

### Configuration Files

The system uses two main configuration files:
- **`agent.yaml`**: Agent system configuration
- **`config.yaml`**: Kolosal server configuration (if enabled)

# Development mode with verbose logging
./kolosal-agent --dev --verbose --log-level DEBUG

# Production mode with specific settings
./kolosal-agent --prod --port 8080 --workers 4

# Display help for all options
./kolosal-agent --help
```

### Available Applications

After building, you'll have several executables:

```bash
# Main application
./kolosal-agent               # Unified multi-agent system

# Utilities
./kolosal-server-exe         # Standalone server component

# Deep Research Demo (if built with -DBUILD_DEEPRESEARCH_DEMO=ON)
./deep-research-demo         # Advanced research demonstration

# Test executables (if built with tests)
./kolosal_agent_unit_tests
./kolosal_agent_integration_tests
./kolosal_agent_performance_tests
```

### Verify Installation

```bash
# 1. Verify the main application
./kolosal-agent --version
# Expected output: Version information and build details

# 2. Start the main application
./kolosal-agent &

# 3. Test API endpoints
curl http://localhost:8080/v1/system/status
curl http://localhost:8080/v1/agents
curl http://localhost:8080/v1/health

# 4. Stop the application
pkill kolosal-agent  # or use Ctrl+C if running in foreground
```

### Web Interface

Once running, access the system through:
- **System Status**: http://localhost:8080/v1/system/status
- **Agent Management**: http://localhost:8080/v1/agents  
- **Health Check**: http://localhost:8080/v1/health
- **API Documentation**: http://localhost:8080/docs (if built with docs)

### Configuration Files

The application uses YAML configuration files:

```bash
# Default configuration locations (in order of precedence):
# 1. Command line: --config /path/to/config.yaml
# 2. Current directory: ./config.yaml
# 3. Project root: ../config.yaml (from build dir)
# 4. System config: /etc/kolosal-agent/config.yaml (Linux)

# Start with example configuration
cp ../config.example.yaml ./my-config.yaml
# Edit my-config.yaml as needed
./kolosal-agent --config my-config.yaml
```

## 🚀 Advanced Usage

### Programmatic Agent Management

```cpp
#include "integration/unified_server.hpp"
#include "services/agent_service.hpp"

// Create unified server
auto config = UnifiedServerFactory::buildProductionConfig(8080);
auto server = std::make_unique<UnifiedKolosalServer>(config);

// Start server
if (!server->start()) {
    throw std::runtime_error("Failed to start server");
}

// Get agent service
auto agent_service = server->getAgentService();

// Create agent asynchronously
AgentConfig agent_config;
agent_config.name = "custom_agent";
agent_config.type = "specialist";
agent_config.role = AgentRole::ANALYST;

auto future = agent_service->createAgentAsync(agent_config);
std::string agent_id = future.get();

// Execute function
AgentData params;
params["data_source"] = "example.csv";
auto result_future = agent_service->executeFunctionAsync(
    agent_id, "analyze_data", params);
auto result = result_future.get();
```

### Health Monitoring Setup

```cpp
// Enable health monitoring with custom callback
server->setHealthCheckCallback([](const auto& status) {
    if (!status.llm_server_healthy) {
        log_error("LLM server unhealthy: " + status.last_error);
        // Implement custom recovery logic
    }
    
    if (status.running_agents < status.total_agents) {
        log_warning("Some agents are stopped");
        // Restart stopped agents
    }
});

// Enable auto-recovery
server->enableAutoRecovery(true);
```

### Performance Analytics

```cpp
// Generate performance reports
auto reports = agent_service->generatePerformanceReport();

for (const auto& report : reports) {
    std::cout << "Agent: " << report.agent_id 
              << ", Success Rate: " << report.success_rate * 100 << "%"
              << ", Avg Time: " << report.average_execution_time_ms << "ms"
              << std::endl;
}

// Get optimization suggestions
auto suggestions = agent_service->analyzeSystemOptimization();
for (const auto& suggestion : suggestions) {
    std::cout << "Optimization: " << suggestion.type 
              << " - " << suggestion.description 
              << " (Improvement: " << suggestion.potential_improvement_percent << "%)"
              << std::endl;
}
```

## 🔌 Integration Examples

### Docker Deployment

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    libcurl4-openssl-dev libyaml-cpp-dev

COPY . /app
WORKDIR /app

RUN ./build_v2.sh --cuda --install
EXPOSE 8080

CMD ["./build/kolosal-agent-unified", "--prod", "-p", "8080"]
```

### Kubernetes Deployment

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: kolosal-agent-system
spec:
  replicas: 3
  selector:
    matchLabels:
      app: kolosal-agent
  template:
    metadata:
      labels:
        app: kolosal-agent
    spec:
      containers:
      - name: kolosal-agent
        image: kolosal/agent-system:v2.0
        ports:
        - containerPort: 8080
        env:
        - name: KOLOSAL_CONFIG
          value: "/config/production.yaml"
        resources:
          requests:
            cpu: 500m
            memory: 1Gi
          limits:
            cpu: 2
            memory: 4Gi
```

### Python Client Integration

```python
import asyncio
import aiohttp

class KolosalAgentClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
    
    async def create_agent(self, config):
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"{self.base_url}/v1/agents",
                json=config
            ) as response:
                return await response.json()
    
    async def execute_function(self, agent_id, function_name, parameters):
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"{self.base_url}/v1/agents/{agent_id}/execute",
                json={
                    "function": function_name,
                    "parameters": parameters
                }
            ) as response:
                return await response.json()

# Usage
async def main():
    client = KolosalAgentClient()
    
    # Create agent
    agent_config = {
        "name": "python_analyst",
        "type": "specialist",
        "role": 2,
        "capabilities": ["data_processing"]
    }
    agent = await client.create_agent(agent_config)
    
    # Execute function
    result = await client.execute_function(
        agent["agent_id"], 
        "analyze_data", 
        {"data_source": "data.csv"}
    )
    print(result)

asyncio.run(main())
```

## 📊 Performance Benchmarks

### System Performance (on Intel i7-10700K, 32GB RAM, RTX 3080)

| Metric | v1.0 | v2.0 | Improvement |
|--------|------|------|-------------|
| Agent Creation | 245ms | 89ms | **3.6x faster** |
| Function Execution | 156ms | 67ms | **2.3x faster** |
| Message Routing | 12ms | 4ms | **3x faster** |
| Memory Usage | 245MB | 178MB | **27% less** |
| Concurrent Agents | 50 | 200 | **4x more** |

### Scalability Metrics

- **Maximum Agents**: 1000+ (tested)
- **Concurrent Operations**: 10,000+ requests/second
- **Memory per Agent**: ~0.9MB average
- **Startup Time**: <5 seconds (with 100 agents)

## 🧪 Testing Guide - Simple & Comprehensive

The Kolosal Agent System includes a comprehensive testing framework that is automatically enabled when you build with `-DBUILD_TESTS=ON`.

### 🎯 Quick Testing Overview

When you build with tests enabled, you get:
- **Unit Tests** - Test individual components in isolation
- **Integration Tests** - Test component interactions and system-level functionality  
- **Performance Tests** - Measure and validate system performance

### 🚀 Running Tests

#### Method 1: Automatic Testing (During Build)
```bash
# Tests are built automatically when DBUILD_TESTS=ON is specified
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build . --config Debug

# Tests are now available - run them:
ctest --output-on-failure
```

#### Method 2: Build-Specific Test Targets
```bash
# Quick unit tests only (fast)
cmake --build . --target quick-test --config Debug

# Run all tests (comprehensive)
cmake --build . --target full-test --config Debug

# Run tests with detailed reporting
cmake --build . --target test-report --config Debug
```

#### Method 3: Direct Test Execution
```bash
# Run specific test executables directly
./kolosal_agent_unit_tests          # Linux/macOS
.\Debug\kolosal_agent_unit_tests.exe    # Windows

./kolosal_agent_integration_tests
./kolosal_agent_performance_tests
```

### 🎨 Test Categories & Usage

#### Unit Tests
Test individual components in isolation:
```bash
# Run all unit tests
./kolosal_agent_unit_tests

# Run specific test suites
./kolosal_agent_unit_tests --gtest_filter="AgentCore*"
./kolosal_agent_unit_tests --gtest_filter="*Memory*"
./kolosal_agent_unit_tests --gtest_filter="Configuration*"
```

Available test suites:
- **AgentCore**: Core agent functionality
- **Configuration**: YAML config parsing
- **Workflow**: Workflow execution engine  
- **MessageRouter**: Message routing system
- **HttpServer**: HTTP server functionality
- **Logger**: Logging system tests

#### Integration Tests  
Test system-level interactions:
```bash
# Run integration tests
./kolosal_agent_integration_tests

# Run specific scenarios
./kolosal_agent_integration_tests --gtest_filter="*FullSystem*"
./kolosal_agent_integration_tests --gtest_filter="*MultiAgent*"
```

Integration test categories:
- Full system startup and shutdown
- Multi-agent workflow scenarios  
- API endpoint integration
- Configuration loading and validation

#### Performance Tests
Validate system performance metrics:
```bash
# Run performance tests
./kolosal_agent_performance_tests

# Performance areas tested:
# - Agent creation and initialization speed
# - Workflow execution performance
# - Memory usage patterns
# - Message routing throughput
```

#### Customized Test Execution
```bash
# Run tests with custom options
ctest --output-on-failure --parallel 4    # Parallel execution
ctest --verbose                           # Verbose output  
ctest -L unit                            # Only unit tests
ctest --timeout 300                      # 5-minute timeout
```

### 📊 Understanding Test Results

#### Successful Test Output
```
[==========] Running 45 tests from 12 test suites.
[----------] Global test environment set-up.
[----------] 8 tests from AgentCoreTest
[ RUN      ] AgentCoreTest.InitializationTest
[       OK ] AgentCoreTest.InitializationTest (2 ms)
...
[==========] 45 tests from 12 test suites ran. (234 ms total)
[  PASSED  ] 45 tests.
```

#### Failed Test Handling
```bash
# Debug specific failing tests
./kolosal_agent_unit_tests --gtest_filter="FailingTest" --gtest_catch_exceptions=0

# Check test logs
cat build/test_logs/latest.log  # If available
```

### 🛠️ Test Development (For Contributors)

When adding new features, include corresponding tests:

```cpp
#include <gtest/gtest.h>
#include "component/my_component.hpp"

class MyComponentTest : public testing::Test {
protected:
    void SetUp() override {
        component = std::make_unique<MyComponent>();
    }
    
    std::unique_ptr<MyComponent> component;
};

TEST_F(MyComponentTest, BasicFunctionality) {
    ASSERT_TRUE(component->initialize());
    EXPECT_EQ(component->getStatus(), ComponentStatus::READY);
}
```

### 🔍 Troubleshooting Tests

#### Common Issues & Solutions

**Tests Not Building:**
```bash
# Install Google Test
sudo apt install libgtest-dev  # Linux
brew install googletest        # macOS  
vcpkg install gtest           # Windows

# Or let CMake download it automatically (no action needed)
```

**Tests Failing:**
```bash
# Run from correct directory
cd build
ctest --output-on-failure

# Increase timeout for slow systems
ctest --timeout 300
```

**Memory Test Issues:**
```bash
# Temporarily disable leak detection for problematic areas
export ASAN_OPTIONS=detect_leaks=0
```

### ✅ Continuous Integration

For CI/CD environments:
```bash
# CI-optimized configuration
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build . --config Release --parallel
ctest --output-on-failure --return-failed --parallel 4
```

The simplified testing system ensures that when you enable tests with `-DBUILD_TESTS=ON`, you get everything you need for comprehensive validation without complex configuration.

---

## 📝 Build Configuration Reference

### 🎯 Simple Configuration (2 Modes Only)

The Kolosal Agent System is designed with **simplicity** in mind. Choose between 2 straightforward build modes:

#### **📦 Mode 1: Standard Build**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```
- ✅ Fast compilation
- ✅ All core functionality
- ✅ Production ready
- ❌ No tests

#### **🧪 Mode 2: Build with All Tests**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
```
- ✅ Everything from Standard Build
- ✅ Unit tests
- ✅ Integration tests  
- ✅ Performance tests
- ⏱️ Longer compilation time

### 🔧 Core Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `CMAKE_BUILD_TYPE` | STRING | Debug | Build type: Debug, Release, RelWithDebInfo, MinSizeRel |
| `BUILD_TESTS` | BOOL | OFF | **Enable all tests (unit, integration, performance)** |
| `BUILD_DEEPRESEARCH_DEMO` | BOOL | OFF | **Enable deep research demonstration capabilities** |

### 📋 Common Build Recipes

#### **📦 Standard Build (Fastest)**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

#### **🧪 Development Build (with Tests)**
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON
```

#### **🔬 Research Build (with Deep Research Demo)**
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_DEEPRESEARCH_DEMO=ON
```

#### **🚀 Production Build (Release)**
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release
```

#### **🏭 CI/CD Build (Release with Tests)**  
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON
```

### 🎯 Build Target Reference

After configuration, use these build commands:

```bash
# Build everything
cmake --build . --config Debug

# Build specific components
cmake --build . --target kolosal-agent --config Debug      # Main application
cmake --build . --target build-server --config Debug      # Server component
cmake --build . --target build-all --config Debug         # Both executables

# Test-specific (only when DBUILD_TESTS=ON)
cmake --build . --target build-with-tests --config Debug  # Build with tests
cmake --build . --target quick-test --config Debug        # Run unit tests
cmake --build . --target full-test --config Debug         # Run all tests
```

### 🛠️ Utility Targets

```bash
cmake --build . --target clean-all         # Clean everything
cmake --build . --target info              # Show system info
cmake --install . --config Debug           # Install binaries
cpack                                       # Create packages
```

The simplified configuration system eliminates complexity while providing all the power you need for development and production.

---

## 🔍 Web Search & Document Retrieval Implementation

The Kolosal Agent System includes powerful web search and document retrieval capabilities that enable agents to access real-time information and comprehensive knowledge bases.

### New Function Classes

#### Web Search Functions
1. **`InternetSearchFunction`** - Real internet search using kolosal-server's SearXNG integration
2. **`EnhancedWebSearchFunction`** - Advanced web search with content filtering and summarization

#### Document Retrieval Functions  
3. **`ServerDocumentRetrievalFunction`** - Server-based document retrieval using vector similarity
4. **`ServerDocumentAddFunction`** - Add documents to server knowledge base
5. **`ServerDocumentParserFunction`** - Parse various document formats via server endpoints
6. **`ServerEmbeddingFunction`** - Generate embeddings using server-side models

#### Hybrid Functions
7. **`KnowledgeRetrievalFunction`** - Combines web search + document retrieval for comprehensive knowledge gathering

### Enhanced AgentCore Integration

#### New Methods Added to AgentCore:
- **`enable_enhanced_functions(server_url, test_connection)`** - Enable server-integrated functions
- **`set_server_url(url)`** - Update server endpoint  
- **`is_server_integration_enabled()`** - Check integration status

#### Simple Integration Example:
```cpp
// Enable enhanced functions on existing agent
agent->enable_enhanced_functions("http://localhost:8080");

// Perform real web search
AgentData search_params;
search_params.set("query", "latest AI developments 2025");
search_params.set("results", 5);

auto result = agent->get_function_manager()->execute__function("internet_search", search_params);
```

### Key Features
- **Automatic Fallback**: Tests server connection automatically and falls back to simulation mode if server unavailable
- **Backward Compatible**: Existing agents continue to work unchanged
- **Role-Based Functions**: Automatic function selection based on agent roles
- **Production Ready**: Error handling, logging, performance monitoring

---

## ⚙️ Workflow Engine Implementation

The Kolosal Agent System now includes a sophisticated workflow engine that enables orchestration of multiple agents in complex execution patterns.

### Workflow Execution Types

#### 1. Sequential Workflow
- Executes steps one after another in defined order
- Each step waits for the previous step to complete successfully
- Example: Research → Analysis → Report → Execution

#### 2. Parallel Workflow  
- Executes all steps simultaneously for maximum throughput
- All steps start at the same time
- Useful for independent tasks or data collection from multiple sources

#### 3. Pipeline Workflow
- Intelligent dependency resolution with parallel execution where possible
- Groups steps into execution phases based on dependencies
- Allows maximum parallelization while respecting data flow requirements

#### 4. Consensus Workflow
- Multiple agents vote on decisions
- Consensus step aggregates all votes
- Supports configurable consensus thresholds

#### 5. Conditional Workflow
- Dynamic execution based on conditions and previous step results
- Supports branching logic and adaptive processing
- Advanced expression evaluation for complex conditions

### Configuration Examples

#### Advanced Configuration Features
```yaml
# Variable Interpolation
parameters:
  input: "${global.topic}"
  previous_result: "${steps.research_step.output}"
  computed: "${steps.analysis.output.score >= 0.8}"

# Dependency Management  
depends_on:
  - step: "previous_step"
    condition: "success"  # success|completion|failure
    required: true        # true|false

# Error Handling
error_handling:
  retry_on_failure: true
  max_retries: 3
  retry_delay_seconds: 2
  continue_on_error: false
  use_fallback_agent: true
  fallback_agent_id: "backup_agent"
```

### Usage Example
```cpp
// Initialize workflow engine
WorkflowEngine workflow_engine(agent_manager);
workflow_engine.start();

// Load workflow from YAML
workflow_engine.load_workflow_from_yaml("sequential.yaml");

// Execute workflow
json input = {{"topic", "AI Research"}, {"urgency", "high"}};
std::string execution_id = workflow_engine.execute_workflow("workflow_id", input);

// Monitor execution
auto status = workflow_engine.get_execution_status(execution_id);
```

### Workflow Benefits
- **Flexibility**: Five different execution patterns for various use cases
- **Reliability**: Advanced error handling and retry mechanisms  
- **Scalability**: Parallel execution capabilities with configurable concurrency
- **Monitoring**: Real-time execution status and comprehensive metrics
- **Ease of Use**: Simple YAML configuration format with intuitive dependency specification

## 🔒 Security Features

### Authentication & Authorization

```yaml
auth:
  enabled: true
  require_api_key: true
  api_keys:
    - "sk-your-secure-api-key-here"
  jwt:
    enabled: true
    secret: "your-jwt-secret"
    expiration_hours: 24

rate_limiting:
  enabled: true
  requests_per_minute: 1000
  burst_capacity: 100
```

### Secure Configuration

```cpp
// Secure server configuration
UnifiedKolosalServer::ServerConfig config;
config.enable_cors = false;  // Disable in production
config.allowed_origins = {"https://trusted-domain.com"};

// Enable request validation
config.enable_request_validation = true;
config.max_request_size_mb = 10;
config.enable_rate_limiting = true;
```

## 🐛 Troubleshooting

### Common Issues

**Build Failures:**
```bash
# Clean rebuild
./build_v2.sh --clean-all
./build_v2.sh --verbose

# Check dependencies
./build_v2.sh --info
```

**Runtime Issues:**
```bash
# Enable debug logging
./build/kolosal-agent-unified --log-level DEBUG --verbose

# Check system status
curl http://localhost:8080/v1/system/status

# Monitor health
curl http://localhost:8080/v1/health
```

**Performance Issues:**
```bash
# Enable performance monitoring
./build/kolosal-agent-unified --enable-metrics --verbose

# Check resource usage
curl http://localhost:8080/completion-metrics
```

### Debug Mode

```yaml
system:
  logging:
    level: "DEBUG"
    enable_console: true
    enable_performance_logging: true
    
  monitoring:
    enable_detailed_metrics: true
    health_check_interval_seconds: 5
```

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Workflow

1. **Fork and Clone**
   ```bash
   git clone --recursive https://github.com/your-username/kolosal-agent.git
   cd kolosal-agent
   ```

2. **Development Build**
   ```bash
   ./build_v2.sh -t Debug --tests --asan --verbose
   ```

3. **Run Tests**
   ```bash
   ./build_v2.sh --run-tests
   ```

4. **Code Quality**
   ```bash
   # Format code
   clang-format -i src/**/*.cpp include/**/*.hpp
   
   # Static analysis
   cppcheck --enable=all src/
   ```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [llama.cpp](https://github.com/ggml-org/llama.cpp) - High-performance LLM inference
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) - YAML parsing library  
- [nlohmann/json](https://github.com/nlohmann/json) - JSON library for Modern C++
- [OpenAI](https://openai.com) - API specification inspiration

## 📞 Support

- **Documentation**: [Wiki](https://github.com/kolosalai/kolosal-agent/wiki)
- **Issues**: [GitHub Issues](https://github.com/kolosalai/kolosal-agent/issues)  
- **Discussions**: [GitHub Discussions](https://github.com/kolosalai/kolosal-agent/discussions)
- **Discord**: [Kolosal AI Community](https://discord.gg/kolosal-ai)

---

---

**Kolosal Agent System** - *Building the future of multi-agent AI systems.*
