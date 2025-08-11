# MCP Protocol Integration for Kolosal Agent System

This document describes the integration of the Model Context Protocol (MCP) with the Kolosal Agent System v2.0, enabling seamless interoperability between agents and external MCP-compatible systems.

## Overview

The MCP integration provides:

- **Agent-to-MCP Server**: Expose Kolosal agents as MCP servers, making their functions, memory, and capabilities available to external MCP clients
- **Agent-to-MCP Client**: Enable Kolosal agents to connect to external MCP servers and use their tools and resources
- **Cross-Agent Communication**: Allow agents to communicate with each other using the MCP protocol
- **Unified Management**: Centralized management of all MCP connections and exposures through the integrated server

## Components

### 1. MCPAgentAdapter

The core component that adapts individual agents for MCP protocol compatibility.

**Features:**
- Exposes agent functions as MCP tools
- Exposes agent memory as MCP resources  
- Exposes agent capabilities as MCP prompts
- Provides MCP client functionality for connecting to external servers
- Automatic registration of agent components
- Event-driven notifications

**Key Classes:**
- `kolosal::services::MCPAgentAdapter` - Main adapter class
- `MCPConfig` - Configuration structure for MCP settings

### 2. MCPServerIntegration

High-level integration layer that manages multiple agents and MCP connections at the system level.

**Features:**
- Bulk agent exposure via MCP
- External MCP server connection management
- Load balancing and request routing
- Health monitoring and statistics
- Batch operations
- Rate limiting

**Key Classes:**
- `kolosal::server::MCPServerIntegration` - Main integration class
- `MCPIntegrationConfig` - System-wide MCP configuration
- `MCPStats` - Statistics and monitoring data

### 3. Unified Server Integration

Integration with the main `UnifiedKolosalServer` to provide MCP functionality alongside LLM and agent services.

**Features:**
- Automatic MCP setup during server startup
- Configuration through server config
- Health monitoring integration
- REST API endpoints for MCP management

## Usage Examples

### Basic Agent MCP Server

```cpp
#include "agent/services/mcp_agent_adapter.hpp"
#include "mcp/transport/stdio_transport.hpp"

// Create agent
auto agent = agent_manager->get__agent("my-agent");

// Create MCP adapter
kolosal::services::MCPAgentAdapter::MCPConfig config;
config.server_name = "my-agent-mcp";
config.enable_tool_streaming = true;

auto mcp_adapter = std::make_shared<kolosal::services::MCPAgentAdapter>(agent, config);

// Auto-register agent functions as MCP tools
mcp_adapter->autoRegisterAgentFunctions();
mcp_adapter->autoRegisterAgentMemory();
mcp_adapter->autoRegisterAgentCapabilities();

// Start MCP server
auto transport = std::make_shared<mcp::transport::StdioTransport>();
mcp_adapter->startServer(transport);
```

### System-Wide MCP Integration

```cpp
#include "server/mcp_server_integration.hpp"

// Create MCP integration
kolosal::server::MCPServerIntegration::MCPIntegrationConfig config;
config.auto_expose_all_agents = true;
config.enable_cross_agent_communication = true;

auto mcp_integration = std::make_shared<kolosal::server::MCPServerIntegration>(
    agent_manager, config);

// Initialize and start
mcp_integration->initialize();
mcp_integration->start();

// Expose specific agent
mcp_integration->exposeAgent("research-agent", "research-mcp-server");

// Connect to external MCP server
auto external_transport = std::make_shared<mcp::transport::StdioTransport>();
mcp_integration->connectToExternalServer("external-tools", external_transport);
```

### Using AgentService for MCP

```cpp
#include "agent/services/agent_service.hpp"

auto agent_service = std::make_shared<kolosal::services::AgentService>(agent_manager);

// Auto-setup MCP for all agents
size_t setup_count = agent_service->autoSetupMCPForAllAgents(true);

// Start MCP server for specific agent
agent_service->startAgentMCPServer("my-agent", "stdio");

// Connect agent to external MCP server
auto connection_future = agent_service->connectAgentToMCPServer("my-agent", "stdio://external-server");
bool connected = connection_future.get();
```

## Configuration

### Agent-Level Configuration

```cpp
kolosal::services::MCPAgentAdapter::MCPConfig config;
config.server_name = "my-agent-mcp";
config.server_version = "1.0.0";
config.server_instructions = "AI agent with specialized capabilities";
config.enable_tool_streaming = true;
config.enable_resource_templates = true;
config.enable_prompt_templates = true;
config.default_timeout = std::chrono::milliseconds(30000);
```

### System-Level Configuration

```cpp
kolosal::server::MCPServerIntegration::MCPIntegrationConfig config;
config.server_name = "kolosal-mcp-server";
config.server_port = 8080;
config.max_client_connections = 100;
config.enable_stdio_transport = true;
config.enable_http_sse_transport = true;
config.auto_expose_all_agents = true;
config.enable_agent_discovery = true;
config.enable_cross_agent_communication = true;
config.enable_rate_limiting = true;
config.max_requests_per_minute = 1000;
```

### Unified Server Configuration

```cpp
// In UnifiedKolosalServer::ServerConfig
#ifdef MCP_PROTOCOL_ENABLED
bool enable_mcp_integration = true;
bool auto_expose_all_agents_via_mcp = true;
std::string mcp_server_name = "kolosal-unified-server";
int mcp_server_port = 8082;
bool enable_mcp_stdio_transport = true;
bool enable_mcp_http_sse_transport = false;
bool enable_cross_agent_mcp_communication = true;
size_t max_mcp_connections = 100;
#endif
```

## Transport Options

### STDIO Transport

For command-line and process-to-process communication:

```cpp
auto stdio_transport = std::make_shared<mcp::transport::StdioTransport>();
```

### HTTP Server-Sent Events Transport

For web-based integration:

```cpp
auto sse_transport = std::make_shared<mcp::transport::HttpSseTransport>(host, port);
```

## Tool Registration

### Manual Registration

```cpp
// Register agent function as MCP tool
mcp_adapter->registerAgentFunctionAsTool(
    "analyze_data",           // Agent function name
    "data_analyzer",          // MCP tool name
    "Analyze provided data",  // Description
    input_schema              // JSON schema (optional)
);

// Register agent memory as MCP resource
mcp_adapter->registerAgentMemoryAsResource(
    "episodic",                                    // Memory type
    "kolosal://agent/my-agent/memory/episodic",   // Resource URI
    "Agent Episodic Memory",                      // Resource name
    "Access to agent's episodic memory"          // Description
);

// Register agent capability as MCP prompt
mcp_adapter->registerAgentCapabilityAsPrompt(
    "reasoning",              // Capability name
    "agent_reasoning",        // Prompt name
    "Execute reasoning task", // Description
    {{"context", "Reasoning context"}, {"question", "Question to answer"}}
);
```

### Automatic Registration

```cpp
// Auto-register all agent functions
size_t tools_count = mcp_adapter->autoRegisterAgentFunctions(false); // exclude built-ins

// Auto-register all memory types
size_t resources_count = mcp_adapter->autoRegisterAgentMemory();

// Auto-register all capabilities
size_t prompts_count = mcp_adapter->autoRegisterAgentCapabilities();
```

## Client Operations

### Connecting to External MCP Server

```cpp
auto transport = std::make_shared<mcp::transport::StdioTransport>();
auto init_future = mcp_adapter->initializeClient(transport, std::chrono::seconds(30));

// Wait for connection
auto init_result = init_future.get();
```

### Using External Tools

```cpp
// List available tools
auto tools_future = mcp_adapter->listRemoteTools();
auto tools_result = tools_future.get();

// Call external tool
nlohmann::json params = {{"query", "search term"}};
auto tool_future = mcp_adapter->callRemoteTool("web_search", params);
auto tool_result = tool_future.get();
```

### Reading External Resources

```cpp
// List available resources
auto resources_future = mcp_adapter->listRemoteResources();
auto resources_result = resources_future.get();

// Read resource
auto resource_future = mcp_adapter->readRemoteResource("file://data.json");
auto resource_result = resource_future.get();
```

## Monitoring and Statistics

### Real-time Statistics

```cpp
auto stats = mcp_integration->getStatistics();
std::cout << "Active connections: " << stats.active_connections << std::endl;
std::cout << "Total requests: " << stats.total_requests << std::endl;
std::cout << "Success rate: " << (stats.successful_requests / stats.total_requests) << std::endl;
```

### Health Monitoring

```cpp
auto health = mcp_integration->getHealthStatus();
// health is a JSON object with detailed status information

// Set up callbacks
mcp_integration->setConnectionCallback([](const std::string& id, bool connected) {
    std::cout << "Connection " << id << (connected ? " established" : " lost") << std::endl;
});

mcp_integration->setRequestCallback([](const std::string& agent_id, const std::string& op, double ms) {
    std::cout << "Agent " << agent_id << " " << op << " took " << ms << "ms" << std::endl;
});
```

## Error Handling

### Exception Handling

```cpp
try {
    mcp_adapter->startServer(transport);
} catch (const mcp::utils::TransportException& e) {
    std::cerr << "Transport error: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "General error: " << e.what() << std::endl;
}
```

### Event-Based Error Handling

```cpp
mcp_adapter->registerEventCallback("server_error", [](const std::string& event, const nlohmann::json& data) {
    std::cerr << "MCP Server Error: " << data["error"].get<std::string>() << std::endl;
});

mcp_adapter->registerEventCallback("client_error", [](const std::string& event, const nlohmann::json& data) {
    std::cerr << "MCP Client Error: " << data["error"].get<std::string>() << std::endl;
});
```

## Best Practices

### Security

1. **Authentication**: Enable authentication in production environments
2. **Rate Limiting**: Use rate limiting to prevent abuse
3. **Input Validation**: Validate all MCP requests and parameters
4. **Transport Security**: Use secure transports for network communication

### Performance

1. **Connection Pooling**: Reuse connections where possible
2. **Async Operations**: Use asynchronous operations for better scalability
3. **Resource Management**: Properly clean up connections and resources
4. **Load Balancing**: Use load balancing for high-traffic scenarios

### Reliability

1. **Health Monitoring**: Implement comprehensive health checks
2. **Auto-Recovery**: Enable auto-recovery for transient failures
3. **Graceful Degradation**: Handle partial failures gracefully
4. **Logging**: Implement detailed logging for debugging

## Troubleshooting

### Common Issues

1. **Build Errors**: Ensure MCP_PROTOCOL_ENABLED is defined during compilation
2. **Transport Failures**: Check network connectivity and transport configuration
3. **Authentication Issues**: Verify credentials and authentication settings
4. **Performance Issues**: Monitor connection counts and request rates

### Debug Information

Enable debug logging:

```cpp
mcp::utils::setLogLevel(mcp::utils::LogLevel::Debug);
```

Check MCP integration status:

```cpp
auto health = mcp_integration->getHealthStatus();
std::cout << health.dump(2) << std::endl;
```

## Integration Examples

See `examples/mcp_integration_example.cpp` for a complete working example that demonstrates:

- Creating and configuring agents
- Setting up MCP integration
- Exposing agents via MCP protocol
- Connecting to external MCP servers
- Monitoring and statistics
- Proper shutdown procedures

## API Reference

For detailed API documentation, see:

- `include/agent/services/mcp_agent_adapter.hpp` - Individual agent MCP adapter
- `include/server/mcp_server_integration.hpp` - System-wide MCP integration
- `include/server/unified_server.hpp` - Unified server with MCP support

## Future Enhancements

Planned improvements include:

- WebSocket transport support
- Enhanced security features
- Plugin system for custom transports
- Advanced routing and load balancing
- Integration with external service discovery
- Support for MCP extensions and custom protocols
