# Kolosal Agent Workflow Engine

## Overview

The Kolosal Agent Workflow Engine is a sophisticated orchestration system that enables complex, multi-agent workflows. It provides five distinct workflow execution patterns with comprehensive error handling, progress tracking, and dynamic execution control.

## Architecture

### Core Components

1. **WorkflowOrchestrator**: Main orchestration engine
2. **WorkflowManager**: Individual agent function execution
3. **WorkflowDefinition**: Workflow schema and metadata
4. **WorkflowExecution**: Runtime execution context
5. **HTTPServer**: REST API layer for workflow management

### Workflow Types

The engine supports five execution patterns:

- **SEQUENTIAL**: Execute steps one by one in order
- **PARALLEL**: Execute steps simultaneously 
- **CONDITIONAL**: Execute steps based on runtime conditions
- **LOOP**: Execute steps repeatedly until termination condition
- **PIPELINE**: Pass output of one step as input to next

## Workflow Definition Format

### Required Top-Level Fields

```yaml
id: string               # Unique workflow identifier
name: string            # Human-readable workflow name
type: string            # One of: sequential, parallel, conditional, loop, pipeline
steps: array            # Array of workflow steps
```

### Optional Top-Level Fields

```yaml
description: string                    # Workflow description
max_execution_time_ms: integer        # Maximum execution time (default: 300000)
allow_partial_failure: boolean        # Continue on step failures (default: false)
global_context: object               # Shared context across all steps
loop_config:                          # Loop-specific configuration
  max_iterations: integer             # Maximum loop iterations (default: 10)
  termination_condition: object       # Condition to exit loop
```

### Step Definition Fields

#### Required Step Fields

```yaml
id: string                  # Unique step identifier within workflow
agent_name: string          # Target agent name
function_name: string       # Function to execute on agent
parameters: array|object    # Function parameters (see formats below)
```

#### Optional Step Fields

```yaml
llm_model: string          # Specific LLM model for this step
timeout_ms: integer        # Step timeout (default: 60000)
optional: boolean          # Whether step failure should stop workflow (default: false)
dependencies: array        # Array of step IDs this step depends on
conditions: object         # Conditions for conditional execution
context_injection: object  # Additional context for this step
retry_policy:              # Retry configuration
  max_retries: integer     # Maximum retry attempts (default: 0)
  backoff_multiplier: float # Backoff multiplier (default: 1.5)
  initial_delay_ms: integer # Initial retry delay (default: 1000)
```

### Parameter Formats

#### New Array Format (Recommended)

```yaml
parameters: ["query", "depth", "model"]  # Parameter names only
```

The actual values are resolved at runtime from:
1. Input data
2. Previous step outputs
3. Global context
4. Default values

#### Legacy Object Format

```yaml
parameters:
  query: "{{input.question}}"
  depth: "detailed"
  model: "gpt-4"
```

### Condition Syntax

Conditions support simple field-based evaluation:

```yaml
conditions:
  field: "status"           # Field to evaluate
  operator: "equals"        # Operator: equals, not_equals, exists, contains
  value: "success"          # Expected value
```

## Complete Workflow Examples

### 1. Sequential Research Workflow

```yaml
id: "research_workflow"
name: "Research and Analysis Workflow"
type: "sequential"
description: "Comprehensive research workflow: question -> research -> analyze -> summarize"
max_execution_time_ms: 600000
allow_partial_failure: false
global_context:
  research_depth: "detailed"
  output_format: "markdown"

steps:
  - id: "initial_research"
    agent_name: "Researcher"
    function_name: "research"
    parameters: ["query", "depth"]
    timeout_ms: 120000
    
  - id: "analyze_findings"
    agent_name: "Analyzer"
    function_name: "analyze"
    parameters: ["text", "analysis_type"]
    dependencies: ["initial_research"]
    timeout_ms: 90000
    
  - id: "generate_summary"
    agent_name: "Assistant"
    function_name: "chat"
    parameters: ["message", "model"]
    dependencies: ["analyze_findings"]
    llm_model: "gpt-4"
    timeout_ms: 60000
```

### 2. Parallel Processing Workflow

```yaml
id: "parallel_analysis"
name: "Parallel Analysis Workflow"
type: "parallel"
description: "Analyze data from multiple perspectives simultaneously"
allow_partial_failure: true
max_execution_time_ms: 300000

steps:
  - id: "sentiment_analysis"
    agent_name: "SentimentAnalyzer"
    function_name: "analyze_sentiment"
    parameters: ["text", "model"]
    llm_model: "bert-sentiment"
    
  - id: "topic_extraction"
    agent_name: "TopicExtractor"
    function_name: "extract_topics"
    parameters: ["text", "num_topics"]
    
  - id: "keyword_analysis"
    agent_name: "KeywordAnalyzer"
    function_name: "extract_keywords"
    parameters: ["text", "max_keywords"]
    optional: true
    
  - id: "language_detection"
    agent_name: "LanguageDetector"
    function_name: "detect_language"
    parameters: ["text"]
```

### 3. Conditional Workflow

```yaml
id: "conditional_processing"
name: "Conditional Processing Workflow"
type: "conditional"
description: "Execute different steps based on input conditions"
max_execution_time_ms: 400000

steps:
  - id: "classify_input"
    agent_name: "Classifier"
    function_name: "classify"
    parameters: ["text", "categories"]
    
  - id: "handle_technical"
    agent_name: "TechnicalExpert"
    function_name: "analyze"
    parameters: ["text", "domain"]
    dependencies: ["classify_input"]
    conditions:
      field: "classify_input_output.category"
      operator: "equals"
      value: "technical"
      
  - id: "handle_creative"
    agent_name: "CreativeExpert"
    function_name: "generate"
    parameters: ["prompt", "style"]
    dependencies: ["classify_input"]
    conditions:
      field: "classify_input_output.category"
      operator: "equals"
      value: "creative"
      
  - id: "fallback_handler"
    agent_name: "GeneralAssistant"
    function_name: "chat"
    parameters: ["message", "model"]
    dependencies: ["classify_input"]
    conditions:
      field: "classify_input_output.category"
      operator: "not_equals"
      value: "technical"
```

### 4. Loop Workflow

```yaml
id: "iterative_refinement"
name: "Iterative Refinement Workflow"
type: "loop"
description: "Iteratively refine content until quality threshold is met"
max_execution_time_ms: 900000
loop_config:
  max_iterations: 5
  termination_condition:
    field: "quality_score"
    operator: "greater_than"
    value: 0.8

steps:
  - id: "generate_content"
    agent_name: "ContentGenerator"
    function_name: "generate"
    parameters: ["prompt", "style", "model"]
    llm_model: "gpt-4"
    
  - id: "evaluate_quality"
    agent_name: "QualityEvaluator"
    function_name: "evaluate"
    parameters: ["content", "criteria"]
    dependencies: ["generate_content"]
    
  - id: "refine_content"
    agent_name: "ContentRefiner"
    function_name: "refine"
    parameters: ["content", "feedback"]
    dependencies: ["evaluate_quality"]
    conditions:
      field: "quality_score"
      operator: "less_than"
      value: 0.8
```

### 5. Pipeline Workflow

```yaml
id: "data_pipeline"
name: "Data Processing Pipeline"
type: "pipeline"
description: "Process data through multiple transformation stages"
max_execution_time_ms: 500000
allow_partial_failure: false

steps:
  - id: "extract_data"
    agent_name: "DataExtractor"
    function_name: "extract"
    parameters: ["source", "format"]
    timeout_ms: 60000
    
  - id: "clean_data"
    agent_name: "DataCleaner"
    function_name: "clean"
    parameters: ["data", "rules"]
    timeout_ms: 120000
    
  - id: "transform_data"
    agent_name: "DataTransformer"
    function_name: "transform"
    parameters: ["data", "schema"]
    timeout_ms: 90000
    
  - id: "validate_data"
    agent_name: "DataValidator"
    function_name: "validate"
    parameters: ["data", "schema"]
    timeout_ms: 60000
    
  - id: "load_data"
    agent_name: "DataLoader"
    function_name: "load"
    parameters: ["data", "destination"]
    timeout_ms: 120000
```

## Workflow-Level Configuration Flags

### Execution Control

```yaml
allow_partial_failure: boolean     # Continue execution on step failures
max_execution_time_ms: integer     # Global workflow timeout
fail_fast: boolean                 # Stop on first error (default: true)
```

### Retry Policies

```yaml
default_retry_policy:
  max_retries: 3
  backoff_multiplier: 2.0
  initial_delay_ms: 1000
  max_delay_ms: 30000
```

### Loop Configuration

```yaml
loop_config:
  max_iterations: 10
  termination_condition:
    field: "completion_status"
    operator: "equals"
    value: "complete"
  iteration_delay_ms: 5000         # Delay between iterations
```

### Pipeline Configuration

```yaml
pipeline_config:
  pass_through_on_error: true      # Pass input to next step on error
  merge_outputs: false             # Merge all step outputs
  output_format: "last_step"       # "last_step", "all_steps", "merged"
```

## Dynamic Variables and Context

### Available Context Variables

- `{{input.*}}`: Input data fields
- `{{step_name_output.*}}`: Previous step output fields
- `{{context.*}}`: Global context fields
- `{{execution.*}}`: Execution metadata (id, start_time, etc.)
- `{{loop.*}}`: Loop-specific variables (iteration, max_iterations)

### Context Injection

```yaml
steps:
  - id: "example_step"
    # ... other fields ...
    context_injection:
      custom_var: "custom_value"
      computed_field: "{{previous_step_output.result}}"
```

## Parameter Resolution Priority

1. Explicit step parameters
2. Context injection
3. Previous step outputs (for pipeline workflows)
4. Global context
5. Input data
6. Default values

## Built-in Parameter Mappings

For the new array parameter format, the following mappings are automatically applied:

- `"query"` → `input.query` or `input.question`
- `"text"` → Pipeline input or `input.text`
- `"message"` → `input.message` or `input.query`
- `"model"` → Step's `llm_model` or default
- `"context"` → Previous step outputs concatenated
- `"results"` → `input.results` or 10
- `"depth"` → `input.depth` or "basic"
- `"analysis_type"` → `input.analysis_type` or "general"

## Error Handling and Recovery

### Step-Level Error Handling

```yaml
steps:
  - id: "fallible_step"
    # ... other fields ...
    optional: true                 # Don't fail workflow on step failure
    retry_policy:
      max_retries: 3
      backoff_multiplier: 1.5
      initial_delay_ms: 2000
    fallback_step: "backup_step"   # Execute this step on failure
```

### Workflow-Level Error Handling

```yaml
allow_partial_failure: true       # Continue on individual step failures
error_handling:
  on_step_failure: "continue"     # "continue", "retry", "abort"
  on_timeout: "cancel"            # "cancel", "force_complete"
  max_step_failures: 2           # Abort after N step failures
```

## Best Practices

### 1. Step Dependencies

- Always declare dependencies explicitly
- Avoid circular dependencies
- Use optional steps for non-critical paths

### 2. Timeouts

- Set realistic timeouts for each step
- Consider network latency and processing time
- Use shorter timeouts for simple operations

### 3. Error Handling

- Mark optional steps as `optional: true`
- Use `allow_partial_failure` for fault-tolerant workflows
- Implement proper retry policies for network operations

### 4. Parameter Design

- Prefer the new array parameter format
- Use descriptive parameter names
- Leverage context injection for dynamic values

### 5. Performance

- Use parallel workflows for independent operations
- Minimize pipeline steps for better performance
- Consider step caching for expensive operations

## Monitoring and Observability

### Progress Tracking

All workflows provide real-time progress updates:

```json
{
  "execution_id": "wf-abc123",
  "workflow_id": "research_workflow",
  "state": "running",
  "progress_percentage": 66.7,
  "current_step": "analyze_findings",
  "completed_steps": ["initial_research"],
  "remaining_steps": ["generate_summary"]
}
```

### Execution Metrics

```json
{
  "execution_time_ms": 45000,
  "step_count": 3,
  "completed_steps": 2,
  "failed_steps": 0,
  "average_step_duration_ms": 15000,
  "steps": [
    {
      "id": "initial_research",
      "status": "completed",
      "duration_ms": 30000,
      "start_time": "2024-01-01T10:00:00Z",
      "end_time": "2024-01-01T10:00:30Z"
    }
  ]
}
```

## API Integration

### REST Endpoints

- `POST /v1/workflows` - Register workflow definition
- `GET /v1/workflows` - List available workflows
- `POST /v1/workflows/{id}/execute` - Execute workflow
- `GET /v1/workflow_executions/{id}` - Get execution status
- `POST /v1/workflow_executions/{id}/pause` - Pause execution
- `POST /v1/workflow_executions/{id}/resume` - Resume execution
- `POST /v1/workflow_executions/{id}/cancel` - Cancel execution

### Example API Usage

```bash
# Register a workflow
curl -X POST http://localhost:8080/v1/workflows \
  -H "Content-Type: application/json" \
  -d @research_workflow.yaml

# Execute workflow
curl -X POST http://localhost:8080/v1/workflows/research_workflow/execute \
  -H "Content-Type: application/json" \
  -d '{"input_data": {"query": "What is quantum computing?"}}'

# Check execution status
curl http://localhost:8080/v1/workflow_executions/{execution_id}
```

## Advanced Features

### Conditional Execution

Use complex conditions to control step execution:

```yaml
conditions:
  and:
    - field: "previous_step.confidence"
      operator: "greater_than"
      value: 0.7
    - field: "input.priority"
      operator: "equals"
      value: "high"
```

### Dynamic Step Generation

Generate steps dynamically based on runtime data:

```yaml
dynamic_steps:
  source_field: "input.tasks"
  template:
    id: "task_{{index}}"
    agent_name: "TaskProcessor"
    function_name: "process"
    parameters: ["task_data"]
```

### Step Output Transformation

Transform step outputs before passing to next steps:

```yaml
steps:
  - id: "transform_output"
    output_transform:
      field_mapping:
        result: "transformed_result"
        metadata: "step_metadata"
      filter_fields: ["sensitive_data"]
```

This comprehensive workflow engine provides the foundation for building sophisticated, reliable, and maintainable multi-agent workflows in the Kolosal Agent system.
