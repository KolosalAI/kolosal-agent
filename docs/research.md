# Deep Research Workflow Guide

## Overview

The Deep Research Workflow is a comprehensive, multi-phase research system designed to conduct thorough investigations on complex topics. It combines multiple research methodologies, cross-validation, and iterative refinement to produce high-quality, well-sourced research reports.

## Workflow Architecture

The Deep Research Workflow consists of 9 sequential phases:

### Phase 1: Research Planning
- **Agent**: `DeepResearcher`
- **Function**: `plan_research`
- **Purpose**: Analyze the research query and create a comprehensive research strategy
- **Outputs**: Research plan with key questions, required sources, and methodology

### Phase 2: Primary Internet Research
- **Agent**: `Researcher`
- **Function**: `internet_search`
- **Purpose**: Conduct initial broad research using internet sources
- **Outputs**: Primary research data from web sources

### Phase 3: Knowledge Base Search
- **Agent**: `RetrievalAgent`
- **Function**: `search_documents`
- **Purpose**: Search internal knowledge base for relevant information
- **Outputs**: Relevant documents from existing knowledge base

### Phase 4: Research Synthesis and Gap Analysis
- **Agent**: `Analyzer`
- **Function**: `synthesize_research`
- **Purpose**: Combine primary and knowledge base data, identify research gaps
- **Outputs**: Synthesis of findings and identified research gaps

### Phase 5: Secondary Research (Targeted)
- **Agent**: `DeepResearcher`
- **Function**: `targeted_research`
- **Purpose**: Conduct focused research on identified gaps
- **Outputs**: Targeted research data addressing specific gaps

### Phase 6: Cross-validation and Fact-checking
- **Agent**: `DeepResearcher`
- **Function**: `verify_facts`
- **Purpose**: Verify facts across multiple sources
- **Outputs**: Fact-checked findings with credibility scores

### Phase 7: Final Analysis and Insights
- **Agent**: `Analyzer`
- **Function**: `analyze`
- **Purpose**: Deep analysis of all collected data
- **Outputs**: Comprehensive analysis with insights and conclusions

### Phase 8: Report Generation
- **Agent**: `Assistant`
- **Function**: `generate_research_report`
- **Purpose**: Create formatted research report with citations
- **Outputs**: Professional research report with proper citations

### Phase 9: Knowledge Base Update
- **Agent**: `RetrievalAgent`
- **Function**: `add_document`
- **Purpose**: Store research findings for future use
- **Outputs**: Updated knowledge base with new findings

## Usage

### Basic Usage

Execute the deep research workflow with a simple query:

```json
{
  "workflow_id": "deep_research_workflow",
  "parameters": {
    "query": "impact of artificial intelligence on healthcare",
    "research_scope": "comprehensive",
    "depth_level": "advanced"
  }
}
```

### Advanced Configuration

Customize the research process:

```json
{
  "workflow_id": "deep_research_workflow", 
  "parameters": {
    "query": "blockchain applications in supply chain management",
    "research_scope": "comprehensive",
    "depth_level": "expert",
    "report_format": "academic",
    "include_citations": true
  }
}
```

## Parameters

### Required Parameters

- **query** (string): The primary research question or topic

### Optional Parameters

- **research_scope** (string): Scope of research
  - `narrow`: Limited sources and focused approach
  - `broad`: Moderate source coverage
  - `comprehensive`: Extensive source coverage (default)

- **depth_level** (string): Research depth level
  - `basic`: Surface-level research (3-4 key questions)
  - `intermediate`: Moderate depth (4-6 key questions) 
  - `advanced`: Deep research (7+ key questions, default)
  - `expert`: Expert-level research (9+ key questions, comprehensive analysis)

- **report_format** (string): Format of the final report
  - `summary`: Brief summary format
  - `business`: Business-oriented format
  - `academic`: Academic paper format
  - `detailed`: Comprehensive detailed format (default)

- **include_citations** (boolean): Whether to include source citations (default: true)

## Research Depth Levels

### Basic Level
- 3-4 fundamental questions
- 2-3 primary sources
- ~10 minute execution time
- Basic definition and importance

### Intermediate Level  
- 4-6 research questions
- 4-5 source types
- ~20 minute execution time
- Historical context and current applications

### Advanced Level (Default)
- 7+ comprehensive questions
- 6+ source types
- ~30 minute execution time
- Technical aspects, trends, and challenges

### Expert Level
- 9+ detailed questions
- 8+ source types
- ~45 minute execution time
- Complete theoretical framework, interdisciplinary connections, future implications

## Research Scopes

### Narrow Scope
- Academic papers and official documentation only
- ~10 expected sources
- Focused, specific research

### Broad Scope
- Academic papers, news articles, industry reports, documentation
- ~15 expected sources
- Balanced coverage

### Comprehensive Scope (Default)
- All source types including expert interviews, case studies, statistical data
- ~20+ expected sources
- Exhaustive research coverage

## Advanced Features

### Iterative Refinement
The workflow automatically refines search queries based on previous results to improve research quality.

### Cross-validation
Facts and findings are verified across multiple sources to ensure accuracy and reliability.

### Source Credibility Analysis
All sources are analyzed and scored for credibility using multiple criteria:
- Authority (domain reputation, author credentials)
- Accuracy (factual correctness, peer review)
- Currency (publication date, information freshness)
- Objectivity (bias analysis, perspective balance)

### Gap Identification
The system identifies research gaps and conducts targeted follow-up research to ensure comprehensive coverage.

## Output Structure

The workflow produces a comprehensive research report with the following structure:

```json
{
  "title": "Comprehensive Research Report",
  "executive_summary": "...",
  "sections": [
    {
      "title": "Introduction",
      "content": "..."
    },
    {
      "title": "Methodology", 
      "content": "..."
    },
    {
      "title": "Key Findings",
      "content": "..."
    },
    {
      "title": "Analysis and Insights",
      "content": "..."
    },
    {
      "title": "Conclusions and Recommendations",
      "content": "..."
    }
  ],
  "citations": [...],
  "metadata": {
    "research_phases_completed": 9,
    "total_sources_analyzed": 25,
    "report_confidence": 0.85,
    "word_count": 2500
  }
}
```

## Integration with Knowledge Base

The workflow integrates seamlessly with the system's knowledge base:

1. **Knowledge Base Search**: Leverages existing knowledge during Phase 3
2. **Knowledge Base Update**: Stores new findings during Phase 9
3. **Cumulative Learning**: Each research session improves the knowledge base for future research

## Best Practices

### Query Formulation
- Be specific but not overly narrow
- Use clear, unambiguous language
- Include context when necessary

### Scope and Depth Selection
- Choose `comprehensive` scope for important topics
- Use `expert` depth for critical business decisions
- Consider time constraints when selecting parameters

### Source Validation
- Review credibility scores in the final report
- Pay attention to source diversity
- Note any conflicting information identified

## Error Handling

The workflow includes robust error handling:
- **Partial Failure Support**: Can continue with missing optional phases
- **Timeout Management**: Each phase has configurable timeouts
- **Retry Logic**: Failed steps are automatically retried
- **Graceful Degradation**: Provides partial results if full workflow cannot complete

## Performance Considerations

### Execution Time
- Basic: ~10 minutes
- Intermediate: ~20 minutes  
- Advanced: ~30 minutes
- Expert: ~45 minutes

### Resource Usage
- Memory: Up to 1GB per workflow execution
- Network: Intensive during research phases
- Storage: Results stored in knowledge base

## Monitoring and Logging

The workflow provides comprehensive monitoring:
- **Step Progress**: Real-time progress tracking
- **Performance Metrics**: Timing and resource usage
- **Quality Metrics**: Source credibility, verification rates
- **Error Tracking**: Detailed error logging and diagnostics

## API Examples

### REST API Call

```bash
curl -X POST http://localhost:8080/api/workflows/deep_research_workflow/execute \
  -H "Content-Type: application/json" \
  -d '{
    "query": "renewable energy storage technologies",
    "research_scope": "comprehensive",
    "depth_level": "advanced",
    "report_format": "business"
  }'
```

### Response

```json
{
  "execution_id": "dr_exec_123456",
  "status": "started",
  "estimated_completion": "2024-01-01T15:30:00Z"
}
```

### Check Status

```bash
curl http://localhost:8080/api/workflows/executions/dr_exec_123456
```

## Troubleshooting

### Common Issues

1. **Timeout Errors**: Increase timeout values in configuration
2. **Source Access Issues**: Check internet connectivity and API limits
3. **Memory Issues**: Reduce research scope or enable streaming mode
4. **Quality Issues**: Increase depth level or verification depth

### Debug Mode

Enable debug logging for detailed execution information:

```yaml
# In workflow.yaml
logging:
  level: "debug"
  log_step_inputs: true
  log_step_outputs: true
```

## Future Enhancements

Planned improvements to the Deep Research Workflow:

- **Multi-language Support**: Research in multiple languages
- **Domain-specific Templates**: Specialized workflows for different fields
- **Real-time Collaboration**: Multiple researchers working on same topic
- **Advanced NLP**: Better understanding of research context and nuance
- **External API Integration**: Direct access to academic databases and APIs
