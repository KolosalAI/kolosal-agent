# REST API Reference

Complete reference for the Kolosal Agent System v1.0 REST API.

## 📖 API Overview

The Kolosal Agent System provides a comprehensive REST API for managing agents, executing functions, and monitoring system health.

**Base URL**: `http://localhost:8081`  
**Content Type**: `application/json`  
**API Version**: `v1`

## 🚀 Endpoint Quick Reference

### Recommended Endpoints

| Endpoint | Use Case | Response Type |
|----------|----------|---------------|
| `POST /agent/execute` | **General queries & research** | Comprehensive multi-tool execution |
| `POST /v1/agents/{id}/execute` | **Specific function calls** | Single function result |
| `GET /v1/agents` | **Agent management** | Agent list and status |
| `GET /v1/system/status` | **Health monitoring** | System metrics |

### Endpoint Comparison

**Simple Execute (`/agent/execute`)**:
- ✅ Automatic agent selection
- ✅ Multi-tool execution (21+ tools)
- ✅ Comprehensive research capabilities
- ✅ LLM-powered synthesis
- ✅ Graceful fallback handling
- 🎯 **Best for**: Research, analysis, general queries

**Function Execute (`/v1/agents/{id}/execute`)**:
- ✅ Precise control over execution
- ✅ Single function focus
- ✅ Lower latency
- ✅ Specific agent targeting
- 🎯 **Best for**: Targeted tasks, integration workflows

## 🔐 Authentication

### API Key Authentication (Optional)

When authentication is enabled, include the API key in the request header:

```http
X-API-Key: your-api-key-here
```

### Rate Limiting

Default rate limits:
- **100 requests per minute** per IP address
- **Burst capacity**: 100 requests
- **Headers returned**:
  - `X-RateLimit-Limit`: Request limit
  - `X-RateLimit-Remaining`: Remaining requests
  - `X-RateLimit-Reset`: Reset time

## 🤖 Agent Management

### List All Agents

Get information about all agents in the system.

```http
GET /v1/agents
```

**Response**:
```json
{
  "agents": [
    {
      "id": "agent-001",
      "name": "Assistant",
      "type": "general",
      "running": true,
      "role": "assistant",
      "capabilities": ["chat", "analysis", "reasoning"],
      "created_at": "2025-08-28T10:00:00Z",
      "statistics": {
        "total_functions_executed": 142,
        "total_tools_executed": 89,
        "average_execution_time_ms": 245.6,
        "success_rate": 0.98,
        "last_executed": "2025-08-28T12:30:00Z"
      }
    }
  ],
  "total_count": 3,
  "system_running": true,
  "timestamp": "2025-08-28T12:35:00Z"
}
```

### Get Specific Agent

Get detailed information about a specific agent.

```http
GET /v1/agents/{agent_id}
```

**Parameters**:
- `agent_id` (string, required): Agent identifier

**Response**:
```json
{
  "id": "agent-001",
  "name": "Assistant",
  "type": "general",
  "running": true,
  "role": "assistant",
  "capabilities": ["chat", "analysis", "reasoning"],
  "functions": ["chat", "analyze", "status"],
  "config": {
    "auto_start": true,
    "max_concurrent_tasks": 5,
    "timeout": 30000
  },
  "statistics": {
    "total_functions_executed": 142,
    "average_execution_time_ms": 245.6,
    "success_rate": 0.98,
    "memory_usage_mb": 45.2,
    "cpu_usage_percent": 12.5
  },
  "status": {
    "health": "healthy",
    "last_heartbeat": "2025-08-28T12:34:00Z",
    "current_tasks": 2,
    "queue_size": 0
  }
}
```

### Create Agent

Create a new agent with specified configuration.

```http
POST /v1/agents
Content-Type: application/json
```

**Request Body**:
```json
{
  "name": "CustomAnalyst",
  "id": "analyst-002",
  "type": "specialist",
  "role": "analyst",
  "capabilities": ["data_processing", "research_synthesis"],
  "functions": ["analyze_data", "research_topic"],
  "config": {
    "auto_start": true,
    "max_concurrent_tasks": 5,
    "timeout": 60000,
    "priority": 2
  },
  "system_prompt": "You are an AI analyst specialized in data processing..."
}
```

**Response**:
```json
{
  "agent_id": "analyst-002",
  "name": "CustomAnalyst",
  "status": "created",
  "message": "Agent created successfully",
  "created_at": "2025-08-28T12:35:00Z"
}
```

### Update Agent

Update an existing agent's configuration.

```http
PUT /v1/agents/{agent_id}
Content-Type: application/json
```

**Request Body**:
```json
{
  "capabilities": ["data_processing", "research_synthesis", "reporting"],
  "config": {
    "max_concurrent_tasks": 10,
    "timeout": 90000
  },
  "system_prompt": "Updated system prompt..."
}
```

**Response**:
```json
{
  "agent_id": "analyst-002",
  "status": "updated",
  "message": "Agent configuration updated successfully",
  "updated_at": "2025-08-28T12:40:00Z"
}
```

### Delete Agent

Remove an agent from the system.

```http
DELETE /v1/agents/{agent_id}
```

**Response**:
```json
{
  "agent_id": "analyst-002",
  "status": "deleted",
  "message": "Agent deleted successfully",
  "deleted_at": "2025-08-28T12:45:00Z"
}
```

### Start Agent

Start a stopped agent.

```http
POST /v1/agents/{agent_id}/start
```

**Response**:
```json
{
  "agent_id": "agent-001",
  "status": "started",
  "message": "Agent started successfully",
  "started_at": "2025-08-28T12:50:00Z"
}
```

### Stop Agent

Stop a running agent.

```http
POST /v1/agents/{agent_id}/stop
```

**Response**:
```json
{
  "agent_id": "agent-001",
  "status": "stopped",
  "message": "Agent stopped successfully",
  "stopped_at": "2025-08-28T12:55:00Z"
}
```

## ⚡ Function Execution

### Simple Agent Execute (Recommended)

Execute a query with automatic tool execution and LLM response generation. This endpoint automatically selects the best available agent, runs all relevant tools, and provides a comprehensive response.

```http
POST /agent/execute
Content-Type: application/json
```

**Request Body**:
```json
{
  "query": "What is artificial intelligence?",
  "context": "Explain in simple terms for beginners",
  "model": "qwen2.5-0.5b",
  "agent": "Assistant"
}
```

**Parameters**:
- `query` (string, required): The main question or task to execute
- `context` (string, optional): Additional context to guide the response
- `model` (string, optional): Specific model to use for LLM responses (defaults to system default)
- `agent` (string, optional): Specific agent name to use (if not provided, selects best available agent automatically)

**Response**:
```json
{
  "agent_id": "abb29fc0-f07d-4e85-9894-3298a61ebc5b",
  "agent_name": "RetrievalAgent",
  "context": "Explain in simple terms for beginners",
  "query": "What is artificial intelligence?",
  "model": "qwen2.5-0.5b",
  "timestamp": "1756722373",
  "summary": {
    "total_tools": 21,
    "successful": 10,
    "failed": 11
  },
  "execution_log": [
    {
      "function": "analyze",
      "status": "success",
      "result_summary": "Data retrieved"
    },
    {
      "function": "add_document",
      "status": "failed",
      "error": "Retrieval system not available"
    }
  ],
  "tool_responses": {
    "analyze": {
      "agent": "RetrievalAgent",
      "analysis_type": "basic",
      "basic_stats": {
        "characters": 32,
        "lines": 1,
        "words": 4
      },
      "summary": "Text analysis completed by RetrievalAgent"
    },
    "cross_reference_search": {
      "correlation_threshold": 0.7,
      "databases_searched": ["internet", "knowledge_base"],
      "overall_correlation_score": 0.78,
      "status": "completed"
    }
  },
  "tools_executed": [
    "add_document", "analyze", "cross_reference_search", 
    "generate_research_report", "plan_research", "verify_facts"
  ],
  "llm_response": {
    "agent": "RetrievalAgent",
    "model_used": "qwen2.5-0.5b",
    "status": "fallback_success",
    "response": "I apologize, but I'm currently unable to connect to the specified model. However, I can provide information based on the tool execution results...",
    "timestamp": "2025-09-01 17:26:11"
  }
}
```

**Features**:
- **Automatic Agent Selection**: Intelligently selects running agents over stopped ones
- **Multi-Tool Execution**: Automatically runs all available tools (21 functions)
- **Comprehensive Results**: Provides both individual tool results and synthesized LLM response
- **Graceful Fallback**: Returns useful information even when LLM models are unavailable
- **Rich Context**: Includes tool results in LLM context for enhanced responses

**Use Cases**:
- Quick research and analysis tasks
- Comprehensive information gathering
- Multi-tool workflow execution
- System capability demonstration

### Execute Function

Execute a specific function on a designated agent.

```http
POST /v1/agents/{agent_id}/execute
Content-Type: application/json
```

**Request Body**:
```json
{
  "function": "analyze_data",
  "parameters": {
    "data_source": "sales_report.csv",
    "analysis_type": "comprehensive",
    "output_format": "json",
    "model": "qwen2.5-0.5b"
  },
  "timeout": 60000,
  "async": false
}
```

**Synchronous Response**:
```json
{
  "execution_id": "exec-12345",
  "agent_id": "analyst-002",
  "function": "analyze_data",
  "status": "completed",
  "result": {
    "summary": "Analysis completed successfully",
    "insights": [
      "Sales increased 15% from previous quarter",
      "Top performing product: Widget A"
    ],
    "data": {
      "total_sales": 125000,
      "growth_rate": 0.15,
      "top_products": ["Widget A", "Widget B"]
    }
  },
  "execution_time_ms": 2341,
  "started_at": "2025-08-28T13:00:00Z",
  "completed_at": "2025-08-28T13:00:02Z"
}
```

**Asynchronous Response** (when `async: true`):
```json
{
  "execution_id": "exec-12345",
  "agent_id": "analyst-002",
  "function": "analyze_data",
  "status": "running",
  "message": "Function execution started",
  "started_at": "2025-08-28T13:00:00Z",
  "status_url": "/v1/executions/exec-12345"
}
```

### Get Execution Status

Check the status of an asynchronous function execution.

```http
GET /v1/executions/{execution_id}
```

**Response**:
```json
{
  "execution_id": "exec-12345",
  "agent_id": "analyst-002",
  "function": "analyze_data",
  "status": "completed",
  "progress": 100,
  "result": {
    "summary": "Analysis completed successfully",
    "data": {...}
  },
  "execution_time_ms": 2341,
  "started_at": "2025-08-28T13:00:00Z",
  "completed_at": "2025-08-28T13:00:02Z"
}
```

### Cancel Execution

Cancel a running function execution.

```http
DELETE /v1/executions/{execution_id}
```

**Response**:
```json
{
  "execution_id": "exec-12345",
  "status": "cancelled",
  "message": "Execution cancelled successfully",
  "cancelled_at": "2025-08-28T13:01:00Z"
}
```

## 🏥 System Management

### System Status

Get overall system status and health information.

```http
GET /v1/system/status
```

**Response**:
```json
{
  "system_running": true,
  "status": "healthy",
  "version": "1.0.0",
  "uptime_seconds": 86400,
  "total_agents": 4,
  "running_agents": 3,
  "total_executions": 1250,
  "active_executions": 5,
  "system_metrics": {
    "cpu_usage_percent": 25.5,
    "memory_usage_mb": 1024,
    "memory_usage_percent": 32.1,
    "disk_usage_mb": 2048,
    "network_connections": 15
  },
  "kolosal_server": {
    "running": true,
    "health": "healthy",
    "models_loaded": 2,
    "active_requests": 3
  },
  "timestamp": "2025-08-28T13:05:00Z"
}
```

### Health Check

Simple health check endpoint for monitoring.

```http
GET /v1/health
```

**Response**:
```json
{
  "status": "healthy",
  "timestamp": "2025-08-28T13:05:00Z",
  "uptime": 86400,
  "system_running": true,
  "checks": {
    "database": "healthy",
    "kolosal_server": "healthy",
    "agents": "healthy"
  }
}
```

### System Metrics

Get detailed system performance metrics.

```http
GET /v1/system/metrics
```

**Response**:
```json
{
  "timestamp": "2025-08-28T13:05:00Z",
  "system": {
    "cpu_usage_percent": 25.5,
    "memory_total_mb": 3200,
    "memory_used_mb": 1024,
    "memory_available_mb": 2176,
    "disk_total_gb": 100,
    "disk_used_gb": 45,
    "network_rx_bytes": 1048576,
    "network_tx_bytes": 524288
  },
  "application": {
    "total_requests": 5000,
    "requests_per_second": 12.5,
    "average_response_time_ms": 156,
    "error_rate": 0.02,
    "active_connections": 15
  },
  "agents": {
    "total_agents": 4,
    "running_agents": 3,
    "total_executions": 1250,
    "average_execution_time_ms": 234,
    "success_rate": 0.98
  }
}
```

### Reload Configuration

Reload system configuration without restart.

```http
POST /v1/system/reload
Content-Type: application/json
```

**Request Body**:
```json
{
  "config_file": "new-config.yaml",
  "restart_agents": false
}
```

**Response**:
```json
{
  "status": "success",
  "message": "Configuration reloaded successfully",
  "reloaded_at": "2025-08-28T13:10:00Z",
  "changes": {
    "modified_settings": ["logging.level", "server.timeout"],
    "agents_restarted": 0
  }
}
```

### Shutdown System

Gracefully shutdown the system.

```http
POST /v1/system/shutdown
Content-Type: application/json
```

**Request Body**:
```json
{
  "force": false,
  "timeout_seconds": 30
}
```

**Response**:
```json
{
  "status": "shutting_down",
  "message": "System shutdown initiated",
  "shutdown_at": "2025-08-28T13:15:00Z",
  "estimated_completion": "2025-08-28T13:15:30Z"
}
```

## 📋 Function Registry

### List Available Functions

Get all available functions in the system.

```http
GET /v1/functions
```

**Response**:
```json
{
  "functions": [
    {
      "name": "chat",
      "description": "Interactive chat functionality with AI model support",
      "category": "communication",
      "timeout": 30000,
      "parameters": [
        {
          "name": "message",
          "type": "string",
          "required": true,
          "description": "Message to send to the agent"
        },
        {
          "name": "model",
          "type": "string",
          "required": true,
          "description": "Name of the AI model to use"
        }
      ],
      "returns": {
        "type": "object",
        "description": "Chat response with message and metadata"
      }
    }
  ],
  "total_count": 12
}
```

### Get Function Details

Get detailed information about a specific function.

```http
GET /v1/functions/{function_name}
```

**Response**:
```json
{
  "name": "analyze_data",
  "description": "Text and data analysis functionality",
  "category": "analysis",
  "timeout": 60000,
  "version": "1.0.0",
  "parameters": [
    {
      "name": "text",
      "type": "string",
      "required": true,
      "description": "Text to analyze",
      "validation": {
        "min_length": 1,
        "max_length": 10000
      }
    },
    {
      "name": "analysis_type",
      "type": "string",
      "required": false,
      "default": "general",
      "description": "Type of analysis to perform",
      "allowed_values": ["sentiment", "summary", "keywords", "general"]
    }
  ],
  "returns": {
    "type": "object",
    "properties": {
      "summary": {"type": "string"},
      "insights": {"type": "array"},
      "data": {"type": "object"}
    }
  },
  "examples": [
    {
      "description": "Basic text analysis",
      "request": {
        "text": "This is sample text for analysis",
        "analysis_type": "sentiment"
      },
      "response": {
        "summary": "Positive sentiment detected",
        "data": {"sentiment": "positive", "score": 0.85}
      }
    }
  ]
}
```

## 🔍 Search and Retrieval

### Internet Search

Perform web search using the integrated search functionality.

```http
POST /v1/search/internet
Content-Type: application/json
```

**Request Body**:
```json
{
  "query": "latest AI developments 2025",
  "max_results": 10,
  "category": "general",
  "language": "en",
  "safe_search": true,
  "engines": ["google", "bing"]
}
```

**Response**:
```json
{
  "query": "latest AI developments 2025",
  "results": [
    {
      "title": "Latest AI Developments in 2025",
      "url": "https://example.com/ai-developments-2025",
      "snippet": "Comprehensive overview of AI advancements...",
      "source": "google",
      "score": 0.95,
      "published_date": "2025-08-15"
    }
  ],
  "total_results": 8,
  "search_time_ms": 1250,
  "timestamp": "2025-08-28T13:20:00Z"
}
```

### Document Retrieval

Search documents in the knowledge base.

```http
POST /v1/search/documents
Content-Type: application/json
```

**Request Body**:
```json
{
  "query": "machine learning algorithms",
  "max_results": 5,
  "collection": "research_papers",
  "similarity_threshold": 0.8
}
```

**Response**:
```json
{
  "query": "machine learning algorithms",
  "results": [
    {
      "document_id": "doc-001",
      "title": "Introduction to Machine Learning",
      "content": "Machine learning is a subset of artificial intelligence...",
      "similarity_score": 0.92,
      "metadata": {
        "author": "Dr. Smith",
        "publication_date": "2024-12-01",
        "pages": "1-25"
      }
    }
  ],
  "total_results": 3,
  "search_time_ms": 89,
  "timestamp": "2025-08-28T13:25:00Z"
}
```

## 📊 Error Responses

### Standard Error Format

All error responses follow a consistent format:

```json
{
  "error": {
    "code": "AGENT_NOT_FOUND",
    "message": "Agent with ID 'invalid-id' not found",
    "details": {
      "agent_id": "invalid-id",
      "available_agents": ["agent-001", "agent-002"]
    },
    "timestamp": "2025-08-28T13:30:00Z",
    "request_id": "req-12345"
  }
}
```

### Common Error Codes

| HTTP Status | Error Code | Description |
|-------------|------------|-------------|
| 400 | `INVALID_REQUEST` | Request validation failed |
| 400 | `MISSING_QUERY` | Query parameter is required |
| 400 | `INVALID_MODEL` | Specified model not available |
| 401 | `UNAUTHORIZED` | Authentication required |
| 403 | `FORBIDDEN` | Insufficient permissions |
| 404 | `AGENT_NOT_FOUND` | Agent does not exist |
| 404 | `FUNCTION_NOT_FOUND` | Function does not exist |
| 409 | `AGENT_ALREADY_EXISTS` | Agent with same ID exists |
| 409 | `NO_AGENTS_AVAILABLE` | No running agents available |
| 422 | `TOOL_EXECUTION_FAILED` | All tool executions failed |
| 429 | `RATE_LIMIT_EXCEEDED` | Too many requests |
| 500 | `INTERNAL_ERROR` | Internal server error |
| 503 | `SERVICE_UNAVAILABLE` | System temporarily unavailable |
| 503 | `MODEL_UNAVAILABLE` | LLM model service unavailable |

### Error Examples

**Agent Not Found (404)**:
```json
{
  "error": {
    "code": "AGENT_NOT_FOUND",
    "message": "Agent with ID 'nonexistent' not found",
    "timestamp": "2025-08-28T13:30:00Z"
  }
}
```

**Rate Limit Exceeded (429)**:
```json
{
  "error": {
    "code": "RATE_LIMIT_EXCEEDED",
    "message": "Rate limit exceeded. Try again in 60 seconds.",
    "details": {
      "limit": 100,
      "remaining": 0,
      "reset_time": "2025-08-28T13:31:00Z"
    },
    "timestamp": "2025-08-28T13:30:00Z"
  }
}
```

## 📝 Request/Response Examples

### Quick Start with Simple Execute

The fastest way to get started is using the simple execute endpoint:

```bash
# Simple AI question with automatic tool execution
curl -X POST http://localhost:8081/agent/execute \
  -H "Content-Type: application/json" \
  -d '{
    "query": "What is machine learning?",
    "context": "Explain for beginners",
    "model": "qwen2.5-0.5b"
  }'
```

**Response Summary**:
- Automatically selects best available agent
- Executes 21+ research and analysis tools
- Provides comprehensive results with tool outputs
- Includes LLM-generated summary response
- Returns execution statistics and error details

### Complete Agent Workflow

1. **Create Agent**:
```bash
curl -X POST http://localhost:8081/v1/agents \
  -H "Content-Type: application/json" \
  -d '{
    "name": "DataAnalyst",
    "capabilities": ["data_processing", "analysis"],
    "config": {"auto_start": true}
  }'
```

2. **Execute Function**:
```bash
curl -X POST http://localhost:8081/v1/agents/data-analyst-001/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "analyze_data",
    "parameters": {
      "text": "Sample data for analysis",
      "analysis_type": "comprehensive"
    }
  }'
```

3. **Check Status**:
```bash
curl http://localhost:8081/v1/agents/data-analyst-001
```

4. **Clean Up**:
```bash
curl -X DELETE http://localhost:8081/v1/agents/data-analyst-001
```

## 🔧 SDK Examples

### Python Client

```python
import requests

class KolosalClient:
    def __init__(self, base_url="http://localhost:8081", api_key=None):
        self.base_url = base_url
        self.headers = {"Content-Type": "application/json"}
        if api_key:
            self.headers["X-API-Key"] = api_key
    
    def simple_execute(self, query, context=None, model=None, agent=None):
        """Execute query with automatic agent and tool selection"""
        payload = {"query": query}
        if context:
            payload["context"] = context
        if model:
            payload["model"] = model
        if agent:
            payload["agent"] = agent
            
        response = requests.post(
            f"{self.base_url}/agent/execute",
            json=payload,
            headers=self.headers
        )
        return response.json()
    
    def create_agent(self, name, capabilities):
        response = requests.post(
            f"{self.base_url}/v1/agents",
            json={"name": name, "capabilities": capabilities},
            headers=self.headers
        )
        return response.json()
    
    def execute_function(self, agent_id, function, parameters):
        response = requests.post(
            f"{self.base_url}/v1/agents/{agent_id}/execute",
            json={"function": function, "parameters": parameters},
            headers=self.headers
        )
        return response.json()

# Usage Examples
client = KolosalClient()

# Simple execute (recommended for most use cases)
result = client.simple_execute(
    query="What is artificial intelligence?",
    context="Explain for beginners",
    model="qwen2.5-0.5b"
)
print(f"Tools executed: {len(result['tools_executed'])}")
print(f"Success rate: {result['summary']['successful']}/{result['summary']['total_tools']}")

# Traditional agent workflow
agent = client.create_agent("TestAgent", ["chat"])
result = client.execute_function(
    agent["agent_id"], 
    "chat", 
    {"message": "Hello!", "model": "qwen2.5-0.5b"}
)
```

### JavaScript Client

```javascript
class KolosalClient {
    constructor(baseUrl = 'http://localhost:8081', apiKey = null) {
        this.baseUrl = baseUrl;
        this.headers = {'Content-Type': 'application/json'};
        if (apiKey) {
            this.headers['X-API-Key'] = apiKey;
        }
    }
    
    async simpleExecute(query, options = {}) {
        const payload = { query, ...options };
        const response = await fetch(`${this.baseUrl}/agent/execute`, {
            method: 'POST',
            headers: this.headers,
            body: JSON.stringify(payload)
        });
        return response.json();
    }
    
    async createAgent(name, capabilities) {
        const response = await fetch(`${this.baseUrl}/v1/agents`, {
            method: 'POST',
            headers: this.headers,
            body: JSON.stringify({name, capabilities})
        });
        return response.json();
    }
    
    async executeFunction(agentId, functionName, parameters) {
        const response = await fetch(`${this.baseUrl}/v1/agents/${agentId}/execute`, {
            method: 'POST',
            headers: this.headers,
            body: JSON.stringify({function: functionName, parameters})
        });
        return response.json();
    }
}

// Usage Examples
const client = new KolosalClient();

// Simple execute (recommended)
const result = await client.simpleExecute(
    'What is machine learning?',
    {
        context: 'Explain for beginners',
        model: 'qwen2.5-0.5b'
    }
);
console.log(`Tools executed: ${result.tools_executed.length}`);
console.log(`Success rate: ${result.summary.successful}/${result.summary.total_tools}`);

// Traditional workflow
const agent = await client.createAgent('TestAgent', ['chat']);
const chatResult = await client.executeFunction(
    agent.agent_id, 
    'chat', 
    {message: 'Hello!', model: 'qwen2.5-0.5b'}
);
```

---

For more examples and advanced usage, see the [Examples Guide](examples.md) and [Developer Guide](development.md).
