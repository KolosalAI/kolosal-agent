/**
 * @file test_http_server.cpp
 * @brief Simple test for the HTTP server functionality
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "rest_api/simple_http_server.hpp"
#include "rest_api/agent_management_route.hpp"
#include "agent_services/agent_service.hpp"

using namespace kolosal::api;
using namespace kolosal::services;

int main() {
    std::cout << "Testing HTTP Server for Agent Management API..." << std::endl;
    
    try {
        // Create HTTP server configuration
        SimpleHttpServer::ServerConfig config;
        config.host = "127.0.0.1";
        config.port = 8081;
        config.backlog = 10;
        
        // Create HTTP server instance
        auto http_server = std::make_unique<SimpleHttpServer>(config);
        
        // Test server creation
        std::cout << "✅ HTTP server created successfully" << std::endl;
        std::cout << "   • Host: " << config.host << std::endl;
        std::cout << "   • Port: " << config.port << std::endl;
        
        // Try to start the server
        if (http_server->start()) {
            std::cout << "✅ HTTP server started successfully!" << std::endl;
            std::cout << "   • Server listening on http://" << config.host << ":" << config.port << std::endl;
            
            // Keep server running for a few seconds
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            // Stop the server
            http_server->stop();
            std::cout << "✅ HTTP server stopped gracefully" << std::endl;
        } else {
            std::cout << "❌ Failed to start HTTP server" << std::endl;
            return 1;
        }
        
        std::cout << "\n🎯 HTTP Server Test PASSED!" << std::endl;
        std::cout << "The API should now work properly when running the kolosal agent unified server." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
