# Web Search and Retrieval Function Integration Guide

This guide explains how to use the newly integrated web search and retrieval functions that connect with the kolosal-server endpoints.

## Overview

The Kolosal Agent System now includes enhanced functions that provide real web search and document retrieval capabilities by integrating with kolosal-server endpoints. This enables agents to:

- Perform real internet searches using SearXNG backend
- Retrieve documents from the vector database
- Combine web search with document retrieval for comprehensive knowledge gathering
- Parse and process various document formats
- Generate embeddings using server-side models

## Quick Start

### 1. Basic Setup

```cpp
#include "tools/enhanced_function_registry.hpp"
#include "execution/function_execution_manager.hpp"

// Create enhanced registry with server URL
auto enhanced_registry = std::make_shared<EnhancedFunctionRegistry>("http://localhost:8080");

// Create function manager
auto function_manager = std::make_shared<FunctionExecutionManager>();

// Register all functions (built-in + server-integrated)
enhanced_registry->register_all_functions(function_manager, true);
```

### 2. Testing Server Connection

```cpp
// Test if kolosal-server is available
bool connected = enhanced_registry->test_server_connection();

if (connected) {
    std::cout << "Server integration available!" << std::endl;
} else {
    std::cout << "Using simulation mode only." << std::endl;
}
```

## Available Functions

### Web Search Functions

#### 1. `internet_search` - Real Internet Search
Performs actual web searches using the kolosal-server's SearXNG integration.

**Parameters:**
- `query` (string, required): Search query
- `results` (int, optional): Number of results (default: 10)
- `engines` (string, optional): Comma-separated search engines
- `categories` (string, optional): Search categories (default: "general")
- `language` (string, optional): Language code (default: "en")
- `safe_search` (bool, optional): Enable safe search (default: true)
- `timeout` (int, optional): Request timeout in seconds

**Example:**
```cpp
AgentData search_params;
search_params.set("query", "artificial intelligence latest developments");
search_params.set("results", 5);
search_params.set("safe_search", true);

auto result = function_manager->execute__function("internet_search", search_params);

if (result.success) {
    auto titles = result.result_data.get_array_string("titles");
    auto urls = result.result_data.get_array_string("urls");
    auto snippets = result.result_data.get_array_string("snippets");
}
```

#### 2. `enhanced_web_search` - Advanced Web Search
Extends basic internet search with content filtering and summarization.

**Parameters:** Same as `internet_search`

**Additional Features:**
- Content filtering for quality results
- Optional summary extraction
- Duplicate removal
- Format normalization

### Document Retrieval Functions

#### 1. `server_document_retrieval` - Server-Based Document Search
Retrieves documents from the kolosal-server's vector database using semantic similarity.

**Parameters:**
- `query` (string, required): Search query
- `limit` (int, optional): Maximum documents to retrieve (default: 10)
- `collection` (string, optional): Collection name (default: "documents")
- `threshold` (double, optional): Similarity threshold (default: 0.7)

**Example:**
```cpp
AgentData retrieval_params;
retrieval_params.set("query", "machine learning algorithms");
retrieval_params.set("limit", 5);
retrieval_params.set("threshold", 0.8);

auto result = function_manager->execute__function("server_document_retrieval", retrieval_params);

if (result.success) {
    auto contents = result.result_data.get_array_string("contents");
    auto sources = result.result_data.get_array_string("sources");
    auto document_ids = result.result_data.get_array_string("document_ids");
}
```

#### 2. `server_add_document` - Add Documents to Knowledge Base
Adds new documents to the server's vector database.

#### 3. `server_parse_document` - Parse Various Document Formats
Parses PDF, DOCX, HTML, and other document formats using server endpoints.

#### 4. `server_generate_embedding` - Generate Text Embeddings
Creates embeddings for text content using server-side models.

### Hybrid Functions

#### `knowledge_retrieval` - Comprehensive Knowledge Gathering
Combines internet search and document retrieval for comprehensive knowledge gathering.

**Parameters:**
- `query` (string, required): Search/retrieval query
- `max_results` (int, optional): Total maximum results (default: 15)
- `web_only` (bool, optional): Use web search only (default: false)
- `local_only` (bool, optional): Use document retrieval only (default: false)

**Example:**
```cpp
AgentData knowledge_params;
knowledge_params.set("query", "quantum computing applications");
knowledge_params.set("max_results", 12);

auto result = function_manager->execute__function("knowledge_retrieval", knowledge_params);

if (result.success) {
    int total_results = result.result_data.get_int("total_results", 0);
    int web_results = result.result_data.get_int("web_results_found", 0);
    int local_docs = result.result_data.get_int("local_documents_found", 0);
    
    auto contents = result.result_data.get_array_string("contents");
    auto sources = result.result_data.get_array_string("sources");
    auto types = result.result_data.get_array_string("types"); // "web" or "document"
}
```

## Role-Based Function Registration

### Automatic Role-Based Setup

```cpp
// Get recommended functions for specific roles
auto research_functions = FunctionRegistryUtils::get_recommended_functions_for_role("researcher");
auto assistant_functions = FunctionRegistryUtils::get_recommended_functions_for_role("assistant");

// Register functions for specific use cases
FunctionRegistryUtils::register_web_search_functions(function_manager, server_url, true);
FunctionRegistryUtils::register_document_functions(function_manager, server_url, "documents");
FunctionRegistryUtils::register_knowledge_functions(function_manager, server_url);
```

### Recommended Functions by Role

#### Researcher Role
- `internet_search`, `enhanced_web_search`
- `server_document_retrieval`, `knowledge_retrieval`
- `parse_pdf`, `parse_docx`
- `context_retrieval`

#### Assistant Role
- `internet_search`, `knowledge_retrieval`
- `text_processing`, `data_transform`
- `inference`, `add_document`

#### Developer Role
- `code_generation`, `internet_search`
- `server_document_retrieval`, `text_processing`
- `data_analysis`

#### Specialist Role
- `enhanced_web_search`, `server_document_retrieval`
- `knowledge_retrieval`, `server_generate_embedding`
- `inference`, `data_analysis`

## Creating Enhanced Agents

### Method 1: Using Enhanced Function Registry

```cpp
#include "tools/enhanced_function_registry.hpp"

// Create agent with enhanced capabilities
auto agent = std::make_unique<AgentCore>("ResearchAgent", "research", AgentRole::RESEARCHER);

// Create enhanced registry
auto enhanced_registry = std::make_shared<EnhancedFunctionRegistry>("http://localhost:8080");

// Register functions with the agent's function manager
enhanced_registry->register_all_functions(agent->get_function_manager(), true);
```

### Method 2: Selective Registration

```cpp
auto function_manager = agent->get_function_manager();

// Register only web search functions
FunctionRegistryUtils::register_web_search_functions(function_manager, "http://localhost:8080");

// Register only document functions
FunctionRegistryUtils::register_document_functions(function_manager, "http://localhost:8080");
```

## Configuration

### Server Configuration

Ensure your kolosal-server is configured with search capabilities:

```yaml
# config.yaml
search:
  enabled: true
  searxng_url: "http://localhost:4000"
  timeout: 30
  max_results: 20
  enable_safe_search: true

retrieval:
  enabled: true
  vector_db_url: "http://localhost:6333"
  collection: "documents"
```

### Agent Configuration

```cpp
// Configure server URL
enhanced_registry->set_server_url("http://your-server:8080");

// Enable/disable server functions
enhanced_registry->set_server_functions_enabled(true);

// Test connection before use
if (!enhanced_registry->test_server_connection()) {
    std::cout << "Server unavailable, using simulation mode" << std::endl;
}
```

## Error Handling

All functions return `FunctionResult` objects with error information:

```cpp
auto result = function_manager->execute__function("internet_search", params);

if (!result.success) {
    std::cout << "Error: " << result.error_message << std::endl;
    // Handle error - maybe fall back to simulation
} else {
    // Process successful results
    std::cout << "Execution time: " << result.execution_time_ms << "ms" << std::endl;
}
```

## Performance Considerations

- **Connection Pooling**: HTTP client connections are reused when possible
- **Timeouts**: All functions have configurable timeouts
- **Caching**: Results can be cached at the application level
- **Concurrency**: Functions are thread-safe and can be called concurrently

## Troubleshooting

### Common Issues

1. **Server Connection Failed**
   - Check if kolosal-server is running
   - Verify the server URL and port
   - Check firewall settings

2. **Search Returns No Results**
   - Verify SearXNG is configured and running
   - Check search query formatting
   - Try different search engines or categories

3. **Document Retrieval Empty**
   - Ensure documents are indexed in the vector database
   - Check collection name
   - Adjust similarity threshold

### Debug Mode

Enable detailed logging:

```cpp
ServerLogger::set_level(LogLevel::DEBUG);
```

## Examples

See the `examples/web_search_demo.cpp` file for complete working examples of:
- Basic web search integration
- Document retrieval usage
- Hybrid knowledge gathering
- Role-based function registration
- Error handling patterns

## Migration from Simulation Functions

If you're currently using the built-in simulation functions (`WebSearchFunction`), you can easily upgrade:

```cpp
// Old way (simulation)
function_manager->register_function(std::make_unique<WebSearchFunction>());

// New way (real search)
function_manager->register_function(std::make_unique<InternetSearchFunction>("http://localhost:8080"));

// Or use the registry for automatic handling
enhanced_registry->register_all_functions(function_manager, true);
```

The new functions maintain backward compatibility with the same parameter names and result structures.
