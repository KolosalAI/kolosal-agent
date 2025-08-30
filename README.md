# Kolosal Agent System 🤖

A powerful multi-agent platform with LLM integration, designed for building sophisticated AI-powered applications with dynamic agent management and real-time inference capabilities.

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/KolosalAI/kolosal-agent)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](#building)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](#building)

## 📚 Table of Contents

- [🚀 Quick Start](#-quick-start)
- [🌟 Features](#-features)
- [🏗️ Architecture](#️-architecture)
- [🛠️ Installation](#️-installation)
- [⚙️ Configuration](#️-configuration)
- [🌐 API Reference](#-api-reference)
- [🧪 Testing](#-testing)
- [🔌 Integration](#-integration)
- [📊 Performance](#-performance)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)

---

## 🚀 Quick Start

Get the Kolosal Agent System up and running in minutes:

### Prerequisites
- **C++20** compatible compiler (GCC 9+, Clang 10+, or Visual Studio 2019+)
- **CMake 3.14** or higher
- **Git** with submodule support
- **4GB RAM** minimum (8GB+ recommended)

### Installation
```bash
# Clone the repository
git clone --recursive https://github.com/KolosalAI/kolosal-agent.git
cd kolosal-agent

# Build the application
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug

# Run the system
./kolosal-agent                    # Linux/macOS (uses workflow.yaml by default)
.\Debug\kolosal-agent.exe          # Windows (uses workflow.yaml by default)

# Run with custom configuration files
./kolosal-agent --workflow my-workflows.yaml    # Custom workflow config
./kolosal-agent --config my-agents.yaml --workflow my-workflows.yaml  # Custom configs
```

### Verification
```bash
# Test the API
curl http://localhost:8080/status

# List default agents
curl http://localhost:8080/agents
```

### First API Call
```bash
# Chat with the assistant
curl -X POST http://localhost:8080/agents/Assistant/execute \
  -H "Content-Type: application/json" \
  -d '{"function": "chat", "params": {"message": "Hello!", "model": "gemma3-1b"}}'

# Execute a workflow
curl -X POST http://localhost:8080/workflows/simple_research/execute \
  -H "Content-Type: application/json" \
  -d '{"input_data": {"query": "AI trends"}}'

# List available workflows
curl http://localhost:8080/workflows
```

## 🌟 Features

### 🤖 Multi-Agent System
- **Dynamic Agent Creation** - Create specialized agents with custom capabilities
- **Lifecycle Management** - Start, stop, and manage multiple agents simultaneously
- **Built-in Functions** - Chat, analysis, research, and custom function execution
- **System Prompts** - Configure agents with specialized instructions

### 🔄 Advanced Workflows
- **Multiple Execution Patterns** - Sequential, parallel, conditional, loop, and pipeline workflows
- **Workflow Templates** - Pre-built workflows for research, analysis, decision-making, and data processing
- **Custom Workflows** - Design and register your own multi-step workflows via YAML or API
- **Agent-LLM Pairing** - Configure specific LLM models for each agent in workflows
- **Execution Control** - Pause, resume, and cancel long-running workflows in real-time
- **Pipeline Processing** - Chain agent outputs as inputs to subsequent steps
- **Progress Monitoring** - Real-time execution tracking and detailed progress reporting
- **Conditional Logic** - Dynamic step execution based on previous results

### 🌐 High-Performance Server
- **HTTP REST API** - OpenAPI-compatible endpoints for all operations
- **Real-time Communication** - Instant agent responses and status updates
- **Configuration Management** - Hot-reloadable YAML configurations
- **Health Monitoring** - System status and agent health tracking

### 🧠 LLM Integration
- **Kolosal Server Integration** - Built-in integration with high-performance inference server
- **Model Interface** - Flexible model communication and parameter handling
- **Retrieval Augmented Generation** - Document retrieval and web search capabilities
- **Multi-Model Support** - Support for various LLM models and embedding models

### 🔧 Developer-Friendly
- **Simple Configuration** - YAML-based configuration files
- **Cross-Platform** - Windows, Linux, and macOS support
- **Comprehensive Testing** - Unit, integration, and performance tests
- **Extensive Documentation** - Complete API documentation and examples

## 🏗️ Architecture

The Kolosal Agent System features a layered architecture designed for scalability and maintainability:

```
┌─────────────────────────────────────────────────────────────┐
│                    🌐 REST API Layer                        │
│  ┌─────────────────┬─────────────────┬─────────────────┐    │
│  │   Agent APIs    │   Workflow APIs │   System APIs   │    │
│  └─────────────────┴─────────────────┴─────────────────┘    │
├─────────────────────────────────────────────────────────────┤
│              🔄 Workflow Orchestration Layer                │
│  ┌─────────────────┬─────────────────┬─────────────────┐    │
│  │Workflow Manager │Workflow Builder │Template Engine  │    │
│  │  & Executor     │ & Orchestrator  │ & Config Loader │    │
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

- **🔄 Workflow System** - Advanced workflow orchestration with multiple execution patterns
- **🤖 Agent System** - Manages agent lifecycle, creation, and function execution  
- **🌐 HTTP Server** - REST API with concurrent request processing and workflow endpoints
- **⚡ Kolosal Server Integration** - Optional high-performance LLM inference
- **📊 Configuration Management** - Hot-reloadable YAML configurations for agents and workflows

### Workflow System Features

- **Multiple Execution Types** - Sequential, parallel, conditional, loop, and pipeline workflows
- **Agent-LLM Pairing** - Configurable agent-model mappings for optimal performance
- **Built-in Templates** - Pre-defined workflow templates for common patterns
- **Dynamic Execution** - Real-time workflow execution control (pause, resume, cancel)
- **Progress Monitoring** - Detailed execution tracking and progress reporting

## 🛠️ Installation

### System Requirements
- **C++20** compatible compiler
- **CMake 3.14+**
- **Git** with submodule support
- **4GB+ RAM**

### Platform-Specific Instructions

<details>
<summary><b>🪟 Windows (PowerShell)</b></summary>

```powershell
# Clone the repository
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# Create build directory
New-Item -ItemType Directory -Path "build" -Force
Set-Location "build"

# Configure and build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug --parallel

# Run
.\Debug\kolosal-agent.exe
```
</details>

<details>
<summary><b>🐧 Linux (Ubuntu/Debian)</b></summary>

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake git libcurl4-openssl-dev

# Clone and build
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent && mkdir build && cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run
./kolosal-agent
```
</details>

<details>
<summary><b>🍎 macOS</b></summary>

```bash
# Install dependencies
brew install cmake git curl yaml-cpp

# Clone and build
git clone --recursive https://github.com/kolosal-ai/kolosal-agent.git
cd kolosal-agent && mkdir build && cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(sysctl -n hw.ncpu)

# Run
./kolosal-agent
```
</details>

### Build Options

```bash
# Standard build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# With tests
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

# With kolosal-server integration
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_KOLOSAL_SERVER=ON
```

### Verification

```bash
# Check build outputs
ls build/Debug/                     # Windows
ls build/                           # Linux/macOS

# Test the application
curl http://localhost:8080/status
```

## ⚙️ Configuration

The system uses YAML configuration files for customization.

### Agent System Configuration (`agent.yaml`)

```yaml
system:
  name: "Kolosal Agent System"
  version: "1.0.0"
  host: "127.0.0.1"
  port: 8080
  log_level: "info"

system_instruction: |
  You are a helpful AI assistant that is part of the Kolosal Agent System.
  Be helpful, accurate, and professional in your responses.

agents:
  - name: "Assistant"
    capabilities: ["chat", "analysis", "reasoning"]
    auto_start: true
    system_prompt: |
      You are an AI assistant specialized in general conversation and help.

  - name: "Analyzer" 
    capabilities: ["analysis", "data_processing", "summarization"]
    auto_start: true

  - name: "RetrievalAgent"
    capabilities: ["retrieval", "document_management", "semantic_search"]
    auto_start: true

functions:
  chat:
    description: "Interactive chat functionality"
    timeout: 30000
    parameters:
      - name: "message"
        type: "string"
        required: true
      - name: "model"
        type: "string"
        required: true

  analyze:
    description: "Text and data analysis"
    timeout: 60000
    parameters:
      - name: "text"
        type: "string"
        required: true
```

### Workflow Configuration (`workflow.yaml`)

```yaml
settings:
  max_concurrent_workflows: 10
  default_timeout_ms: 300000
  cleanup_completed_after_hours: 24

workflows:
  - id: "simple_research"
    name: "Simple Research Workflow"
    description: "Basic research workflow"
    type: "sequential"
    steps:
      - id: "research_step"
        agent_name: "Researcher"
        llm_model: "gemma3-1b"
        function_name: "research"
        parameters: ["query", "depth"]
        timeout_ms: 120000

  - id: "parallel_analysis"
    name: "Parallel Analysis Workflow"
    type: "parallel"
    steps:
      - id: "sentiment_analysis"
        agent_name: "Analyzer"
        llm_model: "gemma3-1b"
        function_name: "analyze"
      - id: "keyword_extraction"
        agent_name: "Analyzer"
        llm_model: "gemma3-1b"
        function_name: "analyze"

agent_llm_mappings:
  Assistant:
    default_model: "gemma3-1b"
    supported_models: ["gemma3-1b"]
  Analyzer:
    default_model: "gemma3-1b"
    supported_models: ["gemma3-1b"]
```

### Kolosal Server Configuration (`config.yaml`)

*Optional - for LLM integration*

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

database:
  vector_database: faiss
  retrieval_embedding_model: qwen3-embedding-0.6b

search:
  enabled: true
  searxng_url: https://searx.stream/
  timeout: 30
```

## 🌐 API Reference

### System Status
```http
GET /status
```
Returns system health and agent statistics.

### Agent Management

#### List All Agents
```http
GET /agents
```

#### Create Agent
```http
POST /agents
Content-Type: application/json

{
  "name": "CustomAgent",
  "capabilities": ["chat", "research"]
}
```

#### Execute Function
```http
POST /agents/{agent_name}/execute
Content-Type: application/json

{
  "function": "chat",
  "params": {
    "message": "Hello!",
    "model": "your_model"
  }
}
```

### Workflow Management

#### List Available Workflows
```http
GET /workflows
```

#### Register New Workflow
```http
POST /workflows
Content-Type: application/json

{
  "id": "custom_workflow",
  "name": "Custom Workflow",
  "type": 0,
  "steps": [
    {
      "id": "step1",
      "agent_name": "Assistant",
      "function_name": "chat",
      "parameters": {"message": "Hello", "model": "gemma3-1b"}
    }
  ]
}
```

#### Execute Workflow
```http
POST /workflows/{id}/execute
Content-Type: application/json

{
  "input_data": {"query": "AI research trends"}
}
```

### Example API Calls

```bash
# System status
curl http://localhost:8080/status

# List agents
curl http://localhost:8080/agents

# Chat with assistant
curl -X POST http://localhost:8080/agents/Assistant/execute \
  -H "Content-Type: application/json" \
  -d '{"function": "chat", "params": {"message": "Hello!", "model": "your_model"}}'

# Analyze text
curl -X POST http://localhost:8080/agents/Analyzer/execute \
  -H "Content-Type: application/json" \
  -d '{"function": "analyze", "params": {"text": "Sample text"}}'

# Document retrieval
curl -X POST http://localhost:8080/agents/RetrievalAgent/execute \
  -H "Content-Type: application/json" \
  -d '{"function": "retrieve_and_answer", "params": {"question": "What is AI?"}}'

# List available workflows
curl http://localhost:8080/workflows

# Execute a workflow
curl -X POST http://localhost:8080/workflows/simple_research/execute \
  -H "Content-Type: application/json" \
  -d '{"input_data": {"query": "machine learning trends"}}'
```

## 🧪 Testing

The system includes comprehensive testing support with automated test runners and multiple test categories.

### Quick Start - Run All Tests

```bash
# Windows PowerShell
.\tests\run_tests.ps1

# Linux/macOS
./tests/run_tests.sh
```

### Test Categories

| Category | Description | Tests Included |
|----------|-------------|----------------|
| **`all`** | Complete test suite | All available tests |
| **`quick`** | Fast unit tests | `minimal_demo`, `model_interface`, `config_manager`, `workflow_config` |
| **`demo`** | Demo functionality | `minimal_demo`, `simple_demo` |
| **`workflow`** | Workflow system | `workflow_config`, `workflow_manager`, `workflow_orchestrator` |
| **`integration`** | API & system integration | `agent_execution`, `http_server` |
| **`retrieval`** | Document retrieval | `retrieval_agent` |
| **`stress`** | Error & stress testing | `error_scenarios` |

### Test Runner Usage

<details>
<summary><b>🪟 Windows PowerShell</b></summary>

```powershell
# Run all tests
.\tests\run_tests.ps1 -TestType all

# Run quick tests with verbose output
.\tests\run_tests.ps1 -TestType quick -VerboseOutput

# Run specific category
.\tests\run_tests.ps1 -TestType workflow

# Run with custom timeout
.\tests\run_tests.ps1 -TestType integration -TimeoutMinutes 5

# Run specific test
.\tests\run_tests.ps1 -TestType agent_execution

# Save results to file
.\tests\run_tests.ps1 -TestType all -OutputFile "test_results.json"
```

**Available Options:**
- `-TestType` - Test category or specific test name
- `-BuildFirst $false` - Skip building before testing  
- `-VerboseOutput` - Enable detailed output
- `-OutputFile` - Save results to JSON file
- `-TimeoutMinutes` - Set timeout per test (default: 1)

</details>

<details>
<summary><b>🐧 Linux/macOS Bash</b></summary>

```bash
# Make executable (first time only)
chmod +x tests/run_tests.sh

# Run all tests  
./tests/run_tests.sh

# Run quick tests with verbose output
./tests/run_tests.sh --test-type quick --verbose

# Run specific category
./tests/run_tests.sh -t workflow

# Run with custom timeout
./tests/run_tests.sh --test-type integration --timeout 5

# Run specific test
./tests/run_tests.sh -t agent_execution

# Save results to file
./tests/run_tests.sh -t all -o test_results.json
```

**Available Options:**
- `-t, --test-type` - Test category or specific test name
- `-b, --build-first false` - Skip building before testing
- `-v, --verbose` - Enable detailed output  
- `-o, --output-file` - Save results to JSON file
- `--timeout` - Set timeout per test in minutes (default: 1)
- `-h, --help` - Show help message

</details>

### Manual Testing & Building

```bash
# Build tests manually
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build . --config Debug

# Run tests via CMake
ctest --output-on-failure

# Run individual test executables
./test_agent_execution          # Linux/macOS
.\Debug\test_agent_execution.exe # Windows

# Available test executables:
# - minimal_test_demo / minimal_test_demo.exe
# - test_agent_execution
# - test_config_manager  
# - test_workflow_config
# - test_workflow_manager
# - test_workflow_orchestrator
# - test_http_server
# - test_model_interface
# - test_error_scenarios
# - test_retrieval_agent
```

### Test Output Examples

**✅ Successful Test Output:**
```
🧪 Running Kolosal Agent System Tests
📋 Test Type: quick
🔨 Building tests... ✅ Build completed successfully
🚀 Running tests...

✅ minimal_demo                 ✅ PASSED (0.8s)
✅ model_interface              ✅ PASSED (1.2s)  
✅ config_manager               ✅ PASSED (0.6s)
✅ workflow_config              ✅ PASSED (1.1s)

📊 Results: 4/4 tests passed (100% success)
⏱️  Total time: 3.7 seconds
```

**❌ Failed Test Output:**
```
🧪 Running Kolosal Agent System Tests
🚀 Running tests...

✅ minimal_demo                 ✅ PASSED (0.8s)
❌ workflow_manager             ❌ FAILED (2.3s)
   └── Error: Workflow execution timeout

📊 Results: 1/2 tests passed (50% success)  
❌ Some tests failed. Check logs for details.
```

### Verification & Smoke Tests

```bash
# Quick verification after build
./tests/run_tests.sh -t demo

# Basic functionality check  
curl http://localhost:8080/status
curl http://localhost:8080/agents

# Workflow system check
curl http://localhost:8080/workflows
```

### Troubleshooting Tests

| Issue | Solution |
|-------|----------|
| Build fails | Ensure C++20 compiler and CMake 3.14+ |
| Tests timeout | Increase timeout: `--timeout 10` |
| Missing dependencies | Run `git submodule update --init --recursive` |
| Permission denied | Run `chmod +x tests/run_tests.sh` |

For complete testing documentation, see [`tests/README.md`](tests/README.md).

## � Integration

### C++ Integration

```cpp
#include "agent_manager.hpp"
#include "agent_config.hpp"
#include "workflow_manager.hpp"
#include "workflow_types.hpp"

int main() {
    // Load configuration
    auto config_manager = std::make_shared<AgentConfigManager>();
    config_manager->load_config("agent.yaml");
    
    // Create agent manager
    auto agent_manager = std::make_shared<AgentManager>(config_manager);
    
    // Create workflow manager
    auto workflow_manager = std::make_shared<WorkflowManager>(agent_manager);
    workflow_manager->start();
    
    // Create workflow orchestrator
    auto workflow_orchestrator = std::make_shared<WorkflowOrchestrator>(workflow_manager);
    workflow_orchestrator->start();
    
    // Create and use agent
    std::string agent_id = agent_manager->create_agent("MyAgent", {"chat"});
    agent_manager->start_agent(agent_id);
    
    // Execute simple function
    json params = {{"message", "Hello!"}, {"model", "gemma3-1b"}};
    auto result = agent_manager->execute_function(agent_id, "chat", params);
    
    // Execute workflow
    json workflow_input = {{"query", "AI trends"}};
    std::string execution_id = workflow_orchestrator->execute_workflow("simple_research", workflow_input);
    
    // Monitor workflow execution
    auto execution_status = workflow_orchestrator->get_execution_status(execution_id);
    
    return 0;
}
```

### Python Client

```python
import requests
import json
import time

class KolosalAgentClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
    
    def create_agent(self, name, capabilities):
        response = requests.post(f"{self.base_url}/agents",
                               json={"name": name, "capabilities": capabilities})
        return response.json()
    
    def execute_function(self, agent_name, function, params):
        response = requests.post(f"{self.base_url}/agents/{agent_name}/execute",
                               json={"function": function, "params": params})
        return response.json()
    
    def list_workflows(self):
        response = requests.get(f"{self.base_url}/workflows")
        return response.json()
    
    def execute_workflow(self, workflow_id, input_data=None):
        payload = {}
        if input_data:
            payload["input_data"] = input_data
        response = requests.post(f"{self.base_url}/workflows/{workflow_id}/execute", json=payload)
        return response.json()

# Usage Examples
client = KolosalAgentClient()

# Simple agent function call
result = client.execute_function("Assistant", "chat", {
    "message": "Hello from Python!",
    "model": "gemma3-1b"
})
print("Agent Response:", result)

# List available workflows
workflows = client.list_workflows()
print("Available Workflows:", workflows)

# Execute a workflow
workflow_result = client.execute_workflow("simple_research", {
    "query": "machine learning trends 2025"
})
print("Workflow Execution ID:", workflow_result.get("execution_id"))
```

### Docker Deployment

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential cmake git libcurl4-openssl-dev

COPY . /app
WORKDIR /app

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --config Release

# Copy configuration files
COPY agent.yaml workflow.yaml config.yaml ./build/

EXPOSE 8080
CMD ["./build/kolosal-agent"]
```

## � Performance

### Benchmarks
- **Agent Creation**: Sub-second initialization
- **Memory Usage**: ~0.9MB per agent
- **Startup Time**: <5 seconds with 100 agents
- **Concurrent Requests**: 1000+ requests/second
- **Function Execution**: Low-latency with concurrent processing

### Optimization Tips
- Tune `max_concurrent_requests` and `worker_threads` in configuration
- Use local models for faster inference
- Enable response caching for repeated queries
- Set appropriate memory limits per agent

## 🤝 Contributing

We welcome contributions! Please see our [Developer Guide](docs/DEVELOPER_GUIDE.md) for details.

### Quick Start for Contributors

```bash
# Fork and clone
git clone --recursive https://github.com/your-username/kolosal-agent.git
cd kolosal-agent

# Build with tests
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build . --config Debug

# Run tests
ctest --output-on-failure

# Format code (optional)
clang-format -i src/**/*.cpp include/**/*.hpp
```

### Development Workflow
1. Fork the repository
2. Create a feature branch
3. Make changes and add tests
4. Run the test suite
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [nlohmann/json](https://github.com/nlohmann/json) - JSON library for Modern C++
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) - YAML parsing library
- [Kolosal Server](kolosal-server/) - High-performance inference server integration

## 📞 Support & Documentation

- **📖 Complete Documentation**: [`docs/`](docs/) directory
  - [API Reference](docs/API_REFERENCE.md)
  - [Architecture Guide](docs/ARCHITECTURE.md)
  - [Configuration Guide](docs/CONFIGURATION.md)
  - [Developer Guide](docs/DEVELOPER_GUIDE.md)
  - [Installation Guide](docs/INSTALLATION.md)
  - [Quick Start Guide](docs/QUICK_START.md)
  - [Testing Guide](docs/TESTING.md)
  - [Troubleshooting](docs/TROUBLESHOOTING.md)

- **🐛 Issues & Support**: [GitHub Issues](https://github.com/KolosalAI/kolosal-agent/issues)
- **💡 Examples & Demos**: [`tests/`](tests/) directory
- **🔧 Build Scripts**: Available via VS Code tasks or direct cmake commands

---
