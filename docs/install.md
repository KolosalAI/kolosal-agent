# Installation Guide

Complete installation instructions for Kolosal Agent System v1.0 across different platforms.

## 📋 System Requirements

### Minimum Requirements
- **Operating System**: Windows 10+, Linux (Ubuntu 20.04+), macOS 11+
- **Memory**: 4GB RAM
- **Storage**: 2GB free space
- **Network**: Internet connection for web search features

### Recommended Requirements
- **Memory**: 8GB+ RAM
- **Storage**: 10GB+ free space
- **CPU**: Multi-core processor (4+ cores)
- **GPU**: Optional, for AI model acceleration

## 🛠️ Prerequisites

### Development Tools

#### Windows
```powershell
# Install Visual Studio Build Tools or Visual Studio Community
# Download from: https://visualstudio.microsoft.com/downloads/

# Install Git for Windows
winget install Git.Git

# Install CMake
winget install Kitware.CMake

# Optional: Install vcpkg for dependency management
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

#### Linux (Ubuntu/Debian)
```bash
# Update package list
sudo apt update

# Install build essentials
sudo apt install build-essential cmake git

# Install development libraries
sudo apt install libcurl4-openssl-dev libyaml-cpp-dev

# Optional: Install testing frameworks
sudo apt install libgtest-dev libgmock-dev

# Install additional tools
sudo apt install curl wget unzip
```

#### Linux (CentOS/RHEL/Fedora)
```bash
# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install cmake git libcurl-devel yaml-cpp-devel

# Fedora
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git libcurl-devel yaml-cpp-devel
```

#### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake git curl yaml-cpp

# Optional: Install testing frameworks
brew install googletest
```

### Python (Optional, for testing and utilities)
```bash
# Install Python 3.8+
python3 --version

# Install pip packages for testing
pip install requests pyyaml pytest
```

## 📦 Installation Methods

### Method 1: Quick Installation (Recommended)

#### Step 1: Clone Repository
```bash
# Clone with submodules
git clone --recursive https://github.com/KolosalAI/kolosal-agent.git
cd kolosal-agent

# If you already cloned without --recursive, initialize submodules:
git submodule update --init --recursive
```

#### Step 2: Build the Application
```bash
# Create build directory
mkdir build && cd build

# Configure build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the application
cmake --build . --config Release

# Optional: Build with tests
# cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
# cmake --build . --config Release
```

#### Step 3: Verify Installation
```bash
# Run the application
./kolosal-agent                     # Linux/macOS
.\Release\kolosal-agent.exe         # Windows

# Test basic functionality
curl http://localhost:8081/v1/health
```

### Method 2: Docker Installation

#### Prerequisites
- Docker 20.10+
- Docker Compose 2.0+

#### Using Docker Compose
```bash
# Clone repository
git clone --recursive https://github.com/KolosalAI/kolosal-agent.git
cd kolosal-agent

# Build and start with Docker Compose
docker-compose up -d

# Check status
docker-compose ps

# View logs
docker-compose logs -f kolosal-agent
```

#### Using Docker directly
```bash
# Build image
docker build -t kolosal-agent:latest .

# Run container
docker run -d \
  --name kolosal-agent \
  -p 8081:8081 \
  -v $(pwd)/config:/app/config \
  kolosal-agent:latest

# Check container status
docker ps
docker logs kolosal-agent
```

### Method 3: Package Manager Installation

#### Windows (using Chocolatey)
```powershell
# Install Chocolatey (if not installed)
Set-ExecutionPolicy Bypass -Scope Process -Force
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# Install Kolosal Agent (when available)
choco install kolosal-agent
```

#### Linux (using APT)
```bash
# Add repository (when available)
curl -fsSL https://packages.kolosal.ai/gpg | sudo apt-key add -
echo "deb https://packages.kolosal.ai/apt/ stable main" | sudo tee /etc/apt/sources.list.d/kolosal.list

# Install
sudo apt update
sudo apt install kolosal-agent
```

#### macOS (using Homebrew)
```bash
# Add tap (when available)
brew tap kolosalai/tap

# Install
brew install kolosal-agent
```

## 🔧 Build Configuration Options

### Standard Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Development Build with Tests
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### Optimized Production Build
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" \
  -DBUILD_SHARED_LIBS=OFF
```

### Cross-Platform Build Options
```bash
# Linux with specific compiler
cmake .. -DCMAKE_CXX_COMPILER=g++-11

# Windows with specific generator
cmake .. -G "Visual Studio 17 2022" -A x64

# macOS with specific SDK
cmake .. -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
```

## 🎯 Platform-Specific Instructions

### Windows Detailed Setup

#### Visual Studio Setup
```powershell
# Download Visual Studio Community (free)
# https://visualstudio.microsoft.com/vs/community/

# Required workloads:
# - Desktop development with C++
# - CMake tools for Visual Studio

# Alternative: Build Tools for Visual Studio
# https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
```

#### PowerShell Build Script
```powershell
# build.ps1
param(
    [string]$BuildType = "Release",
    [switch]$Tests,
    [switch]$Clean
)

if ($Clean) {
    Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
}

New-Item -ItemType Directory -Path "build" -Force
Set-Location "build"

$cmakeArgs = @(
    ".."
    "-G", "Visual Studio 17 2022"
    "-A", "x64"
    "-DCMAKE_BUILD_TYPE=$BuildType"
)

if ($Tests) {
    $cmakeArgs += "-DBUILD_TESTS=ON"
}

& cmake @cmakeArgs
& cmake --build . --config $BuildType --parallel

Set-Location ".."
```

#### Usage
```powershell
# Standard build
.\build.ps1

# Debug build with tests
.\build.ps1 -BuildType Debug -Tests

# Clean rebuild
.\build.ps1 -Clean
```

### Linux Detailed Setup

#### Ubuntu/Debian Package Installation
```bash
#!/bin/bash
# install-deps.sh

set -e

echo "Installing Kolosal Agent dependencies..."

# Update package list
sudo apt update

# Install build tools
sudo apt install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    unzip

# Install development libraries
sudo apt install -y \
    libcurl4-openssl-dev \
    libyaml-cpp-dev \
    libssl-dev \
    zlib1g-dev

# Install optional dependencies
sudo apt install -y \
    libgtest-dev \
    libgmock-dev \
    valgrind \
    gdb

echo "Dependencies installed successfully!"
```

#### Build Script
```bash
#!/bin/bash
# build.sh

set -e

BUILD_TYPE="${1:-Release}"
BUILD_TESTS="${2:-OFF}"
CLEAN="${3:-false}"

if [ "$CLEAN" = "true" ]; then
    rm -rf build
fi

mkdir -p build
cd build

cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_TESTS="$BUILD_TESTS" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

make -j$(nproc)

cd ..

echo "Build completed successfully!"
echo "Executable: build/kolosal-agent"
```

#### Usage
```bash
chmod +x install-deps.sh build.sh

# Install dependencies
./install-deps.sh

# Standard build
./build.sh

# Debug build with tests
./build.sh Debug ON

# Clean rebuild
./build.sh Release OFF true
```

### macOS Detailed Setup

#### Homebrew Installation
```bash
#!/bin/bash
# install-deps-macos.sh

set -e

echo "Installing Kolosal Agent dependencies on macOS..."

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# Install dependencies
brew install \
    cmake \
    git \
    curl \
    yaml-cpp \
    openssl

# Install optional dependencies
brew install \
    googletest \
    llvm

echo "Dependencies installed successfully!"
```

#### Build Script
```bash
#!/bin/bash
# build-macos.sh

set -e

BUILD_TYPE="${1:-Release}"
BUILD_TESTS="${2:-OFF}"

mkdir -p build
cd build

# Set up environment for Homebrew
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"

cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_TESTS="$BUILD_TESTS" \
    -DCMAKE_PREFIX_PATH="/opt/homebrew" \
    -DOPENSSL_ROOT_DIR="/opt/homebrew/opt/openssl"

make -j$(sysctl -n hw.ncpu)

cd ..

echo "Build completed successfully!"
echo "Executable: build/kolosal-agent"
```

## 🐳 Docker Installation

### Dockerfile
```dockerfile
# Dockerfile
FROM ubuntu:22.04 AS builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libcurl4-openssl-dev \
    libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
WORKDIR /app
COPY . .

# Build application
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Runtime image
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libcurl4 \
    libyaml-cpp0.7 \
    && rm -rf /var/lib/apt/lists/*

# Copy built application
WORKDIR /app
COPY --from=builder /app/build/kolosal-agent /app/
COPY --from=builder /app/config.yaml /app/
COPY --from=builder /app/agent.yaml /app/

# Create non-root user
RUN useradd -m -s /bin/bash kolosal
USER kolosal

# Expose port
EXPOSE 8081

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8081/v1/health || exit 1

# Start application
CMD ["./kolosal-agent"]
```

### Docker Compose
```yaml
# docker-compose.yml
version: '3.8'

services:
  kolosal-agent:
    build: .
    ports:
      - "8081:8081"
    volumes:
      - ./config:/app/config:ro
      - ./data:/app/data
      - ./logs:/app/logs
    environment:
      - KOLOSAL_LOG_LEVEL=INFO
      - KOLOSAL_SERVER_HOST=0.0.0.0
      - KOLOSAL_SERVER_PORT=8081
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8081/v1/health"]
      interval: 30s
      timeout: 10s
      retries: 3
      start_period: 10s

  # Optional: Add monitoring
  prometheus:
    image: prom/prometheus:latest
    ports:
      - "9090:9090"
    volumes:
      - ./monitoring/prometheus.yml:/etc/prometheus/prometheus.yml
    depends_on:
      - kolosal-agent

  grafana:
    image: grafana/grafana:latest
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=admin
    volumes:
      - grafana-storage:/var/lib/grafana
    depends_on:
      - prometheus

volumes:
  grafana-storage:
```

## ✅ Installation Verification

### Basic Verification
```bash
# Check if application starts
./kolosal-agent --version

# Test HTTP server
curl http://localhost:8081/v1/health

# Check system status
curl http://localhost:8081/v1/system/status
```

### Comprehensive Testing
```bash
# Run built-in tests (if built with -DBUILD_TESTS=ON)
cd build
ctest --output-on-failure

# Test API endpoints
curl -X GET http://localhost:8081/v1/agents
curl -X GET http://localhost:8081/v1/functions
curl -X GET http://localhost:8081/v1/system/metrics
```

### Performance Testing
```bash
# Simple load test
for i in {1..10}; do
    curl -s http://localhost:8081/v1/health > /dev/null &
done
wait

# Check response times
time curl http://localhost:8081/v1/system/status
```

## 🔧 Post-Installation Configuration

### Service Setup (Linux)
```bash
# Create systemd service
sudo tee /etc/systemd/system/kolosal-agent.service > /dev/null <<EOF
[Unit]
Description=Kolosal Agent System
After=network.target

[Service]
Type=simple
User=kolosal
WorkingDirectory=/opt/kolosal-agent
ExecStart=/opt/kolosal-agent/kolosal-agent
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# Enable and start service
sudo systemctl enable kolosal-agent
sudo systemctl start kolosal-agent
sudo systemctl status kolosal-agent
```

### Windows Service Setup
```powershell
# Install as Windows Service using NSSM
# Download NSSM from https://nssm.cc/download

# Install service
nssm install "Kolosal Agent" "C:\kolosal-agent\kolosal-agent.exe"
nssm set "Kolosal Agent" AppDirectory "C:\kolosal-agent"
nssm set "Kolosal Agent" DisplayName "Kolosal Agent System"
nssm set "Kolosal Agent" Description "Multi-agent AI system"

# Start service
nssm start "Kolosal Agent"
```

### Environment Configuration
```bash
# Create configuration directory
sudo mkdir -p /etc/kolosal-agent
sudo cp config.yaml /etc/kolosal-agent/
sudo cp agent.yaml /etc/kolosal-agent/

# Set permissions
sudo chown -R kolosal:kolosal /etc/kolosal-agent
sudo chmod 644 /etc/kolosal-agent/*.yaml

# Create data directory
sudo mkdir -p /var/lib/kolosal-agent/data
sudo chown -R kolosal:kolosal /var/lib/kolosal-agent

# Create log directory
sudo mkdir -p /var/log/kolosal-agent
sudo chown -R kolosal:kolosal /var/log/kolosal-agent
```

## 🐛 Troubleshooting Installation

### Common Build Issues

#### CMake Issues
```bash
# CMake version too old
cmake --version
# Update: pip install cmake

# Clear CMake cache
rm -rf build/CMakeCache.txt build/CMakeFiles
```

#### Dependency Issues
```bash
# Missing dependencies on Ubuntu
sudo apt install --fix-missing
sudo apt install -y pkg-config

# Missing headers
sudo apt install -y libcurl4-openssl-dev libyaml-cpp-dev
```

#### Compiler Issues
```bash
# GCC version check
gcc --version
g++ --version

# Install newer GCC (Ubuntu)
sudo apt install gcc-11 g++-11
export CC=gcc-11
export CXX=g++-11
```

### Runtime Issues

#### Port Conflicts
```bash
# Check port usage
netstat -tlnp | grep 8081
lsof -i :8081

# Use different port
./kolosal-agent --port 9090
```

#### Permission Issues
```bash
# Fix file permissions
chmod +x kolosal-agent
chmod 644 *.yaml

# SELinux issues (CentOS/RHEL)
setsebool -P httpd_can_network_connect 1
```

#### Memory Issues
```bash
# Check available memory
free -h

# Monitor memory usage
top -p $(pgrep kolosal-agent)
```

### Getting Help

#### Log Analysis
```bash
# Check application logs
./kolosal-agent --log-level DEBUG

# System logs
journalctl -u kolosal-agent -f  # Linux
Get-EventLog Application | Where-Object Source -eq "Kolosal Agent"  # Windows
```

#### Diagnostic Information
```bash
# System information
uname -a
cat /etc/os-release

# Build information
./kolosal-agent --version --verbose

# Configuration validation
./kolosal-agent --validate-config
```

#### Support Resources
- **GitHub Issues**: [Report bugs](https://github.com/kolosalai/kolosal-agent/issues)
- **Documentation**: [Online docs](https://docs.kolosal.ai)
- **Community**: [Discord server](https://discord.gg/kolosal-ai)

---

## 📚 Next Steps

After successful installation:

1. **Configure the system**: See [Configuration Guide](config.md)
2. **Learn the API**: Review [API Reference](api.md)
3. **Try examples**: Check [Examples Guide](examples.md)

Congratulations! You now have Kolosal Agent System v1.0 installed and ready to use.
