# Kolosal Agent System v2.0 🤖

A next-generation unified multi-agent AI system that seamlessly integrates advanced language model inference with sophisticated agent orchestration capabilities.

[![Version](https://img.shields.io/badge/version-2.0.0-blue.svg)](https://github.com/kolosalai/kolosal-agent)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](#platform-support)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](#building)

## 📚 Table of Contents

- [🚀 Quick Start](#-quick-start)
- [🌟 What's New in v2.0](#-whats-new-in-v20)
- [🏗️ Architecture Overview](#️-architecture-overview)
- [🛠️ Building the Application - Complete Tutorial](#️-building-the-application---complete-tutorial)
- [🧪 Testing Tutorial - Complete Guide](#-testing-tutorial---complete-guide)
- [🌐 API Reference](#-api-reference)
- [⚙️ Configuration](#️-configuration)
- [🚀 Advanced Usage](#-advanced-usage)
- [🔌 Integration Examples](#-integration-examples)
- [📊 Performance Benchmarks](#-performance-benchmarks)
- [🔒 Security Features](#-security-features)
- [🐛 Troubleshooting](#-troubleshooting)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)

---

## 🌟 What's New in v2.0

- **🔄 Unified Architecture**: Single binary that manages both LLM inference and multi-agent systems
- **🚀 Enhanced Performance**: Improved agent coordination and reduced latency
- **🌐 REST API**: Comprehensive RESTful API for agent management and system control
- **📊 Advanced Monitoring**: Real-time health monitoring, metrics, and auto-recovery
- **🛠️ Service Layer**: High-level service abstractions for complex operations
- **⚡ Async Operations**: Full asynchronous support for non-blocking operations
- **🔧 Hot Configuration**: Dynamic configuration reloading without system restart
- **📈 Analytics**: Built-in performance analytics and optimization suggestions
- **🔗 MCP Protocol Integration**: Full Model Context Protocol (MCP) support for standardized AI tool and resource interoperability
- **🔍 Web Search Integration (NEW!)**: Real-time internet search with SearXNG backend
- **📚 Enhanced Document Retrieval (NEW!)**: Advanced semantic search through vector databases
- **🧠 Hybrid Knowledge System (NEW!)**: Combines web search with document retrieval for comprehensive knowledge gathering

## 🌐 Web Search & Retrieval Features (NEW!)

The Kolosal Agent System now includes powerful web search and document retrieval capabilities that enable agents to access real-time information and comprehensive knowledge bases.

### 🔍 Internet Search
- **Real-time Web Search**: Live internet searches using SearXNG backend
- **Multiple Search Engines**: Support for Google, Bing, DuckDuckGo, and more
- **Advanced Filtering**: Content quality assessment, safe search, and result optimization
- **Multilingual Support**: Search in multiple languages with localization
- **Category Search**: Specialized searches for news, images, academic content, etc.

### 📚 Document Retrieval
- **Semantic Search**: Vector-based similarity search through document collections
- **Multiple Formats**: Support for PDF, DOCX, HTML, and plain text documents
- **Intelligent Chunking**: Smart document segmentation for better retrieval
- **Metadata Support**: Rich metadata extraction and filtering
- **Collection Management**: Organize documents in separate collections

### 🧠 Hybrid Knowledge System
- **Comprehensive Retrieval**: Combines web search with local document retrieval
- **Smart Prioritization**: Automatically balances web vs. local content based on query
- **Result Fusion**: Intelligent merging and ranking of diverse knowledge sources
- **Context Awareness**: Maintains context across multiple knowledge sources

### ⚡ Quick Web Search Example

```cpp
#include "tools/enhanced_function_registry.hpp"

// Create agent with web search capabilities
auto agent = std::make_unique<AgentCore>("WebSearchAgent");

// Enable enhanced functions with kolosal-server integration
agent->enable_enhanced_functions("http://localhost:8080");

// Perform real internet search
AgentData search_params;
search_params.set("query", "latest AI developments 2025");
search_params.set("results", 5);

auto result = agent->get_function_manager()->execute__function("internet_search", search_params);

if (result.success) {
    auto titles = result.result_data.get_array_string("titles");
    auto urls = result.result_data.get_array_string("urls");
    // Process search results...
}
```

### 📖 Documentation
- **[Web Search Integration Guide](docs/WEB_SEARCH_INTEGRATION_GUIDE.md)**: Complete guide to web search functions
- **[Function Registry Guide](docs/FUNCTION_REGISTRY_GUIDE.md)**: How to register and manage functions
- **[Examples](examples/web_search_demo.cpp)**: Working examples of web search integration

## 🚀 Quick Start

### ⚡ Super Quick Setup (5 minutes)

Get the Kolosal Agent System running in just a few commands:

```bash
# 1. Clone the repository
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# 2. Build with test mode (recommended for first-time users)
mkdir build && cd build
cmake .. -DENABLE_TEST_MODE=ON
cmake --build . --config Debug

# 3. Run the application
./kolosal-agent  # Linux/macOS
.\Debug\kolosal-agent.exe  # Windows

# 4. Test the installation
curl http://localhost:8080/v1/system/status
```

**That's it!** The system is now running with a web interface at `http://localhost:8080`.

### 📋 What You Get Out of the Box

- **🤖 Multi-Agent System**: Pre-configured coordinator and specialist agents
- **🌐 REST API**: Complete API for agent management at `http://localhost:8080`
- **📊 Web Dashboard**: System status and agent monitoring interface
- **🧪 Test Suite**: Comprehensive tests to verify everything works
- **📚 Examples**: Working examples in the `/examples` directory

### 🎯 Next Steps

1. **Explore the API**: Visit `http://localhost:8080/v1/agents` to see available agents
2. **Run Examples**: Check out `/examples` directory for usage patterns
3. **Read the Tutorials**: Continue with the detailed build and testing guides below
4. **Customize Configuration**: Edit `config.yaml` for your specific needs

---

## 🏗️ Architecture Overview

Kolosal Agent System v2.0 features a revolutionary unified architecture:

```
┌─────────────────────────────────────────────────┐
│              🌐 REST API Layer                   │
├─────────────────────────────────────────────────┤
│          📊 Service Layer (AgentService)        │
├─────────────────────────────────────────────────┤
│     🤖 Multi-Agent System (Agent Manager)       │
├─────────────────┬───────────────────────────────┤
│  🧠 Agent Core  │     🔄 Message Router        │
├─────────────────┼───────────────────────────────┤
│  ⚡ LLM Server  │     📡 Kolosal Server        │
└─────────────────┴───────────────────────────────┘
```

### Core Components

#### 🔄 **Unified Server** (`UnifiedKolosalServer`)
- Manages both LLM inference and agent systems
- Automatic health monitoring and recovery
- Process lifecycle management
- Configuration hot-reloading

#### 📊 **Service Layer** (`AgentService`)
- High-level agent operations
- Bulk operations and batch processing
- Performance analytics and optimization
- Event-driven notifications

#### 🌐 **REST API** (`AgentManagementRoute`)
- Comprehensive agent management endpoints
- OpenAPI-compatible documentation
- Real-time system status and metrics
- Secure authentication and rate limiting

#### 🤖 **Multi-Agent Core**
- Advanced agent lifecycle management
- Sophisticated message routing
- Memory and planning systems
- Tool registry and function management

## 🌐 API Reference

### Agent Management

#### List All Agents
```http
GET /v1/agents
```

**Response:**
```json
{
  "agents": [
    {
      "id": "coord-001",
      "name": "system_coordinator",
      "type": "coordinator",
      "running": true,
      "role": 1,
      "capabilities": ["plan_execution", "task_delegation"],
      "statistics": {
        "total_functions_executed": 142,
        "total_tools_executed": 89,
        "average_execution_time_ms": 245.6
      }
    }
  ],
  "total_count": 4,
  "system_running": true
}
```

#### Create New Agent
```http
POST /v1/agents
Content-Type: application/json
```

```json
{
  "name": "custom_analyst",
  "id": "analyst-002",
  "type": "specialist",
  "role": 2,
  "specializations": [1, 2],
  "capabilities": ["data_processing", "research_synthesis"],
  "functions": ["analyze_data", "research_topic"],
  "config": {
    "priority": 2,
    "auto_start": true,
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

### Enhanced YAML Configuration

```yaml
# Kolosal Agent System v2.0 Configuration
system:
  name: "Kolosal Multi-Agent System v2.0"
  version: "2.0.0"
  environment: "production"
  
  server:
    host: "0.0.0.0"
    port: 8080
    timeout: 60
    enable_cors: true
    allowed_origins: ["https://my-frontend.com"]
    
  monitoring:
    enable_health_checks: true
    health_check_interval_seconds: 30
    enable_metrics: true
    enable_performance_analytics: true
    enable_auto_recovery: true
    max_recovery_attempts: 3

# Enhanced agent definitions
agents:
  - name: "system_coordinator"
    id: "coord-001"
    type: "coordinator"
    role: "COORDINATOR"
    priority: 1
    
    specializations:
      - "TASK_PLANNING"
      - "RESOURCE_MANAGEMENT"
      - "SYSTEM_MONITORING"
    
    capabilities:
      - "plan_execution"
      - "task_delegation"
      - "system_monitoring"
      - "resource_optimization"
    
    functions:
      - "plan_tasks"
      - "delegate_work" 
      - "monitor_progress"
      - "optimize_resources"
    
    config:
      auto_start: true
      max_concurrent_tasks: 10
      memory_limit_mb: 512
      enable_persistence: true
      heartbeat_interval_seconds: 10

# Advanced function definitions
functions:
  - name: "plan_tasks"
    type: "builtin"
    category: "planning"
    description: "Create comprehensive execution plans"
    version: "2.0"
    
    parameters:
      - name: "goal"
        type: "string"
        required: true
        description: "The main objective"
      - name: "priority"
        type: "integer"
        required: false
        default: 5
        min: 1
        max: 10
        
    returns:
      type: "object"
      description: "Execution plan with steps and dependencies"

# System templates for quick agent creation
templates:
  data_processor:
    type: "specialist"
    role: "ANALYST"
    specializations: ["DATA_ANALYSIS"]
    capabilities: ["data_processing", "report_generation"]
    functions: ["analyze_data", "generate_report"]
    config:
      auto_start: false
      max_concurrent_tasks: 3
      memory_limit_mb: 512
```

## 🛠️ Building the Application - Complete Tutorial

### Prerequisites

Before building the Kolosal Agent System, ensure you have the following installed:

#### System Requirements
- **CMake** 3.14 or higher
- **C++17** compatible compiler:
  - Windows: Visual Studio 2019/2022 or Build Tools for Visual Studio
  - Linux: GCC 9+ or Clang 10+
  - macOS: Xcode Command Line Tools
- **Git** with submodule support
- **4GB RAM** minimum (8GB+ recommended for compilation)

#### Platform-Specific Dependencies

**Windows:**
```powershell
# Install Visual Studio Build Tools or Visual Studio Community
# Install Git for Windows
# Install CMake (or use Visual Studio installer)

# Optional: Install vcpkg for easier dependency management
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install build-essential cmake git libcurl4-openssl-dev libyaml-cpp-dev

# For testing (optional)
sudo apt install libgtest-dev libgmock-dev
```

**macOS:**
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew dependencies
brew install cmake git curl yaml-cpp

# For testing (optional)
brew install googletest
```

### Step-by-Step Build Instructions

#### Step 1: Clone the Repository
```bash
# Clone with all submodules
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# If you already cloned without --recursive, initialize submodules
git submodule update --init --recursive
```

#### Step 2: Basic Build (Recommended for First-Time Users)
```bash
# Create and enter build directory
mkdir build
cd build

# Configure the project (Debug build for development)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the project
cmake --build . --config Debug

# The executables will be in build/Debug/ or build/ depending on platform
```

#### Step 3: Build with Tests (Recommended for Development)
```bash
# Configure with test mode enabled (includes all testing features)
cmake .. -DENABLE_TEST_MODE=ON

# Build everything (this may take several minutes)
cmake --build . --config Debug

# Tests will run automatically after build in test mode
# Look for "All tests passed" message
```

#### Step 4: Advanced Build Configuration

For different use cases, you can customize the build:

**Development Build (Full Features):**
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_TEST_MODE=ON \
  -DENABLE_MEMORY_TESTING=ON \
  -DENABLE_TEST_COVERAGE=ON \
  -DBUILD_EXAMPLES=ON \
  -DBUILD_DOCS=ON

cmake --build . --config Debug --parallel
```

**Production Build (Optimized):**
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_NATIVE_OPTS=ON \
  -DBUILD_TESTS=OFF

cmake --build . --config Release --parallel
```

**Minimal Build (Fastest):**
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=OFF \
  -DBUILD_EXAMPLES=OFF \
  -DBUILD_DOCS=OFF

cmake --build . --config Release
```

### CMake Build Options Reference

| Option | Description | Default | Use Case |
|--------|-------------|---------|----------|
| `CMAKE_BUILD_TYPE` | Build type (Debug, Release, RelWithDebInfo) | Debug | All builds |
| `ENABLE_TEST_MODE` | Enable comprehensive testing features | OFF | Development |
| `BUILD_TESTS` | Build unit tests | OFF | Testing |
| `BUILD_EXAMPLES` | Build example applications | OFF | Learning |
| `BUILD_DOCS` | Build documentation (requires Doxygen) | OFF | Documentation |
| `ENABLE_MEMORY_TESTING` | Enable memory sanitizers | OFF | Debug builds |
| `ENABLE_TEST_COVERAGE` | Enable test coverage reporting | OFF | CI/CD |
| `ENABLE_NATIVE_OPTS` | Enable native CPU optimizations | OFF | Performance |
| `ENABLE_CUDA` | Enable CUDA GPU acceleration | OFF | GPU builds |
| `USE_SYSTEM_LIBS` | Use system-installed libraries | OFF | System integration |

### Platform-Specific Build Instructions

#### Windows (PowerShell)
```powershell
# Clone the repository
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# Create build directory
New-Item -ItemType Directory -Path "build" -Force
Set-Location "build"

# Configure for Visual Studio
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug

# Build the project
cmake --build . --config Debug --parallel

# Optional: Create installer
cpack -G WIX  # Requires WiX Toolset
```

#### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake git libcurl4-openssl-dev

# Clone and build
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)  # Parallel build using all CPU cores

# Optional: Install system-wide
sudo cmake --install . --config Debug
```

#### macOS
```bash
# Install dependencies via Homebrew
brew install cmake git curl yaml-cpp

# Clone and build
git clone --recursive https://github.com/kolosal-ai/kolosal-agent.git
cd kolosal-agent

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(sysctl -n hw.ncpu)  # Parallel build using all CPU cores

# Optional: Create application bundle
cmake --build . --target package
```

### Verification and Installation

#### Verify Build Success
```bash
# Check if executables were created
ls -la build/Debug/  # or build/ on Linux/macOS

# Expected executables:
# - kolosal-agent          (main application)
# - kolosal-launcher       (launcher utility)
# - kolosal-server-exe     (server executable)
# - simple-test           (basic functionality test)
```

#### Quick Functionality Test
```bash
# Run basic functionality test
./build/Debug/simple-test  # Windows
./build/simple-test        # Linux/macOS

# Should output: "Simple test passed!"
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

After successfully building the application, you can run it in various modes:

### Basic Application Launch

```bash
# Navigate to build directory
cd build

# Run the main application (default port 8080)
./kolosal-agent                    # Linux/macOS
.\Debug\kolosal-agent.exe          # Windows Debug build
.\Release\kolosal-agent.exe        # Windows Release build

# Check if it's running
curl http://localhost:8080/v1/system/status
```

### Command Line Options

The Kolosal Agent System supports various command-line options:

```bash
# Basic usage with custom port
./kolosal-agent --port 9090

# Custom configuration file
./kolosal-agent --config /path/to/custom-config.yaml

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
./kolosal-launcher           # System launcher and manager
./kolosal-server-exe         # Standalone server component
./simple-test               # Basic functionality test

# Test executables (if built with tests)
./kolosal_agent_unit_tests
./kolosal_agent_integration_tests
./kolosal_agent_performance_tests
```

### Verify Installation

```bash
# 1. Run basic functionality test
./simple-test
# Expected output: "Simple test passed!"

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

## 🧪 Testing Tutorial - Complete Guide

The Kolosal Agent System includes a comprehensive testing framework designed to ensure code quality, performance, and reliability. This section provides everything you need to know about building, running, and understanding tests.

### Testing Overview

The test suite is organized into four main categories:

1. **Unit Tests** - Test individual components in isolation
2. **Integration Tests** - Test component interactions and system-level functionality  
3. **Performance Tests** - Measure and validate system performance
4. **Benchmark Tests** - Detailed performance analysis and optimization

### Prerequisites for Testing

#### Required Dependencies
```bash
# Ubuntu/Debian
sudo apt install libgtest-dev libgmock-dev

# macOS (Homebrew)
brew install googletest

# Windows (vcpkg)
vcpkg install gtest gmock

# Or let CMake handle dependencies automatically (recommended)
# CMake will download and build GTest if not found
```

### Step-by-Step Testing Instructions

#### Step 1: Build with Test Support

**Quick Test Mode (Recommended for Development):**
```bash
cd kolosal-agent
mkdir build && cd build

# Enable comprehensive test mode
cmake .. -DENABLE_TEST_MODE=ON
cmake --build . --config Debug

# Tests run automatically after build in test mode
# Look for test results in the output
```

**Manual Test Configuration:**
```bash
# Configure with specific test options
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DBUILD_UNIT_TESTS=ON \
  -DBUILD_INTEGRATION_TESTS=ON \
  -DBUILD_PERFORMANCE_TESTS=ON

# Build the project and tests
cmake --build . --config Debug
```

#### Step 2: Running Tests

**Run All Tests (Using CTest):**
```bash
cd build
ctest --output-on-failure

# Run tests in parallel (faster)
ctest --parallel 4 --output-on-failure

# Run tests with verbose output
ctest --verbose
```

**Run Specific Test Categories:**
```bash
# Run only unit tests
ctest -L unit

# Run only integration tests  
ctest -L integration

# Run only performance tests
ctest -L performance

# Run only benchmark tests (if enabled)
ctest -L benchmark
```

**Run Individual Test Executables:**
```bash
# Windows
.\Debug\kolosal_agent_unit_tests.exe
.\Debug\kolosal_agent_integration_tests.exe
.\Debug\kolosal_agent_performance_tests.exe

# Linux/macOS
./kolosal_agent_unit_tests
./kolosal_agent_integration_tests  
./kolosal_agent_performance_tests
```

#### Step 3: Understanding Test Output

**Successful Test Run Example:**
```
[==========] Running 45 tests from 12 test suites.
[----------] Global test environment set-up.
[----------] 8 tests from AgentCoreTest
[ RUN      ] AgentCoreTest.InitializationTest
[       OK ] AgentCoreTest.InitializationTest (2 ms)
[ RUN      ] AgentCoreTest.AgentCreation
[       OK ] AgentCoreTest.AgentCreation (1 ms)
...
[----------] 8 tests from AgentCoreTest (15 ms total)

[==========] 45 tests from 12 test suites ran. (234 ms total)
[  PASSED  ] 45 tests.
```

**Failed Test Example:**
```
[ RUN      ] AgentCoreTest.InvalidConfiguration
test_agent_core.cpp:123: Failure
Expected equality of these values:
  agent.isValid()
    Which is: false
  true
[  FAILED  ] AgentCoreTest.InvalidConfiguration (5 ms)
```

### Test Categories in Detail

#### Unit Tests
Test individual components in isolation:

```bash
# Run specific unit test suites
./kolosal_agent_unit_tests --gtest_filter="AgentCore*"
./kolosal_agent_unit_tests --gtest_filter="*Memory*"
./kolosal_agent_unit_tests --gtest_filter="Configuration*"

# Available test suites:
# - AgentCore: Core agent functionality
# - Configuration: YAML config parsing
# - Workflow: Workflow execution engine
# - MessageRouter: Message routing system
# - HttpServer: HTTP server functionality
# - Logger: Logging system tests
```

#### Integration Tests
Test system-level interactions:

```bash
# Run integration tests with specific scenarios
./kolosal_agent_integration_tests --gtest_filter="*FullSystem*"
./kolosal_agent_integration_tests --gtest_filter="*MultiAgent*"
./kolosal_agent_integration_tests --gtest_filter="*ServerIntegration*"

# Integration test categories:
# - Full system startup and shutdown
# - Multi-agent workflow scenarios  
# - API endpoint integration
# - Configuration loading and validation
# - MCP protocol integration
```

#### Performance Tests
Validate system performance metrics:

```bash
# Run performance tests with timing validation
./kolosal_agent_performance_tests

# Performance test areas:
# - Agent creation and initialization speed
# - Workflow execution performance
# - Memory usage patterns
# - Concurrent operation handling
# - Message routing throughput
```

#### Benchmark Tests (Optional)
Detailed performance analysis with Google Benchmark:

```bash
# Enable benchmark tests in build
cmake .. -DENABLE_BENCHMARK_TESTS=ON -DENABLE_TEST_MODE=ON
cmake --build . --config Debug

# Run benchmarks
./kolosal_agent_benchmark_tests

# Example benchmark output:
# Benchmark                     Time           CPU Iterations
# BM_AgentCreation            245 ns        243 ns    2876543
# BM_MessageRouting            89 ns         88 ns    7864321
# BM_WorkflowExecution       1234 us       1230 us        568
```

### Advanced Testing Features

#### Test Coverage Analysis
Generate test coverage reports to see what code is tested:

```bash
# Build with coverage support (GCC/Clang only)
cmake .. \
  -DENABLE_TEST_MODE=ON \
  -DENABLE_TEST_COVERAGE=ON \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build . --config Debug

# Run tests to generate coverage data
ctest

# Generate coverage report
cmake --build . --target coverage

# View HTML coverage report
# Linux/macOS: open build/coverage_report/index.html
# Windows: start build/coverage_report/index.html
```

#### Memory Testing with Sanitizers
Detect memory leaks and corruption:

```bash
# Build with memory testing (GCC/Clang only)
cmake .. \
  -DENABLE_TEST_MODE=ON \
  -DENABLE_MEMORY_TESTING=ON \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build . --config Debug

# Run tests with memory checking
# Any memory issues will be reported automatically
ctest --output-on-failure
```

### Custom Test Targets

The build system provides convenient test targets:

```bash
# Run only quick unit tests
cmake --build . --target quick-test --config Debug

# Run comprehensive test suite  
cmake --build . --target full-test --config Debug

# Run specific test categories
cmake --build . --target run_unit_tests --config Debug
cmake --build . --target run_integration_tests --config Debug
cmake --build . --target run_performance_tests --config Debug

# Validate test environment setup
cmake --build . --target validate_test_environment --config Debug
```

### Test Configuration and Customization

#### Running Specific Tests
Use Google Test filters to run specific tests:

```bash
# Run all tests containing "Agent" in the name
./kolosal_agent_unit_tests --gtest_filter="*Agent*"

# Run tests from a specific test fixture
./kolosal_agent_unit_tests --gtest_filter="AgentCoreTest.*"

# Exclude specific tests
./kolosal_agent_unit_tests --gtest_filter="*:-*Slow*"

# Run a specific test case
./kolosal_agent_unit_tests --gtest_filter="AgentCoreTest.InitializationTest"
```

#### Test Output Options
Customize test output for different needs:

```bash
# Minimal output (only failures)
./kolosal_agent_unit_tests --gtest_brief

# Colored output
./kolosal_agent_unit_tests --gtest_color=yes

# Output to XML (for CI/CD)
./kolosal_agent_unit_tests --gtest_output=xml:test_results.xml

# Repeat tests multiple times
./kolosal_agent_unit_tests --gtest_repeat=10

# Shuffle test order
./kolosal_agent_unit_tests --gtest_shuffle
```

### Test Data and Fixtures

Tests use various data files and configurations:

```bash
# Test data location
ls tests/fixtures/
# - test_configs/: Sample configuration files
# - test_data/: Input data for tests  
# - expected_outputs/: Expected test results

# Runtime test output (created during test runs)
ls build/test_output/
# - test_logs/: Test execution logs
# - temp_files/: Temporary test files
# - coverage_data/: Coverage information
```

### Continuous Integration Testing

For CI/CD environments, use these commands:

```bash
# CI-optimized test configuration
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_TEST_MODE=ON \
  -DENABLE_TEST_COVERAGE=ON \
  -DRUN_TESTS_ON_BUILD=OFF

# Build and test
cmake --build . --config Release --parallel

# Run tests with proper error codes for CI
ctest --output-on-failure --return-failed

# Generate reports
cmake --build . --target coverage --config Release
```

### Troubleshooting Test Issues

#### Common Test Problems

**Problem 1: Tests Not Building**
```bash
# Error: Could not find GTest
# Solution: Install Google Test or let CMake download it
sudo apt install libgtest-dev  # Linux
brew install googletest        # macOS
vcpkg install gtest           # Windows
```

**Problem 2: Tests Failing Due to Missing Files**
```bash
# Error: Cannot open config file
# Solution: Run tests from correct directory
cd build
ctest  # Not from project root
```

**Problem 3: Performance Tests Timing Out**
```bash
# Error: Test timeout
# Solution: Increase timeout or run with more resources
ctest --timeout 300  # 5 minute timeout
```

**Problem 4: Memory Tests Showing False Positives**
```bash
# Error: False memory leak reports
# Solution: Use proper suppressions or disable for problematic areas
export ASAN_OPTIONS=detect_leaks=0  # Temporarily disable leak detection
```

#### Debug Test Failures

```bash
# Run a specific failing test with debug output
./kolosal_agent_unit_tests --gtest_filter="FailingTest" --gtest_catch_exceptions=0

# Run under debugger (Linux/macOS)
gdb ./kolosal_agent_unit_tests
(gdb) run --gtest_filter="FailingTest"

# Run with verbose logging
./kolosal_agent_unit_tests --gtest_filter="FailingTest" --verbose

# Check test logs
cat build/test_output/test_logs/latest.log
```

### Test Development and Contributing

#### Writing New Tests
When adding features, include corresponding tests:

```cpp
// Example unit test structure
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "component/my_component.hpp"
#include "../fixtures/test_fixtures.hpp"

using namespace testing;
using namespace kolosal::agents;

class MyComponentTest : public ComponentTestFixture {
protected:
    void SetUp() override {
        ComponentTestFixture::SetUp();
        component = std::make_unique<MyComponent>();
    }
    
    std::unique_ptr<MyComponent> component;
};

TEST_F(MyComponentTest, BasicFunctionality) {
    ASSERT_TRUE(component->initialize());
    EXPECT_EQ(component->getStatus(), ComponentStatus::READY);
}
```

#### Test Best Practices
1. **Test Independence**: Each test should run independently
2. **Clear Names**: Use descriptive test names explaining what's tested
3. **Setup/Teardown**: Use proper fixtures for clean test environment
4. **Mock Dependencies**: Use mocks to isolate components
5. **Cover Edge Cases**: Test both success and failure scenarios
6. **Performance Awareness**: Include performance expectations

### Test Results and Reporting

Test results are automatically collected and can be analyzed:

```bash
# Test results summary
cat build/Testing/Temporary/LastTest.log

# Detailed test output  
cat build/Testing/Temporary/LastTestsFailed.log

# Coverage report (if enabled)
open build/coverage_report/index.html

# Performance test results
cat build/test_output/performance_results.json
```

For more detailed testing information, see the [Test Mode Guide](tests/TEST_MODE_GUIDE.md).
```

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

**Kolosal Agent System v2.0** - *Empowering the future of AI through unified, intelligent, multi-agent systems.*
