# Simple Agent Execute Endpoint

The Kolosal Agent System now includes a simplified endpoint for executing agent functions with automatic tool orchestration.

## Endpoint

```
POST /agent/execute
```

## Request Format

```json
{
  "query": "Your question or request",
  "context": "Additional context (optional)",
  "model": "qwen2.5-0.5b",
  "agent": "Assistant" 
}
```

### Parameters

- **query** (required): The main question or request you want the agent to process
- **context** (optional): Additional context or instructions to help the agent understand your request better
- **model** (optional): The AI model to use for processing (defaults to "default")
- **agent** (optional): Specific agent name or ID to use (uses first available agent if not specified)

## Response Format

```json
{
  "query": "What is artificial intelligence?",
  "context": "Additional context",
  "model": "qwen2.5-0.5b",
  "agent_id": "agent-uuid",
  "agent_name": "Assistant",
  "tools_executed": ["analyze", "research", "search_documents"],
  "execution_log": [
    {
      "function": "analyze",
      "status": "success",
      "result_summary": "Data retrieved"
    }
  ],
  "tool_responses": {
    "analyze": {
      "result": "Analysis results...",
      "status": "success"
    },
    "research": {
      "research_result": "Research findings...",
      "status": "success"
    }
  },
  "llm_response": {
    "agent": "Assistant",
    "response": "Based on the tool execution results, artificial intelligence is...",
    "timestamp": "1693872000",
    "model_used": "qwen2.5-0.5b",
    "status": "success"
  },
  "summary": {
    "total_tools": 3,
    "successful": 3,
    "failed": 0
  },
  "timestamp": "1693872000"
}
```

## How It Works

1. **Tool Execution**: The endpoint automatically executes all available tool functions (except basic ones like `echo`, `status`) with your query
2. **Context Building**: Results from all tools are compiled into a comprehensive context
3. **LLM Processing**: The agent's chat function is called with your original query plus all the tool results as context
4. **Unified Response**: You get both the raw tool outputs and the AI-generated response in one call

## Example Usage

### Python

```python
import requests

response = requests.post('http://127.0.0.1:8080/agent/execute', json={
    "query": "What are the benefits of renewable energy?",
    "context": "Focus on environmental and economic aspects",
    "model": "qwen2.5-0.5b"
})

result = response.json()
print("AI Response:", result['llm_response']['response'])
print("Tools used:", result['tools_executed'])
```

### curl

```bash
curl -X POST http://127.0.0.1:8080/agent/execute \
  -H "Content-Type: application/json" \
  -d '{
    "query": "Explain quantum computing",
    "context": "Make it accessible for beginners",
    "model": "qwen2.5-0.5b"
  }'
```

### JavaScript/Fetch

```javascript
const response = await fetch('http://127.0.0.1:8080/agent/execute', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json'
  },
  body: JSON.stringify({
    query: "How does machine learning work?",
    context: "Provide practical examples",
    model: "qwen2.5-0.5b"
  })
});

const result = await response.json();
console.log('AI Response:', result.llm_response.response);
```

## Benefits

- **Simplified Integration**: One endpoint call gets you comprehensive results
- **Automatic Tool Orchestration**: All relevant tools are executed automatically
- **Rich Context**: LLM responses are enhanced with tool execution results
- **Complete Transparency**: See exactly what tools were executed and their results
- **Flexible**: Works with any available agent and model

## Error Handling

If individual tools fail, the endpoint continues executing other tools and provides a fallback response. The response will include error details in the `execution_log` and `tool_responses` for any failed tools.

## Configuration

Available models and agents are determined by your `agent.yaml` configuration file. Make sure the models you want to use are properly configured and loaded.
