# Sequential Workflow API Integration Guide

## Overview

The kolosal agent system now includes comprehensive API integration for sequential workflows. This integration provides REST endpoints for creating, managing, and executing sequential workflows through HTTP requests.

## API Endpoints

### Workflow Management

#### List Workflows
```http
GET /v1/workflows
```
Returns a list of all available workflows.

**Response:**
```json
{
  "workflows": [
    {
      "id": "sequential_workflow_1",
      "name": "Sequential Research Workflow",
      "description": "A comprehensive research workflow",
      "type": 0,
      "status": "pending",
      "step_count": 4,
      "created_by": "api"
    }
  ],
  "total": 1
}
```

#### Create Workflow
```http
POST /v1/workflows
```
Creates a new workflow from JSON configuration.

**Request Body:**
```json
{
  "id": "my_sequential_workflow",
  "name": "My Sequential Workflow",
  "description": "Custom sequential workflow",
  "type": 0,
  "global_context": {
    "research_question": "${input.research_question}",
    "topic": "${input.topic}"
  },
  "steps": [
    {
      "id": "topic_research",
      "name": "Topic Research",
      "agent_id": "research_analyst",
      "function": "research_topic",
      "parameters": {
        "topic": "${global.topic}",
        "depth": "comprehensive"
      },
      "timeout": 60,
      "max_retries": 2,
      "depends_on": []
    }
  ],
  "settings": {
    "max_execution_time": 300,
    "max_concurrent_steps": 1
  }
}
```

#### Get Workflow
```http
GET /v1/workflows/{workflow_id}
```
Retrieves detailed information about a specific workflow.

#### Update Workflow
```http
PUT /v1/workflows/{workflow_id}
```
Updates an existing workflow configuration.

#### Delete Workflow
```http
DELETE /v1/workflows/{workflow_id}
```
Deletes a workflow from the system.

### Workflow Execution

#### Execute Workflow
```http
POST /v1/workflows/{workflow_id}/execute
```
Starts execution of a workflow with input parameters.

**Request Body:**
```json
{
  "research_question": "What are the benefits of AI in healthcare?",
  "topic": "AI healthcare applications",
  "research_focus": "benefits and applications"
}
```

**Response:**
```json
{
  "execution_id": "exec_1234567890",
  "workflow_id": "my_workflow",
  "status": "started",
  "message": "Workflow execution started"
}
```

#### Get Execution Status
```http
GET /v1/workflows/{workflow_id}/executions/{execution_id}/status
```
Retrieves the current status of a workflow execution.

**Response:**
```json
{
  "execution_id": "exec_1234567890",
  "workflow_id": "my_workflow",
  "status": "running",
  "current_step": "data_analysis",
  "completed_steps": 1,
  "failed_steps": 0,
  "total_steps": 4,
  "progress": 25.0,
  "execution_time": 45.2
}
```

#### Get Execution Result
```http
GET /v1/workflows/{workflow_id}/executions/{execution_id}/result
```
Retrieves the final results of a completed workflow execution.

**Response:**
```json
{
  "execution_id": "exec_1234567890",
  "workflow_id": "my_workflow",
  "success": true,
  "status": "completed",
  "completed_steps": 4,
  "failed_steps": 0,
  "total_steps": 4,
  "results": {
    "topic_research": {
      "summary": "Comprehensive research completed",
      "confidence": 0.87
    },
    "data_analysis": {
      "insights": ["Key findings..."],
      "quality_score": 0.92
    }
  },
  "execution_time": 125.5,
  "step_results": {
    "topic_research": {
      "status": "completed",
      "output": {...}
    }
  }
}
```

### Workflow Control

#### Pause Execution
```http
PUT /v1/workflows/{workflow_id}/executions/{execution_id}/pause
```
Pauses a running workflow execution.

#### Resume Execution
```http
PUT /v1/workflows/{workflow_id}/executions/{execution_id}/resume
```
Resumes a paused workflow execution.

#### Cancel Execution
```http
PUT /v1/workflows/{workflow_id}/executions/{execution_id}/cancel
```
Cancels a running workflow execution.

### Templates

#### Create from Template
```http
POST /v1/workflows/templates/{template_type}
```
Creates a workflow from predefined templates.

Supported template types:
- `sequential` - Sequential execution workflow
- `parallel` - Parallel execution workflow
- `pipeline` - Pipeline workflow
- `consensus` - Consensus-based workflow

**Request Body for Sequential Template:**
```json
{
  "name": "My Sequential Workflow",
  "steps": [
    {"agent_id": "research_analyst", "function": "research_topic"},
    {"agent_id": "research_analyst", "function": "analyze_data"},
    {"agent_id": "research_analyst", "function": "generate_report"}
  ]
}
```

### Metrics

#### Get System Metrics
```http
GET /v1/workflows/metrics
```
Retrieves workflow system performance metrics.

**Response:**
```json
{
  "total_workflows": 5,
  "running_workflows": 2,
  "completed_workflows": 15,
  "failed_workflows": 1,
  "cancelled_workflows": 0,
  "average_execution_time_ms": 45230.5,
  "success_rate": 0.94,
  "error_counts": {
    "timeout": 1,
    "agent_error": 2
  }
}
```

## Configuration

### Sequential Workflow Configuration

The sequential workflow can be configured through YAML files that are automatically loaded at server startup:

**sequential.yaml example:**
```yaml
id: "simple_research_workflow"
name: "Simple Research and Analysis Workflow"
description: "A simplified sequential workflow using only the research_analyst agent"
type: "sequential"

global_context:
  project_name: "Simple Research Analysis"
  research_question: "${input.research_question}"
  topic: "${input.topic}"
  output_format: "comprehensive_report"

steps:
  - id: "topic_research"
    name: "Topic Research and Analysis"
    agent_id: "research_analyst"
    function: "research_topic"
    parameters:
      topic: "${global.topic}"
      depth: "comprehensive"
    timeout: 60
    max_retries: 2

  - id: "data_analysis"
    name: "Research Data Analysis"
    agent_id: "research_analyst"
    function: "analyze_data"
    parameters:
      data_source: "${steps.topic_research.output}"
    depends_on:
      - "topic_research"
    timeout: 45
    max_retries: 2

settings:
  max_execution_time: 300
  max_concurrent_steps: 1
  auto_cleanup: true
  persist_state: true

error_handling:
  retry_on_failure: true
  max_retries: 2
  continue_on_error: false
```

## Usage Examples

### Python Client Example

```python
import requests
import json

# Configuration
api_base = "http://127.0.0.1:8081"
headers = {"Content-Type": "application/json"}

# 1. List available workflows
response = requests.get(f"{api_base}/v1/workflows", headers=headers)
workflows = response.json()
print(f"Available workflows: {len(workflows['workflows'])}")

# 2. Execute a sequential workflow
input_data = {
    "research_question": "What are the impacts of AI on education?",
    "topic": "AI in education",
    "research_focus": "learning outcomes and accessibility"
}

response = requests.post(
    f"{api_base}/v1/workflows/simple_research_workflow/execute",
    headers=headers,
    json=input_data
)

execution = response.json()
execution_id = execution["execution_id"]
workflow_id = execution["workflow_id"]

# 3. Monitor execution status
import time
while True:
    response = requests.get(
        f"{api_base}/v1/workflows/{workflow_id}/executions/{execution_id}/status",
        headers=headers
    )
    status = response.json()
    
    print(f"Status: {status['status']} - Progress: {status['progress']}%")
    
    if status['status'] in ['completed', 'failed', 'cancelled']:
        break
    
    time.sleep(5)

# 4. Get final results
response = requests.get(
    f"{api_base}/v1/workflows/{workflow_id}/executions/{execution_id}/result",
    headers=headers
)

results = response.json()
print(f"Execution completed: {results['success']}")
print(f"Results: {json.dumps(results['results'], indent=2)}")
```

### cURL Examples

```bash
# List workflows
curl -X GET http://127.0.0.1:8081/v1/workflows

# Create workflow from template
curl -X POST http://127.0.0.1:8081/v1/workflows/templates/sequential \
  -H "Content-Type: application/json" \
  -d '{"name": "My Workflow", "steps": [{"agent_id": "research_analyst", "function": "research_topic"}]}'

# Execute workflow
curl -X POST http://127.0.0.1:8081/v1/workflows/simple_research_workflow/execute \
  -H "Content-Type: application/json" \
  -d '{"research_question": "AI benefits", "topic": "artificial intelligence"}'

# Check execution status
curl -X GET http://127.0.0.1:8081/v1/workflows/my_workflow/executions/exec_123/status

# Get system metrics
curl -X GET http://127.0.0.1:8081/v1/workflows/metrics
```

## Integration Features

### Automatic Workflow Loading

The system automatically loads workflows from:
- `sequential.yaml` in the current directory
- `workflow.yaml` files in the project root
- YAML files in `./workflows/` directory
- Example workflows from `./examples/` directory

### Error Handling

The API provides comprehensive error handling with:
- Detailed error messages in JSON format
- HTTP status codes following REST conventions
- Retry mechanisms for transient failures
- Graceful degradation when components are unavailable

### CORS Support

The API includes CORS (Cross-Origin Resource Sharing) support for web applications:
- Configurable allowed origins
- Support for preflight OPTIONS requests
- Appropriate CORS headers in responses

### Authentication and Security

- Optional API key authentication
- Rate limiting support
- Input validation and sanitization
- Secure parameter interpolation

## Testing

Use the provided test script to verify API integration:

```bash
# Quick connection test
python test_sequential_api_integration.py --quick

# Comprehensive API tests
python test_sequential_api_integration.py

# Test against custom endpoint
python test_sequential_api_integration.py --url http://localhost:9000
```

## Troubleshooting

### Common Issues

1. **Connection Refused**
   - Ensure the kolosal agent server is running
   - Check that port 8081 is not blocked by firewall
   - Verify the agent API is enabled in configuration

2. **Workflow Not Found**
   - Check that workflows are properly loaded at startup
   - Verify YAML syntax in workflow configuration files
   - Use the workflow list endpoint to see available workflows

3. **Execution Failures**
   - Ensure required agents are configured and running
   - Check agent function availability
   - Review execution logs for detailed error information

4. **API Errors**
   - Validate JSON request format
   - Check required fields in request bodies
   - Review HTTP status codes and error messages

### Logs and Monitoring

Monitor the server logs for detailed information about:
- Workflow loading and creation
- Execution progress and errors
- API request/response details
- System health and performance

## Architecture

The sequential workflow API integration consists of:

1. **WorkflowRoute**: REST API endpoint handler
2. **WorkflowEngine**: Core workflow execution engine
3. **WorkflowLoader**: YAML configuration loader
4. **UnifiedServer**: Integrated server managing all components

This architecture ensures:
- Clean separation of concerns
- Scalable workflow execution
- Robust error handling
- Easy configuration management

## Conclusion

The sequential workflow API integration provides a complete solution for managing and executing sequential workflows through REST APIs. It supports both programmatic access and manual testing, with comprehensive documentation and examples for easy adoption.
