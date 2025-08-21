# Deep Research Agent - Comprehensive Information Gathering System

## Overview

The Deep Research Agent is a sophisticated research system that combines web search, document retrieval, and advanced analysis capabilities to conduct comprehensive research on any given topic. It integrates seamlessly with the kolosal-server to provide real-time web search and document processing capabilities.

## Features

### 🔍 **Multi-Source Research**
- **Web Search**: Real internet search using SearXNG backend through kolosal-server
- **Document Retrieval**: Semantic search through local knowledge base
- **Hybrid Knowledge Retrieval**: Combines web and document sources intelligently

### 🧠 **Advanced Analysis**
- **Source Credibility Analysis**: Evaluates reliability and authority of sources
- **Fact Verification**: Cross-references claims across multiple sources
- **Information Synthesis**: Combines findings from diverse sources into coherent analysis
- **Quality Assessment**: Evaluates research completeness and reliability

### 📊 **Flexible Research Modes**
- **Quick Research** (5-10 minutes): Fast overview with key findings
- **Comprehensive Research** (15-30 minutes): Detailed analysis with multiple sources
- **Academic Research** (20-40 minutes): Scholarly approach with citation management
- **Market Research** (10-20 minutes): Business-focused analysis with trend identification

### 🛠️ **Workflow-Based Processing**
- Pre-defined research workflows for different use cases
- Custom workflow creation for specialized research needs
- Sequential execution with error handling and retries
- Progress tracking and intermediate results

## Quick Start

### Prerequisites

1. **Kolosal Server Running**:
   ```bash
   # Start the kolosal-server (in a separate terminal)
   ./kolosal-server
   ```

2. **Build with Deep Research Demo**:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_DEEPRESEARCH_DEMO=ON
   cmake --build . --config Debug
   ```

### Running the Demo

```bash
# Windows
.\Debug\deep-research-demo.exe

# Linux/macOS  
./deep-research-demo

# With custom server URL
./deep-research-demo --server http://your-server:8080
```

## Usage Examples

### 1. Quick Research
Perfect for getting an overview of a topic quickly:
```
Research Question: "What are the latest developments in quantum computing?"
Mode: Quick Research
Duration: ~5-10 minutes
Output: Summary with key findings and 10-15 sources
```

### 2. Comprehensive Research
Detailed analysis with multiple perspectives:
```
Research Question: "Impact of artificial intelligence on employment markets"
Mode: Comprehensive Research  
Duration: ~15-30 minutes
Output: Full report with executive summary, detailed analysis, 20-30 sources
```

### 3. Academic Research
Scholarly approach with rigorous source evaluation:
```
Research Question: "Effectiveness of renewable energy policies in developing countries"
Mode: Academic Research
Duration: ~20-40 minutes
Output: Academic-style report with methodology, citations, and peer-reviewed sources
```

### 4. Market Research
Business-focused analysis:
```
Research Question: "Market opportunities for electric vehicles in Southeast Asia"
Mode: Market Research
Duration: ~10-20 minutes
Output: Business report with market trends, opportunities, and competitive analysis
```

## Configuration

### Server Configuration
Edit `deep_research_config.yaml`:
```yaml
server:
  url: "http://localhost:8080"
  timeout: 30
  max_retries: 3

research:
  methodology: "systematic"
  depth_level: "comprehensive"
  max_sources: 30
  max_web_results: 20
  relevance_threshold: 0.75
```

### Custom Research Parameters
You can customize research behavior:
- **Methodology**: systematic, exploratory, market_analysis
- **Depth Level**: shallow, moderate, comprehensive, exhaustive  
- **Source Types**: academic, news, documents, web
- **Output Format**: summary, comprehensive_report, academic_paper, business_report

## Architecture

### Core Components

1. **DeepResearchAgent**: Main research orchestrator
2. **EnhancedFunctionRegistry**: Manages web search and document retrieval functions
3. **SequentialWorkflowExecutor**: Handles multi-step research workflows
4. **ResearchFunctions**: Specialized functions for analysis and synthesis

### Research Workflow

```
1. Research Planning
   ├── Query decomposition
   ├── Methodology selection
   └── Source identification

2. Information Gathering
   ├── Web search (SearXNG)
   ├── Document retrieval (Vector DB)
   └── Knowledge synthesis

3. Analysis & Verification
   ├── Source credibility analysis
   ├── Fact verification
   └── Information synthesis

4. Report Generation
   ├── Structured report creation
   ├── Citation management
   └── Quality assessment
```

### Integration with Kolosal Server

The system leverages kolosal-server endpoints:
- `/search/internet` - Real web search via SearXNG
- `/retrieve` - Document retrieval from vector database  
- `/add_documents` - Add documents to knowledge base
- `/parse_document` - Parse various document formats
- `/generate_embedding` - Create embeddings for semantic search

## API Reference

### DeepResearchAgent Methods

```cpp
// Initialize and start agent
bool initialize();
bool start();

// Conduct research
ResearchResult conduct_research(
    const std::string& research_question,
    const ResearchConfig& config = ResearchConfig()
);

// Workflow-based research
ResearchResult conduct_research_with_workflow(
    const std::string& workflow_id,
    const std::string& research_question,
    const AgentData& additional_params = AgentData()
);

// Configuration
void set_research_config(const ResearchConfig& config);
void set_server_url(const std::string& url);
bool test_server_connection();
```

### ResearchConfig Structure

```cpp
struct ResearchConfig {
    std::string methodology;        // Research approach
    std::string depth_level;        // Analysis depth
    int max_sources;               // Maximum sources to consider
    int max_web_results;           // Web search result limit
    double relevance_threshold;     // Minimum relevance score
    bool include_academic;         // Include academic sources
    bool include_news;             // Include news sources  
    bool include_documents;        // Include local documents
    std::string output_format;     // Report format
    std::string language;          // Research language
};
```

### ResearchResult Structure

```cpp
struct ResearchResult {
    bool success;                           // Research success status
    std::string research_question;          // Original question
    std::string methodology_used;           // Applied methodology
    std::vector<std::string> sources_found; // Source list
    std::vector<std::string> key_findings;  // Main discoveries
    std::string comprehensive_analysis;     // Detailed analysis
    std::string executive_summary;          // Summary
    std::string full_report;               // Complete report
    double confidence_score;               // Quality score (0-1)
    std::string error_message;             // Error details if failed
};
```

## Troubleshooting

### Common Issues

1. **Server Connection Failed**
   ```
   Problem: ❌ Server connection failed - Using simulation mode
   Solution: Ensure kolosal-server is running on the specified URL
   Check: curl http://localhost:8080/health
   ```

2. **Research Takes Too Long**
   ```
   Problem: Research exceeds time limits
   Solution: Reduce max_sources or use quick_research mode
   Configure: Lower depth_level to "moderate" or "shallow"
   ```

3. **Poor Quality Results**
   ```
   Problem: Low confidence scores or incomplete analysis
   Solution: Increase relevance_threshold, enable more source types
   Check: Verify server has access to quality data sources
   ```

4. **Memory Issues**
   ```
   Problem: High memory usage during research
   Solution: Reduce max_sources and max_web_results
   Monitor: Check server resource usage
   ```

### Debug Mode

Enable detailed logging:
```bash
# Set environment variable
export KOLOSAL_LOG_LEVEL=DEBUG

# Run with verbose output
./deep-research-demo --server http://localhost:8080
```

### Performance Optimization

1. **Adjust Research Parameters**:
   - Reduce `max_sources` for faster execution
   - Lower `relevance_threshold` for broader results
   - Use "moderate" `depth_level` for balanced speed/quality

2. **Server Optimization**:
   - Ensure kolosal-server has adequate resources
   - Use SSD storage for document database
   - Configure appropriate connection pooling

3. **Network Optimization**:
   - Use local kolosal-server instance when possible
   - Configure appropriate timeouts
   - Enable connection keep-alive

## Examples and Use Cases

### Business Intelligence
```cpp
ResearchConfig business_config;
business_config.methodology = "market_analysis";
business_config.include_academic = false;
business_config.include_news = true;
business_config.output_format = "business_report";

auto result = agent->conduct_research(
    "Emerging trends in fintech industry 2025", 
    business_config
);
```

### Academic Research
```cpp
ResearchConfig academic_config;
academic_config.methodology = "systematic_review";
academic_config.depth_level = "exhaustive";
academic_config.relevance_threshold = 0.8;
academic_config.include_academic = true;
academic_config.output_format = "academic_paper";

auto result = agent->conduct_research(
    "Machine learning applications in healthcare diagnostics",
    academic_config
);
```

### Technology Analysis
```cpp
ResearchConfig tech_config;
tech_config.methodology = "systematic";
tech_config.depth_level = "comprehensive";
tech_config.max_sources = 40;
tech_config.include_news = true;
tech_config.include_documents = true;

auto result = agent->conduct_research(
    "Security implications of quantum computing on cryptography",
    tech_config
);
```

## Contributing

When contributing to the Deep Research Agent:

1. **Function Development**: Add new research functions to `research_functions.hpp/cpp`
2. **Workflow Creation**: Define workflows in YAML format  
3. **Integration**: Ensure compatibility with kolosal-server endpoints
4. **Testing**: Test with various research scenarios and configurations
5. **Documentation**: Update this README with new features

## License

Part of the Kolosal Agent System v2.0 - See main project license.
