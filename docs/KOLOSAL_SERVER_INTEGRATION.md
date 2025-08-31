# Kolosal Server Integration Guide

This guide explains how the Kolosal Agent System integrates with the Kolosal Server for model inference.

## Overview

The Kolosal Agent System now includes automatic management of the Kolosal Server, which provides model inference capabilities. The system can automatically start, monitor, and stop the server as needed.

## Architecture

```
┌─────────────────────────────────────┐
│         Kolosal Agent System        │
│  ┌────────────────────────────────┐ │
│  │        Agent Manager           │ │
│  │  ┌───────────────────────────┐ │ │
│  │  │   KolosalServerLauncher   │ │ │
│  │  │                           │ │ │
│  │  │  - Starts kolosal-server  │ │ │
│  │  │  - Monitors process       │ │ │
│  │  │  - Handles failures       │ │ │
│  │  └───────────────────────────┘ │ │
│  │                                │ │
│  │  ┌──────┐ ┌──────┐ ┌──────┐    │ │
│  │  │Agent1│ │Agent2│ │Agent3│    │ │
│  │  └──────┘ └──────┘ └──────┘    │ │
│  └────────────────────────────────┘ │
│                │                    │
└────────────────┼────────────────────┘
                 │ HTTP Requests
                 ▼
┌─────────────────────────────────────┐
│         Kolosal Server              │
│                                     │
│  - Model inference                  │
│  - Chat completions                 │
│  - Text completions                 │
│  - Embeddings                       │
│                                     │
│  Port: 8081 (default)               │
└─────────────────────────────────────┘
```

## Features

### Automatic Server Management

- **Auto-start**: The Kolosal Server is automatically started when the agent system starts
- **Health monitoring**: Continuous monitoring of server health and status
- **Auto-recovery**: Automatic restart if the server crashes (planned for future release)
- **Graceful shutdown**: Proper shutdown when the agent system stops

### Model Configuration

- **Model mapping**: Agent configurations can specify model aliases that map to actual server models
- **Automatic discovery**: The system automatically detects available models from the server
- **Fallback handling**: If the server is unavailable, agents provide fallback responses

## Configuration

### Agent Configuration (agent.yaml)

```yaml
# Model configuration for agents - maps to Kolosal Server models
models:
  - name: "default"
    actual_name: "gemma3-1b"
    type: "llm"
    server_url: "http://127.0.0.1:8081"
    description: "Default LLM model (Gemma3-1B)"
  
  - name: "gemma3-1b"
    actual_name: "gemma3-1b"
    type: "llm"
    server_url: "http://127.0.0.1:8081"
    description: "Gemma3-1B language model"

agents:
  - name: "Assistant"
    model: "gemma3-1b"  # Uses the model configuration above
    capabilities: ["chat", "research"]
    # ... other configuration
```

### Server Configuration (config.yaml)

The Kolosal Server uses the existing `config.yaml` configuration file:

```yaml
server:
  port: 8081  # Different port from agent system (8080)
  allow_public_access: false

models:
  - id: gemma3-1b
    path: D:\Works\Genta\codes\kolosal-agent\models\gemma3-1b.gguf
    type: llm
    load_immediately: true
    inference_engine: llama-cpu
```

## API Endpoints

### Server Management

#### Start Kolosal Server
```http
POST /kolosal-server/start
```

Response:
```json
{
  "success": true,
  "message": "Kolosal server started successfully",
  "server_url": "http://127.0.0.1:8081",
  "status": 2
}
```

#### Stop Kolosal Server
```http
POST /kolosal-server/stop
```

Response:
```json
{
  "success": true,
  "message": "Kolosal server stopped successfully",
  "status": 0
}
```

#### Get Server Status
```http
GET /kolosal-server/status
```

Response:
```json
{
  "running": true,
  "status": 2,
  "status_string": "RUNNING",
  "server_url": "http://127.0.0.1:8081"
}
```

Status codes:
- `0`: STOPPED
- `1`: STARTING  
- `2`: RUNNING
- `3`: STOPPING
- `4`: ERROR

## Usage Examples

### Python Client

```python
import requests
import json

# Check server status
response = requests.get("http://localhost:8080/kolosal-server/status")
status = response.json()
print(f"Server status: {status['status_string']}")

# Start server if not running
if not status['running']:
    response = requests.post("http://localhost:8080/kolosal-server/start")
    result = response.json()
    print(f"Start result: {result['message']}")

# Use an agent with the server
payload = {
    "function": "chat",
    "model": "gemma3-1b",
    "params": {
        "message": "Hello, how are you?"
    }
}

response = requests.post("http://localhost:8080/agents/agent_123/execute", json=payload)
result = response.json()
print(f"Agent response: {result['response']}")
```

### curl Examples

```bash
# Check server status
curl http://localhost:8080/kolosal-server/status

# Start server
curl -X POST http://localhost:8080/kolosal-server/start

# Chat with agent using the server
curl -X POST http://localhost:8080/agents/agent_123/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "chat",
    "model": "gemma3-1b", 
    "params": {
      "message": "Explain quantum computing"
    }
  }'
```

## Troubleshooting

### Server Won't Start

1. **Check executable path**: Ensure the `kolosal-server.exe` is built and available
2. **Check port availability**: Make sure port 8081 is not in use
3. **Check configuration**: Verify `config.yaml` has valid model paths
4. **Check logs**: Look at agent system logs for detailed error messages

### Server Starts But Agents Can't Connect

1. **Check firewall**: Ensure Windows Firewall allows local connections
2. **Check URL configuration**: Verify the server URL is correctly configured
3. **Check model availability**: Ensure models are properly loaded on the server

### Poor Performance

1. **Hardware requirements**: Ensure adequate RAM and CPU for model inference
2. **Model size**: Consider using smaller models for faster inference
3. **Concurrent requests**: Limit the number of simultaneous agent requests

## Development

### Building with Server Support

```bash
# Configure with Kolosal Server support
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_KOLOSAL_SERVER=ON

# Build both agent system and server
cmake --build . --config Debug
```

### Testing

Use the provided test scripts:

```bash
# Windows
test_integration.bat

# Python (cross-platform)
python test_integration.py
```

### Adding Custom Models

1. Place model files in the `models/` directory
2. Update `config.yaml` with model configuration
3. Update `agent.yaml` with model mappings
4. Restart the system

## Future Enhancements

- **Auto-recovery**: Automatic restart of failed servers
- **Load balancing**: Multiple server instances for high availability
- **Model hot-swapping**: Dynamic model loading/unloading
- **Performance monitoring**: Detailed metrics and monitoring
- **GPU support**: CUDA and Vulkan acceleration
- **Distributed inference**: Multi-node model serving
