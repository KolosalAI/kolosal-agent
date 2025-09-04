# Configuration Guide

Complete reference for configuring the Kolosal Agent System v2.0.

## 📋 Configuration Overview

The Kolosal Agent System uses two main configuration files:
- **`config.yaml`** - Server and system-level configuration
- **`agent.yaml`** - Agent definitions and behavior

## 🔧 Server Configuration (`config.yaml`)

### Server Settings

```yaml
server:
  port: 8080                    # HTTP server port
  host: "0.0.0.0"              # Bind address (0.0.0.0 for all interfaces)
  idle_timeout: 300            # Connection idle timeout in seconds
  allow_public_access: false   # Allow external connections
  allow_internet_access: false # Enable internet access features
```

### Logging Configuration

```yaml
logging:
  level: INFO                  # Log level: DEBUG, INFO, WARN, ERROR
  file: ""                     # Log file path (empty = console only)
  access_log: false            # Log HTTP requests
  quiet_mode: false            # Suppress non-error output
  show_request_details: true   # Show detailed request information
```

### Authentication & Security

```yaml
auth:
  enabled: false               # Enable authentication
  require_api_key: false       # Require API key for requests
  api_key_header: "X-API-Key"  # Header name for API key
  api_keys: []                 # List of valid API keys
  
  rate_limit:
    enabled: true              # Enable rate limiting
    max_requests: 100          # Max requests per window
    window_size: 60            # Time window in seconds
  
  cors:
    enabled: true              # Enable CORS
    allow_credentials: false   # Allow credentials in CORS
    max_age: 86400            # CORS preflight cache time
    allowed_origins:          # Allowed origins
      - "*"
    allowed_methods:          # Allowed HTTP methods
      - GET
      - POST
      - PUT
      - DELETE
      - OPTIONS
      - HEAD
      - PATCH
    allowed_headers:          # Allowed headers
      - Content-Type
      - Authorization
      - X-Requested-With
      - Accept
      - Origin
```

### Web Search Configuration

```yaml
search:
  enabled: true                # Enable web search functionality
  searxng_url: "https://searx.stream/"  # SearXNG instance URL
  timeout: 30                  # Search timeout in seconds
  max_results: 20              # Maximum search results
  default_engine: ""           # Default search engine
  api_key: ""                  # API key if required
  enable_safe_search: true     # Enable safe search filtering
  default_format: "json"       # Response format
  default_language: "en"       # Default search language
  default_category: "general"  # Default search category
```

### Database Configuration

```yaml
database:
  vector_database: "faiss"     # Vector database type
  retrieval_embedding_model: "qwen3-embedding-0.6b"  # Embedding model
  
  faiss:
    index_type: "Flat"         # FAISS index type
    index_path: ".\data\faiss_index"  # Index storage path
    dimensions: 1536           # Vector dimensions
    normalize_vectors: true    # Normalize vectors
    nlist: 100                # Number of clusters (for IVF indices)
    nprobe: 10                # Number of clusters to search
    use_gpu: false            # Use GPU acceleration
    gpu_device: 0             # GPU device ID
    metric_type: "IP"         # Distance metric (IP/L2)
```

### Model Configuration

```yaml
models:
  - id: "gemma3-1b"           # Model identifier
    path: "https://huggingface.co/kolosal/gemma-3-1b/resolve/main/google_gemma-3-1b-it_q4_k_m.gguf"
    type: "llm"               # Model type: llm/embedding
    load_immediately: true    # Load on startup
    main_gpu_id: 0           # Primary GPU device
    inference_engine: "llama-cpu"  # Inference engine to use
    
    load_params:
      n_ctx: 2048             # Context window size
      n_keep: 1024            # Tokens to keep
      use_mmap: true          # Use memory mapping
      use_mlock: false        # Lock memory pages
      n_parallel: 1           # Parallel sequences
      cont_batching: true     # Continuous batching
      warmup: false           # Warmup on load
      n_gpu_layers: 100       # GPU layers (0 = CPU only)
      n_batch: 2048           # Batch size
      n_ubatch: 512           # Micro batch size
```

### Inference Engines

```yaml
inference_engines:
  - name: "llama-cpu"         # Engine name
    library_path: "./build/Release/llama-cpu.dll"  # Library path
    version: "1.0.0"          # Engine version
    description: "CPU-based inference engine for LLaMA models"
    load_on_startup: true     # Load on startup

default_inference_engine: "llama-cpu"  # Default engine
```

### Feature Flags

```yaml
features:
  health_check: true          # Enable health check endpoint
  metrics: true               # Enable metrics collection
```

## 🤖 Agent Configuration (`agent.yaml`)

### System Settings

```yaml
system:
  name: "Kolosal Agent System"  # System name
  version: "1.0.0"              # System version
  host: "127.0.0.1"            # Host address
  port: 8080                   # Port number
  log_level: "info"            # Log level
  max_concurrent_requests: 100  # Max concurrent requests
```

### System Instruction

```yaml
system_instruction: |
  You are a helpful AI assistant that is part of the Kolosal Agent System.
  You have been designed to assist users with various tasks including:
  
  - Answering questions and providing information
  - Analyzing text and data
  - Helping with research and problem-solving
  - Providing explanations and tutorials
  - Assisting with creative tasks
  
  You should always:
  - Be helpful, accurate, and honest
  - Admit when you don't know something
  - Provide clear and well-structured responses
  - Be respectful and professional
  - Follow ethical guidelines
```

### Agent Definitions

```yaml
agents:
  - name: "Assistant"           # Agent name
    capabilities:               # Agent capabilities
      - "chat"
      - "analysis" 
      - "reasoning"
    auto_start: true           # Start automatically
    system_prompt: |           # Agent-specific prompt
      You are an AI assistant specialized in general conversation and help.
      You excel at answering questions, providing explanations, and helping
      users with various tasks. Be friendly, helpful, and informative.
  
  - name: "Analyzer"
    capabilities:
      - "analysis"
      - "data_processing"
      - "summarization"
    auto_start: true
    system_prompt: |
      You are an AI analyst specialized in text and data analysis.
      Your role is to examine, process, and summarize information effectively.
      Provide detailed analysis with clear insights and conclusions.
```

### Function Definitions

```yaml
functions:
  chat:
    description: "Interactive chat functionality with AI model support"
    timeout: 30000            # Timeout in milliseconds
    parameters:
      - name: "message"
        type: "string"
        required: true
        description: "Message to send to the agent"
      - name: "model"
        type: "string"
        required: true
        description: "Name of the AI model to use for chat"
  
  analyze:
    description: "Text and data analysis functionality"
    timeout: 60000
    parameters:
      - name: "text"
        type: "string"
        required: true
        description: "Text to analyze"
      - name: "model"
        type: "string"
        required: false
        description: "AI model for enhanced analysis"
      - name: "analysis_type"
        type: "string"
        required: false
        description: "Type of analysis (sentiment, summary, keywords)"
```

### Performance Configuration

```yaml
performance:
  max_memory_usage: "2GB"     # Maximum memory usage
  cache_size: "512MB"         # Cache size
  worker_threads: 4           # Number of worker threads
  request_timeout: 30000      # Request timeout in milliseconds
  max_request_size: "10MB"    # Maximum request size
```

### Security Configuration

```yaml
security:
  enable_cors: true           # Enable CORS
  allowed_origins:            # Allowed CORS origins
    - "http://localhost:3000"
    - "http://127.0.0.1:3000"
  max_request_rate: 100       # Requests per minute per IP
  enable_auth: false          # Enable authentication
  api_key: ""                 # API key (if auth enabled)
```

## 🎯 Configuration Examples

### Development Configuration

```yaml
# config.yaml (Development)
server:
  port: 8080
  host: "127.0.0.1"
  allow_public_access: false

logging:
  level: DEBUG
  show_request_details: true
  quiet_mode: false

auth:
  enabled: false

features:
  health_check: true
  metrics: true
```

### Production Configuration

```yaml
# config.yaml (Production)
server:
  port: 8080
  host: "0.0.0.0"
  allow_public_access: true
  idle_timeout: 600

logging:
  level: INFO
  file: "/var/log/kolosal-agent/app.log"
  access_log: true
  quiet_mode: true

auth:
  enabled: true
  require_api_key: true
  api_keys:
    - "sk-your-production-api-key-here"
  
  rate_limit:
    enabled: true
    max_requests: 1000
    window_size: 60

features:
  health_check: true
  metrics: true
```

### High-Performance Configuration

```yaml
# agent.yaml (High Performance)
performance:
  max_memory_usage: "8GB"
  cache_size: "2GB"
  worker_threads: 16
  request_timeout: 60000
  max_request_size: "50MB"

system:
  max_concurrent_requests: 500

# Use GPU acceleration
models:
  - id: "gemma3-1b"
    load_params:
      n_gpu_layers: -1        # Use all GPU layers
      n_parallel: 4           # More parallel sequences
      n_batch: 4096          # Larger batch size
```

## 🔧 Configuration Management

### Loading Configuration

```bash
# Default configuration loading order:
# 1. Command line specified: --config /path/to/config.yaml
# 2. Current directory: ./config.yaml
# 3. Project root: ../config.yaml
# 4. System: /etc/kolosal-agent/config.yaml (Linux)

# Specify custom configuration
./kolosal-agent --config /path/to/custom-config.yaml
```

### Environment Variables

You can override configuration values using environment variables:

```bash
# Override server port
export KOLOSAL_SERVER_PORT=9090

# Override log level
export KOLOSAL_LOG_LEVEL=DEBUG

# Override auth settings
export KOLOSAL_AUTH_ENABLED=true
export KOLOSAL_API_KEY="your-api-key"
```

### Configuration Validation

The system validates configuration on startup:

```bash
# Validate configuration without starting
./kolosal-agent --validate-config

# Show current configuration
./kolosal-agent --show-config
```

## 🔍 Advanced Configuration

### Dynamic Configuration Reload

```bash
# Reload configuration without restart
curl -X POST http://localhost:8080/v1/system/reload \
  -H "Content-Type: application/json" \
  -d '{"config_file": "new-config.yaml"}'
```

### Configuration Templates

Create reusable configuration templates:

```yaml
# template-base.yaml
server: &server_defaults
  host: "0.0.0.0"
  idle_timeout: 300

logging: &logging_defaults
  level: INFO
  show_request_details: true

# production.yaml
<<: *server_defaults
<<: *logging_defaults
server:
  port: 8080
  allow_public_access: true
```

### Conditional Configuration

```yaml
# Use different settings based on environment
server:
  port: ${PORT:-8080}
  host: ${HOST:-127.0.0.1}

logging:
  level: ${LOG_LEVEL:-INFO}
  file: ${LOG_FILE:-""}

auth:
  enabled: ${AUTH_ENABLED:-false}
```

## 🐛 Configuration Troubleshooting

### Common Issues

**Configuration File Not Found:**
```bash
# Check file path and permissions
ls -la config.yaml
./kolosal-agent --config ./config.yaml
```

**Invalid YAML Syntax:**
```bash
# Validate YAML syntax
python -c "import yaml; yaml.safe_load(open('config.yaml'))"
```

**Permission Denied:**
```bash
# Check file permissions
chmod 644 config.yaml
```

**Port Already in Use:**
```yaml
# Change port in config.yaml
server:
  port: 9090  # Use different port
```

### Debugging Configuration

```bash
# Enable debug logging
./kolosal-agent --log-level DEBUG

# Show loaded configuration
./kolosal-agent --show-config

# Validate configuration
./kolosal-agent --validate-config
```

## 📚 Configuration Reference

### Complete Example Files

See the repository for complete example configurations:
- [`config.yaml`](../config.yaml) - Server configuration
- [`agent.yaml`](../agent.yaml) - Agent configuration
- [`examples/`](../examples/) - Additional configuration examples

### Best Practices

1. **Security**: Never commit API keys to version control
2. **Environment**: Use environment variables for sensitive data
3. **Validation**: Always validate configuration before deployment
4. **Backup**: Keep backup copies of working configurations
5. **Documentation**: Document custom configuration changes

---

For more advanced configuration topics, see the [Developer Guide](DEVELOPER_GUIDE.md) and [Deployment Guide](DEPLOYMENT.md).
