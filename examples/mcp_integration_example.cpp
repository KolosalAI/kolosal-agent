/**
 * @file mcp_integration_example.cpp
 * @brief Example demonstrating MCP protocol integration with Kolosal agents
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Example file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "agent/core/multi_agent_system.hpp"
#include "agent/services/agent_service.hpp"

#ifdef MCP_PROTOCOL_ENABLED
#include "agent/services/mcp_agent_adapter.hpp"
#include "server/mcp_server_integration.hpp"
#include "mcp/transport/stdio_transport.hpp"
#include <nlohmann/json.hpp>
#endif

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

using namespace kolosal;

int main() {
    std::cout << "Kolosal Agent MCP Integration Example\n";
    std::cout << "=====================================\n\n";

#ifndef MCP_PROTOCOL_ENABLED
    std::cout << "MCP Protocol support is not enabled in this build.\n";
    std::cout << "Please rebuild with MCP_PROTOCOL_ENABLED defined.\n";
    return 1;
#else

    try {
        // 1. Create and configure the agent manager
        std::cout << "1. Creating agent manager...\n";
        auto agent_manager = std::make_shared<agents::YAMLConfigurableAgentManager>();
        
        // Load configuration (or use defaults)
        if (!agent_manager->load_configuration("config.yaml")) {
            std::cout << "   Warning: Could not load config.yaml, using defaults\n";
        }
        
        // Start the agent system
        agent_manager->start();
        
        // 2. Create the agent service
        std::cout << "2. Creating agent service...\n";
        auto agent_service = std::make_shared<services::AgentService>(agent_manager);
        
        // 3. Create some example agents (if not already loaded from config)
        std::cout << "3. Creating example agents...\n";
        
        agents::AgentConfig research_agent_config;
        research_agent_config.name = "research-agent";
        research_agent_config.type = "research";
        research_agent_config.role = agents::AgentRole::ANALYST;
        research_agent_config.capabilities = {"web_search", "data_analysis", "report_generation"};
        
        agents::AgentConfig code_agent_config;
        code_agent_config.name = "code-agent";
        code_agent_config.type = "coding";
        code_agent_config.role = agents::AgentRole::SPECIALIST;
        code_agent_config.capabilities = {"code_generation", "code_review", "debugging"};
        
        auto research_agent_future = agent_service->createAgentAsync(research_agent_config);
        auto code_agent_future = agent_service->createAgentAsync(code_agent_config);
        
        std::string research_agent_id = research_agent_future.get();
        std::string code_agent_id = code_agent_future.get();
        
        if (research_agent_id.empty() || code_agent_id.empty()) {
            std::cout << "   Failed to create agents\n";
            return 1;
        }
        
        std::cout << "   Created research agent: " << research_agent_id << "\n";
        std::cout << "   Created code agent: " << code_agent_id << "\n";
        
        // Start the agents
        agent_service->startAgentAsync(research_agent_id).get();
        agent_service->startAgentAsync(code_agent_id).get();
        
        // 4. Set up MCP integration
        std::cout << "4. Setting up MCP integration...\n";
        
        // Create MCP server integration
        server::MCPServerIntegration::MCPIntegrationConfig mcp_config;
        mcp_config.server_name = "kolosal-mcp-example";
        mcp_config.auto_expose_all_agents = true;
        mcp_config.enable_agent_discovery = true;
        
        auto mcp_integration = std::make_shared<server::MCPServerIntegration>(
            agent_manager, mcp_config);
        
        // Initialize and start MCP integration
        if (!mcp_integration->initialize()) {
            std::cout << "   Failed to initialize MCP integration\n";
            return 1;
        }
        
        if (!mcp_integration->start()) {
            std::cout << "   Failed to start MCP integration\n";
            return 1;
        }
        
        std::cout << "   MCP integration started successfully\n";
        
        // 5. Expose agents via MCP
        std::cout << "5. Exposing agents via MCP protocol...\n";
        
        if (mcp_integration->exposeAgent(research_agent_id, "research-mcp-server")) {
            std::cout << "   Exposed research agent via MCP\n";
        }
        
        if (mcp_integration->exposeAgent(code_agent_id, "code-mcp-server")) {
            std::cout << "   Exposed code agent via MCP\n";
        }
        
        // 6. Set up MCP adapters directly (alternative approach)
        std::cout << "6. Setting up direct MCP adapters...\n";
        
        // Auto-setup MCP for all agents via agent service
        size_t mcp_setup_count = agent_service->autoSetupMCPForAllAgents(true);
        std::cout << "   Set up MCP adapters for " << mcp_setup_count << " agents\n";
        
        // Get MCP adapter for research agent
        auto research_mcp_adapter = agent_service->getMCPAdapter(research_agent_id);
        if (research_mcp_adapter) {
            std::cout << "   Got MCP adapter for research agent\n";
            
            // Start MCP server for the research agent
            auto stdio_transport = std::make_shared<mcp::transport::StdioTransport>();
            if (research_mcp_adapter->startServer(stdio_transport)) {
                std::cout << "   Started MCP server for research agent\n";
            }
        }
        
        // 7. Demonstrate MCP functionality
        std::cout << "7. Demonstrating MCP functionality...\n";
        
        // Show MCP integration statistics
        auto mcp_stats = mcp_integration->getStatistics();
        std::cout << "   MCP Statistics:\n";
        std::cout << "     - Exposed agents: " << mcp_stats.exposed_agents << "\n";
        std::cout << "     - Active connections: " << mcp_stats.active_connections << "\n";
        std::cout << "     - Registered tools: " << mcp_stats.registered_tools << "\n";
        std::cout << "     - Registered resources: " << mcp_stats.registered_resources << "\n";
        
        // Show health status
        auto health_status = mcp_integration->getHealthStatus();
        std::cout << "   MCP Health Status:\n";
        std::cout << "     " << health_status.dump(2) << "\n";
        
        // 8. Simulate some activity
        std::cout << "8. Simulating MCP activity...\n";
        
        // Execute some functions on agents to generate activity
        using namespace std::chrono_literals;
        
        for (int i = 0; i < 3; ++i) {
            std::cout << "   Activity round " << (i + 1) << "...\n";
            
            // Execute a function on the research agent
            agents::AgentData params;
            auto research_future = agent_service->executeFunctionAsync(
                research_agent_id, "analyze_data", params, 1);
            
            // Execute a function on the code agent
            auto code_future = agent_service->executeFunctionAsync(
                code_agent_id, "generate_code", params, 1);
            
            // Wait for completion
            std::this_thread::sleep_for(1s);
            
            // Show updated statistics
            auto updated_stats = mcp_integration->getStatistics();
            std::cout << "     Total requests: " << updated_stats.total_requests << "\n";
            std::cout << "     Successful requests: " << updated_stats.successful_requests << "\n";
        }
        
        // 9. Demonstrate cross-agent communication via MCP
        std::cout << "9. Demonstrating cross-agent MCP communication...\n";
        
        if (mcp_integration->enableCrossAgentCommunication()) {
            std::cout << "   Enabled cross-agent MCP communication\n";
            
            // This would allow agents to call each other's tools via MCP
            // In a real scenario, you would set up the communication channels
            std::cout << "   Cross-agent communication is now active\n";
        }
        
        // 10. Display final status
        std::cout << "10. Final status check...\n";
        
        auto final_stats = mcp_integration->getStatistics();
        std::cout << "    Final MCP Statistics:\n";
        std::cout << "      - Total requests: " << final_stats.total_requests << "\n";
        std::cout << "      - Successful requests: " << final_stats.successful_requests << "\n";
        std::cout << "      - Failed requests: " << final_stats.failed_requests << "\n";
        std::cout << "      - Average response time: " << final_stats.average_response_time_ms << "ms\n";
        
        std::cout << "\n=== MCP Integration Example Completed Successfully ===\n";
        
        // Give user a chance to see the output
        std::cout << "\nPress Enter to shutdown...\n";
        std::cin.get();
        
        // 11. Clean shutdown
        std::cout << "11. Shutting down...\n";
        
        // Stop MCP integration
        mcp_integration->stop();
        std::cout << "    MCP integration stopped\n";
        
        // Stop agent service
        agent_service->stopHealthMonitoring();
        
        // Stop agent manager
        agent_manager->stop();
        std::cout << "    Agent system stopped\n";
        
        std::cout << "Shutdown completed.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;

#endif // MCP_PROTOCOL_ENABLED
}
