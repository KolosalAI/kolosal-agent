# Kolosal Agent Workflow System

## Overview

The Kolosal Agent Workflow System is a sophisticated orchestration layer that enables complex multi-agent workflows. It provides both simple function execution and advanced workflow orchestration capabilities.

## Architecture

### Components

1. **WorkflowManager**: Manages individual agent function executions
2. **WorkflowOrchestrator**: Orchestrates complex multi-step workflows
3. **HTTPServer**: Provides REST API endpoints for workflow management
4. **WorkflowTypes**: Defines workflow patterns and execution strategies

### Workflow Types

- **SEQUENTIAL**: Execute steps one by one in order
- **PARALLEL**: Execute steps simultaneously 
- **CONDITIONAL**: Execute steps based on conditions
- **LOOP**: Execute steps repeatedly until condition is met
- **PIPELINE**: Pass output of one step as input to next

## API Endpoints

### Workflow Orchestration

#### List Workflow Definitions
```http
GET /workflows
```

#### Register Workflow Definition
```http
POST /workflows
Content-Type: application/json

{
  "id": "my_workflow",
  "name": "My Custom Workflow",
  "description": "A custom workflow example",
  "type": 0,
  "steps": [
    {
      "id": "step1",
      "agent_name": "Assistant",
      "function_name": "chat",
      "parameters": {
        "message": "{{input.question}}",
        "model": "default"
      }
    }
  ]
}
```

#### Execute Workflow
```http
POST /workflows/{id}/execute
Content-Type: application/json

{
  "input_data": {
    "question": "What is machine learning?"
  },
  "async": true
}
```

#### Get Execution Status
```http
GET /workflows/executions/{execution_id}
```

#### Control Execution
```http
PUT /workflows/executions/{execution_id}/{action}
```
Actions: `pause`, `resume`, `cancel`

#### List Executions
```http
GET /workflows/executions
```

## Built-in Workflow Templates

### Research Workflow
**ID**: `research_workflow`
**Type**: Sequential
**Steps**: Research → Analyze → Summarize

```json
{
  "workflow_id": "research_workflow",
  "input_data": {
    "question": "What are the applications of AI in healthcare?"
  }
}
```

### Analysis Workflow  
**ID**: `analysis_workflow`
**Type**: Sequential
**Steps**: Preprocess → Analyze → Report

```json
{
  "workflow_id": "analysis_workflow", 
  "input_data": {
    "data": "Text data to analyze..."
  }
}
```

### Data Pipeline Workflow
**ID**: `data_pipeline_workflow`
**Type**: Pipeline
**Steps**: Extract → Transform → Validate → Load

```json
{
  "workflow_id": "data_pipeline_workflow",
  "input_data": "Raw data input..."
}
```

### Decision Workflow
**ID**: `decision_workflow`  
**Type**: Sequential
**Steps**: Gather Info → Analyze Options → Decide → Execute

```json
{
  "workflow_id": "decision_workflow",
  "input_data": {
    "decision_topic": "Should we implement feature X?"
  }
}
```

## Workflow Definition Format

### Basic Structure
```json
{
  "id": "workflow_id",
  "name": "Workflow Name", 
  "description": "Workflow description",
  "type": 0,
  "max_execution_time_ms": 300000,
  "allow_partial_failure": false,
  "global_context": {},
  "steps": []
}
```

### Step Definition
```json
{
  "id": "step_id",
  "agent_name": "Agent Name",
  "function_name": "function_name",
  "parameters": {},
  "conditions": {},
  "dependencies": [],
  "timeout_ms": 30000,
  "optional": false
}
```

### Parameter Templating
Use `{{variable}}` syntax to reference context variables:

```json
{
  "parameters": {
    "message": "Process this: {{input.text}}",
    "previous_result": "{{step1_output}}"
  }
}
```

## Usage Examples

### Custom Sequential Workflow
```bash
curl -X POST http://localhost:8080/workflows \
  -H "Content-Type: application/json" \
  -d '{
    "id": "sentiment_analysis",
    "name": "Sentiment Analysis Workflow",
    "type": 0,
    "steps": [
      {
        "id": "analyze",
        "agent_name": "Analyzer", 
        "function_name": "analyze",
        "parameters": {
          "text": "{{input.text}}",
          "analysis_type": "sentiment"
        }
      },
      {
        "id": "report",
        "agent_name": "Assistant",
        "function_name": "chat", 
        "parameters": {
          "message": "Generate report: {{analyze_output}}",
          "model": "default"
        },
        "dependencies": ["analyze"]
      }
    ]
  }'
```

### Execute Custom Workflow
```bash
curl -X POST http://localhost:8080/workflows/sentiment_analysis/execute \
  -H "Content-Type: application/json" \
  -d '{
    "input_data": {
      "text": "I love this product! It works great."
    }
  }'
```

### Conditional Workflow
```bash
curl -X POST http://localhost:8080/workflows \
  -H "Content-Type: application/json" \
  -d '{
    "id": "conditional_workflow",
    "name": "Conditional Processing",
    "type": 2,
    "steps": [
      {
        "id": "check_length",
        "agent_name": "Analyzer",
        "function_name": "analyze",
        "parameters": {
          "text": "{{input.text}}",
          "analysis_type": "length"
        }
      },
      {
        "id": "detailed_analysis", 
        "agent_name": "Analyzer",
        "function_name": "analyze",
        "parameters": {
          "text": "{{input.text}}",
          "analysis_type": "detailed"
        },
        "conditions": {
          "field": "check_length_output.word_count",
          "operator": "greater_than",
          "value": 100
        }
      }
    ]
  }'
```

## Monitoring and Management

### Request States
- `PENDING`: Request submitted, waiting to execute
- `PROCESSING`: Currently executing  
- `COMPLETED`: Successfully completed
- `FAILED`: Execution failed
- `TIMEOUT`: Execution timed out
- `CANCELLED`: Cancelled by user

### Execution States
- `PENDING`: Execution created, waiting to start
- `RUNNING`: Currently executing
- `PAUSED`: Temporarily paused
- `COMPLETED`: Successfully completed
- `FAILED`: Execution failed
- `CANCELLED`: Cancelled by user
- `TIMEOUT`: Execution timed out

## Configuration

### Agent.yaml Function Configuration
```yaml
functions:
  chat:
    description: "Interactive chat functionality"
    timeout: 30000
    parameters:
      - name: "message"
        type: "string"
        required: true
      - name: "model"
        type: "string"
        required: true
        
  analyze:
    description: "Text analysis"
    timeout: 60000
    parameters:
      - name: "text"
        type: "string"
        required: true
      - name: "analysis_type"
        type: "string"
        required: false
```

### Performance Settings
```yaml
performance:
  max_memory_usage: "2GB"
  cache_size: "512MB"
  worker_threads: 4
  request_timeout: 30000
  max_request_size: "10MB"
```

## Error Handling

### Common Error Responses

#### Invalid Request
```json
{
  "error": "Missing required fields: agent_name, function_name",
  "status": 400
}
```

#### Agent Not Found
```json
{
  "error": "Agent 'InvalidAgent' not found",
  "status": 404
}
```

#### Request Timeout
```json
{
  "request_id": "abc123",
  "state": "timeout", 
  "error": "Request execution timed out",
  "timeout_ms": 30000
}
```

#### Workflow Execution Failure
```json
{
  "execution_id": "wf-abc123",
  "state": 3,
  "error_message": "Step 'analyze' failed: Invalid parameters",
  "progress_percentage": 50.0
}
```

## Best Practices

### Workflow Design
1. **Keep steps focused**: Each step should have a single responsibility
2. **Use meaningful IDs**: Step IDs should be descriptive
3. **Handle failures**: Use `optional: true` for non-critical steps
4. **Set appropriate timeouts**: Based on expected execution time
5. **Use dependencies**: Ensure proper execution order

### Performance
1. **Parallel execution**: Use parallel workflows for independent steps
2. **Timeout management**: Set realistic timeouts to avoid hanging
3. **Resource limits**: Monitor memory and CPU usage
4. **Queue management**: Monitor request queue size

### Monitoring
1. **Track execution progress**: Use progress percentage
2. **Monitor failure rates**: Check system statistics
3. **Log important events**: Enable appropriate logging
4. **Set up alerts**: For system health monitoring

## Integration Examples

### Python Client
```python
import requests
import json

class WorkflowClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
    
    def execute_workflow(self, workflow_id, input_data):
        response = requests.post(f"{self.base_url}/workflows/{workflow_id}/execute",
            json={
                "input_data": input_data
            })
        return response.json()

# Usage
client = WorkflowClient()
result = client.submit_request("Assistant", "chat", 
    {"message": "Hello!", "model": "default"})
print(f"Request ID: {result['request_id']}")
```

### JavaScript/Node.js Client
```javascript
class WorkflowClient {
    constructor(baseUrl = 'http://localhost:8080') {
        this.baseUrl = baseUrl;
    }
    
    async executeWorkflow(workflowId, inputData) {
        const response = await fetch(`${this.baseUrl}/workflows/${workflowId}/execute`, {
            method: 'POST', 
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({
                input_data: inputData
            })
        });
        return response.json();
    }
}

// Usage
const client = new WorkflowClient();
const result = await client.executeWorkflow('research_workflow', 
    {question: 'What is quantum computing?'});
console.log(`Execution ID: ${result.execution_id}`);
```

## Troubleshooting

### Common Issues

1. **Request stuck in PENDING**: Check worker thread availability
2. **Timeouts**: Increase timeout values for long-running operations  
3. **Agent not found**: Verify agent name and ensure agent is running
4. **Function not found**: Check function name and agent capabilities
5. **Parameter validation**: Ensure required parameters are provided

### Debug Steps
1. Check system status: `GET /status`
2. Review execution details: `GET /workflows/executions/{execution_id}`
3. Check agent availability: `GET /agents`
4. Verify function configuration in agent.yaml
5. Review server logs for detailed error information

## Future Enhancements

- Workflow versioning and rollback
- Visual workflow designer
- Workflow templates marketplace
- Advanced scheduling capabilities
- Workflow performance analytics
- Integration with external systems
- Workflow testing framework
