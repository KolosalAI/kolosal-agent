# Configuration Migration Guide

This guide helps you migrate from hardcoded configurations to environment variable-based configurations for the Kolosal Agent System.

## Quick Migration Steps

### 1. Backup Your Current Configuration

```bash
# Create backups
cp config.yaml config.yaml.backup
cp agent.yaml agent.yaml.backup
```

### 2. Copy the Environment Template

```bash
# Copy the template to create your .env file
cp .env.template .env
```

### 3. Update Your .env File

Edit `.env` with your specific values:

```bash
# Replace these with your actual values
KOLOSAL_API_KEY=your-actual-api-key
KOLOSAL_SEARCH_API_KEY=your-search-api-key
KOLOSAL_QDRANT_API_KEY=your-qdrant-api-key

# Update paths for your environment
KOLOSAL_MODEL_PATH=./models
KOLOSAL_ENGINE_PATH=./build/Debug  # Windows
# KOLOSAL_ENGINE_PATH=./build      # Linux/macOS
```

### 4. Set File Permissions (Linux/macOS)

```bash
# Protect your .env file
chmod 600 .env
```

### 5. Test the Configuration

```bash
# Load environment variables (Linux/macOS)
source .env

# Or on Windows PowerShell:
# Get-Content .env | ForEach-Object { if ($_ -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process') } }

# Start the application
./kolosal-agent
```

## Before and After Examples

### config.yaml Migration

#### Before (Hardcoded Paths)
```yaml
database:
  faiss:
    index_path: D:\Works\Genta\codes\kolosal-agent\data\faiss_index
  qdrant:
    api_key: ""
    host: localhost

models:
  - id: qwen2.5-0.5b
    path: D:\Works\Genta\codes\kolosal-agent\models\qwen2.5-0.5b-instruct-q4_k_m.gguf

search:
  api_key: ""
```

#### After (Environment Variables)
```yaml
database:
  faiss:
    index_path: ${KOLOSAL_FAISS_INDEX_PATH:-./data/faiss_index}
  qdrant:
    api_key: ${KOLOSAL_QDRANT_API_KEY:-}
    host: ${KOLOSAL_QDRANT_HOST:-localhost}

models:
  - id: qwen2.5-0.5b
    path: ${KOLOSAL_MODEL_PATH:-./models}/qwen2.5-0.5b-instruct-q4_k_m.gguf

search:
  api_key: ${KOLOSAL_SEARCH_API_KEY:-}
```

#### .env File
```bash
KOLOSAL_FAISS_INDEX_PATH=./data/faiss_index
KOLOSAL_QDRANT_API_KEY=your-qdrant-api-key
KOLOSAL_QDRANT_HOST=localhost
KOLOSAL_MODEL_PATH=./models
KOLOSAL_SEARCH_API_KEY=your-search-api-key
```

### agent.yaml Migration

#### Before
```yaml
system:
  host: "127.0.0.1"
  port: 8080

security:
  api_key: ""

kolosal_server:
  models_directory: "./models"
```

#### After
```yaml
system:
  host: ${KOLOSAL_HOST:-127.0.0.1}
  port: ${KOLOSAL_PORT:-8080}

security:
  api_key: ${KOLOSAL_API_KEY:-}

kolosal_server:
  models_directory: ${KOLOSAL_MODEL_PATH:-./models}
```

## Platform-Specific Migration

### Windows Development

```powershell
# Set Windows-specific variables
$env:KOLOSAL_ENGINE_PATH = "./build/Debug"
$env:KOLOSAL_ENGINE_LIBRARY = "llama-cpu.dll"
$env:KOLOSAL_MODEL_PATH = "./models"
$env:KOLOSAL_LOG_LEVEL = "DEBUG"
```

### Linux/macOS Development

```bash
# Set Unix-specific variables
export KOLOSAL_ENGINE_PATH="./build"
export KOLOSAL_ENGINE_LIBRARY="libllama-cpu.so"
export KOLOSAL_MODEL_PATH="./models"
export KOLOSAL_LOG_LEVEL="DEBUG"
```

### Production Deployment

#### Linux Server
```bash
# /etc/systemd/system/kolosal-agent.service
[Unit]
Description=Kolosal Agent System
After=network.target

[Service]
Type=simple
User=kolosal
WorkingDirectory=/opt/kolosal
ExecStart=/opt/kolosal/kolosal-agent
Environment=KOLOSAL_API_KEY=prod-api-key-12345
Environment=KOLOSAL_MODEL_PATH=/opt/kolosal/models
Environment=KOLOSAL_ENGINE_PATH=/opt/kolosal/engines
Environment=KOLOSAL_LOG_LEVEL=INFO
Environment=KOLOSAL_HOST=0.0.0.0
Environment=KOLOSAL_PORT=8081
Restart=always

[Install]
WantedBy=multi-user.target
```

#### Docker Deployment
```dockerfile
# Dockerfile
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y build-essential cmake

# Copy application
COPY . /app
WORKDIR /app

# Build
RUN mkdir build && cd build && cmake .. && make

# Set environment variables
ENV KOLOSAL_MODEL_PATH=/app/models
ENV KOLOSAL_ENGINE_PATH=/app/build
ENV KOLOSAL_DATA_PATH=/app/data
ENV KOLOSAL_HOST=0.0.0.0
ENV KOLOSAL_PORT=8081

# Create required directories
RUN mkdir -p /app/models /app/data /app/logs

# Expose port
EXPOSE 8081

# Start command
CMD ["./kolosal-agent"]
```

```yaml
# docker-compose.yml
version: '3.8'

services:
  kolosal-agent:
    build: .
    ports:
      - "8081:8081"
    environment:
      - KOLOSAL_API_KEY=${KOLOSAL_API_KEY}
      - KOLOSAL_QDRANT_API_KEY=${KOLOSAL_QDRANT_API_KEY}
      - KOLOSAL_SEARCH_API_KEY=${KOLOSAL_SEARCH_API_KEY}
      - KOLOSAL_MODEL_PATH=/app/models
      - KOLOSAL_QDRANT_HOST=qdrant
      - KOLOSAL_LOG_LEVEL=INFO
    volumes:
      - ./models:/app/models:ro
      - kolosal_data:/app/data
    depends_on:
      - qdrant

  qdrant:
    image: qdrant/qdrant:latest
    ports:
      - "6333:6333"
    environment:
      - QDRANT__SERVICE__HTTP_PORT=6333
    volumes:
      - qdrant_storage:/qdrant/storage

volumes:
  kolosal_data:
  qdrant_storage:
```

## Troubleshooting Common Issues

### Issue: Environment Variables Not Expanding

**Problem**: Configuration still uses literal `${VARIABLE_NAME}` instead of actual values.

**Solution**: Ensure your YAML parser supports environment variable expansion. Some parsers require explicit enabling of this feature.

### Issue: Permission Denied Errors

**Problem**: Application can't access model files or create directories.

**Solution**:
```bash
# Fix file permissions
chmod -R 755 ./models
chmod -R 755 ./data

# Or create directories with proper permissions
mkdir -p ./models ./data ./logs
chmod 755 ./models ./data ./logs
```

### Issue: API Keys Not Working

**Problem**: Authentication failures despite setting environment variables.

**Solutions**:
1. Verify variables are set:
   ```bash
   echo $KOLOSAL_API_KEY
   ```
2. Check for extra spaces or special characters
3. Restart the application after setting variables
4. Use quoted values if keys contain special characters:
   ```bash
   export KOLOSAL_API_KEY="key-with-special-chars!"
   ```

### Issue: Relative Paths Not Resolving

**Problem**: Application can't find models or data files.

**Solutions**:
1. Verify working directory:
   ```bash
   pwd  # Should be the kolosal-agent root directory
   ```
2. Use absolute paths if needed:
   ```bash
   export KOLOSAL_MODEL_PATH="/absolute/path/to/models"
   ```
3. Check file existence:
   ```bash
   ls -la ./models/
   ```

### Issue: Platform-Specific Library Loading

**Problem**: Inference engine library not found.

**Solutions**:

Windows:
```powershell
$env:KOLOSAL_ENGINE_LIBRARY = "llama-cpu.dll"
$env:KOLOSAL_ENGINE_PATH = "./build/Debug"
```

Linux:
```bash
export KOLOSAL_ENGINE_LIBRARY="libllama-cpu.so"
export KOLOSAL_ENGINE_PATH="./build"
```

macOS:
```bash
export KOLOSAL_ENGINE_LIBRARY="libllama-cpu.dylib"
export KOLOSAL_ENGINE_PATH="./build"
```

## Validation Script

Create a validation script to check your configuration:

```bash
#!/bin/bash
# validate-config.sh

echo "🔍 Validating Kolosal Agent Configuration..."

# Check environment variables
required_vars=(
    "KOLOSAL_MODEL_PATH"
    "KOLOSAL_ENGINE_PATH"
)

optional_vars=(
    "KOLOSAL_API_KEY"
    "KOLOSAL_QDRANT_API_KEY"
    "KOLOSAL_SEARCH_API_KEY"
)

echo "📋 Required Variables:"
for var in "${required_vars[@]}"; do
    if [ -n "${!var}" ]; then
        echo "  ✅ $var = ${!var}"
    else
        echo "  ❌ $var (not set)"
    fi
done

echo "📋 Optional Variables:"
for var in "${optional_vars[@]}"; do
    if [ -n "${!var}" ]; then
        echo "  ✅ $var = [REDACTED]"
    else
        echo "  ⚠️  $var (not set - will use default)"
    fi
done

# Check file paths
echo "📁 File Path Validation:"
if [ -d "${KOLOSAL_MODEL_PATH:-./models}" ]; then
    echo "  ✅ Model directory exists: ${KOLOSAL_MODEL_PATH:-./models}"
    echo "    Models found:"
    ls -1 "${KOLOSAL_MODEL_PATH:-./models}"/*.gguf 2>/dev/null | sed 's/^/      /' || echo "      ⚠️  No .gguf files found"
else
    echo "  ❌ Model directory missing: ${KOLOSAL_MODEL_PATH:-./models}"
fi

if [ -d "${KOLOSAL_ENGINE_PATH:-./build}" ]; then
    echo "  ✅ Engine directory exists: ${KOLOSAL_ENGINE_PATH:-./build}"
else
    echo "  ❌ Engine directory missing: ${KOLOSAL_ENGINE_PATH:-./build}"
fi

echo "✅ Validation complete!"
```

Run the validation:
```bash
chmod +x validate-config.sh
./validate-config.sh
```

## Security Checklist

- [ ] API keys removed from configuration files
- [ ] `.env` file added to `.gitignore`
- [ ] File permissions set correctly (`chmod 600 .env`)
- [ ] Production uses secure secret management
- [ ] Default values are safe for development
- [ ] No hardcoded paths in configuration files
- [ ] Environment-specific configurations tested

## Next Steps

1. **Update your CI/CD**: Configure environment variables in your deployment pipeline
2. **Document team setup**: Share this guide with your team
3. **Test different environments**: Verify configuration works in dev, staging, and production
4. **Monitor logs**: Check that configurations are loading correctly
5. **Regular security review**: Rotate API keys and review access periodically
