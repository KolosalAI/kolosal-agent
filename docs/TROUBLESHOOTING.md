# Troubleshooting Guide

Common issues and solutions for the Kolosal Agent System v2.0.

## 🚨 Quick Diagnosis

### System Health Check
```bash
# Check if the system is running
curl http://localhost:8080/v1/health

# Check system status
curl http://localhost:8080/v1/system/status

# Check if agents are running
curl http://localhost:8080/v1/agents
```

### Log Analysis
```bash
# View application logs (if enabled)
tail -f /var/log/kolosal-agent/app.log

# Check system logs
journalctl -u kolosal-agent -f  # Linux systemd
tail -f /var/log/system.log | grep kolosal  # macOS

# Debug mode
./kolosal-agent --log-level DEBUG
```

## 🔧 Installation Issues

### Build Problems

#### CMake Configuration Fails
**Symptoms:**
```
CMake Error: CMake was unable to find a build program corresponding to "Unix Makefiles"
```

**Solutions:**
```bash
# Install build tools
# Ubuntu/Debian
sudo apt install build-essential cmake

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install cmake

# macOS
xcode-select --install
brew install cmake

# Clear CMake cache and retry
rm -rf build/CMakeCache.txt build/CMakeFiles
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

#### Missing Dependencies
**Symptoms:**
```
fatal error: curl/curl.h: No such file or directory
fatal error: yaml-cpp/yaml.h: No such file or directory
```

**Solutions:**
```bash
# Ubuntu/Debian
sudo apt install libcurl4-openssl-dev libyaml-cpp-dev

# CentOS/RHEL
sudo yum install libcurl-devel yaml-cpp-devel

# macOS
brew install curl yaml-cpp

# Windows (vcpkg)
vcpkg install curl yaml-cpp
```

#### Git Submodule Issues
**Symptoms:**
```
fatal: No such file or directory: external/yaml-cpp/CMakeLists.txt
```

**Solutions:**
```bash
# Initialize submodules
git submodule update --init --recursive

# If still failing, force update
git submodule sync
git submodule update --init --recursive --force

# Clone with submodules from scratch
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
```

#### Compiler Issues
**Symptoms:**
```
error: 'std::make_unique' is not a member of 'std'
```

**Solutions:**
```bash
# Check compiler version
gcc --version
g++ --version

# Install newer compiler (Ubuntu)
sudo apt install gcc-11 g++-11
export CC=gcc-11
export CXX=g++-11

# Set C++17 standard explicitly
cmake .. -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
```

### Linking Problems

#### Library Not Found
**Symptoms:**
```
undefined reference to `curl_easy_init'
undefined reference to `YAML::LoadFile'
```

**Solutions:**
```bash
# Check library installation
pkg-config --libs libcurl
pkg-config --libs yaml-cpp

# Install development packages
sudo apt install libcurl4-openssl-dev libyaml-cpp-dev

# Add library paths (if needed)
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

## 🚀 Runtime Issues

### Server Won't Start

#### Port Already in Use
**Symptoms:**
```
Error: Failed to bind to port 8080
Address already in use
```

**Solutions:**
```bash
# Check what's using the port
netstat -tlnp | grep 8080
lsof -i :8080

# Kill process using the port
kill $(lsof -t -i :8080)

# Use different port
./kolosal-agent --port 9090

# Change default port in config
server:
  port: 9090
```

#### Permission Denied
**Symptoms:**
```
Error: Permission denied to bind to port 80
Error: Cannot write to log file
```

**Solutions:**
```bash
# Use port > 1024 for non-root
./kolosal-agent --port 8080

# Run with sudo (not recommended for production)
sudo ./kolosal-agent

# Fix file permissions
chmod 755 kolosal-agent
chmod 644 config.yaml

# Create proper directories
sudo mkdir -p /var/log/kolosal-agent
sudo chown $USER:$USER /var/log/kolosal-agent
```

#### Configuration File Not Found
**Symptoms:**
```
Error: Configuration file 'config.yaml' not found
Warning: Using default configuration
```

**Solutions:**
```bash
# Specify config file path
./kolosal-agent --config /path/to/config.yaml

# Check current directory
ls -la config.yaml

# Copy example configuration
cp config.example.yaml config.yaml

# Validate configuration
./kolosal-agent --validate-config
```

### Agent Creation Issues

#### Agent Creation Fails
**Symptoms:**
```
{
  "error": "Failed to create agent",
  "message": "Invalid configuration"
}
```

**Solutions:**
```bash
# Check agent configuration syntax
python -c "import yaml; yaml.safe_load(open('agent.yaml'))"

# Validate required fields
curl -X POST http://localhost:8080/v1/agents \
  -H "Content-Type: application/json" \
  -d '{
    "name": "TestAgent",
    "capabilities": ["chat"],
    "config": {"auto_start": true}
  }'

# Check system resources
curl http://localhost:8080/v1/system/metrics
```

#### Function Execution Fails
**Symptoms:**
```
{
  "success": false,
  "error_message": "Function 'unknown_function' not found"
}
```

**Solutions:**
```bash
# List available functions
curl http://localhost:8080/v1/functions

# Check function parameters
curl http://localhost:8080/v1/functions/chat

# Verify agent capabilities
curl http://localhost:8080/v1/agents/{agent_id}

# Test with simple function
curl -X POST http://localhost:8080/v1/agents/{agent_id}/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "status",
    "parameters": {}
  }'
```

### Performance Issues

#### Slow Response Times
**Symptoms:**
- API requests taking > 30 seconds
- High CPU usage
- Memory consumption growing

**Diagnosis:**
```bash
# Check system resources
top -p $(pgrep kolosal-agent)
htop

# Monitor memory usage
free -h
watch -n 1 'free -h'

# Check disk usage
df -h
iostat -x 1

# Network monitoring
netstat -i
ss -tuln
```

**Solutions:**
```bash
# Increase worker threads
performance:
  worker_threads: 8
  cache_size: "1GB"

# Optimize memory settings
performance:
  max_memory_usage: "4GB"
  cache_size: "512MB"

# Use release build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Enable performance monitoring
features:
  metrics: true
  health_check: true
```

#### Memory Leaks
**Symptoms:**
- Memory usage continuously growing
- System becoming unresponsive
- Out of memory errors

**Diagnosis:**
```bash
# Build with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Run with Valgrind
valgrind --leak-check=full --show-leak-kinds=all ./kolosal-agent

# Use AddressSanitizer
cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=address -g"
cmake --build . --config Debug
./kolosal-agent
```

**Solutions:**
```bash
# Restart the service periodically
systemctl restart kolosal-agent

# Monitor memory usage
curl http://localhost:8080/v1/system/metrics

# Implement memory limits
ulimit -m 4194304  # 4GB limit
```

## 🌐 Network Issues

### Connection Problems

#### Can't Connect to API
**Symptoms:**
```
curl: (7) Failed to connect to localhost port 8080: Connection refused
```

**Solutions:**
```bash
# Check if service is running
ps aux | grep kolosal-agent
systemctl status kolosal-agent

# Check listening ports
netstat -tlnp | grep kolosal
ss -tlnp | grep kolosal

# Test with telnet
telnet localhost 8080

# Check firewall settings
sudo ufw status
sudo iptables -L

# Start the service
./kolosal-agent
systemctl start kolosal-agent
```

#### CORS Issues
**Symptoms:**
```
Access to fetch at 'http://localhost:8080/v1/agents' from origin 'http://localhost:3000' 
has been blocked by CORS policy
```

**Solutions:**
```yaml
# Enable CORS in config.yaml
auth:
  cors:
    enabled: true
    allowed_origins:
      - "http://localhost:3000"
      - "https://your-domain.com"
```

#### SSL/TLS Issues
**Symptoms:**
```
SSL certificate verification failed
```

**Solutions:**
```bash
# Disable SSL verification (development only)
curl -k https://localhost:8080/v1/health

# Configure proper SSL certificates
# Update config.yaml with certificate paths

# Use HTTP for development
./kolosal-agent --no-ssl
```

### Timeout Issues

#### Request Timeouts
**Symptoms:**
```
curl: (28) Operation timed out after 30000 milliseconds
```

**Solutions:**
```yaml
# Increase timeouts in config.yaml
server:
  idle_timeout: 600

performance:
  request_timeout: 60000

functions:
  analyze:
    timeout: 120000
```

```bash
# Increase curl timeout
curl --max-time 120 http://localhost:8080/v1/agents

# Check function execution times
curl http://localhost:8080/v1/system/metrics
```

## 🔐 Authentication Issues

### API Key Problems
**Symptoms:**
```
{
  "error": "Unauthorized",
  "message": "Valid API key required"
}
```

**Solutions:**
```bash
# Include API key in request
curl -H "X-API-Key: your-api-key-here" \
  http://localhost:8080/v1/agents

# Check API key configuration
auth:
  enabled: true
  require_api_key: true
  api_keys:
    - "your-api-key-here"

# Disable authentication for testing
auth:
  enabled: false
```

### Rate Limiting
**Symptoms:**
```
{
  "error": "Rate limit exceeded",
  "message": "Too many requests"
}
```

**Solutions:**
```yaml
# Increase rate limits
auth:
  rate_limit:
    enabled: true
    max_requests: 1000
    window_size: 60
```

```bash
# Wait before retrying
sleep 60

# Check rate limit headers
curl -I http://localhost:8080/v1/health
```

## 🔧 Configuration Issues

### YAML Syntax Errors
**Symptoms:**
```
Error parsing configuration file: YAML syntax error at line 15
```

**Solutions:**
```bash
# Validate YAML syntax
python -c "import yaml; yaml.safe_load(open('config.yaml'))"

# Use online YAML validator
# Check indentation (use spaces, not tabs)

# Common YAML mistakes:
# - Missing spaces after colons
# - Incorrect indentation
# - Special characters not quoted
```

### Invalid Configuration Values
**Symptoms:**
```
Warning: Invalid log level 'VERBOSE', using INFO
Error: Port 99999 is out of valid range
```

**Solutions:**
```yaml
# Use valid log levels
logging:
  level: DEBUG  # DEBUG, INFO, WARN, ERROR

# Use valid port range
server:
  port: 8080  # 1-65535

# Check configuration reference
./kolosal-agent --show-config
./kolosal-agent --validate-config
```

## 🔍 Debugging Strategies

### Enable Debug Logging
```bash
# Command line
./kolosal-agent --log-level DEBUG --verbose

# Configuration file
logging:
  level: DEBUG
  show_request_details: true
  access_log: true

# Environment variable
export KOLOSAL_LOG_LEVEL=DEBUG
```

### System Information Collection
```bash
#!/bin/bash
# debug-info.sh

echo "=== System Information ==="
uname -a
cat /etc/os-release

echo "=== Kolosal Agent Version ==="
./kolosal-agent --version

echo "=== Configuration ==="
./kolosal-agent --show-config

echo "=== System Status ==="
curl -s http://localhost:8080/v1/system/status | jq '.'

echo "=== Process Information ==="
ps aux | grep kolosal-agent

echo "=== Network Status ==="
netstat -tlnp | grep kolosal

echo "=== Resource Usage ==="
free -h
df -h

echo "=== Log Files ==="
tail -50 /var/log/kolosal-agent/app.log 2>/dev/null || echo "No log file found"
```

### Performance Profiling
```bash
# CPU profiling
perf record -g ./kolosal-agent
perf report

# Memory profiling
valgrind --tool=massif ./kolosal-agent

# Strace for system calls
strace -o trace.log ./kolosal-agent

# Monitor with htop
htop -p $(pgrep kolosal-agent)
```

## 📞 Getting Help

### Information to Provide

When reporting issues, include:

1. **System Information:**
   ```bash
   uname -a
   ./kolosal-agent --version
   ```

2. **Configuration:**
   ```bash
   ./kolosal-agent --show-config
   ```

3. **Error Messages:**
   - Full error output
   - Log file contents
   - Stack traces

4. **Steps to Reproduce:**
   - Exact commands used
   - Configuration files
   - Expected vs actual behavior

5. **Environment:**
   - Operating system and version
   - Compiler version
   - Dependency versions

### Support Channels

- **GitHub Issues**: [kolosalai/kolosal-agent/issues](https://github.com/kolosalai/kolosal-agent/issues)
- **Documentation**: [Project Wiki](https://github.com/kolosalai/kolosal-agent/wiki)
- **Community**: [GitHub Discussions](https://github.com/kolosalai/kolosal-agent/discussions)

### Self-Help Resources

1. **Log Analysis**: Always check logs first
2. **Configuration Validation**: Verify configuration syntax
3. **Resource Monitoring**: Check system resources
4. **Network Testing**: Test connectivity
5. **Documentation Review**: Read relevant guides

## ⚠️ Known Issues

### Current Limitations

1. **Windows Specific:**
   - Long path names may cause issues
   - Some UNIX-specific features not available

2. **Memory Usage:**
   - Large model files require significant RAM
   - Memory usage may grow during heavy load

3. **Concurrency:**
   - Default configuration optimized for 4-8 cores
   - May need tuning for high-concurrency scenarios

### Workarounds

1. **Windows Long Paths:**
   ```bash
   # Use shorter installation paths
   # Enable long path support in Windows
   ```

2. **Memory Management:**
   ```yaml
   # Limit memory usage
   performance:
     max_memory_usage: "4GB"
     cache_size: "512MB"
   ```

3. **High Concurrency:**
   ```yaml
   # Increase worker threads
   performance:
     worker_threads: 16
     max_concurrent_requests: 500
   ```

---

For additional help, please refer to the [Developer Guide](DEVELOPER_GUIDE.md) or open an issue on GitHub.
