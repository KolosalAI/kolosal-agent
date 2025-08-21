/**
 * @file workflow_loader.hpp
 * @brief Utility for loading workflows from YAML configuration files
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_WORKFLOW_WORKFLOW_LOADER_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_WORKFLOW_WORKFLOW_LOADER_HPP_INCLUDED

#include "../export.hpp"
#include "workflow_engine.hpp"
#include <memory>
#include <string>
#include <vector>

namespace kolosal::agents {

/**
 * @brief Utility class for loading workflows from YAML files
 */
class KOLOSAL_AGENT_API WorkflowLoader {
public:
    /**
     * @brief Constructor
     * @param workflow_engine Shared pointer to the workflow engine
     */
    explicit WorkflowLoader(std::shared_ptr<WorkflowEngine> workflow_engine);

    /**
     * @brief Load a single workflow from YAML file
     * @param yaml_file Path to the YAML file
     * @return Workflow ID if successful, empty string on failure
     */
    std::string loadWorkflow_FromFile(const std::string& yaml_file);

    /**
     * @brief Load all workflows from a directory
     * @param directory_path Path to directory containing YAML files
     * @return Vector of loaded workflow IDs
     */
    std::vector<std::string> loadWorkflows_FromDirectory(const std::string& directory_path);

    /**
     * @brief Load sequential workflow from configuration
     * @param yaml_content YAML content as string
     * @return Workflow ID if successful, empty string on failure
     */
    std::string loadSequential_WorkflowFromYaml(const std::string& yaml_content);

    /**
     * @brief Auto-load default workflows including sequential.yaml
     * @return Number of workflows loaded
     */
    int autoLoad_DefaultWorkflows();

    /**
     * @brief Validate workflow configuration
     * @param yaml_content YAML content to validate
     * @return true if valid, false otherwise
     */
    bool validateWorkflow_Configuration(const std::string& yaml_content);

private:
    std::shared_ptr<WorkflowEngine> workflow_engine_;
    
    // Helper methods
    Workflow parseWorkflow_FromYaml(const std::string& yaml_content);
    WorkflowStep parseStep_FromYaml(const nlohmann::json& step_json);
    ErrorHandlingStrategy parseErrorHandling_FromYaml(const nlohmann::json& error_json);
    WorkflowType parseWorkflow_Type(const std::string& type_str);
    
    // Validation helpers
    bool validateStep_Configuration(const nlohmann::json& step_json);
    bool validateDependencies(const std::vector<WorkflowStep>& steps);
    
    // Utility methods
    std::string readFile_Content(const std::string& file_path);
    bool isYaml_File(const std::string& file_path);
    void logLoad_Event(const std::string& level, const std::string& message);
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_WORKFLOW_WORKFLOW_LOADER_HPP_INCLUDED
