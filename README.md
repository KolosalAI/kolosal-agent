# Kolosal Agent System

A powerful multi-agent AI system that integrates with the Kolosal Server for advanced language model capabilities and agent orchestration.

## Overview

The Kolosal Agent System is a comprehensive framework that combines:
- **Multi-Agent Architecture**: Coordinate multiple specialized AI agents
- **Kolosal Server Integration**: Built-in language model server with GPU acceleration
- **YAML Configuration**: Easy-to-configure agent behaviors and capabilities
- **Cross-Platform Support**: Works on Windows, macOS, and Linux

## Features

- 🤖 **Multi-Agent Coordination**: Orchestrate multiple specialized AI agents
- ⚡ **High Performance**: GPU acceleration support (CUDA, Vulkan, Metal)
- 🔧 **Configurable**: YAML-based configuration system
- 🌐 **Server Integration**: Built-in Kolosal server with REST API
- 📊 **Monitoring**: Real-time system and agent monitoring
- 🛠️ **Extensible**: Plugin architecture for custom tools and functions
- 🔄 **Auto-Recovery**: Automatic server management and health monitoring

## Quick Start

### Prerequisites

- **CMake** 3.14 or higher
- **C++17** compatible compiler (GCC 8+, Clang 8+, MSVC 2019+)
- **Git** with submodule support
- **Python 3.8+** (optional, for advanced features)

### Platform-Specific Requirements

#### Windows
- Visual Studio 2019/2022 or Build Tools
- Windows 10/11 recommended

#### macOS
- Xcode Command Line Tools
- macOS 10.14+ recommended
- Homebrew (for dependencies): `brew install cmake`

#### Linux
- GCC 8+ or Clang 8+
- Development packages: `sudo apt install build-essential cmake git`

### Building from Source

1. **Clone the repository with submodules:**
   ```bash
   git clone --recursive https://github.com/Evintkoo/kolosal-agent.git
   cd kolosal-agent
   ```

2. **Build the project:**

   **Linux/macOS:**
   ```bash
   chmod +x build.sh
   ./build.sh
   ```

   **Windows:**
   ```batch
   build.bat
   ```

   **Manual build:**
   ```bash
   mkdir build && cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build . --parallel
   ```

   **Build with specific options:**
   ```bash
   mkdir build && cd build
   
   # Basic release build (default - no PDF support)
   cmake -DCMAKE_BUILD_TYPE=Release ..
   
   # Build with CUDA support
   cmake -DCMAKE_BUILD_TYPE=Release -DUSE_CUDA=ON ..
   
   # Build with PDF support (requires PoDoFo installed)
   cmake -DCMAKE_BUILD_TYPE=Release -DUSE_PODOFO=ON ..
   
   # Build with multiple features
   cmake -DCMAKE_BUILD_TYPE=Release -DUSE_CUDA=ON -DUSE_VULKAN=ON ..
   
   cmake --build . --config Release --parallel
   ```

3. **Run the agent system:**
   ```bash
   cd build
   ./kolosal-agent --help
   ```

### Quick Test

Create a simple configuration and test the system:

```bash
# The system will create a default config.yaml if none exists
./kolosal-agent --demo
```

## Configuration

The system uses YAML configuration files to define agents, their roles, and capabilities.

### Basic Configuration Structure

```yaml
system:
  name: "My Agent System"
  version: "1.0.0"
  server:
    host: "127.0.0.1"
    port: 8080
    timeout: 30

agents:
  - name: "coordinator"
    id: "coord-001"
    type: "coordinator"
    role: "COORDINATOR"
    specializations:
      - "TASK_PLANNING"
      - "RESOURCE_MANAGEMENT"
    capabilities:
      - "plan_execution"
      - "task_delegation"
    functions:
      - "plan_tasks"
      - "delegate_work"
    config:
      auto_start: true
      max_concurrent_tasks: 5

  - name: "analyst"
    id: "analyst-001"
    type: "specialist" 
    role: "ANALYST"
    specializations:
      - "DATA_ANALYSIS"
      - "RESEARCH"
    capabilities:
      - "data_processing"
      - "research_synthesis"
    functions:
      - "analyze_data"
      - "research_topic"
```

### Agent Roles

- **COORDINATOR**: Orchestrates other agents and manages workflows
- **ANALYST**: Specializes in data analysis and research
- **EXECUTOR**: Handles task execution and tool usage
- **SPECIALIST**: Domain-specific expertise
- **GENERIC**: General-purpose agent

### Built-in Functions

The system includes several built-in functions:
- `plan_tasks`: Create execution plans for complex tasks
- `analyze_data`: Process and analyze data
- `execute_task`: Execute specific tasks
- `research_topic`: Conduct research on topics
- `generate_report`: Create reports from data

## Usage Examples

### Basic Usage

```bash
# Run with default configuration
./kolosal-agent

# Use custom configuration file
./kolosal-agent -c my_config.yaml

# Run on custom port
./kolosal-agent -p 9090

# Run system demonstration
./kolosal-agent --demo

# Don't start server (assume it's already running)
./kolosal-agent --no-server
```

### Advanced Configuration

```bash
# Build with CUDA support
./build.sh --cuda

# Build with Vulkan support (Linux/Windows)
./build.sh --vulkan

# Build with Metal support (macOS)
./build.sh --metal

# Debug build with verbose output
./build.sh -t Debug -v

# Clean build with native optimizations
./build.sh -c --native
```

## Architecture

The system consists of several key components:

### Core Components

- **AgentCore**: Base agent implementation with lifecycle management
- **MultiAgentSystem**: Manages multiple agents and coordination
- **MessageRouter**: Handles inter-agent communication
- **FunctionManager**: Manages available functions and execution
- **ToolRegistry**: Plugin system for external tools
- **MemoryManager**: Persistent and working memory for agents
- **PlanningSystem**: Task planning and reasoning capabilities

### Server Integration

- **KolosalServerClient**: Interface to the language model server
- **ServerLauncher**: Automatic server lifecycle management
- **HealthMonitoring**: Real-time server health checks

### Configuration System

- **YAMLConfig**: Configuration parsing and validation
- **AgentFactory**: Creates agents from configuration
- **RuntimeReconfiguration**: Hot-reload configuration changes

## API Reference

### Command Line Options

```
Usage: kolosal-agent [OPTIONS]

Options:
  -c, --config FILE     Use custom configuration file (default: config.yaml)
  -p, --port PORT       Server port (default: 8080) 
  -s, --server PATH     Path to kolosal-server executable
  --no-server           Don't start server (assume it's already running)
  --demo                Run system demonstration
  -h, --help            Show help message
  -v, --version         Show version information
```

### Build Options

```
Options:
  -t, --type TYPE       Build type (Debug, Release, RelWithDebInfo)
  -d, --dir DIR         Build directory [default: build]
  -j, --jobs N          Number of parallel jobs
  -c, --clean           Clean build
  -v, --verbose         Verbose build output
  --cuda                Enable CUDA support
  --vulkan              Enable Vulkan support
  --metal               Enable Metal support (macOS)
  --native              Enable native CPU optimization
```

## Development

### Project Structure

```
kolosal-agent/
├── include/           # Header files
├── src/              # Source files  
├── kolosal-server/   # Server submodule
├── external/         # External dependencies
├── tests/            # Unit tests (optional)
├── docs/             # Documentation
├── CMakeLists.txt    # Build configuration
├── build.sh          # Unix build script
├── build.bat         # Windows build script
└── README.md         # This file
```

### Adding Custom Agents

1. Create agent configuration in YAML
2. Implement custom functions if needed
3. Register functions in the system
4. Deploy and test

### Custom Functions

```cpp
// Example custom function
class CustomFunction : public AgentFunction {
public:
    FunctionResult execute(const AgentData& params) override {
        // Implementation here
        return FunctionResult::success("Task completed");
    }
    
    std::string getName() const override { return "custom_function"; }
    std::string getDescription() const override { return "Custom function"; }
};
```

## Troubleshooting

### Common Issues

**Build fails with PoDoFo PDF support errors:**
```bash
# PoDoFo is disabled by default to avoid dependency issues
# If you need PDF support, install PoDoFo first:
# Windows: Use vcpkg or build from source
# macOS: brew install podofo
# Linux: sudo apt-get install libpodofo-dev

# Then enable it during build:
cmake -DUSE_PODOFO=ON -DCMAKE_BUILD_TYPE=Release ..
```

**Build fails with submodule errors:**
```bash
git submodule update --init --recursive
```

**Server fails to start:**
- Check if port is already in use
- Verify firewall settings
- Check server executable permissions

**Agent configuration errors:**
- Validate YAML syntax
- Check agent ID uniqueness
- Verify function names exist

**Performance issues:**
- Enable GPU acceleration (--cuda, --vulkan, --metal)
- Increase server resources
- Optimize agent task distribution

### Debug Mode

Build and run in debug mode for detailed logging:
```bash
./build.sh -t Debug
./kolosal-agent -c config.yaml 2>&1 | tee debug.log
```

### Logs and Monitoring

- Agent logs: `agent_system.log`
- Server logs: Available via REST API `/logs`
- System status: Available via `get_system_status()`

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Commit changes: `git commit -m 'Add amazing feature'`
4. Push to branch: `git push origin feature/amazing-feature`
5. Open a Pull Request

### Development Setup

```bash
# Clone with submodules
git clone --recursive https://github.com/Evintkoo/kolosal-agent.git

# Build in debug mode
./build.sh -t Debug

# Run tests (if available)
./build.sh --tests
cd build && ctest
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Kolosal Server](https://github.com/Evintkoo/kolosal-server) - Integrated language model server
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) - YAML parsing library
- [nlohmann/json](https://github.com/nlohmann/json) - JSON parsing library

## Support

For support, please:
1. Check the [documentation](docs/)
2. Search existing [issues](https://github.com/Evintkoo/kolosal-agent/issues)
3. Create a new issue with detailed information

---

**Kolosal Agent System** - Empowering AI through coordinated multi-agent intelligence.
