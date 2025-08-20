/**
 * @file workflow_example.cpp
 * @brief Example demonstrating workflow engine usage
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Example file for the Kolosal Agent System v2.0.
 * Demonstrates workflow engine capabilities.
 */

#include "workflow/workflow_engine.hpp"
#include "agent/core/multi_agent_system.hpp"
#include "kolosal/logger.hpp"
#include <json.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace kolosal::agents;
using json = nlohmann::json;

int main() {
    try {
        // Initialize agent manager
        auto agent_manager = std::make_shared<YAMLConfigurableAgentManager>();
        
        // Load agent configuration
        if (!agent_manager->load_configuration("agent_config.yaml")) {
            std::cerr << "Failed to load agent configuration\n";
            return 1;
        }
        
        // Start agent manager
        agent_manager->start();
        
        // Initialize workflow engine
        WorkflowEngine workflow_engine(agent_manager);
        workflow_engine.start();
        
        std::cout << "=== Kolosal Agent Workflow Engine Demo ===\n\n";
        
        // Example 1: Load workflows from YAML files
        std::cout << "1. Loading workflows from YAML files...\n";
        
        if (workflow_engine.load_workflow_from_yaml("sequential.yaml")) {
            std::cout << "   ✓ Loaded sequential workflow\n";
        }
        
        if (workflow_engine.load_workflows_from_directory("examples/")) {
            std::cout << "   ✓ Loaded workflows from examples directory\n";
        }
        
        // List available workflows
        auto workflow_ids = workflow_engine.list_workflows();
        std::cout << "\nAvailable workflows (" << workflow_ids.size() << "):\n";
        for (const auto& id : workflow_ids) {
            auto workflow = workflow_engine.get_workflow(id);
            if (workflow) {
                std::cout << "   - " << workflow->name << " [" << id << "]\n";
                std::cout << "     Type: " << static_cast<int>(workflow->type) 
                         << ", Steps: " << workflow->steps.size() << "\n";
            }
        }
        
        // Example 2: Execute a workflow
        if (!workflow_ids.empty()) {
            // Find simple_test_workflow
            std::string simple_workflow_id;
            for (const auto& id : workflow_ids) {
                auto workflow = workflow_engine.get_workflow(id);
                if (workflow && workflow->name == "Simple Test Workflow") {
                    simple_workflow_id = id;
                    break;
                }
            }
            
            // If simple workflow not found, use the first one
            if (simple_workflow_id.empty()) {
                simple_workflow_id = workflow_ids[0];
            }
            
            std::cout << "\n2. Executing workflow: " << simple_workflow_id << "\n";
            
            // Input context for the workflow
            json input_context = {
                {"topic", "Multi-Agent AI Systems"},
                {"urgency", "medium"},
                {"output_format", "comprehensive"}
            };
            
            // Execute the workflow
            std::string execution_id = workflow_engine.execute_workflow(simple_workflow_id, input_context);
            std::cout << "   Execution ID: " << execution_id << "\n";
            
            // Monitor execution progress
            std::cout << "   Monitoring execution progress...\n";
            
            auto start_time = std::chrono::steady_clock::now();
            while (true) {
                auto status = workflow_engine.get_execution_status(execution_id);
                if (!status) {
                    std::cout << "   ✗ Execution status not found\n";
                    break;
                }
                
                std::cout << "   Status: ";
                switch (status->current_status) {
                    case WorkflowStatus::PENDING:
                        std::cout << "PENDING";
                        break;
                    case WorkflowStatus::RUNNING:
                        std::cout << "RUNNING (step: " << status->current_step_id << ")";
                        break;
                    case WorkflowStatus::COMPLETED:
                        std::cout << "COMPLETED";
                        break;
                    case WorkflowStatus::FAILED:
                        std::cout << "FAILED";
                        break;
                    case WorkflowStatus::CANCELLED:
                        std::cout << "CANCELLED";
                        break;
                    default:
                        std::cout << "UNKNOWN";
                        break;
                }
                
                std::cout << " (completed: " << status->completed_steps.size()
                         << ", failed: " << status->failed_steps.size() << ")\n";
                
                if (status->current_status == WorkflowStatus::COMPLETED ||
                    status->current_status == WorkflowStatus::FAILED ||
                    status->current_status == WorkflowStatus::CANCELLED) {
                    break;
                }
                
                // Timeout after 5 minutes
                auto elapsed = std::chrono::steady_clock::now() - start_time;
                if (elapsed > std::chrono::minutes(5)) {
                    std::cout << "   ⚠ Timeout reached, cancelling workflow\n";
                    workflow_engine.cancel_workflow(execution_id);
                    break;
                }
                
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
        
        // Example 3: Programmatic workflow creation
        std::cout << "\n3. Creating workflow programmatically...\n";
        
        // Create a simple sequential workflow
        auto programmatic_workflow = workflow_engine.create_sequential_workflow(
            "Programmatic Research Workflow",
            {
                {"research_analyst", "research_topic"},
                {"research_analyst", "analyze_data"},
                {"research_analyst", "generate_report"}
            }
        );
        
        // Customize the workflow
        programmatic_workflow.description = "A programmatically created research workflow";
        programmatic_workflow.global_context = {
            {"topic", "Programmatic Workflow Creation"},
            {"format", "summary"}
        };
        
        // Add the workflow to the engine
        std::string prog_id = workflow_engine.create_workflow(programmatic_workflow);
        std::cout << "   Created programmatic workflow: " << prog_id << "\n";
        
        // Example 4: Display workflow metrics
        std::cout << "\n4. Workflow Engine Metrics:\n";
        auto metrics = workflow_engine.get_metrics();
        std::cout << "   Total Workflows: " << metrics.total_workflows << "\n";
        std::cout << "   Running: " << metrics.running_workflows << "\n";
        std::cout << "   Completed: " << metrics.completed_workflows << "\n";
        std::cout << "   Failed: " << metrics.failed_workflows << "\n";
        std::cout << "   Success Rate: " << std::fixed << std::setprecision(1) 
                 << metrics.success_rate << "%\n";
        
        // Example 5: Show execution history
        std::cout << "\n5. Execution History:\n";
        auto history = workflow_engine.get_execution_history();
        if (history.empty()) {
            std::cout << "   No execution history available\n";
        } else {
            for (const auto& exec : history) {
                std::cout << "   - " << exec.execution_id.substr(0, 12) << "... "
                         << "(" << static_cast<int>(exec.current_status) << ") "
                         << "Workflow: " << exec.workflow_id << "\n";
            }
        }
        
        std::cout << "\n=== Demo Complete ===\n";
        
        // Clean shutdown
        workflow_engine.stop();
        agent_manager->stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
