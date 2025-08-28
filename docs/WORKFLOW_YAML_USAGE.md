# Workflow Configuration Usage Guide

## Overview

The Kolosal Agent System now supports workflow configuration through `workflow.yaml`. This allows you to define complex workflows that pair specific agents with specific LLM models without redefining the agents themselves.

## Key Features

1. **Agent-LLM Pairing**: Each workflow step can specify which agent to use with which LLM model
2. **Agent Reference Only**: Workflows reference agents from `agent.yaml` without redefining them
3. **Flexible Configuration**: Support for sequential, parallel, conditional, loop, and pipeline workflows
4. **Built-in Validation**: Validates agent-LLM pairings and workflow definitions

## Configuration Structure

### Agent-LLM Mappings

```yaml
agent_llm_mappings:
  Assistant:
    default_model: "gemma3-1b"
    supported_models: ["gemma3-1b"]
    
  Analyzer:
    default_model: "gemma3-1b"
    supported_models: ["gemma3-1b"]
    
  Researcher:
    default_model: "gemma3-1b"
    supported_models: ["gemma3-1b"]
```

### Workflow Definition

```yaml
workflows:
  - id: "simple_research"
    name: "Simple Research Workflow"
    description: "Basic research workflow using a single agent with specific LLM"
    type: "sequential"
    max_execution_time_ms: 180000
    allow_partial_failure: false
    steps:
      - id: "research_step"
        agent_name: "Researcher"  # References agent from agent.yaml
        llm_model: "gemma3-1b"    # Specific LLM model from config.yaml
        function_name: "research"
        parameters:
          query: "{{input.question}}"
          depth: "detailed"
        timeout_ms: 120000
        optional: false
```

## Usage Examples

### 1. Execute Simple Research Workflow

```bash
curl -X POST http://localhost:8080/workflows/execute \
  -H "Content-Type: application/json" \
  -d '{
    "workflow_id": "simple_research",
    "input_data": {
      "question": "What are the latest developments in AI?"
    }
  }'
```

### 2. Execute Analysis Workflow with Multiple Steps

```bash
curl -X POST http://localhost:8080/workflows/execute \
  -H "Content-Type: application/json" \
  -d '{
    "workflow_id": "analysis_workflow",
    "input_data": {
      "text": "Sample text to analyze using different analysis techniques"
    }
  }'
```

### 3. Execute Parallel Analysis

```bash
curl -X POST http://localhost:8080/workflows/execute \
  -H "Content-Type: application/json" \
  -d '{
    "workflow_id": "parallel_analysis",
    "input_data": {
      "text": "Text to be analyzed in parallel for sentiment, keywords, and summary"
    }
  }'
```

### 4. List Available Workflows

```bash
curl http://localhost:8080/workflows
```

### 5. Get Workflow Execution Status

```bash
curl http://localhost:8080/workflows/executions/{execution_id}
```

## Workflow Types

### Sequential
Steps execute one after another in order.

```yaml
type: "sequential"
```

### Parallel
Steps execute simultaneously.

```yaml
type: "parallel"
```

### Conditional
Steps execute based on conditions from previous steps.

```yaml
type: "conditional"
steps:
  - id: "conditional_step"
    agent_name: "Analyzer"
    llm_model: "gemma3-1b"
    function_name: "analyze"
    condition:
      type: "contains"
      field: "steps.previous_step.result"
      value: "needs processing"
    dependencies: ["previous_step"]
```

### Pipeline
Steps form a data processing pipeline where output of one step becomes input to the next.

```yaml
type: "pipeline"
```

### Loop
Steps execute repeatedly until a condition is met.

```yaml
type: "loop"
```

## Parameter Templates

Use template variables to pass data between steps and from input:

- `{{input.field_name}}` - Access input data
- `{{steps.step_id.result}}` - Access result from previous step
- `{{steps.step_id.output}}` - Access output from previous step

Example:
```yaml
parameters:
  text: "{{input.text}}"
  context: "{{steps.initial_analysis.result}}"
  model: "{{agent.default_model}}"
```

## Configuration Validation

The system validates:

1. **Agent Existence**: All referenced agents must exist in `agent.yaml`
2. **LLM Model Support**: Each agent-LLM pairing must be valid according to `agent_llm_mappings`
3. **Dependencies**: All step dependencies must reference existing steps
4. **Required Fields**: All required workflow and step fields must be present

## Best Practices

1. **Use Descriptive IDs**: Make workflow and step IDs descriptive for easier debugging
2. **Set Appropriate Timeouts**: Configure realistic timeouts for each step
3. **Handle Failures**: Use `optional: true` for non-critical steps or `allow_partial_failure: true` for workflows
4. **Document Workflows**: Provide clear descriptions for each workflow
5. **Test Incrementally**: Start with simple workflows and gradually increase complexity

## Configuration File Location

The system looks for `workflow.yaml` in the root directory of the project. You can also specify a custom path when loading the configuration.

## Error Handling

- Invalid agent-LLM pairings will cause workflow validation to fail
- Missing dependencies will prevent workflow registration
- Runtime errors are captured in workflow execution status
- Detailed error messages are provided for debugging

## Integration with Agent System

The workflow system integrates seamlessly with the existing agent system:

- Agents are defined once in `agent.yaml`
- Workflows reference agents by name only
- LLM models are specified per workflow step
- All existing agent capabilities are available in workflows
- Agent lifecycle management remains unchanged
