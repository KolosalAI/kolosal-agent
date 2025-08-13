# Workflow Engine Configuration Guide

The Kolosal Agent Workflow Engine supports sophisticated multi-agent orchestration with various execution patterns. This guide explains how to create and configure workflows using YAML files.

## Workflow Types

### 1. Sequential Workflow
Executes steps one after another in a defined order.

```yaml
type: "sequential"
```

**Use Cases:**
- Step-by-step processes where each step depends on the previous
- Linear data processing pipelines
- Traditional workflow automation

**Example:** `sequential.yaml` - Research → Analysis → Report → Execution

### 2. Parallel Workflow
Executes all steps simultaneously for maximum throughput.

```yaml
type: "parallel"
```

**Use Cases:**
- Independent tasks that can run concurrently
- Data collection from multiple sources
- Batch processing scenarios

**Example:** `examples/parallel_workflow.yaml` - Multiple research perspectives

### 3. Pipeline Workflow
Executes steps in dependency-resolved order, allowing parallel execution when possible.

```yaml
type: "pipeline"
```

**Use Cases:**
- Complex data processing with multiple stages
- ETL (Extract, Transform, Load) operations
- Multi-stage analysis workflows

**Example:** `examples/pipeline_workflow.yaml` - Data Collection → Validation → Analysis → Synthesis → Report

### 4. Consensus Workflow
Multiple agents vote on decisions, followed by consensus aggregation.

```yaml
type: "consensus"
```

**Use Cases:**
- Multi-perspective decision making
- Quality assurance with multiple reviewers
- Democratic agent voting systems

**Example:** `examples/consensus_workflow.yaml` - Technical/Business/Ethics votes → Consensus decision

### 5. Conditional Workflow
Executes steps based on conditions and previous step results.

```yaml
type: "conditional"
```

**Use Cases:**
- Adaptive processing based on data quality
- Error handling with fallback paths
- Dynamic workflow routing

**Example:** `examples/conditional_workflow.yaml` - Quality-based processing paths

## Configuration Structure

### Basic Workflow Properties

```yaml
id: "unique_workflow_id"
name: "Human Readable Name"
description: "Detailed description of workflow purpose"
type: "sequential|parallel|pipeline|consensus|conditional"
created_by: "creator_name"
```

### Global Context
Variables available to all workflow steps:

```yaml
global_context:
  project_name: "My Project"
  output_format: "json"
  max_tokens: 2048
```

### Execution Settings

```yaml
settings:
  max_execution_time: 600        # Maximum workflow runtime (seconds)
  max_concurrent_steps: 4        # Maximum parallel step execution
  auto_cleanup: true             # Clean up completed workflows
  persist_state: true            # Save state for crash recovery
```

### Error Handling Strategy

```yaml
error_handling:
  retry_on_failure: true         # Retry failed steps
  max_retries: 3                 # Maximum retry attempts
  retry_delay_seconds: 2         # Delay between retries
  continue_on_error: false       # Continue workflow on step failure
  use_fallback_agent: false      # Use fallback agent on failure
  fallback_agent_id: "backup_agent"
  fallback_parameters: {}        # Parameters for fallback agent
```

## Step Configuration

### Basic Step Properties

```yaml
- id: "unique_step_id"
  name: "Step Name"
  description: "What this step does"
  agent_id: "target_agent"        # Agent to execute this step
  function: "function_name"       # Agent function to call
```

### Step Parameters

```yaml
parameters:
  input_data: "static value"
  dynamic_value: "${global.variable_name}"
  step_output: "${steps.previous_step.output}"
  complex_object:
    nested_field: "value"
    computed: "${steps.step1.output.field}"
```

### Dependencies

Simple dependency (wait for success):
```yaml
depends_on:
  - "previous_step_id"
```

Advanced dependency with conditions:
```yaml
depends_on:
  - step: "previous_step"
    condition: "success"     # success|completion|failure
    required: true           # true|false
```

### Conditions

Execute step only if conditions are met:
```yaml
conditions:
  expression: "steps.assessment.output.quality_score >= 0.8"
```

### Execution Settings

```yaml
timeout: 120                    # Step timeout (seconds)
max_retries: 3                  # Step-specific retry count
retry_delay: 2                  # Step-specific retry delay
parallel_allowed: true          # Can run in parallel with others
continue_on_error: false        # Continue workflow if step fails
```

## Variable Interpolation

The workflow engine supports dynamic variable interpolation:

### Global Variables
```yaml
"${global.variable_name}"
```

### Step Outputs
```yaml
"${steps.step_id.output}"
"${steps.step_id.output.nested.field}"
```

### Conditional Values
```yaml
"${steps.step1.output || steps.step2.output}"  # Use first non-null value
```

## Agent Functions

Ensure your agents support the functions referenced in workflows:

### Research Analyst Functions
- `research_topic` - Conduct research on topics
- `analyze_data` - Analyze structured/unstructured data
- `generate_report` - Create formatted reports
- `synthesize_information` - Combine multiple data sources

### Task Executor Functions
- `execute_task` - Execute general tasks with parameters
- `use_tool` - Use specific tools or utilities
- `process_files` - File operations
- `call_api` - External API integration

## Best Practices

### 1. Step Naming
- Use clear, descriptive step IDs
- Follow consistent naming conventions
- Include the step's primary purpose

### 2. Error Handling
- Set appropriate timeouts for long-running steps
- Configure retry strategies based on step reliability
- Use `continue_on_error` carefully to balance robustness and reliability

### 3. Dependencies
- Keep dependency chains as short as possible
- Use parallel execution where steps are independent
- Consider using conditional execution for optional steps

### 4. Parameters
- Use global context for workflow-wide settings
- Leverage step output interpolation for data flow
- Validate parameter structure matches agent function expectations

### 5. Performance
- Set realistic timeouts based on expected execution time
- Limit concurrent steps to available system resources
- Use appropriate workflow types for your use case

## Loading and Executing Workflows

### From Configuration File
```cpp
WorkflowEngine engine(agent_manager);
engine.start();
engine.load_workflow_from_yaml("sequential.yaml");
```

### Programmatic Execution
```cpp
auto workflows = engine.list_workflows();
if (!workflows.empty()) {
    json input_context = {
        {"topic", "Machine Learning Applications"},
        {"urgency", "high"}
    };
    
    std::string execution_id = engine.execute_workflow(workflows[0], input_context);
    
    // Monitor execution
    auto status = engine.get_execution_status(execution_id);
    while (status && status->current_status == WorkflowStatus::RUNNING) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        status = engine.get_execution_status(execution_id);
    }
}
```

### Workflow Management
```cpp
// List all workflows
auto workflow_ids = engine.list_workflows();

// Get workflow details
auto workflow = engine.get_workflow("workflow_id");

// Monitor active executions
auto active = engine.get_active_executions();

// Get execution history
auto history = engine.get_execution_history();

// Control execution
engine.pause_workflow(execution_id);
engine.resume_workflow(execution_id);
engine.cancel_workflow(execution_id);
```

## Monitoring and Debugging

### Execution Status
Monitor workflow progress through execution status:
- `PENDING` - Workflow created, not started
- `RUNNING` - Currently executing
- `PAUSED` - Temporarily paused
- `COMPLETED` - Successfully finished
- `FAILED` - Execution failed
- `CANCELLED` - Manually cancelled
- `TIMEOUT` - Exceeded time limit

### Step Status
Individual step progress tracking:
- `PENDING` - Not yet executed
- `RUNNING` - Currently executing
- `COMPLETED` - Successfully finished
- `FAILED` - Execution failed
- `SKIPPED` - Skipped due to conditions
- `RETRYING` - Being retried after failure

### Metrics and Analytics
```cpp
auto metrics = engine.get_metrics();
std::cout << "Success Rate: " << metrics.success_rate << "%\n";
std::cout << "Average Execution Time: " << metrics.average_execution_time_ms << "ms\n";
```

This workflow system provides a powerful foundation for orchestrating complex multi-agent operations with flexibility, reliability, and monitoring capabilities.
