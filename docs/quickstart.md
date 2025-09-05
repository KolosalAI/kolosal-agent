# Quick Start Guide

Get the Kolosal Agent System running in just 5 minutes!

## 🚀 Su```bash
curl http://localhost:8081/v1/health
```

```bash
curl -X POST http://localhost:8081/v1/agents/agent-001/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "chat",
    "parameters": {
      "message": "Hello, world!",
      "model": "qwen2.5-0.5b"
    }
  }'
```sponse:
```json
{
  "status": "healthy",
  "timestamp": 1703123456,
  "uptime": 120,
  "system_running": true
}
```

### 2. List Agents

```bash
curl http://localhost:8081/v1/agents
```### Prerequisites Check
Before starting, ensure you have:
- **CMake** 3.14 or higher
- **C++17** compatible compiler
- **Git** with submodule support
- **4GB RAM** minimum

### 1. Clone and Build

```bash
# Clone the repository with submodules
git clone --recursive https://github.com/KolosalAI/kolosal-agent.git
cd kolosal-agent

# Create build directory
mkdir build && cd build

# Configure and build (Debug mode)
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

### 2. Run the Application

```bash
# Run the main application
./kolosal-agent                    # Linux/macOS
.\Debug\kolosal-agent.exe          # Windows
```

### 3. Test the Installation

```bash
# Check system status
curl http://localhost:8081/v1/system/status

# List available agents
curl http://localhost:8081/v1/agents
```

## 🎯 Expected Output

When the system starts successfully, you should see:

```
Kolosal Agent System v1.0 Starting...
✓ Configuration loaded: config.yaml
✓ Agent Manager initialized
✓ Default agents created: 3
✓ HTTP Server started on http://localhost:8081
✓ System ready for requests
```

## 🔧 Quick Configuration

### Basic Configuration (Optional)

Create a custom configuration file:

```bash
cp config.yaml my-config.yaml
# Edit my-config.yaml as needed
./kolosal-agent --config my-config.yaml
```

### Essential Settings

```yaml
# my-config.yaml
server:
  port: 8081              # Change port if needed
  host: "0.0.0.0"         # Allow external connections
  
logging:
  level: INFO             # DEBUG for more verbose output
  
auth:
  enabled: false          # Enable authentication if needed
```

## 🧪 Basic Testing

### 1. System Health Check

```bash
curl http://localhost:8081/v1/health
```

Expected response:
```json
{
  "status": "healthy",
  "timestamp": 1703123456,
  "uptime": 120,
  "system_running": true
}
```

### 2. List Agents

```bash
curl http://localhost:8081/v1/agents
```

Expected response:
```json
{
  "agents": [
    {
      "id": "agent-001",
      "name": "Assistant",
      "type": "general",
      "running": true,
      "capabilities": ["chat", "analysis", "reasoning"]
    }
  ],
  "total_count": 3
}
```

### 3. Execute Function

```bash
curl -X POST http://localhost:8081/v1/agents/agent-001/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "chat",
    "parameters": {
      "message": "Hello, world!",
      "model": "gemma3-1b"
    }
  }'
```

## 🎉 Success! What's Next?

You now have a running Kolosal Agent System! Here's what you can do next:

### Explore the Web Interface
- Open http://localhost:8081 in your browser
- View system status and agent information
- Monitor real-time metrics

### Learn More
- 📖 Read the [Configuration Guide](config.md) for advanced settings
- 🏗️ Understand the [Architecture Overview](architecture.md)
- 💻 Try the [Examples](examples.md)
- 🔧 Follow the [Developer Guide](development.md) if you want to contribute

### Build with Tests (Optional)

If you want to run tests or contribute to development:

```bash
# Clean and rebuild with tests
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build . --config Debug

# Run tests
ctest --output-on-failure
```

## 🐛 Troubleshooting Quick Fixes

### Build Issues

**CMake Version Too Old:**
```bash
# Update CMake
pip install cmake  # or download from cmake.org
```

**Missing Dependencies:**
```bash
# Ubuntu/Debian
sudo apt install build-essential cmake git libcurl4-openssl-dev

# macOS
brew install cmake git curl

# Windows
# Install Visual Studio Build Tools
```

**Git Submodules Missing:**
```bash
git submodule update --init --recursive
```

### Runtime Issues

**Port Already in Use:**
```bash
# Use different port
**Port Conflicts
```bash
# Check port usage
netstat -tlnp | grep 8081
lsof -i :8081

# Use different port
./kolosal-agent --port 9090
```
```

**Permission Denied:**
```bash
# Linux/macOS: Check if port 8081 requires privileges
sudo ./kolosal-agent  # or use port > 1024
```

**Configuration File Not Found:**
```bash
# Specify config file explicitly
./kolosal-agent --config ../config.yaml
```

## 🎯 Platform-Specific Notes

### Windows
- Use PowerShell or Command Prompt
- Executables are in `build\Debug\` folder
- Use backslashes in paths: `.\Debug\kolosal-agent.exe`

### Linux
- Executables are in `build/` folder
- May need to install development packages
- Use forward slashes in paths: `./kolosal-agent`

### macOS
- Install Xcode Command Line Tools first
- Use Homebrew for dependencies
- Similar to Linux for execution

## ✅ Quick Verification Checklist

- [ ] Repository cloned with `--recursive` flag
- [ ] Build completed without errors
- [ ] Application starts and shows initialization messages
- [ ] HTTP server responds on port 8081
- [ ] System status endpoint returns healthy status
- [ ] At least one agent is listed and running

## 📞 Need Help?

If you encounter issues:

1. Check the [Troubleshooting Guide](troubleshooting.md)
2. Review your configuration in [Configuration Guide](config.md)
3. Open an issue on [GitHub Issues](https://github.com/KolosalAI/kolosal-agent/issues)

---

**Congratulations!** 🎉 You now have Kolosal Agent System v1.0 running. Happy coding!
