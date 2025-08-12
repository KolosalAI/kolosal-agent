# Kolosal Agent System Documentation

Welcome to the comprehensive documentation for the Kolosal Agent System v2.0 - a next-generation unified multi-agent AI platform.

Documentation generated on 2025-08-12

## 📚 Documentation Sections

### 🏗️ Architecture Documentation
- **[System Overview](architecture/overview.md)** - Complete system architecture and component details
- **[System Diagrams](architecture/system-diagrams.md)** - Visual architecture diagrams with detailed explanations
  - High-Level System Architecture
  - Agent Workflow Execution Flow
  - Multi-Agent Workflow Orchestration
  - Memory System Architecture
  - Function Registry and Tool System
  - REST API Architecture
  - Data Flow Architecture
  - Configuration and Deployment Architecture

### 🔧 API Reference
- **[Classes](api/classes.md)** - Complete API documentation for all system classes
- **[Functions](api/functions.md)** - Global functions and utilities

### 📖 User Guides
- **[Installation](guides/installation.md)** - Step-by-step installation instructions
- **[Usage Guide](guides/usage.md)** - Comprehensive usage documentation
- **[Configuration](guides/configuration.md)** - Configuration reference and examples
- **[Deployment & Operations](guides/deployment-operations.md)** - Production deployment, monitoring, and operations
- **[Developer Guide](guides/developer-guide.md)** - Complete guide for developers extending or contributing to the system

## 🚀 Quick Start

### System Architecture Overview

The Kolosal Agent System v2.0 is built on a unified architecture that seamlessly integrates:

```
┌─────────────────────────────────────────────────────────────┐
│                    🌐 Client Applications                   │
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

### Key Features

- **🔄 Unified Server**: Single binary managing both LLM inference and multi-agent systems
- **🤖 Advanced Agent Orchestration**: Sophisticated workflow execution with multiple collaboration patterns
- **🌐 Comprehensive REST API**: Full-featured management interface with OpenAPI compatibility
- **📊 Service Layer Architecture**: High-level abstractions for complex agent operations
- **⚡ Async-First Design**: Non-blocking operations with Future-based patterns
- **🔧 Dynamic Configuration**: Hot-reloading configuration without system restart
- **💓 Health Monitoring**: Comprehensive monitoring with auto-recovery mechanisms

### Basic Setup

```bash
# Build the system
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug

# Start the unified server
./kolosal-agent-unified --config ../config.yaml

# Access the REST API
curl http://localhost:8081/v1/agents
```

### Agent Types

The system supports four primary agent types:

- **👑 Coordinator Agents**: System planning, task delegation, and resource management
- **📊 Analyst Agents**: Data processing, statistical analysis, and report generation
- **🔍 Research Agents**: Information gathering, web research, and document analysis
- **🎯 Specialist Agents**: Domain-specific expertise and specialized processing

### REST API Endpoints

#### Agent Management
- `GET /v1/agents` - List all agents with status and statistics
- `POST /v1/agents` - Create new agents from configuration
- `POST /v1/agents/{id}/execute` - Execute functions with parameters
- `PUT /v1/agents/{id}/start` - Start specific agent
- `PUT /v1/agents/{id}/stop` - Stop specific agent

#### System Management
- `GET /v1/system/status` - System-wide status and health metrics
- `POST /v1/system/reload` - Hot-reload configuration
- `GET /v1/health` - Health check for monitoring
- `GET /v1/metrics` - Performance metrics

## 📊 System Statistics

- **Classes Documented**: 49+
- **Functions Documented**: 100+
- **Configuration Options**: 285+
- **Namespaces**: 6
- **Agent Types**: 4
- **Function Categories**: 20+
- **Tool Integrations**: 15+

## 🏗️ Core Components

### Unified Server (`UnifiedKolosalServer`)
- Manages both LLM inference and agent systems
- Automatic health monitoring and recovery
- Process lifecycle management
- Configuration hot-reloading

### Service Layer (`AgentService`)
- High-level agent operations
- Bulk operations and batch processing
- Performance analytics and optimization
- Event-driven notifications

### REST API (`AgentManagementRoute`)
- Comprehensive agent management endpoints
- OpenAPI-compatible documentation
- Real-time system status and metrics
- Secure authentication and rate limiting

### Multi-Agent Core
- Advanced agent lifecycle management
- Sophisticated message routing
- Memory and planning systems
- Tool registry and function management

## 🔗 Integration Capabilities

### Document Processing
- PDF, DOCX, HTML parsing
- RAG (Retrieval-Augmented Generation) integration
- Vector database integration
- Content extraction and analysis

### Web Services
- Search engine integration
- Web content fetching and processing
- HTTP client capabilities
- API integration patterns

### External Databases
- Vector databases (Qdrant, Weaviate)
- SQL database connectivity
- NoSQL database support
- Cloud storage integration

## 🚀 Getting Started

1. **[Read the Architecture Overview](architecture/overview.md)** - Understand the system design
2. **[View the System Diagrams](architecture/system-diagrams.md)** - Visual understanding of components
3. **[Follow the Installation Guide](guides/installation.md)** - Set up the system
4. **[Configure Your Agents](guides/configuration.md)** - Create and configure agents
5. **[Explore the API](api/classes.md)** - Integrate with external systems

## 🤝 Contributing

We welcome contributions to improve the documentation and system. Please see our main repository for contributing guidelines.

## 📞 Support

- **Issues**: GitHub Issues for bug reports and feature requests
- **Discussions**: GitHub Discussions for questions and community support
- **Documentation**: This comprehensive documentation suite
- **Examples**: Check the `examples/` directory for integration examples

---

**Kolosal Agent System v2.0** - *Empowering the future of AI through unified, intelligent, multi-agent systems.*

