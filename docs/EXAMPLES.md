# Examples and Code Samples

Practical examples and code samples for using the Kolosal Agent System v2.0.

## 🚀 Quick Start Examples

### Basic Agent Creation and Usage

#### Python Client Example
```python
import requests
import json

class KolosalClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
        self.session = requests.Session()
        self.session.headers.update({"Content-Type": "application/json"})
    
    def create_agent(self, name, capabilities):
        """Create a new agent"""
        data = {
            "name": name,
            "capabilities": capabilities,
            "config": {"auto_start": True}
        }
        response = self.session.post(f"{self.base_url}/v1/agents", json=data)
        return response.json()
    
    def execute_function(self, agent_id, function_name, parameters):
        """Execute a function on an agent"""
        data = {
            "function": function_name,
            "parameters": parameters
        }
        response = self.session.post(
            f"{self.base_url}/v1/agents/{agent_id}/execute",
            json=data
        )
        return response.json()
    
    def list_agents(self):
        """List all agents"""
        response = self.session.get(f"{self.base_url}/v1/agents")
        return response.json()

# Usage example
def main():
    client = KolosalClient()
    
    # Create an agent
    print("Creating agent...")
    agent = client.create_agent("DataAnalyst", ["analysis", "chat"])
    agent_id = agent["agent_id"]
    print(f"Created agent: {agent_id}")
    
    # Execute a function
    print("Executing chat function...")
    result = client.execute_function(
        agent_id,
        "chat",
        {
            "message": "Analyze the sales data trends",
            "model": "gemma3-1b"
        }
    )
    print(f"Response: {result['result']['response']}")
    
    # List all agents
    agents = client.list_agents()
    print(f"Total agents: {agents['total_count']}")

if __name__ == "__main__":
    main()
```

#### JavaScript/Node.js Example
```javascript
const axios = require('axios');

class KolosalClient {
    constructor(baseUrl = 'http://localhost:8080') {
        this.baseUrl = baseUrl;
        this.client = axios.create({
            baseURL: this.baseUrl,
            headers: {'Content-Type': 'application/json'}
        });
    }
    
    async createAgent(name, capabilities) {
        const data = {
            name: name,
            capabilities: capabilities,
            config: {auto_start: true}
        };
        const response = await this.client.post('/v1/agents', data);
        return response.data;
    }
    
    async executeFunction(agentId, functionName, parameters) {
        const data = {
            function: functionName,
            parameters: parameters
        };
        const response = await this.client.post(
            `/v1/agents/${agentId}/execute`,
            data
        );
        return response.data;
    }
    
    async listAgents() {
        const response = await this.client.get('/v1/agents');
        return response.data;
    }
}

// Usage example
async function main() {
    const client = new KolosalClient();
    
    try {
        // Create agent
        console.log('Creating agent...');
        const agent = await client.createAgent('Assistant', ['chat', 'reasoning']);
        const agentId = agent.agent_id;
        console.log(`Created agent: ${agentId}`);
        
        // Execute function
        console.log('Executing function...');
        const result = await client.executeFunction(
            agentId,
            'chat',
            {
                message: 'Hello! Can you help me understand AI agents?',
                model: 'gemma3-1b'
            }
        );
        console.log(`Response: ${result.result.response}`);
        
        // List agents
        const agents = await client.listAgents();
        console.log(`Total agents: ${agents.total_count}`);
        
    } catch (error) {
        console.error('Error:', error.response?.data || error.message);
    }
}

main();
```

### Bash/cURL Examples

#### Agent Management
```bash
#!/bin/bash

BASE_URL="http://localhost:8080"

# Function to make JSON requests
make_request() {
    local method=$1
    local endpoint=$2
    local data=$3
    
    if [ -n "$data" ]; then
        curl -s -X $method \
            -H "Content-Type: application/json" \
            -d "$data" \
            "$BASE_URL$endpoint"
    else
        curl -s -X $method "$BASE_URL$endpoint"
    fi
}

# Check system health
echo "Checking system health..."
make_request GET "/v1/health" | jq '.'

# Create an agent
echo -e "\nCreating agent..."
agent_response=$(make_request POST "/v1/agents" '{
    "name": "ResearchAssistant",
    "capabilities": ["research", "analysis", "summarization"],
    "config": {
        "auto_start": true,
        "max_concurrent_tasks": 3
    }
}')

agent_id=$(echo "$agent_response" | jq -r '.agent_id')
echo "Created agent: $agent_id"

# Execute research function
echo -e "\nExecuting research function..."
research_result=$(make_request POST "/v1/agents/$agent_id/execute" '{
    "function": "research",
    "parameters": {
        "query": "latest developments in artificial intelligence",
        "depth": "comprehensive"
    }
}')

echo "Research result:"
echo "$research_result" | jq '.result'

# List all agents
echo -e "\nListing all agents..."
make_request GET "/v1/agents" | jq '.agents[] | {id: .id, name: .name, running: .running}'

# Get system metrics
echo -e "\nSystem metrics:"
make_request GET "/v1/system/metrics" | jq '.application'
```

## 🤖 Agent Configuration Examples

### Custom Agent Configuration

#### Simple Agent Configuration
```yaml
# simple-agent.yaml
system:
  name: "Simple Agent System"
  version: "1.0.0"
  port: 8080

agents:
  - name: "ChatBot"
    capabilities: ["chat", "conversation"]
    auto_start: true
    system_prompt: |
      You are a friendly chatbot assistant. 
      Help users with their questions and provide helpful responses.
      Keep your responses concise and engaging.

  - name: "DataProcessor"
    capabilities: ["data_processing", "analysis"]
    auto_start: false
    system_prompt: |
      You are a data processing specialist.
      Analyze data, generate insights, and create summaries.
      Focus on accuracy and actionable recommendations.

functions:
  chat:
    description: "Interactive chat with users"
    timeout: 30000
    parameters:
      - name: "message"
        type: "string"
        required: true
      - name: "model"
        type: "string"
        required: true

  process_data:
    description: "Process and analyze data"
    timeout: 60000
    parameters:
      - name: "data"
        type: "object"
        required: true
      - name: "analysis_type"
        type: "string"
        required: false
        default: "basic"
```

#### Advanced Agent Configuration
```yaml
# advanced-agent.yaml
system:
  name: "Advanced Multi-Agent System"
  version: "2.0.0"
  port: 8080
  max_concurrent_requests: 200

# Global system instruction
system_instruction: |
  You are part of an advanced AI agent system designed for complex problem-solving.
  Collaborate with other agents when needed and provide accurate, helpful responses.
  Always consider the context and provide well-reasoned answers.

agents:
  - name: "Coordinator"
    capabilities: ["coordination", "planning", "delegation"]
    auto_start: true
    system_prompt: |
      You are the system coordinator responsible for managing other agents.
      Plan complex tasks, delegate work, and ensure efficient collaboration.
      Monitor progress and optimize resource allocation.

  - name: "Researcher"
    capabilities: ["research", "web_search", "fact_checking"]
    auto_start: true
    system_prompt: |
      You are a research specialist with access to web search capabilities.
      Gather accurate information, verify facts, and provide comprehensive research.
      Always cite sources and present balanced perspectives.

  - name: "Analyst"
    capabilities: ["analysis", "data_processing", "insights"]
    auto_start: true
    system_prompt: |
      You are a data analyst specialized in extracting insights from information.
      Process data thoroughly, identify patterns, and provide actionable insights.
      Use statistical methods and clear visualizations when appropriate.

  - name: "Writer"
    capabilities: ["writing", "summarization", "communication"]
    auto_start: false
    system_prompt: |
      You are a professional writer and communicator.
      Create clear, engaging content and summaries.
      Adapt your writing style to the target audience and purpose.

# Enhanced function definitions
functions:
  coordinate_task:
    description: "Coordinate complex multi-agent tasks"
    timeout: 120000
    parameters:
      - name: "task_description"
        type: "string"
        required: true
      - name: "required_capabilities"
        type: "array"
        required: true
      - name: "priority"
        type: "integer"
        required: false
        default: 5

  research_topic:
    description: "Comprehensive topic research with web search"
    timeout: 180000
    parameters:
      - name: "topic"
        type: "string"
        required: true
      - name: "depth"
        type: "string"
        required: false
        default: "detailed"
      - name: "sources"
        type: "array"
        required: false

  analyze_data:
    description: "Advanced data analysis with insights"
    timeout: 90000
    parameters:
      - name: "data"
        type: "object"
        required: true
      - name: "analysis_methods"
        type: "array"
        required: false
      - name: "output_format"
        type: "string"
        required: false
        default: "comprehensive"

  generate_report:
    description: "Generate professional reports and summaries"
    timeout: 60000
    parameters:
      - name: "content"
        type: "object"
        required: true
      - name: "format"
        type: "string"
        required: false
        default: "markdown"
      - name: "target_audience"
        type: "string"
        required: false

# Performance optimization
performance:
  max_memory_usage: "4GB"
  cache_size: "1GB"
  worker_threads: 8
  request_timeout: 60000
  max_request_size: "25MB"

# Enhanced security
security:
  enable_cors: true
  allowed_origins: 
    - "https://your-frontend.com"
    - "http://localhost:3000"
  max_request_rate: 200
  enable_auth: true
  api_key: "${API_KEY}"  # Environment variable
```

## 🔍 Web Search Integration Examples

### Internet Search Examples

#### Python Web Search Client
```python
import requests

class WebSearchClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
    
    def internet_search(self, query, max_results=10):
        """Perform internet search"""
        data = {
            "query": query,
            "max_results": max_results,
            "category": "general",
            "safe_search": True
        }
        response = requests.post(
            f"{self.base_url}/v1/search/internet",
            json=data
        )
        return response.json()
    
    def search_with_agent(self, agent_id, query):
        """Search using an agent with enhanced capabilities"""
        data = {
            "function": "internet_search",
            "parameters": {
                "query": query,
                "results": 5,
                "include_snippets": True
            }
        }
        response = requests.post(
            f"{self.base_url}/v1/agents/{agent_id}/execute",
            json=data
        )
        return response.json()

# Usage example
def search_example():
    client = WebSearchClient()
    
    # Direct search
    print("Direct internet search:")
    results = client.internet_search("Python machine learning libraries")
    for result in results["results"][:3]:
        print(f"- {result['title']}: {result['url']}")
    
    # Agent-based search (assumes agent exists)
    print("\nAgent-based search:")
    agent_results = client.search_with_agent(
        "researcher-001", 
        "latest AI research papers 2025"
    )
    if agent_results["success"]:
        for item in agent_results["result"]["results"]:
            print(f"- {item['title']}")

search_example()
```

#### JavaScript Web Search
```javascript
class WebSearchClient {
    constructor(baseUrl = 'http://localhost:8080') {
        this.baseUrl = baseUrl;
    }
    
    async searchInternet(query, options = {}) {
        const data = {
            query: query,
            max_results: options.maxResults || 10,
            category: options.category || 'general',
            language: options.language || 'en',
            safe_search: options.safeSearch !== false
        };
        
        const response = await fetch(`${this.baseUrl}/v1/search/internet`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(data)
        });
        
        return response.json();
    }
    
    async searchWithSummary(query) {
        // First search
        const searchResults = await this.searchInternet(query, {maxResults: 5});
        
        // Then create an agent to summarize results
        const agentResponse = await fetch(`${this.baseUrl}/v1/agents`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({
                name: 'SearchSummarizer',
                capabilities: ['analysis', 'summarization']
            })
        });
        
        const agent = await agentResponse.json();
        
        // Summarize the search results
        const summaryResponse = await fetch(
            `${this.baseUrl}/v1/agents/${agent.agent_id}/execute`,
            {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({
                    function: 'analyze',
                    parameters: {
                        text: JSON.stringify(searchResults.results),
                        analysis_type: 'summary'
                    }
                })
            }
        );
        
        return summaryResponse.json();
    }
}

// Usage
async function searchAndSummarize() {
    const client = new WebSearchClient();
    
    try {
        const result = await client.searchWithSummary(
            'renewable energy technologies 2025'
        );
        console.log('Search summary:', result.result.summary);
    } catch (error) {
        console.error('Search error:', error);
    }
}

searchAndSummarize();
```

### Document Retrieval Examples

#### Document Upload and Search
```python
import requests
import os

class DocumentClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
    
    def upload_document(self, file_path, collection="default"):
        """Upload a document to the knowledge base"""
        with open(file_path, 'rb') as f:
            files = {'file': f}
            data = {'collection': collection}
            response = requests.post(
                f"{self.base_url}/v1/documents/upload",
                files=files,
                data=data
            )
        return response.json()
    
    def search_documents(self, query, collection="default", max_results=5):
        """Search documents in the knowledge base"""
        data = {
            "query": query,
            "collection": collection,
            "max_results": max_results,
            "similarity_threshold": 0.7
        }
        response = requests.post(
            f"{self.base_url}/v1/search/documents",
            json=data
        )
        return response.json()
    
    def hybrid_search(self, query, agent_id):
        """Perform hybrid search using both web and documents"""
        data = {
            "function": "knowledge_retrieval",
            "parameters": {
                "query": query,
                "search_web": True,
                "search_documents": True,
                "max_web_results": 5,
                "max_doc_results": 3
            }
        }
        response = requests.post(
            f"{self.base_url}/v1/agents/{agent_id}/execute",
            json=data
        )
        return response.json()

# Usage example
def document_example():
    client = DocumentClient()
    
    # Upload documents
    print("Uploading documents...")
    for doc in ["research_paper.pdf", "manual.docx", "notes.txt"]:
        if os.path.exists(doc):
            result = client.upload_document(doc, "research")
            print(f"Uploaded {doc}: {result.get('document_id', 'Error')}")
    
    # Search documents
    print("\nSearching documents...")
    doc_results = client.search_documents(
        "machine learning algorithms",
        "research"
    )
    for result in doc_results["results"]:
        print(f"- {result['title']} (score: {result['similarity_score']:.2f})")
    
    # Hybrid search (assumes agent exists)
    print("\nHybrid search...")
    hybrid_results = client.hybrid_search(
        "neural network architectures",
        "researcher-001"
    )
    if hybrid_results["success"]:
        combined_results = hybrid_results["result"]
        print(f"Found {len(combined_results['web_results'])} web results")
        print(f"Found {len(combined_results['document_results'])} document results")

document_example()
```

## 🔄 Workflow Examples

### Sequential Workflow
```python
import requests
import time

class WorkflowClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
        self.session = requests.Session()
    
    def create_research_workflow(self):
        """Create a sequential research workflow"""
        agents = []
        
        # Create researcher agent
        researcher = self.session.post(f"{self.base_url}/v1/agents", json={
            "name": "Researcher",
            "capabilities": ["research", "web_search"]
        }).json()
        agents.append(researcher["agent_id"])
        
        # Create analyst agent
        analyst = self.session.post(f"{self.base_url}/v1/agents", json={
            "name": "Analyst", 
            "capabilities": ["analysis", "data_processing"]
        }).json()
        agents.append(analyst["agent_id"])
        
        # Create writer agent
        writer = self.session.post(f"{self.base_url}/v1/agents", json={
            "name": "Writer",
            "capabilities": ["writing", "summarization"]
        }).json()
        agents.append(writer["agent_id"])
        
        return agents
    
    def execute_research_workflow(self, topic, agents):
        """Execute the research workflow"""
        researcher_id, analyst_id, writer_id = agents
        
        print(f"Starting research workflow for: {topic}")
        
        # Step 1: Research
        print("Step 1: Researching topic...")
        research_result = self.session.post(
            f"{self.base_url}/v1/agents/{researcher_id}/execute",
            json={
                "function": "research",
                "parameters": {
                    "query": topic,
                    "depth": "comprehensive"
                }
            }
        ).json()
        
        if not research_result["success"]:
            return {"error": "Research failed"}
        
        research_data = research_result["result"]
        
        # Step 2: Analysis
        print("Step 2: Analyzing research data...")
        analysis_result = self.session.post(
            f"{self.base_url}/v1/agents/{analyst_id}/execute",
            json={
                "function": "analyze",
                "parameters": {
                    "text": str(research_data),
                    "analysis_type": "comprehensive"
                }
            }
        ).json()
        
        if not analysis_result["success"]:
            return {"error": "Analysis failed"}
        
        analysis_data = analysis_result["result"]
        
        # Step 3: Writing
        print("Step 3: Creating final report...")
        report_result = self.session.post(
            f"{self.base_url}/v1/agents/{writer_id}/execute",
            json={
                "function": "generate_report",
                "parameters": {
                    "content": {
                        "research": research_data,
                        "analysis": analysis_data,
                        "topic": topic
                    },
                    "format": "markdown"
                }
            }
        ).json()
        
        return {
            "success": report_result["success"],
            "research": research_data,
            "analysis": analysis_data,
            "report": report_result.get("result", {})
        }

# Usage
def workflow_example():
    client = WorkflowClient()
    
    # Create agents
    print("Creating workflow agents...")
    agents = client.create_research_workflow()
    print(f"Created {len(agents)} agents")
    
    # Execute workflow
    topic = "Impact of artificial intelligence on healthcare"
    result = client.execute_research_workflow(topic, agents)
    
    if result.get("success"):
        print("\nWorkflow completed successfully!")
        print(f"Report: {result['report'].get('summary', 'No summary available')}")
    else:
        print(f"Workflow failed: {result.get('error', 'Unknown error')}")

workflow_example()
```

### Parallel Processing Example
```python
import asyncio
import aiohttp

class ParallelProcessingClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
    
    async def create_processing_agents(self, session, count=3):
        """Create multiple processing agents"""
        tasks = []
        for i in range(count):
            task = session.post(
                f"{self.base_url}/v1/agents",
                json={
                    "name": f"Processor_{i+1}",
                    "capabilities": ["data_processing", "analysis"]
                }
            )
            tasks.append(task)
        
        responses = await asyncio.gather(*tasks)
        agents = []
        for response in responses:
            result = await response.json()
            agents.append(result["agent_id"])
        
        return agents
    
    async def process_data_parallel(self, session, data_chunks, agents):
        """Process data chunks in parallel using multiple agents"""
        tasks = []
        
        for i, chunk in enumerate(data_chunks):
            agent_id = agents[i % len(agents)]  # Round-robin assignment
            
            task = session.post(
                f"{self.base_url}/v1/agents/{agent_id}/execute",
                json={
                    "function": "process_data",
                    "parameters": {
                        "data": chunk,
                        "chunk_id": i,
                        "analysis_type": "fast"
                    }
                }
            )
            tasks.append(task)
        
        responses = await asyncio.gather(*tasks)
        results = []
        for response in responses:
            result = await response.json()
            results.append(result)
        
        return results

async def parallel_example():
    client = ParallelProcessingClient()
    
    async with aiohttp.ClientSession() as session:
        # Create agents
        print("Creating parallel processing agents...")
        agents = await client.create_processing_agents(session, 3)
        print(f"Created {len(agents)} agents")
        
        # Prepare data chunks
        data_chunks = [
            {"sales_data": f"chunk_{i}", "month": f"2025-{i+1:02d}"}
            for i in range(6)
        ]
        
        # Process in parallel
        print("Processing data chunks in parallel...")
        start_time = asyncio.get_event_loop().time()
        
        results = await client.process_data_parallel(session, data_chunks, agents)
        
        end_time = asyncio.get_event_loop().time()
        print(f"Processed {len(data_chunks)} chunks in {end_time - start_time:.2f} seconds")
        
        # Display results
        successful_results = [r for r in results if r.get("success")]
        print(f"Successful: {len(successful_results)}/{len(results)}")

# Run the example
asyncio.run(parallel_example())
```

## 🔧 Integration Examples

### Flask Web Application Integration
```python
from flask import Flask, request, jsonify, render_template
import requests

app = Flask(__name__)

class KolosalIntegration:
    def __init__(self, kolosal_url="http://localhost:8080"):
        self.kolosal_url = kolosal_url
        self.session = requests.Session()
    
    def create_chat_agent(self):
        """Create a chat agent for the web app"""
        response = self.session.post(f"{self.kolosal_url}/v1/agents", json={
            "name": "WebChatAgent",
            "capabilities": ["chat", "conversation"],
            "config": {"auto_start": True}
        })
        return response.json()["agent_id"]
    
    def chat_with_agent(self, agent_id, message):
        """Send a chat message to the agent"""
        response = self.session.post(
            f"{self.kolosal_url}/v1/agents/{agent_id}/execute",
            json={
                "function": "chat",
                "parameters": {
                    "message": message,
                    "model": "gemma3-1b"
                }
            }
        )
        return response.json()

# Initialize Kolosal integration
kolosal = KolosalIntegration()
chat_agent_id = None

@app.before_first_request
def setup():
    global chat_agent_id
    chat_agent_id = kolosal.create_chat_agent()

@app.route('/')
def index():
    return render_template('chat.html')

@app.route('/api/chat', methods=['POST'])
def chat():
    data = request.json
    message = data.get('message', '')
    
    if not message:
        return jsonify({"error": "Message is required"}), 400
    
    try:
        result = kolosal.chat_with_agent(chat_agent_id, message)
        if result.get("success"):
            return jsonify({
                "response": result["result"]["response"],
                "execution_time": result["execution_time_ms"]
            })
        else:
            return jsonify({"error": "Agent execution failed"}), 500
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/system/status')
def system_status():
    try:
        response = kolosal.session.get(f"{kolosal.kolosal_url}/v1/system/status")
        return response.json()
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True, port=5000)
```

### React Frontend Integration
```javascript
// KolosalClient.js
class KolosalClient {
    constructor(baseUrl = 'http://localhost:8080') {
        this.baseUrl = baseUrl;
    }
    
    async createAgent(name, capabilities) {
        const response = await fetch(`${this.baseUrl}/v1/agents`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({
                name: name,
                capabilities: capabilities,
                config: {auto_start: true}
            })
        });
        return response.json();
    }
    
    async executeFunction(agentId, functionName, parameters) {
        const response = await fetch(`${this.baseUrl}/v1/agents/${agentId}/execute`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({
                function: functionName,
                parameters: parameters
            })
        });
        return response.json();
    }
    
    async getSystemStatus() {
        const response = await fetch(`${this.baseUrl}/v1/system/status`);
        return response.json();
    }
}

// ChatComponent.jsx
import React, { useState, useEffect } from 'react';

const ChatComponent = () => {
    const [messages, setMessages] = useState([]);
    const [input, setInput] = useState('');
    const [agentId, setAgentId] = useState(null);
    const [loading, setLoading] = useState(false);
    
    const client = new KolosalClient();
    
    useEffect(() => {
        // Create chat agent on component mount
        const setupAgent = async () => {
            try {
                const agent = await client.createAgent('ChatAgent', ['chat']);
                setAgentId(agent.agent_id);
            } catch (error) {
                console.error('Failed to create agent:', error);
            }
        };
        
        setupAgent();
    }, []);
    
    const sendMessage = async () => {
        if (!input.trim() || !agentId || loading) return;
        
        const userMessage = input.trim();
        setInput('');
        setMessages(prev => [...prev, {type: 'user', content: userMessage}]);
        setLoading(true);
        
        try {
            const result = await client.executeFunction(agentId, 'chat', {
                message: userMessage,
                model: 'gemma3-1b'
            });
            
            if (result.success) {
                setMessages(prev => [...prev, {
                    type: 'agent',
                    content: result.result.response,
                    executionTime: result.execution_time_ms
                }]);
            } else {
                setMessages(prev => [...prev, {
                    type: 'error',
                    content: 'Failed to get response from agent'
                }]);
            }
        } catch (error) {
            setMessages(prev => [...prev, {
                type: 'error',
                content: `Error: ${error.message}`
            }]);
        } finally {
            setLoading(false);
        }
    };
    
    return (
        <div className="chat-container">
            <div className="messages">
                {messages.map((message, index) => (
                    <div key={index} className={`message ${message.type}`}>
                        <div className="content">{message.content}</div>
                        {message.executionTime && (
                            <div className="execution-time">
                                Executed in {message.executionTime}ms
                            </div>
                        )}
                    </div>
                ))}
                {loading && <div className="message loading">Agent is thinking...</div>}
            </div>
            
            <div className="input-area">
                <input
                    type="text"
                    value={input}
                    onChange={(e) => setInput(e.target.value)}
                    onKeyPress={(e) => e.key === 'Enter' && sendMessage()}
                    placeholder="Type your message..."
                    disabled={!agentId || loading}
                />
                <button 
                    onClick={sendMessage}
                    disabled={!agentId || loading || !input.trim()}
                >
                    Send
                </button>
            </div>
        </div>
    );
};

export default ChatComponent;
```

## 🚀 Performance Testing Examples

### Load Testing Script
```python
import asyncio
import aiohttp
import time
import statistics

class LoadTester:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
        self.results = []
    
    async def test_agent_creation(self, session, test_id):
        """Test agent creation performance"""
        start_time = time.time()
        
        try:
            async with session.post(
                f"{self.base_url}/v1/agents",
                json={
                    "name": f"LoadTestAgent_{test_id}",
                    "capabilities": ["chat"]
                }
            ) as response:
                result = await response.json()
                end_time = time.time()
                
                return {
                    "test_id": test_id,
                    "success": response.status == 200,
                    "response_time": (end_time - start_time) * 1000,
                    "agent_id": result.get("agent_id")
                }
        except Exception as e:
            end_time = time.time()
            return {
                "test_id": test_id,
                "success": False,
                "response_time": (end_time - start_time) * 1000,
                "error": str(e)
            }
    
    async def test_function_execution(self, session, agent_id, test_id):
        """Test function execution performance"""
        start_time = time.time()
        
        try:
            async with session.post(
                f"{self.base_url}/v1/agents/{agent_id}/execute",
                json={
                    "function": "chat",
                    "parameters": {
                        "message": f"Test message {test_id}",
                        "model": "gemma3-1b"
                    }
                }
            ) as response:
                result = await response.json()
                end_time = time.time()
                
                return {
                    "test_id": test_id,
                    "success": response.status == 200 and result.get("success", False),
                    "response_time": (end_time - start_time) * 1000,
                    "execution_time": result.get("execution_time_ms", 0)
                }
        except Exception as e:
            end_time = time.time()
            return {
                "test_id": test_id,
                "success": False,
                "response_time": (end_time - start_time) * 1000,
                "error": str(e)
            }
    
    async def run_load_test(self, num_requests=100, concurrency=10):
        """Run load test with specified parameters"""
        print(f"Running load test: {num_requests} requests, {concurrency} concurrent")
        
        # First, create a test agent
        async with aiohttp.ClientSession() as session:
            agent_result = await self.test_agent_creation(session, "setup")
            if not agent_result["success"]:
                print("Failed to create test agent")
                return
            
            agent_id = agent_result["agent_id"]
            print(f"Created test agent: {agent_id}")
            
            # Run concurrent function execution tests
            tasks = []
            semaphore = asyncio.Semaphore(concurrency)
            
            async def limited_test(test_id):
                async with semaphore:
                    return await self.test_function_execution(session, agent_id, test_id)
            
            for i in range(num_requests):
                tasks.append(limited_test(i))
            
            print("Executing load test...")
            start_time = time.time()
            results = await asyncio.gather(*tasks)
            end_time = time.time()
            
            # Analyze results
            successful_results = [r for r in results if r["success"]]
            failed_results = [r for r in results if not r["success"]]
            
            response_times = [r["response_time"] for r in successful_results]
            execution_times = [r["execution_time"] for r in successful_results]
            
            print(f"\nLoad Test Results:")
            print(f"Total time: {end_time - start_time:.2f} seconds")
            print(f"Successful requests: {len(successful_results)}/{num_requests}")
            print(f"Failed requests: {len(failed_results)}")
            print(f"Success rate: {len(successful_results)/num_requests*100:.1f}%")
            print(f"Requests per second: {num_requests/(end_time - start_time):.2f}")
            
            if response_times:
                print(f"\nResponse Time Statistics (ms):")
                print(f"  Mean: {statistics.mean(response_times):.2f}")
                print(f"  Median: {statistics.median(response_times):.2f}")
                print(f"  Min: {min(response_times):.2f}")
                print(f"  Max: {max(response_times):.2f}")
                print(f"  95th percentile: {statistics.quantiles(response_times, n=20)[18]:.2f}")
            
            if execution_times:
                print(f"\nExecution Time Statistics (ms):")
                print(f"  Mean: {statistics.mean(execution_times):.2f}")
                print(f"  Median: {statistics.median(execution_times):.2f}")

# Run load test
async def main():
    tester = LoadTester()
    await tester.run_load_test(num_requests=50, concurrency=5)

if __name__ == "__main__":
    asyncio.run(main())
```

These examples provide a comprehensive foundation for working with the Kolosal Agent System. They demonstrate various integration patterns, configuration options, and usage scenarios that can be adapted for specific use cases.

For more advanced examples and specific use cases, refer to the [API Reference](API_REFERENCE.md) and [Developer Guide](DEVELOPER_GUIDE.md).
