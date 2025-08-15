# 🚀 Kolosal Agent Workflow - Quick Start Guide

## Overview
The Kolosal Agent now supports full workflow execution via REST API endpoints. The sequential research workflow has been converted from a YAML file to accessible REST API endpoints with comprehensive 10-step research intelligence capabilities.

## 📋 Available Workflow Endpoints

### 1. List All Workflows
**GET** `/v1/workflows`
```powershell
Invoke-RestMethod -Uri "http://127.0.0.1:8080/v1/workflows" -Method GET
```

### 2. Execute Sequential Workflow (Default)
**GET** `/v1/workflows/builtin/sequential`
```powershell
Invoke-RestMethod -Uri "http://127.0.0.1:8080/v1/workflows/builtin/sequential" -Method GET
```

### 3. Execute Sequential Workflow (Custom Data)
**POST** `/v1/workflows/builtin/sequential`
```powershell
$body = @{ 
    topic = "Your Research Topic"
    research_focus = "Specific focus area" 
} | ConvertTo-Json

Invoke-RestMethod -Uri "http://127.0.0.1:8080/v1/workflows/builtin/sequential" -Method POST -Body $body -ContentType "application/json"
```

### 4. API Discovery
**GET** `/v1/endpoints`
```powershell
Invoke-RestMethod -Uri "http://127.0.0.1:8080/v1/endpoints" -Method GET
```

## 🛠️ Quick Start Methods

### Method 1: Simple PowerShell Script
Use the quick-start script for immediate workflow execution:

```powershell
# Default execution
.\start_workflow.ps1

# Custom topic and focus
.\start_workflow.ps1 -Topic "AI in Healthcare" -Focus "diagnostic accuracy"

# With different server URL
.\start_workflow.ps1 -Topic "Quantum Computing" -Focus "practical applications" -ServerUrl "http://localhost:8080"
```

### Method 2: Comprehensive API Testing
Run the full test suite to explore all capabilities:

```powershell
.\test_workflow_api.ps1
```

### Method 3: Direct PowerShell Commands
Execute workflows directly with custom parameters:

```powershell
# AI Ethics Research
$request = @{
    topic = "AI Ethics and Algorithmic Bias"
    research_focus = "fairness in machine learning systems"
    methodology = "systematic review"
    domain = "enterprise AI"
    priority = "high"
}
$body = $request | ConvertTo-Json
Invoke-RestMethod -Uri "http://127.0.0.1:8080/v1/workflows/builtin/sequential" -Method POST -Body $body -ContentType "application/json"

# Climate Change Research
$request = @{
    topic = "Climate Change Mitigation"
    research_focus = "renewable energy adoption strategies"
    scope = "global implementation"
    timeline = "5 years"
}
$body = $request | ConvertTo-Json
Invoke-RestMethod -Uri "http://127.0.0.1:8080/v1/workflows/builtin/sequential" -Method POST -Body $body -ContentType "application/json"

# Cybersecurity Research
$request = @{
    topic = "Cybersecurity in IoT"
    research_focus = "vulnerability assessment and protection"
    constraints = @{
        compliance = "GDPR, CCPA"
        budget = "enterprise level"
    }
}
$body = $request | ConvertTo-Json -Depth 3
Invoke-RestMethod -Uri "http://127.0.0.1:8080/v1/workflows/builtin/sequential" -Method POST -Body $body -ContentType "application/json"
```

### Method 4: Using cURL (Cross-platform)
For Linux/Mac users or cross-platform compatibility:

```bash
# List workflows
curl -X GET "http://127.0.0.1:8080/v1/workflows"

# Execute default workflow
curl -X GET "http://127.0.0.1:8080/v1/workflows/builtin/sequential"

# Execute with custom data
curl -X POST "http://127.0.0.1:8080/v1/workflows/builtin/sequential" \
  -H "Content-Type: application/json" \
  -d '{
    "topic": "Machine Learning in Autonomous Vehicles",
    "research_focus": "safety and reliability systems",
    "methodology": "comparative analysis"
  }'
```

## 🔧 Request Customization

### Basic Request Structure
```json
{
  "topic": "Your Research Topic",
  "research_focus": "Specific area of focus"
}
```

### Advanced Request Structure
```json
{
  "topic": "Research Topic",
  "research_focus": "Focus area",
  "methodology": "research approach",
  "domain": "field or industry",
  "scope": "breadth and depth",
  "timeline": "expected duration",
  "priority": "high|medium|low",
  "requirements": ["requirement1", "requirement2"],
  "constraints": {
    "budget": "available resources",
    "access": "data limitations",
    "compliance": "regulatory requirements"
  },
  "deliverables": ["report", "summary", "recommendations"],
  "metadata": {
    "requestor": "name or department",
    "project_id": "identifier",
    "tags": ["tag1", "tag2"]
  }
}
```

## 📊 Workflow Response Structure

### Successful Response
```json
{
  "workflow_id": "sequential",
  "status": "completed",
  "execution_time": "simulated",
  "input": {
    "topic": "Your Research Topic",
    "research_focus": "Your Focus Area"
  },
  "workflow": {
    "name": "Sequential Research Workflow",
    "description": "10-step research intelligence workflow",
    "steps": [
      {
        "name": "research_planning",
        "role": "Research Planner", 
        "goal": "Create comprehensive research plan"
      }
      // ... 9 more steps
    ]
  },
  "results": [
    {
      "step": "research_planning",
      "status": "completed",
      "output": "Research plan generated"
    }
    // ... 9 more results
  ]
}
```

## 🎯 10-Step Sequential Workflow Process

1. **Research Planning** (Research Planner) - Create comprehensive research plan
2. **Methodology Selection** (Methodology Expert) - Select appropriate research methodology  
3. **Knowledge Gathering** (Information Collector) - Gather relevant knowledge and data
4. **Credibility Analysis** (Source Analyst) - Analyze source credibility and reliability
5. **Entity Extraction** (Entity Extractor) - Extract key entities and concepts
6. **Relationship Mapping** (Relationship Mapper) - Map relationships between entities
7. **Fact Verification** (Fact Checker) - Verify facts and validate information
8. **Information Synthesis** (Information Synthesizer) - Synthesize information into coherent insights
9. **Report Generation** (Report Generator) - Generate comprehensive research report
10. **Citation Management** (Citation Manager) - Manage citations and references

## 🚦 Prerequisites

1. **Start the Kolosal Server:**
   ```powershell
   cd D:\Works\Genta\codes\kolosal-agent
   .\build\kolosal-server\Debug\kolosal-server.exe --config config.yaml
   ```

2. **Verify Server Status:**
   ```powershell
   Invoke-RestMethod -Uri "http://127.0.0.1:8080/health" -Method GET
   ```

## 📝 Example Workflow Topics

- **AI & Machine Learning**: "AI Ethics", "Machine Learning in Healthcare", "Autonomous Vehicle Safety"
- **Technology**: "Quantum Computing Applications", "Blockchain in Supply Chain", "5G Network Security"
- **Cybersecurity**: "IoT Device Security", "Zero Trust Architecture", "Threat Intelligence"
- **Environment**: "Climate Change Mitigation", "Renewable Energy Systems", "Carbon Footprint Analysis"
- **Business**: "Digital Transformation", "Remote Work Productivity", "Supply Chain Optimization"

## 🎉 Success Indicators

✅ Server responds with HTTP 200 status
✅ Workflow completes all 10 steps
✅ Each step shows "completed" status  
✅ Response includes detailed workflow information
✅ Input parameters are properly reflected in output

## 🔍 Troubleshooting

- **Connection Refused**: Ensure the Kolosal server is running on the correct port (default: 8080)
- **Invalid JSON**: Verify request body is properly formatted JSON
- **Missing Parameters**: Use the JSON template file for proper request structure
- **Server Errors**: Check server logs for detailed error information

## 📚 Additional Resources

- `test_workflow_api.ps1` - Comprehensive API testing script
- `start_workflow.ps1` - Quick-start workflow execution
- `workflow_request_template.json` - JSON template for custom requests
- `curl_workflow_examples.sh` - Cross-platform cURL examples
