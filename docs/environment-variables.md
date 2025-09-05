# Environment Variables Configuration

This document outlines all environment variables used by the Kolosal Agent System to externalize secrets and configuration paths.

## Overview

The Kolosal Agent System supports environment variable substitution in configuration files using the format `${VARIABLE_NAME:-default_value}`. This allows you to:

- **Externalize secrets**: Keep API keys and sensitive data out of configuration files
- **Use relative paths**: Make configurations portable across different environments  
- **Platform independence**: Avoid hardcoded platform-specific paths
- **Environment-specific configs**: Use different values for development, staging, and production

## Core System Variables

### API Keys and Secrets

| Variable | Default | Description |
|----------|---------|-------------|
| `KOLOSAL_API_KEY` | _(empty)_ | Main API key for Kolosal Agent System authentication |
| `KOLOSAL_SEARCH_API_KEY` | _(empty)_ | API key for search services (SearXNG, Google, etc.) |
| `KOLOSAL_QDRANT_API_KEY` | _(empty)_ | API key for Qdrant vector database authentication |

### Network Configuration

| Variable | Default | Description |
|----------|---------|-------------|
| `KOLOSAL_HOST` | `127.0.0.1` | Host address for the Kolosal Agent System server |
| `KOLOSAL_PORT` | `8081` | Port for the Kolosal Agent System server |
| `KOLOSAL_QDRANT_HOST` | `localhost` | Qdrant database host |
| `KOLOSAL_QDRANT_PORT` | `6333` | Qdrant database port |
| `KOLOSAL_SEARXNG_URL` | `https://searx.stream/` | SearXNG search engine URL |

### File Paths and Directories

| Variable | Default | Description |
|----------|---------|-------------|
| `KOLOSAL_MODEL_PATH` | `./models` | Directory containing AI models (LLM and embedding models) |
| `KOLOSAL_ENGINE_PATH` | `./build/Debug` | Directory containing inference engine libraries |
| `KOLOSAL_FAISS_INDEX_PATH` | `./data/faiss_index` | Path to FAISS vector database index |
| `KOLOSAL_DATA_PATH` | `./data` | General data directory for logs, indices, and temporary files |
| `KOLOSAL_CONFIG_PATH` | `./config` | Directory containing configuration files |

### Database Configuration

| Variable | Default | Description |
|----------|---------|-------------|
| `KOLOSAL_QDRANT_COLLECTION` | `documents` | Qdrant collection name for document storage |
| `KOLOSAL_VECTOR_DB` | `qdrant` | Vector database type (`qdrant` or `faiss`) |

### Logging and Monitoring

| Variable | Default | Description |
|----------|---------|-------------|
| `KOLOSAL_LOG_LEVEL` | `INFO` | Logging level (`DEBUG`, `INFO`, `WARN`, `ERROR`) |
| `KOLOSAL_LOG_FILE` | _(empty)_ | Log file path (if empty, logs to console) |
| `KOLOSAL_ENABLE_METRICS` | `true` | Enable metrics collection and reporting |

## Usage Examples

### Development Environment

```bash
# Linux/macOS
export KOLOSAL_MODEL_PATH="./models"
export KOLOSAL_ENGINE_PATH="./build/Debug"
export KOLOSAL_LOG_LEVEL="DEBUG"
export KOLOSAL_QDRANT_HOST="localhost"

# Start the system
./kolosal-agent
```

```powershell
# Windows PowerShell
$env:KOLOSAL_MODEL_PATH = "./models"
$env:KOLOSAL_ENGINE_PATH = "./build/Debug"
$env:KOLOSAL_LOG_LEVEL = "DEBUG"
$env:KOLOSAL_QDRANT_HOST = "localhost"

# Start the system
.\Debug\kolosal-agent.exe
```

### Production Environment

```bash
# Linux/macOS
export KOLOSAL_API_KEY="your-production-api-key"
export KOLOSAL_QDRANT_API_KEY="your-qdrant-api-key"
export KOLOSAL_SEARCH_API_KEY="your-search-api-key"
export KOLOSAL_MODEL_PATH="/opt/kolosal/models"
export KOLOSAL_ENGINE_PATH="/opt/kolosal/engines"
export KOLOSAL_FAISS_INDEX_PATH="/var/lib/kolosal/faiss_index"
export KOLOSAL_LOG_LEVEL="INFO"
export KOLOSAL_LOG_FILE="/var/log/kolosal/agent.log"
export KOLOSAL_HOST="0.0.0.0"
export KOLOSAL_PORT="8081"

# Start the system
/opt/kolosal/kolosal-agent
```

### Docker Environment

```dockerfile
# Dockerfile
FROM ubuntu:22.04

# Set environment variables
ENV KOLOSAL_MODEL_PATH=/app/models
ENV KOLOSAL_ENGINE_PATH=/app/engines
ENV KOLOSAL_DATA_PATH=/app/data
ENV KOLOSAL_LOG_LEVEL=INFO
ENV KOLOSAL_HOST=0.0.0.0

# Copy application
COPY . /app
WORKDIR /app

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
      - KOLOSAL_MODEL_PATH=/app/models
      - KOLOSAL_QDRANT_HOST=qdrant
      - KOLOSAL_LOG_LEVEL=INFO
    volumes:
      - ./models:/app/models
      - ./data:/app/data
    depends_on:
      - qdrant

  qdrant:
    image: qdrant/qdrant:latest
    ports:
      - "6333:6333"
    volumes:
      - qdrant_data:/qdrant/storage

volumes:
  qdrant_data:
```

### Environment File (.env)

Create a `.env` file in your project root:

```bash
# .env file for local development
KOLOSAL_API_KEY=dev-api-key-12345
KOLOSAL_SEARCH_API_KEY=search-api-key-67890
KOLOSAL_QDRANT_API_KEY=qdrant-api-key-abcdef
KOLOSAL_MODEL_PATH=./models
KOLOSAL_ENGINE_PATH=./build/Debug
KOLOSAL_FAISS_INDEX_PATH=./data/faiss_index
KOLOSAL_LOG_LEVEL=DEBUG
KOLOSAL_QDRANT_HOST=localhost
KOLOSAL_QDRANT_PORT=6333
KOLOSAL_SEARXNG_URL=https://searx.stream/
```

Then load it before starting:

```bash
# Linux/macOS
source .env
./kolosal-agent

# Or use a tool like direnv
echo "source_env .env" > .envrc
direnv allow
./kolosal-agent
```

```powershell
# Windows PowerShell
Get-Content .env | ForEach-Object {
    if ($_ -match '^([^=]+)=(.*)$') {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
    }
}
.\Debug\kolosal-agent.exe
```

## Configuration File Integration

The environment variables work with the configuration files as follows:

### config.yaml
```yaml
search:
  api_key: ${KOLOSAL_SEARCH_API_KEY:-}

database:
  qdrant:
    host: ${KOLOSAL_QDRANT_HOST:-localhost}
    port: ${KOLOSAL_QDRANT_PORT:-6333}
    api_key: ${KOLOSAL_QDRANT_API_KEY:-}
  faiss:
    index_path: ${KOLOSAL_FAISS_INDEX_PATH:-./data/faiss_index}

models:
  - id: qwen2.5-0.5b
    path: ${KOLOSAL_MODEL_PATH:-./models}/qwen2.5-0.5b-instruct-q4_k_m.gguf
```

### agent.yaml
```yaml
system:
  host: ${KOLOSAL_HOST:-127.0.0.1}
  port: ${KOLOSAL_PORT:-8081}
  log_level: ${KOLOSAL_LOG_LEVEL:-info}

security:
  api_key: ${KOLOSAL_API_KEY:-}

kolosal_server:
  models_directory: ${KOLOSAL_MODEL_PATH:-./models}
```

## Security Best Practices

1. **Never commit API keys** to version control
2. **Use environment-specific .env files** (e.g., `.env.dev`, `.env.prod`)
3. **Set restrictive file permissions** on .env files: `chmod 600 .env`
4. **Use secrets management** in production (AWS Secrets Manager, HashiCorp Vault, etc.)
5. **Rotate API keys regularly**
6. **Use IAM roles** instead of API keys when possible (cloud environments)

## Migration Guide

To migrate from hardcoded configurations:

1. **Identify sensitive data** in your config files
2. **Create environment variables** for each secret
3. **Update configuration files** to use `${VAR:-default}` syntax
4. **Set environment variables** in your deployment environment
5. **Test the configuration** to ensure it works correctly

### Before (Insecure)
```yaml
search:
  api_key: "sk-1234567890abcdef"
database:
  qdrant:
    api_key: "qdrant-secret-key"
models:
  - path: "C:\Users\Developer\kolosal-agent\models\model.gguf"
```

### After (Secure)
```yaml
search:
  api_key: ${KOLOSAL_SEARCH_API_KEY:-}
database:
  qdrant:
    api_key: ${KOLOSAL_QDRANT_API_KEY:-}
models:
  - path: ${KOLOSAL_MODEL_PATH:-./models}/model.gguf
```

## Troubleshooting

### Common Issues

1. **Variable not expanded**: Ensure your configuration parser supports environment variable substitution
2. **Permission denied**: Check file permissions on model files and directories
3. **Path not found**: Verify that relative paths resolve correctly from your working directory
4. **API authentication failed**: Double-check that API keys are set correctly

### Debugging

Enable debug logging to see resolved configuration values:

```bash
export KOLOSAL_LOG_LEVEL=DEBUG
./kolosal-agent
```

This will show the final resolved configuration including expanded environment variables.
