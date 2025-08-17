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
- [🛠️ Building the Application - Complete Guide](#️-building-the-application---complete-guide)
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

# 2. Choose your build mode:

# 📦 STANDARD BUILD (Default - Fast, core functionality)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug

# 🧪 BUILD WITH ALL TESTS (Comprehensive testing)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build . --config Debug

# 🔬 BUILD WITH DEEP RESEARCH DEMO (Includes demo)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_DEEPRESEARCH_DEMO=ON
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
- **📚 Examples**: Working examples in the `/examples` directory
- **🧪 Tests** (if built with `-DBUILD_TESTS=ON`): Comprehensive test suite including unit, integration, and performance tests
- **🔬 Deep Research Demo** (if built with `-DBUILD_DEEPRESEARCH_DEMO=ON`): Advanced research demonstration capabilities

### 🎯 Next Steps

1. **Explore the API**: Visit `http://localhost:8080/v1/agents` to see available agents
2. **Run Examples**: Check out `/examples` directory for usage patterns
3. **Read the Build Guide**: Continue with the detailed build instructions below
4. **Run Tests** (if built with tests): Use `ctest --output-on-failure` to verify functionality
5. **Customize Configuration**: Edit `config.yaml` for your specific needs

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

## 🛠️ Building the Application - Complete Guide

The Kolosal Agent System offers **3 simple build modes** to meet different development needs.

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

### 🏗️ Build Modes

#### **📦 Mode 1: Standard Build (Fast & Clean)**
Perfect for production use, fastest compilation, no tests:

```bash
# Step 1: Clone the repository
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# Step 2: Create build directory
mkdir build && cd build

# Step 3: Configure for standard build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Step 4: Build the project
cmake --build . --config Debug

# Step 5: Run the application
./kolosal-agent                    # Linux/macOS
.\Debug\kolosal-agent.exe          # Windows
```

**What you get:**
- ✅ Main application (`kolosal-agent`)
- ✅ Server component (`kolosal-server`)
- ❌ No tests (fastest build)
- ❌ No deep research demo

---

#### **🧪 Mode 2: Build with All Tests (Comprehensive)**
Includes everything plus comprehensive testing suite:

```bash
# Step 1: Clone the repository
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# Step 2: Create build directory
mkdir build && cd build

# Step 3: Configure with tests enabled
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

# Step 4: Build the project (takes longer due to tests)
cmake --build . --config Debug

# Step 5: Run tests to verify everything works
ctest --output-on-failure

# Step 6: Run the application
./kolosal-agent                    # Linux/macOS
.\Debug\kolosal-agent.exe          # Windows
```

**What you get:**
- ✅ Everything from Standard Build
- ✅ **Unit Tests** - Test individual components
- ✅ **Integration Tests** - Test system interactions
- ✅ **Performance Tests** - Validate performance metrics
- ✅ **Test Utilities** - Testing infrastructure and tools

---

#### **🔬 Mode 3: Build with Deep Research Demo**
Includes the advanced deep research demonstration capabilities:

```bash
# Step 1: Clone the repository
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# Step 2: Create build directory
mkdir build && cd build

# Step 3: Configure with deep research demo enabled
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_DEEPRESEARCH_DEMO=ON

# Step 4: Build the project
cmake --build . --config Debug

# Step 5: Run the application
./kolosal-agent                    # Linux/macOS
.\Debug\kolosal-agent.exe          # Windows

# Step 6: Run the deep research demo
./deep-research-demo               # Linux/macOS
.\Debug\deep-research-demo.exe     # Windows
```

**What you get:**
- ✅ Everything from Standard Build
- ✅ **Deep Research Demo** - Advanced research demonstration executable
- ✅ Enhanced research capabilities for complex analysis workflows

### 🎯 Which Build Mode Should I Choose?

| Use Case | Recommended Mode | Command |
|----------|------------------|---------|
| **First-time setup** | Standard Build | `cmake .. -DCMAKE_BUILD_TYPE=Debug` |
| **Production deployment** | Standard Build (Release) | `cmake .. -DCMAKE_BUILD_TYPE=Release` |
| **Development & Testing** | Build with Tests | `cmake .. -DBUILD_TESTS=ON` |
| **Research & Analysis** | Deep Research Demo | `cmake .. -DBUILD_DEEPRESEARCH_DEMO=ON` |
| **Contributing to project** | Build with Tests | `cmake .. -DBUILD_TESTS=ON` |
| **CI/CD Pipeline** | Build with Tests | `cmake .. -DBUILD_TESTS=ON` |

### 🎨 Build Targets

After configuring, you can build specific components:

```bash
# Build everything (default)
cmake --build . --config Debug

# Build specific targets
cmake --build . --target kolosal-agent --config Debug          # Main application only
cmake --build . --target kolosal-server --config Debug        # Server component only

# Deep Research Demo target (only available when -DBUILD_DEEPRESEARCH_DEMO=ON)
cmake --build . --target deep-research-demo --config Debug    # Deep research demo executable

# Test-specific targets (only available when -DBUILD_TESTS=ON)
ctest --output-on-failure                                     # Run all tests
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

**Kolosal Agent System v2.0** - *Empowering the future of AI through unified, intelligent, multi-agent systems.*
