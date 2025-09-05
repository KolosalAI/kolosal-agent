# Workflow Templates

This directory contains pre-built workflow templates that demonstrate the capabilities of the Kolosal Agent workflow engine. Each template is a JSON file that defines a complete workflow with steps, dependencies, retry policies, and execution parameters.

## Available Templates

### 1. Research and Summarize (`research-and-summarize.json`)
**Type:** Sequential  
**Purpose:** Comprehensive research workflow that gathers information on a topic and produces a structured summary.

**Key Features:**
- Initial research with multiple sources
- Deep analysis with focus areas
- Optional fact verification for low-confidence results
- Structured summary generation
- Quality review process

**Required Parameters:**
- `research_topic` - The topic to research
- `focus_areas` - Specific areas to focus on
- `summary_length` - Desired length of summary

### 2. Document Retrieval and Chat (`document-retrieval-chat.json`)
**Type:** Conditional  
**Purpose:** Retrieves relevant documents and engages in informed conversation based on the content.

**Key Features:**
- Query parsing and intent identification
- Document retrieval with relevance scoring
- Relevance evaluation
- Context-aware response generation
- Fallback response for no results

**Required Parameters:**
- `user_input` - The user's query or question

### 3. Data Analysis Pipeline (`data-analysis-pipeline.json`)
**Type:** Pipeline  
**Purpose:** Complete data analysis workflow from validation to reporting.

**Key Features:**
- Data validation and quality checks
- Preprocessing with missing value handling
- Exploratory data analysis
- Statistical analysis with hypothesis testing
- Visualization generation
- Comprehensive reporting

**Required Parameters:**
- `input_data` - Source data to analyze
- `missing_strategy` - Strategy for handling missing values
- `normalization_method` - Data normalization approach
- `research_hypothesis` - Hypothesis to test

### 4. Multi-Agent Conversation (`multi-agent-conversation.json`)
**Type:** Loop  
**Purpose:** Orchestrates a conversation between multiple AI agents with different roles.

**Key Features:**
- Multiple agent perspectives (Expert, Critic, Mediator)
- Iterative conversation rounds
- Progress evaluation
- Consensus seeking (optional)
- Dynamic termination conditions

**Required Parameters:**
- `discussion_topic` - Topic for agents to discuss

### 5. Content Generation Workflow (`content-generation.json`)
**Type:** Sequential  
**Purpose:** Comprehensive content creation from research to final publication.

**Key Features:**
- Topic research and fact gathering
- Structured outline creation
- Draft writing with style guidelines
- Fact-checking and verification
- Style and grammar review
- Final approval and formatting

**Required Parameters:**
- `content_topic` - Topic for content creation
- `content_title` - Title for the content
- `target_length` - Desired content length

## Using Templates

### Via API
```bash
# Load a template
curl -X GET http://localhost:8081/workflows/templates/research-and-summarize

# Execute a template with parameters
curl -X POST http://localhost:8081/workflows/research-and-summarize/execute \
  -H "Content-Type: application/json" \
  -d '{
    "parameters": {
      "research_topic": "artificial intelligence in healthcare",
      "focus_areas": ["diagnosis", "treatment", "ethics"],
      "summary_length": "comprehensive"
    }
  }'
```

### Via YAML Configuration
```yaml
workflows:
  my_research:
    template: "research-and-summarize"
    parameters:
      research_topic: "quantum computing applications"
      focus_areas: ["cryptography", "simulation", "optimization"]
      summary_length: "detailed"
```

## Parameter Templating

Templates use `{{parameter_name}}` syntax for parameter substitution. Parameters can be:

1. **Direct values** - Simple string, number, or boolean values
2. **Context references** - References to previous step outputs (e.g., `{{step_id.result}}`)
3. **Global context** - References to workflow-level context values
4. **System values** - Built-in values like `{{timestamp}}`, `{{iteration}}`

## Customization

Templates can be customized by:

1. **Copying and modifying** existing templates
2. **Adding new steps** with appropriate dependencies
3. **Adjusting retry policies** for different reliability requirements
4. **Modifying conditions** for different branching logic
5. **Changing timeouts** for different performance requirements

## Best Practices

1. **Test with sample data** before using in production
2. **Adjust timeouts** based on your system performance
3. **Configure retry policies** appropriate for your reliability needs
4. **Use descriptive parameter names** for clarity
5. **Document custom modifications** for team collaboration

## Creating New Templates

To create a new template:

1. Define the workflow structure and steps
2. Identify required parameters and dependencies
3. Set appropriate timeouts and retry policies
4. Test with various input scenarios
5. Document usage and requirements
6. Save as JSON in this directory

For detailed workflow definition syntax, see the [Workflow Engine Documentation](../docs/WORKFLOW_ENGINE.md).
