/**
 * @file workflow_loader.cpp
 * @brief Utility for loading workflows from YAML configuration files
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "workflow/workflow_loader.hpp"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <regex>

namespace kolosal::agents {

WorkflowLoader::WorkflowLoader(std::shared_ptr<WorkflowEngine> workflow_engine)
    : workflow_engine_(workflow_engine) {
    if (!workflow_engine_) {
        throw std::invalid_argument("Workflow engine cannot be null");
    }
}

std::string WorkflowLoader::loadWorkflow_FromFile(const std::string& yaml_file) {
    try {
        if (!std::filesystem::exists(yaml_file)) {
            logLoad_Event("ERROR", "Workflow file not found: " + yaml_file);
            return "";
        }
        
        std::string yaml_content = readFile_Content(yaml_file);
        if (yaml_content.empty()) {
            logLoad_Event("ERROR", "Failed to read workflow file: " + yaml_file);
            return "";
        }
        
        if (!validateWorkflow_Configuration(yaml_content)) {
            logLoad_Event("ERROR", "Invalid workflow configuration in: " + yaml_file);
            return "";
        }
        
        Workflow workflow = parseWorkflow_FromYaml(yaml_content);
        if (workflow.workflow_id.empty()) {
            // Generate ID from filename if not specified
            std::filesystem::path file_path(yaml_file);
            workflow.workflow_id = file_path.stem().string() + "_workflow";
        }
        
        std::string workflow_id = workflow_engine_->create_workflow(workflow);
        if (!workflow_id.empty()) {
            logLoad_Event("INFO", "Successfully loaded workflow: " + workflow_id + " from " + yaml_file);
        } else {
            logLoad_Event("ERROR", "Failed to create workflow from: " + yaml_file);
        }
        
        return workflow_id;
        
    } catch (const std::exception& e) {
        logLoad_Event("ERROR", "Exception loading workflow from " + yaml_file + ": " + std::string(e.what()));
        return "";
    }
}

std::vector<std::string> WorkflowLoader::loadWorkflows_FromDirectory(const std::string& directory_path) {
    std::vector<std::string> loaded_workflows;
    
    try {
        if (!std::filesystem::exists(directory_path) || !std::filesystem::is_directory(directory_path)) {
            logLoad_Event("ERROR", "Directory not found: " + directory_path);
            return loaded_workflows;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
            if (entry.is_regular_file() && isYaml_File(entry.path().string())) {
                std::string workflow_id = loadWorkflow_FromFile(entry.path().string());
                if (!workflow_id.empty()) {
                    loaded_workflows.push_back(workflow_id);
                }
            }
        }
        
        logLoad_Event("INFO", "Loaded " + std::to_string(loaded_workflows.size()) + " workflows from " + directory_path);
        
    } catch (const std::exception& e) {
        logLoad_Event("ERROR", "Exception loading workflows from directory " + directory_path + ": " + std::string(e.what()));
    }
    
    return loaded_workflows;
}

std::string WorkflowLoader::loadSequential_WorkflowFromYaml(const std::string& yaml_content) {
    try {
        if (!validateWorkflow_Configuration(yaml_content)) {
            logLoad_Event("ERROR", "Invalid sequential workflow configuration");
            return "";
        }
        
        Workflow workflow = parseWorkflow_FromYaml(yaml_content);
        
        // Ensure it's configured as sequential
        workflow.type = WorkflowType::SEQUENTIAL;
        
        // Validate sequential workflow requirements
        if (workflow.steps.empty()) {
            logLoad_Event("ERROR", "Sequential workflow must have at least one step");
            return "";
        }
        
        // Auto-setup dependencies for sequential execution
        for (size_t i = 1; i < workflow.steps.size(); ++i) {
            if (workflow.steps[i].dependencies.empty()) {
                StepDependency dep;
                dep.step_id = workflow.steps[i-1].step_id;
                dep.condition = "success";
                dep.required = true;
                workflow.steps[i].dependencies.push_back(dep);
            }
        }
        
        std::string workflow_id = workflow_engine_->create_workflow(workflow);
        if (!workflow_id.empty()) {
            logLoad_Event("INFO", "Successfully loaded sequential workflow: " + workflow_id);
        } else {
            logLoad_Event("ERROR", "Failed to create sequential workflow");
        }
        
        return workflow_id;
        
    } catch (const std::exception& e) {
        logLoad_Event("ERROR", "Exception loading sequential workflow: " + std::string(e.what()));
        return "";
    }
}

int WorkflowLoader::autoLoad_DefaultWorkflows() {
    int loaded_count = 0;
    
    // List of default workflow files to auto-load
    std::vector<std::string> default_files = {
        "sequential.yaml",
        "workflow.yaml",
        "default_workflow.yaml"
    };
    
    // Search paths for workflow files
    std::vector<std::string> search_paths = {
        ".",
        "./workflows",
        "../workflows",
        "./configs",
        "../configs"
    };
    
    for (const auto& file_name : default_files) {
        for (const auto& search_path : search_paths) {
            std::string full_path = search_path + "/" + file_name;
            if (std::filesystem::exists(full_path)) {
                std::string workflow_id = loadWorkflow_FromFile(full_path);
                if (!workflow_id.empty()) {
                    loaded_count++;
                    logLoad_Event("INFO", "Auto-loaded default workflow: " + workflow_id);
                }
                break; // Found and processed this file, move to next
            }
        }
    }
    
    // Try to load from examples directory
    std::vector<std::string> example_paths = {
        "./examples",
        "../examples"
    };
    
    for (const auto& example_path : example_paths) {
        if (std::filesystem::exists(example_path)) {
            auto example_workflows = loadWorkflows_FromDirectory(example_path);
            loaded_count += example_workflows.size();
            break; // Only load from first found examples directory
        }
    }
    
    logLoad_Event("INFO", "Auto-loaded " + std::to_string(loaded_count) + " default workflows");
    return loaded_count;
}

bool WorkflowLoader::validateWorkflow_Configuration(const std::string& yaml_content) {
    try {
        YAML::Node yaml_node = YAML::Load(yaml_content);
        
        // Check required fields
        if (!yaml_node["id"] && !yaml_node["name"]) {
            logLoad_Event("ERROR", "Workflow must have either 'id' or 'name' field");
            return false;
        }
        
        if (!yaml_node["steps"]) {
            logLoad_Event("ERROR", "Workflow must have 'steps' field");
            return false;
        }
        
        // Convert YAML to JSON for easier processing
        nlohmann::json config_json = nlohmann::json::parse(yaml_content);
        
        // Validate steps
        if (!config_json["steps"].is_array() || config_json["steps"].empty()) {
            logLoad_Event("ERROR", "Workflow 'steps' must be a non-empty array");
            return false;
        }
        
        for (const auto& step_json : config_json["steps"]) {
            if (!validateStep_Configuration(step_json)) {
                return false;
            }
        }
        
        // Validate dependencies
        std::vector<WorkflowStep> temp_steps;
        for (const auto& step_json : config_json["steps"]) {
            temp_steps.push_back(parseStep_FromYaml(step_json));
        }
        
        if (!validateDependencies(temp_steps)) {
            logLoad_Event("ERROR", "Invalid step dependencies");
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        logLoad_Event("ERROR", "Validation exception: " + std::string(e.what()));
        return false;
    }
}

// Private helper methods

Workflow WorkflowLoader::parseWorkflow_FromYaml(const std::string& yaml_content) {
    YAML::Node yaml_node = YAML::Load(yaml_content);
    
    Workflow workflow;
    workflow.workflow_id = yaml_node["id"].as<std::string>("");
    workflow.name = yaml_node["name"].as<std::string>("");
    workflow.description = yaml_node["description"].as<std::string>("");
    
    // Parse workflow type
    std::string type_str = yaml_node["type"].as<std::string>("sequential");
    workflow.type = parseWorkflow_Type(type_str);
    
    // Parse global context
    if (yaml_node["global_context"]) {
        nlohmann::json global_context = nlohmann::json::parse(YAML::Dump(yaml_node["global_context"]));
        workflow.global_context = global_context;
    }
    
    // Parse steps
    if (yaml_node["steps"]) {
        for (const auto& step_node : yaml_node["steps"]) {
            nlohmann::json step_json = nlohmann::json::parse(YAML::Dump(step_node));
            workflow.steps.push_back(parseStep_FromYaml(step_json));
        }
    }
    
    // Parse settings
    if (yaml_node["settings"]) {
        auto settings = yaml_node["settings"];
        workflow.max_execution_time_seconds = settings["max_execution_time"].as<int>(300);
        workflow.max_concurrent_steps = settings["max_concurrent_steps"].as<int>(4);
        workflow.auto_cleanup = settings["auto_cleanup"].as<bool>(true);
        workflow.persist_state = settings["persist_state"].as<bool>(true);
    }
    
    // Parse error handling
    if (yaml_node["error_handling"]) {
        nlohmann::json error_json = nlohmann::json::parse(YAML::Dump(yaml_node["error_handling"]));
        workflow.error_handling = parseErrorHandling_FromYaml(error_json);
    }
    
    workflow.created_by = "workflow_loader";
    workflow.created_time = std::chrono::system_clock::now();
    
    return workflow;
}

WorkflowStep WorkflowLoader::parseStep_FromYaml(const nlohmann::json& step_json) {
    WorkflowStep step;
    
    step.step_id = step_json.value("id", "");
    step.name = step_json.value("name", "");
    step.description = step_json.value("description", "");
    step.agent_id = step_json.value("agent_id", "");
    step.function_name = step_json.value("function", "");
    step.parameters = step_json.value("parameters", nlohmann::json::object());
    step.timeout_seconds = step_json.value("timeout", 30);
    step.max_retries = step_json.value("max_retries", 3);
    step.retry_delay_seconds = step_json.value("retry_delay", 1);
    step.continue_on_error = step_json.value("continue_on_error", false);
    step.parallel_allowed = step_json.value("parallel_allowed", true);
    
    // Parse dependencies
    if (step_json.contains("depends_on")) {
        for (const auto& dep_id : step_json["depends_on"]) {
            StepDependency dep;
            if (dep_id.is_string()) {
                dep.step_id = dep_id.get<std::string>();
                dep.condition = "success";
                dep.required = true;
            } else if (dep_id.is_object()) {
                dep.step_id = dep_id.value("step_id", "");
                dep.condition = dep_id.value("condition", "success");
                dep.required = dep_id.value("required", true);
            }
            step.dependencies.push_back(dep);
        }
    }
    
    // Parse conditions
    if (step_json.contains("conditions")) {
        step.conditions = step_json["conditions"];
    }
    
    return step;
}

ErrorHandlingStrategy WorkflowLoader::parseErrorHandling_FromYaml(const nlohmann::json& error_json) {
    ErrorHandlingStrategy strategy;
    
    strategy.retry_on_failure = error_json.value("retry_on_failure", true);
    strategy.max_retries = error_json.value("max_retries", 3);
    strategy.retry_delay_seconds = error_json.value("retry_delay_seconds", 1);
    strategy.continue_on_error = error_json.value("continue_on_error", false);
    strategy.use_fallback_agent = error_json.value("use_fallback_agent", false);
    strategy.fallback_agent_id = error_json.value("fallback_agent_id", "");
    
    if (error_json.contains("fallback_parameters")) {
        strategy.fallback_parameters = error_json["fallback_parameters"];
    }
    
    return strategy;
}

WorkflowType WorkflowLoader::parseWorkflow_Type(const std::string& type_str) {
    if (type_str == "sequential") return WorkflowType::SEQUENTIAL;
    if (type_str == "parallel") return WorkflowType::PARALLEL;
    if (type_str == "pipeline") return WorkflowType::PIPELINE;
    if (type_str == "consensus") return WorkflowType::CONSENSUS;
    if (type_str == "conditional") return WorkflowType::CONDITIONAL;
    
    // Default to sequential
    return WorkflowType::SEQUENTIAL;
}

bool WorkflowLoader::validateStep_Configuration(const nlohmann::json& step_json) {
    // Check required fields
    if (!step_json.contains("id") || step_json["id"].get<std::string>().empty()) {
        logLoad_Event("ERROR", "Step must have non-empty 'id' field");
        return false;
    }
    
    if (!step_json.contains("agent_id") || step_json["agent_id"].get<std::string>().empty()) {
        logLoad_Event("ERROR", "Step must have non-empty 'agent_id' field");
        return false;
    }
    
    if (!step_json.contains("function") || step_json["function"].get<std::string>().empty()) {
        logLoad_Event("ERROR", "Step must have non-empty 'function' field");
        return false;
    }
    
    // Validate numeric fields
    if (step_json.contains("timeout") && step_json["timeout"].get<int>() <= 0) {
        logLoad_Event("ERROR", "Step timeout must be positive");
        return false;
    }
    
    if (step_json.contains("max_retries") && step_json["max_retries"].get<int>() < 0) {
        logLoad_Event("ERROR", "Step max_retries must be non-negative");
        return false;
    }
    
    return true;
}

bool WorkflowLoader::validateDependencies(const std::vector<WorkflowStep>& steps) {
    // Create step ID set for validation
    std::set<std::string> step_ids;
    for (const auto& step : steps) {
        step_ids.insert(step.step_id);
    }
    
    // Check that all dependencies exist
    for (const auto& step : steps) {
        for (const auto& dep : step.dependencies) {
            if (step_ids.find(dep.step_id) == step_ids.end()) {
                logLoad_Event("ERROR", "Step '" + step.step_id + "' depends on non-existent step '" + dep.step_id + "'");
                return false;
            }
        }
    }
    
    // Check for circular dependencies (simplified check)
    // TODO: Implement more comprehensive circular dependency detection
    
    return true;
}

std::string WorkflowLoader::readFile_Content(const std::string& file_path) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return "";
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
        
    } catch (const std::exception&) {
        return "";
    }
}

bool WorkflowLoader::isYaml_File(const std::string& file_path) {
    std::filesystem::path path(file_path);
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".yaml" || extension == ".yml";
}

void WorkflowLoader::logLoad_Event(const std::string& level, const std::string& message) {
    std::cout << "[WorkflowLoader] [" << level << "] " << message << std::endl;
}

} // namespace kolosal::agents
