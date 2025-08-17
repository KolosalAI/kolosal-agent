/**
 * @file agent_core.hpp
 * @brief Core agent functionality and lifecycle management
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_AGENT_AGENT_CORE_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_AGENT_AGENT_CORE_HPP_INCLUDED

#include "../../export.hpp"
#include "agent_interfaces.hpp"
#include "agent_roles.hpp"
#include "../../execution/function_execution_manager.hpp"
#include "../../execution/task_job_manager.hpp"
#include "../../tools/system_event_manager.hpp"
#include "../../api/message_router.hpp"
#include "../../tools/system_tool_registry.hpp"
#include "../memory/agent_memory_manager.hpp"
#include "../planning/agent_planning_system.hpp"
#include <memory>
#include <atomic>
#include <vector>
#include <mutex>

namespace kolosal::agents {

// Forward declaration
/**
 * @brief Represents logger functionality
 */
class Logger;

/**
 * @brief Core agent implementation with advanced capabilities
 */
class KOLOSAL_AGENT_API AgentCore : public std::enable_shared_from_this<AgentCore> {
private:
    std::shared_ptr<Logger> logger;
    std::shared_ptr<FunctionManager> function_manager;
    std::shared_ptr<JobManager> job_manager;
    std::shared_ptr<EventSystem> event_system;
    std::shared_ptr<MessageRouter> message_router;
    std::shared_ptr<ToolRegistry> tool_registry;
    std::shared_ptr<MemoryManager> memory_manager;
    std::shared_ptr<PlanningReasoningCoordinator> planning_coordinator;
    std::shared_ptr<AgentRoleManager> role_manager;
    
    std::atomic<bool> running {false};
    std::string agent_id;
    std::string agent_name;
    std::string agent_type;
    AgentRole current_role;
    std::vector<AgentSpecialization> specializations;
    std::vector<std::string> capabilities;
    
    // Instance mutexes to prevent deadlocks
    mutable std::mutex capabilities_mutex;
    mutable std::mutex message_mutex;
    
    // Enhanced function support
    std::shared_ptr<class EnhancedFunctionRegistry> enhanced_registry;
    std::string server_url;
    bool server_integration_enabled;

public:
    AgentCore(const std::string& name = "", const std::string& type = "generic", 
              const AgentRole role = AgentRole::GENERIC);
    ~AgentCore();

    // Lifecycle management
    void start();
    void stop();
    bool is__running() const { return running.load(); }

    // Role and capability management
    void set__role(AgentRole role);
    AgentRole get__role() const { return current_role; }
    void add_specialization(AgentSpecialization spec);
    std::vector<AgentSpecialization> get__specializations() const { return specializations; }
    void set__message_router(std::shared_ptr<MessageRouter> router);
    void add_capability(const std::string& capability);

    // Enhanced function and tool execution
    FunctionResult execute_function(const std::string& name, const AgentData& parameters);
    std::string execute_function_async(const std::string& name, const AgentData& parameters, const int priority = 0);
    FunctionResult execute_tool(const std::string& tool_name, const AgentData& parameters);
    
    // Planning and reasoning
    ExecutionPlan create__plan(const std::string& goal, const std::string& context = "");
    bool execute_plan(const std::string& plan_id);
    std::string reason_about(const std::string& question, const std::string& context = "");
    
    // Memory management
    void store_memory(const std::string& content, const std::string& type = "general");
    std::vector<MemoryEntry> recall_memories(const std::string& query, const int max_results = 5);
    void set__working_context(const std::string& key, const AgentData& data);
    AgentData get__working_context(const std::string& key);

    // Messaging
    void send_message(const std::string& to_agent, const std::string& message_type, 
                      const AgentData& payload = AgentData());
    void broadcast_message(const std::string& message_type, const AgentData& payload = AgentData());

    // Tool discovery and management
    std::vector<std::string> discover_tools(const ToolFilter& filter = ToolFilter());
    bool register_custom_tool(std::unique_ptr<Tool> tool);
    ToolSchema get__tool_schema(const std::string& tool_name);
    
    // Enhanced function registration with kolosal-server integration
    void enable_enhanced_functions(const std::string& server_url = "http://localhost:8080", 
                                  bool test_connection = true);
    void set_server_url(const std::string& url);
    bool is_server_integration_enabled() const;

    // Getters
    const std::string& get__agent_id() const { return agent_id; }
    const std::string& get__agent_name() const { return agent_name; }
    const std::string& get__agent_type() const { return agent_type; }
    const std::vector<std::string>& get__capabilities() const { return capabilities; }
    
    // Component access
    std::shared_ptr<Logger> get__logger() { return logger; }
    std::shared_ptr<FunctionManager> get__function_manager() { return function_manager; }
    std::shared_ptr<JobManager> get__job_manager() { return job_manager; }
    std::shared_ptr<EventSystem> get__event_system() { return event_system; }
    std::shared_ptr<ToolRegistry> get__tool_registry() { return tool_registry; }
    std::shared_ptr<MemoryManager> get__memory_manager() { return memory_manager; }
    std::shared_ptr<PlanningReasoningCoordinator> get__planning_coordinator() { return planning_coordinator; }
    
    // Performance and monitoring
    struct AgentStats {
        size_t total_functions_executed;
        size_t total_tools_executed;
        size_t total_plans_created;
        size_t memory_entries_count;
        double average_execution_time_ms;
        std::chrono::time_point<std::chrono::system_clock> last_activity;
    };
    
    AgentStats get__statistics() const;

private:
    void handle_message(const AgentMessage& message);
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_AGENT_AGENT_CORE_HPP_INCLUDED
