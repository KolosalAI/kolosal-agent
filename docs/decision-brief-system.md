# Decision-Grade Research Brief System

## Overview

The Decision-Grade Research Brief System is a comprehensive workflow implementation that produces high-quality, citable research briefs suitable for executive decision-making. The system orchestrates multiple AI agents to conduct thorough research, verify facts, detect contradictions, and synthesize findings into structured deliverables.

## Key Features

### 🎯 Core Capabilities
- **Multi-Phase Research Workflow**: Systematic plan→gather→verify→synthesize approach
- **Source Credibility Analysis**: Automated assessment of information source reliability
- **Contradiction Detection**: Identifies conflicting information across sources
- **Confidence Scoring**: Quantitative assessment of claim reliability
- **Structured Deliverables**: Professional-grade brief formatting with citations

### 📊 Quality Assurance
- **Minimum Source Requirements**: Configurable threshold for research depth
- **Citation Standards**: URL + accessed date in Asia/Jakarta timezone (YYYY-MM-DD)
- **Fact Verification**: Cross-validation across multiple sources
- **Executive Summary Limits**: Enforced word count constraints (≤200 words default)
- **Quality Validation**: Automated brief quality assessment

### 🔄 Workflow Integration
- **RetrievalManager (RAG)**: Web search + document database integration
- **Agent-LLM Pairing**: Intelligent agent selection for specialized tasks
- **Progress Streaming**: Real-time workflow execution monitoring
- **Error Handling**: Robust failure recovery and partial completion support

## Architecture

### Workflow Steps

1. **Parameter Validation** (`Assistant` agent)
   - Validates input parameters (topic, audience, depth, min_sources)
   - Ensures all required fields are present and valid

2. **Research Planning** (`DeepResearcher` agent)
   - Analyzes research topic complexity
   - Generates comprehensive research strategy
   - Creates search terms and research questions

3. **Primary Internet Research** (`Researcher` agent)
   - Conducts broad web-based research
   - Gathers initial source material
   - Collects diverse perspectives

4. **Knowledge Base Search** (`RetrievalAgent` agent)
   - Searches internal document repository
   - Leverages existing knowledge base
   - Provides institutional memory

5. **Cross-Reference Search** (`DeepResearcher` agent)
   - Correlates findings across sources
   - Identifies information consistency
   - Flags potential research gaps

6. **Source Credibility Analysis** (`DeepResearcher` agent)
   - Evaluates source authority and reliability
   - Scores sources based on multiple criteria
   - Filters high-credibility sources

7. **Targeted Research** (`DeepResearcher` agent)
   - Addresses identified research gaps
   - Conducts focused follow-up research
   - Seeks specialized sources

8. **Fact Verification** (`DeepResearcher` agent)
   - Cross-validates key findings
   - Verifies claims across multiple sources
   - Assigns credibility scores

9. **Synthesis and Analysis** (`Analyzer` agent)
   - Integrates all research findings
   - Identifies key themes and insights
   - Prepares structured data for reporting

10. **Contradiction Detection** (`Analyzer` agent)
    - Identifies conflicting information
    - Analyzes source disagreements
    - Provides resolution strategies

11. **Confidence Scoring** (`Analyzer` agent)
    - Calculates claim confidence levels
    - Factors in source credibility
    - Provides quantitative assessments

12. **Report Generation** (`DeepResearcher` agent)
    - Creates comprehensive research report
    - Includes all required deliverables
    - Formats for target audience

13. **Deliverable Formatting** (`Assistant` agent)
    - Formats final brief structure
    - Applies citation standards
    - Ensures compliance with requirements

14. **Quality Validation** (`Analyzer` agent)
    - Validates brief against criteria
    - Checks source counts and format
    - Confirms deliverable completeness

## API Usage

### Execute Decision-Grade Research Brief

```http
POST /workflows/decision_grade_research_brief/execute
Content-Type: application/json

{
  "input_data": {
    "topic": "Artificial Intelligence in Healthcare",
    "audience": "Healthcare Executives", 
    "depth": "advanced",
    "min_sources": 8
  },
  "async": true
}
```

**Response:**
```json
{
  "execution_id": "exec_12345",
  "status": "started",
  "estimated_duration": "20-30 minutes"
}
```

### Monitor Execution Progress

```http
GET /workflows/executions/{execution_id}
```

**Response:**
```json
{
  "execution_id": "exec_12345",
  "status": "processing",
  "progress": {
    "current_step": {
      "id": "primary_internet_research",
      "name": "Primary Internet Research",
      "status": "running"
    },
    "completed_steps": 3,
    "total_steps": 14,
    "estimated_remaining": "15 minutes"
  }
}
```

### Get Final Results

```http
GET /workflows/executions/{execution_id}/result
```

## Deliverable Structure

The system produces five key deliverables:

### 1. Executive Summary (≤200 words)
- Concise overview of key findings
- Decision-relevant insights
- Actionable intelligence summary

### 2. Numbered Key Findings with Inline Citations
- Structured findings list
- Inline citation references [1], [2], etc.
- Source-backed claims only

### 3. Annotated Source List
- Title, publisher, URL, accessed date
- Credibility scores and classifications
- Source type identification

### 4. Disagreements and Gaps Analysis
- Identified contradictions between sources
- Resolution strategies
- Research gap identification

### 5. Structured JSON Output
```json
{
  "claims": [
    {
      "claim": "AI diagnostics improve accuracy by 15-20%",
      "supporting_sources": ["url1", "url2"],
      "contradicting_sources": [],
      "confidence": 0.85,
      "evidence_summary": "Multiple studies confirm improvement"
    }
  ],
  "contradictions": [
    {
      "topic": "Implementation Costs",
      "description": "Sources disagree on ROI timeline",
      "conflicting_sources": [...],
      "severity": 0.6
    }
  ],
  "sources": [...],
  "confidence": {
    "overall": 0.82,
    "by_topic": {
      "efficacy": 0.89,
      "costs": 0.75,
      "adoption": 0.80
    }
  }
}
```

## Configuration Parameters

### Required Parameters
- **topic** (string): Research subject
- **audience** (string): Target audience for the brief
- **depth** (string): Research depth level
  - `basic`: Surface-level overview
  - `intermediate`: Balanced depth and breadth
  - `advanced`: Comprehensive analysis
  - `expert`: Exhaustive academic-level research
- **min_sources** (integer): Minimum source requirement

### Global Context Settings
- **timezone**: "Asia/Jakarta" (for citation dates)
- **date_format**: "YYYY-MM-DD" (absolute dates only)
- **min_sources_per_claim**: 2 (minimum supporting sources)
- **confidence_threshold**: 0.7 (minimum confidence for inclusion)
- **max_executive_summary_words**: 200 (strict word limit)
- **citation_style**: "url_with_date" (URL + accessed date format)

## Quality Standards

### Citation Requirements
- Every claim must include URL + accessed date
- Dates in Asia/Jakarta timezone (YYYY-MM-DD format)
- Absolute dates only (no relative references)
- Minimum 2 sources per significant claim

### Source Standards
- Credibility scoring based on authority, currency, accuracy
- Source diversification across domains and types
- Preference for primary sources in advanced/expert mode
- Automated bias detection and mitigation

### Contradiction Handling
- Flag all contradictory information
- Provide resolution strategies
- Rate contradiction severity
- Include in final assessment

### Confidence Assessment
- Quantitative confidence scoring (0.0-1.0)
- Factor in source credibility and consensus
- Claim-level and overall confidence metrics
- Distribution analysis (high/medium/low confidence)

## Python Testing Script

Use the provided test script to validate the system:

```bash
python scripts/test_decision_brief.py \
  --topic "Artificial Intelligence in Healthcare" \
  --audience "Healthcare Executives" \
  --depth "advanced" \
  --min-sources 8 \
  --save-file "ai_healthcare_brief.md"
```

### Script Features
- System health validation
- Workflow registration and execution
- Real-time progress monitoring
- Result formatting and display
- File export capabilities

## Integration Examples

### C++ Integration

```cpp
#include "workflow_types.hpp"
#include "research_brief.hpp"

// Execute decision-grade research brief
auto workflow_orchestrator = std::make_shared<WorkflowOrchestrator>(workflow_manager);
json input_data = {
    {"topic", "Climate Change Impact on Agriculture"},
    {"audience", "Policy Makers"},
    {"depth", "expert"},
    {"min_sources", 12}
};

std::string execution_id = workflow_orchestrator->execute_workflow(
    "decision_grade_research_brief", input_data
);

// Monitor execution
auto status = workflow_orchestrator->get_execution_status(execution_id);
while (status->status != WorkflowStatus::COMPLETED) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    status = workflow_orchestrator->get_execution_status(execution_id);
}

// Get formatted brief
auto result = workflow_orchestrator->get_execution_result(execution_id);
```

### REST API Integration

```python
import requests

# Execute workflow
response = requests.post('http://localhost:8080/workflows/decision_grade_research_brief/execute', 
    json={
        "input_data": {
            "topic": "Quantum Computing Applications",
            "audience": "Technology Executives", 
            "depth": "advanced",
            "min_sources": 6
        },
        "async": True
    }
)

execution_id = response.json()["execution_id"]

# Monitor progress
while True:
    status = requests.get(f'http://localhost:8080/workflows/executions/{execution_id}').json()
    if status["status"] in ["completed", "failed"]:
        break
    time.sleep(5)

# Get results
result = requests.get(f'http://localhost:8080/workflows/executions/{execution_id}/result').json()
```

## Error Handling

### Workflow-Level Recovery
- Partial failure tolerance with configurable thresholds
- Automatic retry with exponential backoff
- Graceful degradation for non-critical steps
- Comprehensive error logging and reporting

### Data Quality Safeguards
- Input validation at each step
- Source quality filtering
- Content relevance verification
- Output format validation

### Monitoring and Alerting
- Real-time execution monitoring
- Progress event streaming
- Failure detection and notification
- Performance metrics collection

## Performance Considerations

### Execution Time
- **Basic Research**: 5-10 minutes
- **Intermediate Research**: 10-20 minutes  
- **Advanced Research**: 20-30 minutes
- **Expert Research**: 30-45 minutes

### Resource Usage
- Concurrent agent execution for parallel steps
- Intelligent caching of research results
- Optimized source deduplication
- Memory-efficient large document processing

### Scalability
- Horizontal scaling of research agents
- Distributed source credibility analysis
- Parallel fact verification processes
- Load balancing across model endpoints

## Best Practices

### Topic Formulation
- Use specific, researchable topics
- Avoid overly broad or vague subjects
- Include relevant context and scope
- Consider audience knowledge level

### Audience Targeting
- Specify clear audience personas
- Consider decision-making context
- Tailor technical depth appropriately
- Include relevant business context

### Source Management
- Set realistic minimum source requirements
- Allow sufficient time for comprehensive research
- Consider topic-specific source availability
- Balance source diversity with quality

### Quality Assurance
- Review automated credibility assessments
- Validate key findings manually for critical briefs
- Cross-check contradiction analyses
- Verify citation formatting and accessibility

## Troubleshooting

### Common Issues

**Workflow Timeout**
- Increase timeout settings for complex topics
- Reduce minimum source requirements
- Use lower depth setting for initial testing

**Insufficient Sources**
- Broaden topic scope
- Reduce minimum source requirements
- Check internet connectivity and search service availability

**Low Confidence Scores**
- Review source credibility thresholds
- Validate topic researchability
- Consider alternative search strategies

**Citation Format Errors**
- Verify timezone configuration
- Check URL accessibility
- Validate date formatting

### Debug Commands

```bash
# Check system health
curl http://localhost:8080/health

# List active executions
curl http://localhost:8080/workflows/executions

# Get execution logs
curl http://localhost:8080/workflows/executions/{id}/logs

# Validate workflow definition
curl http://localhost:8080/workflows/decision_grade_research_brief
```

## Future Enhancements

### Planned Features
- Multi-language research support
- Advanced visualization capabilities
- Integration with academic databases
- Real-time collaboration features
- Custom template support

### Research Capabilities
- Domain-specific search optimization
- Expert network integration
- Automated literature reviews
- Trend analysis and forecasting
- Competitive intelligence gathering

### Quality Improvements
- Machine learning-based credibility scoring
- Advanced contradiction resolution
- Sentiment-aware analysis
- Bias detection and mitigation
- Fact-checking automation

---

*This system represents a comprehensive approach to automated research brief generation, combining the power of multi-agent orchestration with rigorous quality standards to produce decision-grade intelligence suitable for executive consumption.*
