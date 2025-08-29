# Retrieval Agent Documentation

The Retrieval Agent is a specialized AI agent designed for document management, semantic search, and knowledge retrieval. It provides advanced capabilities for storing, organizing, and retrieving information from a vector database.

## Features

### Core Retrieval Functions
- **Document Management**: Add, remove, and list documents in the vector database
- **Semantic Search**: Find relevant documents using natural language queries
- **AI-Powered Answers**: Combine document retrieval with LLM responses

### Enhanced Functions
- **Document Analysis**: Analyze document structure and extract metadata
- **Batch Processing**: Add multiple documents simultaneously
- **Knowledge Graph**: Extract entities and relationships from document collections
- **Search Suggestions**: Generate query suggestions for better search results
- **Document Organization**: Cluster documents by similarity

## Configuration

The RetrievalAgent is configured in `agent.yaml`:

```yaml
- name: "RetrievalAgent"
  capabilities: ["retrieval", "document_management", "semantic_search", "knowledge_base", "vector_search"]
  auto_start: true
  system_prompt: |
    You are an AI agent specialized in information retrieval and document management.
    Your primary functions include:
    - Managing and organizing documents in a vector database
    - Performing semantic search across document collections
    - Retrieving relevant information based on user queries
    - Maintaining knowledge bases and document repositories
    - Providing context-aware responses using retrieved information
```

## Available Functions

### 1. Document Management

#### `add_document`
Add a single document to the vector database.

```json
{
  "function": "add_document",
  "params": {
    "content": "Your document content here...",
    "title": "Document Title (optional)",
    "metadata": {
      "author": "Author Name",
      "date": "2025-01-01",
      "tags": ["tag1", "tag2"]
    },
    "chunk_size": 512
  }
}
```

#### `batch_add_documents`
Add multiple documents at once.

```json
{
  "function": "batch_add_documents",
  "params": {
    "documents": [
      {
        "content": "First document content...",
        "metadata": {"title": "Doc 1", "author": "User"}
      },
      {
        "content": "Second document content...",
        "metadata": {"title": "Doc 2", "category": "research"}
      }
    ]
  }
}
```

#### `list_documents`
List all documents with optional pagination and filtering.

```json
{
  "function": "list_documents",
  "params": {
    "offset": 0,
    "limit": 10,
    "filter": {"author": "specific_author"}
  }
}
```

#### `remove_document`
Remove a document by ID.

```json
{
  "function": "remove_document",
  "params": {
    "id": "document_id_here"
  }
}
```

### 2. Search Functions

#### `search_documents`
Perform semantic search across documents.

```json
{
  "function": "search_documents",
  "params": {
    "query": "machine learning algorithms",
    "limit": 5,
    "threshold": 0.7,
    "filter": {"category": "AI"}
  }
}
```

#### `retrieve_and_answer`
Combine document search with AI-generated answers.

```json
{
  "function": "retrieve_and_answer",
  "params": {
    "question": "What are the main types of neural networks?",
    "model": "qwen2.5-0.5b-instruct-q4_k_m",
    "max_docs": 3,
    "include_sources": true
  }
}
```

#### `internet_search`
Search the internet for additional information.

```json
{
  "function": "internet_search",
  "params": {
    "query": "latest AI research 2025",
    "results": 10,
    "language": "en"
  }
}
```

### 3. Analysis Functions

#### `analyze_document`
Analyze document structure and extract metadata.

```json
{
  "function": "analyze_document",
  "params": {
    "content": "Document content to analyze..."
  }
}
```

#### `get_search_suggestions`
Generate search query suggestions.

```json
{
  "function": "get_search_suggestions",
  "params": {
    "query": "machine learning"
  }
}
```

#### `organize_documents`
Cluster documents by similarity.

```json
{
  "function": "organize_documents",
  "params": {
    "threshold": 0.8,
    "max_clusters": 5
  }
}
```

#### `extract_knowledge_graph`
Extract knowledge graph from document collection.

```json
{
  "function": "extract_knowledge_graph",
  "params": {
    "documents": [
      {"content": "Document 1 content..."},
      {"content": "Document 2 content..."}
    ]
  }
}
```

## Usage Examples

### Basic Document Management Workflow

1. **Add documents to the system**:
```bash
curl -X POST http://127.0.0.1:8080/agents/RetrievalAgent/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "add_document",
    "params": {
      "content": "Artificial Intelligence (AI) is a branch of computer science that aims to create intelligent machines...",
      "title": "Introduction to AI",
      "metadata": {
        "author": "AI Expert",
        "category": "education",
        "difficulty": "beginner"
      }
    }
  }'
```

2. **Search for relevant documents**:
```bash
curl -X POST http://127.0.0.1:8080/agents/RetrievalAgent/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "search_documents",
    "params": {
      "query": "what is artificial intelligence",
      "limit": 3
    }
  }'
```

3. **Get AI-powered answers with sources**:
```bash
curl -X POST http://127.0.0.1:8080/agents/RetrievalAgent/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "retrieve_and_answer",
    "params": {
      "question": "Explain the key concepts of artificial intelligence",
      "model": "qwen2.5-0.5b-instruct-q4_k_m",
      "max_docs": 3,
      "include_sources": true
    }
  }'
```

### Advanced Features

#### Batch Document Processing
```bash
curl -X POST http://127.0.0.1:8080/agents/RetrievalAgent/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "batch_add_documents",
    "params": {
      "documents": [
        {
          "content": "Machine learning is a subset of AI...",
          "metadata": {"title": "ML Basics", "category": "machine-learning"}
        },
        {
          "content": "Deep learning uses neural networks...",
          "metadata": {"title": "Deep Learning", "category": "deep-learning"}
        },
        {
          "content": "Natural language processing deals with text...",
          "metadata": {"title": "NLP Introduction", "category": "nlp"}
        }
      ]
    }
  }'
```

#### Document Organization
```bash
curl -X POST http://127.0.0.1:8080/agents/RetrievalAgent/execute \
  -H "Content-Type: application/json" \
  -d '{
    "function": "organize_documents",
    "params": {
      "threshold": 0.7,
      "max_clusters": 5
    }
  }'
```

## Configuration Options

### Retrieval Manager Settings
The retrieval system can be configured through the agent configuration:

```yaml
retrieval:
  vector_db_type: "faiss"  # or "qdrant"
  db_host: "localhost"
  db_port: 6333
  collection_name: "documents"
  search_enabled: true
  searxng_url: "http://localhost:8888"
  max_results: 10
  timeout: 30
```

### Performance Tuning
- **Chunk Size**: Optimal chunk size is typically 256-1024 tokens
- **Overlap**: Use 10-20% overlap between chunks
- **Similarity Threshold**: Start with 0.7 and adjust based on results
- **Batch Size**: Process 10-100 documents per batch for optimal performance

## Integration with Other Agents

The RetrievalAgent can be used in conjunction with other agents:

1. **Assistant + RetrievalAgent**: Provide context-aware conversations
2. **Analyzer + RetrievalAgent**: Analyze retrieved documents
3. **Researcher + RetrievalAgent**: Combine internet and local document search

## Error Handling

The RetrievalAgent includes comprehensive error handling:

- **Missing Parameters**: Clear error messages for required parameters
- **System Unavailable**: Graceful fallback when retrieval system is offline
- **Model Errors**: Fallback responses when AI models are unavailable
- **Document Errors**: Detailed error reporting for batch operations

## Best Practices

1. **Document Structure**: Use clear titles and comprehensive metadata
2. **Query Design**: Use specific, descriptive queries for better results
3. **Chunking Strategy**: Adjust chunk size based on document type
4. **Regular Maintenance**: Periodically organize and clean document collections
5. **Performance Monitoring**: Monitor search response times and accuracy

## Troubleshooting

### Common Issues

1. **"Retrieval system not available"**
   - Ensure kolosal-server is built with retrieval support
   - Check if BUILD_WITH_RETRIEVAL is enabled

2. **Poor search results**
   - Adjust similarity threshold
   - Improve document metadata
   - Use more specific queries

3. **Model not available**
   - Verify model files are present
   - Check model configuration in config.yaml
   - Ensure kolosal-server is running

### Debug Information
Use the `status` function to check system health:

```bash
curl -X POST http://127.0.0.1:8080/agents/RetrievalAgent/execute \
  -H "Content-Type: application/json" \
  -d '{"function": "status", "params": {}}'
```
