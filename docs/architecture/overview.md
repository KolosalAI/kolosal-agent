# Kolosal Agent System Architecture

Comprehensive overview of the Kolosal Agent System v2.0 architecture - a unified multi-agent AI platform that seamlessly integrates advanced language model inference with sophisticated agent orchestration capabilities.

Generated on 2025-08-12

## 📋 Table of Contents

- [System Overview](#system-overview)
- [Core Architecture](#core-architecture)
- [System Components](#system-components)
- [Architecture Diagrams](#architecture-diagrams)
- [Component Details](#component-details)
- [Integration Patterns](#integration-patterns)
- [Deployment Architecture](#deployment-architecture)
- [Namespaces](#namespaces)

## System Overview

The Kolosal Agent System v2.0 represents a next-generation unified multi-agent AI platform that brings together:

- **🔄 Unified Server Architecture**: Single binary managing both LLM inference and multi-agent systems
- **🤖 Advanced Agent Orchestration**: Sophisticated workflow execution and inter-agent communication
- **🌐 Comprehensive REST API**: Full-featured management interface with OpenAPI compatibility
- **📊 Service Layer Architecture**: High-level abstractions for complex operations
- **⚡ Async-First Design**: Non-blocking operations with Future-based patterns
- **🔧 Dynamic Configuration**: Hot-reloading and runtime configuration updates
- **💓 Health Monitoring**: Comprehensive monitoring with auto-recovery mechanisms

## Core Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    🌐 Client Layer                          │
├─────────────────────────────────────────────────────────────┤
│              🔄 Unified Kolosal Server v2.0                 │
├─────────────────┬─────────────────┬─────────────────────────┤
│  ⚡ LLM Server  │  🌐 Agent API   │     📊 Service Layer    │
│  (Port 8080)    │  (Port 8081)    │   (Agent Operations)    │
├─────────────────┼─────────────────┼─────────────────────────┤
│           🤖 Multi-Agent System Core                        │
├─────────────────┬─────────────────┬─────────────────────────┤
│ 🎭 Orchestrator │  ⚙️ Workflows   │   💾 Memory System      │
├─────────────────┼─────────────────┼─────────────────────────┤
│    🛠️ Function Registry & Tool Management                   │
├─────────────────────────────────────────────────────────────┤
│  📄 External Services (Documents, Web, Databases, etc.)    │
└─────────────────────────────────────────────────────────────┘
```

For detailed architectural diagrams, see [System Architecture Diagrams](system-diagrams.md).

## System Components

## Architecture Diagrams

🔗 **[Complete System Architecture Diagrams](system-diagrams.md)** - Comprehensive visual documentation including:

- **High-Level System Architecture** - Overall system structure and component relationships
- **Agent Workflow Execution Flow** - Function execution and processing flow
- **Multi-Agent Workflow Orchestration** - Complex workflow coordination patterns
- **Memory System Architecture** - Memory types and storage backends
- **Function Registry and Tool System** - Function management and tool integration
- **REST API Architecture** - API endpoints and request routing
- **Data Flow Architecture** - Sequence diagrams showing system interactions
- **Configuration and Deployment Architecture** - Deployment patterns and configuration management

## Component Details

### 🔄 Unified Server (`UnifiedKolosalServer`)

The central orchestrator that manages the entire system lifecycle:

**Core Responsibilities:**
- **Process Management**: Coordinates LLM server and agent system processes
- **Configuration Management**: Handles dynamic configuration loading and hot-reloading
- **Health Monitoring**: Continuous monitoring with automatic recovery mechanisms
- **Service Integration**: Bridges LLM capabilities with agent operations
- **API Gateway**: Unified entry point for all system operations

**Key Features:**
- Single binary deployment with integrated process management
- Automatic health monitoring and recovery
- Hot-reloadable configuration system
- Comprehensive logging and metrics collection
- Production-ready with security features

### 📊 Service Layer (`AgentService`)

High-level service abstractions providing business logic and advanced operations:

**Service Categories:**
- **Agent Lifecycle Management**: Creation, configuration, startup, and shutdown
- **Bulk Operations**: Batch processing and multi-agent operations
- **Performance Analytics**: System optimization and performance insights
- **Health Monitoring**: Service-level health checks and diagnostics
- **Event Management**: Event-driven notifications and callbacks

**Advanced Features:**
- Async-first operations with Future-based patterns
- Comprehensive error handling and recovery
- Performance metrics and optimization suggestions
- Auto-scaling and resource management
- Integration with external systems

### 🌐 REST API Layer (`AgentManagementRoute`)

Comprehensive RESTful API providing full system management capabilities:

**API Categories:**

#### Agent Management Endpoints
- `GET /v1/agents` - List all agents with status and statistics
- `POST /v1/agents` - Create new agents from configuration
- `GET /v1/agents/{id}` - Retrieve specific agent details and status
- `PUT /v1/agents/{id}/start` - Start agent with optional parameters
- `PUT /v1/agents/{id}/stop` - Gracefully stop agent
- `DELETE /v1/agents/{id}` - Remove agent and cleanup resources
- `POST /v1/agents/{id}/execute` - Execute functions with parameters

#### System Management Endpoints
- `GET /v1/system/status` - System-wide status and health metrics
- `POST /v1/system/reload` - Hot-reload configuration
- `GET /v1/health` - Health check endpoint for load balancers
- `GET /v1/metrics` - Prometheus-compatible metrics

#### Workflow Management Endpoints
- `POST /v1/workflows/execute` - Execute complex multi-agent workflows
- `GET /v1/workflows/{id}/status` - Monitor workflow execution status
- `PUT /v1/workflows/{id}/cancel` - Cancel running workflows

**API Features:**
- OpenAPI 3.0 specification compliance
- Comprehensive input validation and error handling
- Rate limiting and authentication support
- CORS handling for web applications
- Detailed response formatting with metadata

### 🤖 Multi-Agent Core

The heart of the agent system providing sophisticated agent management:

#### YAMLConfigurableAgentManager
**Primary Functions:**
- Dynamic agent creation from YAML/JSON configurations
- Agent lifecycle management (start, stop, restart, remove)
- Resource allocation and monitoring
- Inter-agent communication facilitation
- Configuration hot-reloading

**Agent Types:**
- **Coordinator Agents** (`COORDINATOR`): System planning and resource management
- **Analyst Agents** (`ANALYST`): Data processing and analysis
- **Research Agents** (`RESEARCHER`): Information gathering and synthesis
- **Specialist Agents** (`SPECIALIST`): Domain-specific expertise and processing

#### Agent Orchestrator
**Orchestration Capabilities:**
- **Workflow Management**: Sequential, parallel, and conditional workflows
- **Resource Coordination**: Agent selection and load balancing
- **Result Aggregation**: Combining outputs from multiple agents
- **Error Handling**: Failure detection and recovery strategies

**Collaboration Patterns:**
- **Sequential Processing**: Step-by-step agent execution
- **Parallel Processing**: Concurrent agent execution
- **Consensus Building**: Multi-agent agreement mechanisms
- **Negotiation**: Inter-agent negotiation protocols

### ⚙️ Workflow Engine

Advanced workflow execution system supporting complex multi-agent scenarios:

**Workflow Types:**
- **Sequential Workflows**: Linear execution with dependencies
- **Parallel Workflows**: Concurrent execution with synchronization
- **Conditional Workflows**: Decision-based routing and execution
- **Hybrid Workflows**: Combined patterns for complex scenarios

**Execution Features:**
- Dependency resolution and execution ordering
- Error handling and recovery mechanisms
- Workflow pause, resume, and cancellation
- Progress monitoring and status reporting
- Persistent workflow state management

### 💾 Memory System

Sophisticated memory architecture supporting different memory types:

**Memory Types:**
- **Episodic Memory**: Experience and interaction history
- **Semantic Memory**: Knowledge base and learned concepts
- **Working Memory**: Current context and active information

**Storage Backends:**
- **Vector Databases**: Semantic search and similarity matching
- **Structured Databases**: Relational data and metadata
- **File Systems**: Document storage and caching
- **In-Memory Caches**: High-performance temporary storage

### 🛠️ Function Registry & Tool Management

Extensible function and tool system:

**Built-in Functions:**
- **Planning Functions**: Task decomposition and orchestration
- **Analysis Functions**: Data processing and insights
- **Communication Functions**: Inter-agent messaging
- **Utility Functions**: Common operations and helpers

**Tool Integration:**
- **Document Processing**: PDF, DOCX, HTML parsers
- **Web Tools**: Search engines, content fetchers
- **Analysis Tools**: Statistical analysis, machine learning
- **Storage Tools**: Database connectors, file managers

### 🔗 External Service Integration

Comprehensive integration with external services:

**Document Services:**
- **RAG Integration**: Retrieval-Augmented Generation
- **Document Processing**: Multi-format document parsing
- **Content Extraction**: Text and metadata extraction
- **Vector Database**: Semantic search capabilities

**Web Services:**
- **Search APIs**: Web search and content discovery
- **HTTP Clients**: RESTful API integration
- **Content Processing**: Web scraping and analysis

**Data Services:**
- **Database Connectivity**: SQL and NoSQL databases
- **File System Operations**: Local and remote file access
- **Cloud Storage**: Integration with cloud providers

## Integration Patterns

### 1. Service-Oriented Architecture (SOA)
- **Clear Separation**: Distinct layers with well-defined interfaces
- **Loose Coupling**: Minimal dependencies between components
- **High Cohesion**: Related functionality grouped together

### 2. Event-Driven Architecture
- **Async Communication**: Non-blocking inter-component communication
- **Event Sourcing**: State changes captured as events
- **Message Queuing**: Reliable message delivery between agents

### 3. Plugin Architecture
- **Dynamic Loading**: Runtime loading of functions and tools
- **Interface-Based**: Common interfaces for extensibility
- **Configuration-Driven**: Plugin configuration through YAML/JSON

### 4. Microservices Pattern
- **Service Decomposition**: Functionality broken into focused services
- **Independent Deployment**: Services can be deployed independently
- **Technology Diversity**: Different technologies for different services

## Deployment Architecture

### Container Deployment
```yaml
# Docker deployment example
services:
  kolosal-agent:
    image: kolosal/agent-system:v2.0
    ports:
      - "8080:8080"  # LLM Server
      - "8081:8081"  # Agent API
    volumes:
      - ./config:/app/config
      - ./models:/app/models
    environment:
      - KOLOSAL_CONFIG=/app/config/production.yaml
```

### Kubernetes Deployment
```yaml
# Kubernetes deployment example
apiVersion: apps/v1
kind: Deployment
metadata:
  name: kolosal-agent-system
spec:
  replicas: 3
  selector:
    matchLabels:
      app: kolosal-agent
  template:
    spec:
      containers:
      - name: kolosal-agent
        image: kolosal/agent-system:v2.0
        ports:
        - containerPort: 8080
        - containerPort: 8081
```

### Configuration Management

**Configuration Sources (Priority Order):**
1. Command-line arguments
2. Environment variables
3. Configuration files (YAML/JSON)
4. Default values

**Hot-Reloading Support:**
- Runtime configuration updates
- Agent configuration changes
- Function registry updates
- Service endpoint modifications

## Namespaces

### PoDoFo

Used in 4 files:
- `complete_backup\kolosal-server\include\kolosal\retrieval\parse_pdf.hpp`
- `kolosal-server\include\kolosal\retrieval\parse_pdf.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\parse_pdf.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\parse_pdf.hpp`

### agents

Used in 12 files:
- `complete_backup\include\agent\agent_data.hpp`
- `complete_backup\include\document_service_manager.hpp`
- `complete_backup\src\document_service_manager.cpp`
- `include\agent\agent_data.hpp`
- `include\document_service_manager.hpp`
- `naming_backup\complete_backup\include\agent\agent_data.hpp`
- `naming_backup\complete_backup\include\document_service_manager.hpp`
- `naming_backup\complete_backup\src\document_service_manager.cpp`
- `naming_backup\include\agent\agent_data.hpp`
- `naming_backup\include\document_service_manager.hpp`
- `naming_backup\src\document_service_manager.cpp`
- `src\document_service_manager.cpp`

### auth

Used in 32 files:
- `complete_backup\kolosal-server\include\kolosal\auth.hpp`
- `complete_backup\kolosal-server\include\kolosal\auth\auth_middleware.hpp`
- `complete_backup\kolosal-server\include\kolosal\auth\cors_handler.hpp`
- `complete_backup\kolosal-server\include\kolosal\auth\rate_limiter.hpp`
- `complete_backup\kolosal-server\include\kolosal\server_api.hpp`
- `complete_backup\kolosal-server\src\auth\auth_middleware.cpp`
- `complete_backup\kolosal-server\src\auth\cors_handler.cpp`
- `complete_backup\kolosal-server\src\auth\rate_limiter.cpp`
- `kolosal-server\include\kolosal\auth.hpp`
- `kolosal-server\include\kolosal\auth\auth_middleware.hpp`
- `kolosal-server\include\kolosal\auth\cors_handler.hpp`
- `kolosal-server\include\kolosal\auth\rate_limiter.hpp`
- `kolosal-server\include\kolosal\server_api.hpp`
- `kolosal-server\src\auth\auth_middleware.cpp`
- `kolosal-server\src\auth\cors_handler.cpp`
- `kolosal-server\src\auth\rate_limiter.cpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\auth.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\auth\auth_middleware.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\auth\cors_handler.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\auth\rate_limiter.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\server_api.hpp`
- `naming_backup\complete_backup\kolosal-server\src\auth\auth_middleware.cpp`
- `naming_backup\complete_backup\kolosal-server\src\auth\cors_handler.cpp`
- `naming_backup\complete_backup\kolosal-server\src\auth\rate_limiter.cpp`
- `naming_backup\kolosal-server\include\kolosal\auth.hpp`
- `naming_backup\kolosal-server\include\kolosal\auth\auth_middleware.hpp`
- `naming_backup\kolosal-server\include\kolosal\auth\cors_handler.hpp`
- `naming_backup\kolosal-server\include\kolosal\auth\rate_limiter.hpp`
- `naming_backup\kolosal-server\include\kolosal\server_api.hpp`
- `naming_backup\kolosal-server\src\auth\auth_middleware.cpp`
- `naming_backup\kolosal-server\src\auth\cors_handler.cpp`
- `naming_backup\kolosal-server\src\auth\rate_limiter.cpp`

### grammars

Used in 4 files:
- `complete_backup\kolosal-server\inference\examples\grammar_example.cpp`
- `kolosal-server\inference\examples\grammar_example.cpp`
- `naming_backup\complete_backup\kolosal-server\inference\examples\grammar_example.cpp`
- `naming_backup\kolosal-server\inference\examples\grammar_example.cpp`

### kolosal

Used in 308 files:
- `complete_backup\include\agent\agent_data.hpp`
- `complete_backup\include\document_service_manager.hpp`
- `complete_backup\include\logger_system.hpp`
- `complete_backup\kolosal-server\include\kolosal\auth.hpp`
- `complete_backup\kolosal-server\include\kolosal\auth\auth_middleware.hpp`
- `complete_backup\kolosal-server\include\kolosal\auth\cors_handler.hpp`
- `complete_backup\kolosal-server\include\kolosal\auth\rate_limiter.hpp`
- `complete_backup\kolosal-server\include\kolosal\download_manager.hpp`
- `complete_backup\kolosal-server\include\kolosal\download_utils.hpp`
- `complete_backup\kolosal-server\include\kolosal\gpu_detection.hpp`
- `complete_backup\kolosal-server\include\kolosal\inference_loader.hpp`
- `complete_backup\kolosal-server\include\kolosal\models\chunking_request_model.hpp`
- `complete_backup\kolosal-server\include\kolosal\models\chunking_response_model.hpp`
- `complete_backup\kolosal-server\include\kolosal\models\embedding_request_model.hpp`
- `complete_backup\kolosal-server\include\kolosal\models\embedding_response_model.hpp`
- `complete_backup\kolosal-server\include\kolosal\node_manager.h`
- `complete_backup\kolosal-server\include\kolosal\qdrant_client.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\add_document_types.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\chunking_types.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\document_list_types.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\document_service.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\remove_document_types.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\retrieve_types.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\config\auth_config_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\downloads_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\engines_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\health_status_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\llm\completion_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\llm\oai_completions_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\models_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\retrieval\chunking_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\retrieval\documents_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\retrieval\embedding_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\retrieval\internet_search_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\retrieval\parse_document_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\routes\server_logs_route.hpp`
- `complete_backup\kolosal-server\include\kolosal\server.hpp`
- `complete_backup\kolosal-server\include\kolosal\server_api.hpp`
- `complete_backup\kolosal-server\include\kolosal\server_config.hpp`
- `complete_backup\kolosal-server\src\auth\auth_middleware.cpp`
- `complete_backup\kolosal-server\src\auth\cors_handler.cpp`
- `complete_backup\kolosal-server\src\auth\rate_limiter.cpp`
- `complete_backup\kolosal-server\src\download_manager.cpp`
- `complete_backup\kolosal-server\src\download_utils.cpp`
- `complete_backup\kolosal-server\src\gpu_detection.cpp`
- `complete_backup\kolosal-server\src\inference_loader.cpp`
- `complete_backup\kolosal-server\src\models\chunking_request_model.cpp`
- `complete_backup\kolosal-server\src\models\chunking_response_model.cpp`
- `complete_backup\kolosal-server\src\models\embedding_request_model.cpp`
- `complete_backup\kolosal-server\src\models\embedding_response_model.cpp`
- `complete_backup\kolosal-server\src\node_manager.cpp`
- `complete_backup\kolosal-server\src\qdrant_client.cpp`
- `complete_backup\kolosal-server\src\retrieval\add_document_types.cpp`
- `complete_backup\kolosal-server\src\retrieval\chunking_types.cpp`
- `complete_backup\kolosal-server\src\retrieval\document_list_types.cpp`
- `complete_backup\kolosal-server\src\retrieval\document_service.cpp`
- `complete_backup\kolosal-server\src\retrieval\remove_document_types.cpp`
- `complete_backup\kolosal-server\src\retrieval\retrieve_types.cpp`
- `complete_backup\kolosal-server\src\routes\config\auth_config_route.cpp`
- `complete_backup\kolosal-server\src\routes\downloads_route.cpp`
- `complete_backup\kolosal-server\src\routes\engines_route.cpp`
- `complete_backup\kolosal-server\src\routes\health_status_route.cpp`
- `complete_backup\kolosal-server\src\routes\llm\completion_route.cpp`
- `complete_backup\kolosal-server\src\routes\llm\oai_completions_route.cpp`
- `complete_backup\kolosal-server\src\routes\models_route.cpp`
- `complete_backup\kolosal-server\src\routes\retrieval\chunking_route.cpp`
- `complete_backup\kolosal-server\src\routes\retrieval\documents_route.cpp`
- `complete_backup\kolosal-server\src\routes\retrieval\embedding_route.cpp`
- `complete_backup\kolosal-server\src\routes\retrieval\internet_search_route.cpp`
- `complete_backup\kolosal-server\src\routes\retrieval\parse_document_route.cpp`
- `complete_backup\kolosal-server\src\routes\server_logs_route.cpp`
- `complete_backup\kolosal-server\src\server.cpp`
- `complete_backup\kolosal-server\src\server_api.cpp`
- `complete_backup\kolosal-server\src\server_config.cpp`
- `complete_backup\src\document_service_manager.cpp`
- `complete_backup\src\logger_system.cpp`
- `include\agent\agent_data.hpp`
- `include\document_service_manager.hpp`
- `include\logger_system.hpp`
- `kolosal-server\include\kolosal\auth.hpp`
- `kolosal-server\include\kolosal\auth\auth_middleware.hpp`
- `kolosal-server\include\kolosal\auth\cors_handler.hpp`
- `kolosal-server\include\kolosal\auth\rate_limiter.hpp`
- `kolosal-server\include\kolosal\download_manager.hpp`
- `kolosal-server\include\kolosal\download_utils.hpp`
- `kolosal-server\include\kolosal\gpu_detection.hpp`
- `kolosal-server\include\kolosal\inference_loader.hpp`
- `kolosal-server\include\kolosal\models\chunking_request_model.hpp`
- `kolosal-server\include\kolosal\models\chunking_response_model.hpp`
- `kolosal-server\include\kolosal\models\embedding_request_model.hpp`
- `kolosal-server\include\kolosal\models\embedding_response_model.hpp`
- `kolosal-server\include\kolosal\node_manager.h`
- `kolosal-server\include\kolosal\qdrant_client.hpp`
- `kolosal-server\include\kolosal\retrieval\add_document_types.hpp`
- `kolosal-server\include\kolosal\retrieval\chunking_types.hpp`
- `kolosal-server\include\kolosal\retrieval\document_list_types.hpp`
- `kolosal-server\include\kolosal\retrieval\document_service.hpp`
- `kolosal-server\include\kolosal\retrieval\remove_document_types.hpp`
- `kolosal-server\include\kolosal\retrieval\retrieve_types.hpp`
- `kolosal-server\include\kolosal\routes\config\auth_config_route.hpp`
- `kolosal-server\include\kolosal\routes\downloads_route.hpp`
- `kolosal-server\include\kolosal\routes\engines_route.hpp`
- `kolosal-server\include\kolosal\routes\health_status_route.hpp`
- `kolosal-server\include\kolosal\routes\llm\completion_route.hpp`
- `kolosal-server\include\kolosal\routes\llm\oai_completions_route.hpp`
- `kolosal-server\include\kolosal\routes\models_route.hpp`
- `kolosal-server\include\kolosal\routes\retrieval\chunking_route.hpp`
- `kolosal-server\include\kolosal\routes\retrieval\documents_route.hpp`
- `kolosal-server\include\kolosal\routes\retrieval\embedding_route.hpp`
- `kolosal-server\include\kolosal\routes\retrieval\internet_search_route.hpp`
- `kolosal-server\include\kolosal\routes\retrieval\parse_document_route.hpp`
- `kolosal-server\include\kolosal\routes\server_logs_route.hpp`
- `kolosal-server\include\kolosal\server.hpp`
- `kolosal-server\include\kolosal\server_api.hpp`
- `kolosal-server\include\kolosal\server_config.hpp`
- `kolosal-server\src\auth\auth_middleware.cpp`
- `kolosal-server\src\auth\cors_handler.cpp`
- `kolosal-server\src\auth\rate_limiter.cpp`
- `kolosal-server\src\download_manager.cpp`
- `kolosal-server\src\download_utils.cpp`
- `kolosal-server\src\gpu_detection.cpp`
- `kolosal-server\src\inference_loader.cpp`
- `kolosal-server\src\models\chunking_request_model.cpp`
- `kolosal-server\src\models\chunking_response_model.cpp`
- `kolosal-server\src\models\embedding_request_model.cpp`
- `kolosal-server\src\models\embedding_response_model.cpp`
- `kolosal-server\src\node_manager.cpp`
- `kolosal-server\src\qdrant_client.cpp`
- `kolosal-server\src\retrieval\add_document_types.cpp`
- `kolosal-server\src\retrieval\chunking_types.cpp`
- `kolosal-server\src\retrieval\document_list_types.cpp`
- `kolosal-server\src\retrieval\document_service.cpp`
- `kolosal-server\src\retrieval\remove_document_types.cpp`
- `kolosal-server\src\retrieval\retrieve_types.cpp`
- `kolosal-server\src\routes\config\auth_config_route.cpp`
- `kolosal-server\src\routes\downloads_route.cpp`
- `kolosal-server\src\routes\engines_route.cpp`
- `kolosal-server\src\routes\health_status_route.cpp`
- `kolosal-server\src\routes\llm\completion_route.cpp`
- `kolosal-server\src\routes\llm\oai_completions_route.cpp`
- `kolosal-server\src\routes\models_route.cpp`
- `kolosal-server\src\routes\retrieval\chunking_route.cpp`
- `kolosal-server\src\routes\retrieval\documents_route.cpp`
- `kolosal-server\src\routes\retrieval\embedding_route.cpp`
- `kolosal-server\src\routes\retrieval\internet_search_route.cpp`
- `kolosal-server\src\routes\retrieval\parse_document_route.cpp`
- `kolosal-server\src\routes\server_logs_route.cpp`
- `kolosal-server\src\server.cpp`
- `kolosal-server\src\server_api.cpp`
- `kolosal-server\src\server_config.cpp`
- `naming_backup\complete_backup\include\agent\agent_data.hpp`
- `naming_backup\complete_backup\include\document_service_manager.hpp`
- `naming_backup\complete_backup\include\logger_system.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\auth.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\auth\auth_middleware.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\auth\cors_handler.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\auth\rate_limiter.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\download_manager.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\download_utils.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\gpu_detection.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\inference_loader.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\models\chunking_request_model.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\models\chunking_response_model.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\models\embedding_request_model.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\models\embedding_response_model.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\node_manager.h`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\qdrant_client.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\add_document_types.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\chunking_types.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\document_list_types.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\document_service.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\remove_document_types.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\retrieve_types.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\config\auth_config_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\downloads_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\engines_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\health_status_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\llm\completion_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\llm\oai_completions_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\models_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\retrieval\chunking_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\retrieval\documents_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\retrieval\embedding_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\retrieval\internet_search_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\retrieval\parse_document_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\routes\server_logs_route.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\server.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\server_api.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\server_config.hpp`
- `naming_backup\complete_backup\kolosal-server\src\auth\auth_middleware.cpp`
- `naming_backup\complete_backup\kolosal-server\src\auth\cors_handler.cpp`
- `naming_backup\complete_backup\kolosal-server\src\auth\rate_limiter.cpp`
- `naming_backup\complete_backup\kolosal-server\src\download_manager.cpp`
- `naming_backup\complete_backup\kolosal-server\src\download_utils.cpp`
- `naming_backup\complete_backup\kolosal-server\src\gpu_detection.cpp`
- `naming_backup\complete_backup\kolosal-server\src\inference_loader.cpp`
- `naming_backup\complete_backup\kolosal-server\src\models\chunking_request_model.cpp`
- `naming_backup\complete_backup\kolosal-server\src\models\chunking_response_model.cpp`
- `naming_backup\complete_backup\kolosal-server\src\models\embedding_request_model.cpp`
- `naming_backup\complete_backup\kolosal-server\src\models\embedding_response_model.cpp`
- `naming_backup\complete_backup\kolosal-server\src\node_manager.cpp`
- `naming_backup\complete_backup\kolosal-server\src\qdrant_client.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\add_document_types.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\chunking_types.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\document_list_types.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\document_service.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\remove_document_types.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\retrieve_types.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\config\auth_config_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\downloads_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\engines_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\health_status_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\llm\completion_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\llm\oai_completions_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\models_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\retrieval\chunking_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\retrieval\documents_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\retrieval\embedding_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\retrieval\internet_search_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\retrieval\parse_document_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\routes\server_logs_route.cpp`
- `naming_backup\complete_backup\kolosal-server\src\server.cpp`
- `naming_backup\complete_backup\kolosal-server\src\server_api.cpp`
- `naming_backup\complete_backup\kolosal-server\src\server_config.cpp`
- `naming_backup\complete_backup\src\document_service_manager.cpp`
- `naming_backup\complete_backup\src\logger_system.cpp`
- `naming_backup\include\agent\agent_data.hpp`
- `naming_backup\include\document_service_manager.hpp`
- `naming_backup\include\logger_system.hpp`
- `naming_backup\kolosal-server\include\kolosal\auth.hpp`
- `naming_backup\kolosal-server\include\kolosal\auth\auth_middleware.hpp`
- `naming_backup\kolosal-server\include\kolosal\auth\cors_handler.hpp`
- `naming_backup\kolosal-server\include\kolosal\auth\rate_limiter.hpp`
- `naming_backup\kolosal-server\include\kolosal\download_manager.hpp`
- `naming_backup\kolosal-server\include\kolosal\download_utils.hpp`
- `naming_backup\kolosal-server\include\kolosal\gpu_detection.hpp`
- `naming_backup\kolosal-server\include\kolosal\inference_loader.hpp`
- `naming_backup\kolosal-server\include\kolosal\models\chunking_request_model.hpp`
- `naming_backup\kolosal-server\include\kolosal\models\chunking_response_model.hpp`
- `naming_backup\kolosal-server\include\kolosal\models\embedding_request_model.hpp`
- `naming_backup\kolosal-server\include\kolosal\models\embedding_response_model.hpp`
- `naming_backup\kolosal-server\include\kolosal\node_manager.h`
- `naming_backup\kolosal-server\include\kolosal\qdrant_client.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\add_document_types.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\chunking_types.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\document_list_types.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\document_service.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\remove_document_types.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\retrieve_types.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\config\auth_config_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\downloads_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\engines_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\health_status_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\llm\completion_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\llm\oai_completions_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\models_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\retrieval\chunking_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\retrieval\documents_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\retrieval\embedding_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\retrieval\internet_search_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\retrieval\parse_document_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\routes\server_logs_route.hpp`
- `naming_backup\kolosal-server\include\kolosal\server.hpp`
- `naming_backup\kolosal-server\include\kolosal\server_api.hpp`
- `naming_backup\kolosal-server\include\kolosal\server_config.hpp`
- `naming_backup\kolosal-server\src\auth\auth_middleware.cpp`
- `naming_backup\kolosal-server\src\auth\cors_handler.cpp`
- `naming_backup\kolosal-server\src\auth\rate_limiter.cpp`
- `naming_backup\kolosal-server\src\download_manager.cpp`
- `naming_backup\kolosal-server\src\download_utils.cpp`
- `naming_backup\kolosal-server\src\gpu_detection.cpp`
- `naming_backup\kolosal-server\src\inference_loader.cpp`
- `naming_backup\kolosal-server\src\models\chunking_request_model.cpp`
- `naming_backup\kolosal-server\src\models\chunking_response_model.cpp`
- `naming_backup\kolosal-server\src\models\embedding_request_model.cpp`
- `naming_backup\kolosal-server\src\models\embedding_response_model.cpp`
- `naming_backup\kolosal-server\src\node_manager.cpp`
- `naming_backup\kolosal-server\src\qdrant_client.cpp`
- `naming_backup\kolosal-server\src\retrieval\add_document_types.cpp`
- `naming_backup\kolosal-server\src\retrieval\chunking_types.cpp`
- `naming_backup\kolosal-server\src\retrieval\document_list_types.cpp`
- `naming_backup\kolosal-server\src\retrieval\document_service.cpp`
- `naming_backup\kolosal-server\src\retrieval\remove_document_types.cpp`
- `naming_backup\kolosal-server\src\retrieval\retrieve_types.cpp`
- `naming_backup\kolosal-server\src\routes\config\auth_config_route.cpp`
- `naming_backup\kolosal-server\src\routes\downloads_route.cpp`
- `naming_backup\kolosal-server\src\routes\engines_route.cpp`
- `naming_backup\kolosal-server\src\routes\health_status_route.cpp`
- `naming_backup\kolosal-server\src\routes\llm\completion_route.cpp`
- `naming_backup\kolosal-server\src\routes\llm\oai_completions_route.cpp`
- `naming_backup\kolosal-server\src\routes\models_route.cpp`
- `naming_backup\kolosal-server\src\routes\retrieval\chunking_route.cpp`
- `naming_backup\kolosal-server\src\routes\retrieval\documents_route.cpp`
- `naming_backup\kolosal-server\src\routes\retrieval\embedding_route.cpp`
- `naming_backup\kolosal-server\src\routes\retrieval\internet_search_route.cpp`
- `naming_backup\kolosal-server\src\routes\retrieval\parse_document_route.cpp`
- `naming_backup\kolosal-server\src\routes\server_logs_route.cpp`
- `naming_backup\kolosal-server\src\server.cpp`
- `naming_backup\kolosal-server\src\server_api.cpp`
- `naming_backup\kolosal-server\src\server_config.cpp`
- `naming_backup\src\document_service_manager.cpp`
- `naming_backup\src\logger_system.cpp`
- `src\document_service_manager.cpp`
- `src\logger_system.cpp`

### retrieval

Used in 76 files:
- `complete_backup\kolosal-server\include\kolosal\retrieval\add_document_types.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\chunking_types.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\document_list_types.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\document_service.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\parse_docx.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\parse_html.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\parse_pdf.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\remove_document_types.hpp`
- `complete_backup\kolosal-server\include\kolosal\retrieval\retrieve_types.hpp`
- `complete_backup\kolosal-server\src\retrieval\add_document_types.cpp`
- `complete_backup\kolosal-server\src\retrieval\chunking_types.cpp`
- `complete_backup\kolosal-server\src\retrieval\document_list_types.cpp`
- `complete_backup\kolosal-server\src\retrieval\document_service.cpp`
- `complete_backup\kolosal-server\src\retrieval\parse_docx.cpp`
- `complete_backup\kolosal-server\src\retrieval\parse_html.cpp`
- `complete_backup\kolosal-server\src\retrieval\parse_pdf.cpp`
- `complete_backup\kolosal-server\src\retrieval\parse_pdf_stub.cpp`
- `complete_backup\kolosal-server\src\retrieval\remove_document_types.cpp`
- `complete_backup\kolosal-server\src\retrieval\retrieve_types.cpp`
- `kolosal-server\include\kolosal\retrieval\add_document_types.hpp`
- `kolosal-server\include\kolosal\retrieval\chunking_types.hpp`
- `kolosal-server\include\kolosal\retrieval\document_list_types.hpp`
- `kolosal-server\include\kolosal\retrieval\document_service.hpp`
- `kolosal-server\include\kolosal\retrieval\parse_docx.hpp`
- `kolosal-server\include\kolosal\retrieval\parse_html.hpp`
- `kolosal-server\include\kolosal\retrieval\parse_pdf.hpp`
- `kolosal-server\include\kolosal\retrieval\remove_document_types.hpp`
- `kolosal-server\include\kolosal\retrieval\retrieve_types.hpp`
- `kolosal-server\src\retrieval\add_document_types.cpp`
- `kolosal-server\src\retrieval\chunking_types.cpp`
- `kolosal-server\src\retrieval\document_list_types.cpp`
- `kolosal-server\src\retrieval\document_service.cpp`
- `kolosal-server\src\retrieval\parse_docx.cpp`
- `kolosal-server\src\retrieval\parse_html.cpp`
- `kolosal-server\src\retrieval\parse_pdf.cpp`
- `kolosal-server\src\retrieval\parse_pdf_stub.cpp`
- `kolosal-server\src\retrieval\remove_document_types.cpp`
- `kolosal-server\src\retrieval\retrieve_types.cpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\add_document_types.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\chunking_types.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\document_list_types.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\document_service.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\parse_docx.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\parse_html.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\parse_pdf.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\remove_document_types.hpp`
- `naming_backup\complete_backup\kolosal-server\include\kolosal\retrieval\retrieve_types.hpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\add_document_types.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\chunking_types.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\document_list_types.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\document_service.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\parse_docx.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\parse_html.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\parse_pdf.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\parse_pdf_stub.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\remove_document_types.cpp`
- `naming_backup\complete_backup\kolosal-server\src\retrieval\retrieve_types.cpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\add_document_types.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\chunking_types.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\document_list_types.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\document_service.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\parse_docx.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\parse_html.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\parse_pdf.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\remove_document_types.hpp`
- `naming_backup\kolosal-server\include\kolosal\retrieval\retrieve_types.hpp`
- `naming_backup\kolosal-server\src\retrieval\add_document_types.cpp`
- `naming_backup\kolosal-server\src\retrieval\chunking_types.cpp`
- `naming_backup\kolosal-server\src\retrieval\document_list_types.cpp`
- `naming_backup\kolosal-server\src\retrieval\document_service.cpp`
- `naming_backup\kolosal-server\src\retrieval\parse_docx.cpp`
- `naming_backup\kolosal-server\src\retrieval\parse_html.cpp`
- `naming_backup\kolosal-server\src\retrieval\parse_pdf.cpp`
- `naming_backup\kolosal-server\src\retrieval\parse_pdf_stub.cpp`
- `naming_backup\kolosal-server\src\retrieval\remove_document_types.cpp`
- `naming_backup\kolosal-server\src\retrieval\retrieve_types.cpp`

