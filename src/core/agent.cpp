#include "../include/agent.hpp"
#include "../include/model_interface.hpp"
#ifdef BUILD_WITH_RETRIEVAL
#include "../include/retrieval_manager.hpp"
#endif
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>

#ifdef _WIN32
#include <rpc.h>
#pragma comment(lib, "rpcrt4.lib")
#else
#include <uuid/uuid.h>
#endif

namespace {
    std::string generate_uuid() {
#ifdef _WIN32
        UUID uuid;
        UuidCreate(&uuid);
        char* str;
        UuidToStringA(&uuid, (RPC_CSTR*)&str);
        std::string result(str);
        RpcStringFreeA((RPC_CSTR*)&str);
        return result;
#else
        uuid_t uuid;
        uuid_generate_random(uuid);
        char uuid_str[37];
        uuid_unparse(uuid, uuid_str);
        return std::string(uuid_str);
#endif
    }
    
    std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
}

Agent::Agent(const std::string& name) 
    : id_(generate_uuid()), name_(name) {
    
    // Validate agent name
    if (name.empty()) {
        throw std::invalid_argument("Agent name cannot be empty");
    }
    
    // Set default system instruction
    system_instruction_ = "You are a helpful AI assistant. Be accurate, helpful, and professional in your responses.";
    
    // Initialize model interface
    model_interface_ = std::make_unique<ModelInterface>();
    
    setup_builtin_functions();
#ifdef BUILD_WITH_RETRIEVAL
    setup_retrieval_functions();
#endif
}

bool Agent::start() {
    if (running_.load()) {
        return true;
    }
    
    running_.store(true);
    std::cout << "[" << get_timestamp() << "] Agent '" << name_ << "' (" << id_ << ") started\n";
    return true;
}

void Agent::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_.store(false);
    std::cout << "[" << get_timestamp() << "] Agent '" << name_ << "' (" << id_ << ") stopped\n";
}

json Agent::execute_function(const std::string& function_name, const json& params) {
    if (!running_.load()) {
        throw std::runtime_error("Agent is not running");
    }
    
    auto it = functions_.find(function_name);
    if (it == functions_.end()) {
        throw std::runtime_error("Function '" + function_name + "' not found");
    }
    
    try {
        std::cout << "[" << get_timestamp() << "] Agent '" << name_ << "' executing function: " << function_name << "\n";
        auto result = it->second(params);
        std::cout << "[" << get_timestamp() << "] Function '" << function_name << "' completed successfully\n";
        return result;
    } catch (const std::exception& e) {
        std::cout << "[" << get_timestamp() << "] Function '" << function_name << "' failed: " << e.what() << "\n";
        throw;
    }
}

void Agent::register_function(const std::string& name, std::function<json(const json&)> func) {
    functions_[name] = std::move(func);
    std::cout << "[" << get_timestamp() << "] Function '" << name << "' registered for agent '" << name_ << "'\n";
}

void Agent::add_capability(const std::string& capability) {
    if (std::find(capabilities_.begin(), capabilities_.end(), capability) == capabilities_.end()) {
        capabilities_.push_back(capability);
        std::cout << "[" << get_timestamp() << "] Capability '" << capability << "' added to agent '" << name_ << "'\n";
    }
}

void Agent::set_system_instruction(const std::string& instruction) {
    system_instruction_ = instruction;
    std::cout << "[" << get_timestamp() << "] System instruction updated for agent '" << name_ << "'\n";
}

void Agent::set_agent_specific_prompt(const std::string& prompt) {
    agent_specific_prompt_ = prompt;
    std::cout << "[" << get_timestamp() << "] Agent-specific prompt updated for agent '" << name_ << "'\n";
}

std::string Agent::get_combined_prompt() const {
    std::string combined = system_instruction_;
    
    if (!agent_specific_prompt_.empty()) {
        combined += "\n\nRole-specific instructions:\n" + agent_specific_prompt_;
    }
    
    return combined;
}

json Agent::get_info() const {
    json info;
    info["id"] = id_;
    info["name"] = name_;
    info["running"] = running_.load();
    info["capabilities"] = capabilities_;
    info["system_instruction"] = system_instruction_;
    info["agent_specific_prompt"] = agent_specific_prompt_;
    
    json available_functions = json::array();
    for (const auto& [func_name, _] : functions_) {
        available_functions.push_back(func_name);
    }
    info["functions"] = available_functions;
    info["created_at"] = get_timestamp();
    
    return info;
}

void Agent::setup_builtin_functions() {
    // Basic chat function with model parameter support
    register_function("chat", [this](const json& params) -> json {
        std::string message = params.value("message", "");
        if (message.empty()) {
            throw std::runtime_error("Missing 'message' parameter");
        }
        
        std::string model_name = params.value("model", "");
        if (model_name.empty()) {
            throw std::runtime_error("Missing 'model' parameter. Please specify which model to use.");
        }
        
        // Check if context from tool execution is provided
        std::string context = params.value("context", "");
        json tool_results = params.value("tool_results", json::object());
        
        json response;
        response["agent"] = name_;
        response["timestamp"] = get_timestamp();
        response["model_used"] = model_name;
        response["system_prompt"] = get_combined_prompt();
        
        try {
            // Check if model is available
            if (!model_interface_->is_model_available(model_name)) {
                // Get available models for better error message
                json available_models = model_interface_->get_available_models();
                std::string error_msg = "Model '" + model_name + "' is not available. ";
                
                if (available_models.contains("models") && !available_models["models"].empty()) {
                    error_msg += "Available models: ";
                    for (const auto& model : available_models["models"]) {
                        if (model.contains("model_id") && model.value("available", false)) {
                            error_msg += model["model_id"].get<std::string>() + " ";
                        }
                    }
                }
                
                throw std::runtime_error(error_msg);
            }
            
            std::string ai_response;
            
            if (!context.empty()) {
                // Enhanced response with tool context
                std::string enhanced_prompt = "Based on the following tool execution results, please provide a comprehensive response to the user's message.\n\n";
                enhanced_prompt += "Tool Results:\n" + context + "\n\n";
                enhanced_prompt += "User Message: " + message + "\n\n";
                enhanced_prompt += "Please analyze the tool results and provide a helpful, informative response.";
                
                ai_response = model_interface_->chat_with_model(
                    model_name, 
                    enhanced_prompt, 
                    get_combined_prompt()
                );
                
                response["context_used"] = true;
                response["tool_results_summary"] = tool_results;
            } else {
                // Direct chat with model
                ai_response = model_interface_->chat_with_model(
                    model_name, 
                    message, 
                    get_combined_prompt()
                );
                
                response["context_used"] = false;
            }
            
            response["response"] = ai_response;
            response["status"] = "success";
            
        } catch (const std::exception& e) {
            // Fallback response if model communication fails
            std::string fallback_response = "I apologize, but I'm currently unable to connect to the specified model '" + model_name + "'. ";
            fallback_response += "Error: " + std::string(e.what()) + "\n\n";
            
            if (!context.empty()) {
                fallback_response += "However, I can provide information based on the tool execution results:\n" + context;
            } else {
                fallback_response += "Please check if the model is loaded and available, or try using a different model.";
            }
            
            response["response"] = fallback_response;
            response["status"] = "error";
            response["error"] = e.what();
        }
        
        return response;
    });
    
    // Analysis function with optional AI assistance
    register_function("analyze", [this](const json& params) -> json {
        std::string text = params.value("text", "");
        if (text.empty()) {
            throw std::runtime_error("Missing 'text' parameter");
        }
        
        std::string model_name = params.value("model", "");
        
        json analysis;
        analysis["agent"] = name_;
        analysis["text_length"] = text.length();
        analysis["word_count"] = std::count(text.begin(), text.end(), ' ') + 1;
        analysis["char_count"] = text.length();
        analysis["analysis_time"] = get_timestamp();
        
        // Basic analysis
        analysis["basic_stats"] = {
            {"characters", text.length()},
            {"words", std::count(text.begin(), text.end(), ' ') + 1},
            {"lines", std::count(text.begin(), text.end(), '\n') + 1}
        };
        
        // If model is specified, add AI-powered analysis
        if (!model_name.empty()) {
            try {
                if (model_interface_->is_model_available(model_name)) {
                    std::string ai_prompt = "Please analyze the following text and provide insights about its content, structure, tone, and key themes:\n\n" + text;
                    std::string ai_analysis = model_interface_->chat_with_model(
                        model_name, 
                        ai_prompt, 
                        "You are an expert text analyst. Provide comprehensive, structured analysis."
                    );
                    
                    analysis["ai_analysis"] = ai_analysis;
                    analysis["model_used"] = model_name;
                    analysis["analysis_type"] = "enhanced";
                } else {
                    analysis["ai_analysis"] = "Model '" + model_name + "' not available for enhanced analysis";
                    analysis["analysis_type"] = "basic";
                }
            } catch (const std::exception& e) {
                analysis["ai_analysis_error"] = e.what();
                analysis["analysis_type"] = "basic";
            }
        } else {
            analysis["analysis_type"] = "basic";
        }
        
        analysis["summary"] = "Text analysis completed by " + name_;
        return analysis;
    });
    
    // Echo function for testing
    register_function("echo", [this](const json& params) -> json {
        json response;
        response["agent"] = name_;
        response["echo"] = params;
        response["timestamp"] = get_timestamp();
        return response;
    });
    
    // Status function
    register_function("status", [this](const json& params) -> json {
        json status;
        status["agent"] = name_;
        status["id"] = id_;
        status["running"] = running_.load();
        status["capabilities"] = capabilities_;
        status["function_count"] = functions_.size();
        status["timestamp"] = get_timestamp();
        
#ifdef BUILD_WITH_RETRIEVAL
        // Add retrieval status if available
        if (retrieval_manager_) {
            status["retrieval"] = retrieval_manager_->get_status();
        }
#endif
        
        return status;
    });
}

#ifdef BUILD_WITH_RETRIEVAL
void Agent::setup_retrieval_functions() {
    // Initialize with default config (will be overridden by configure_retrieval)
    RetrievalManager::Config config;
    retrieval_manager_ = std::make_unique<RetrievalManager>(config);
    
    // Document management functions
    register_function("add_document", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            throw std::runtime_error("Retrieval system not available");
        }
        return retrieval_manager_->add_document(params);
    });
    
    register_function("search_documents", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            throw std::runtime_error("Retrieval system not available");
        }
        return retrieval_manager_->search_documents(params);
    });
    
    register_function("list_documents", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            throw std::runtime_error("Retrieval system not available");
        }
        return retrieval_manager_->list_documents(params);
    });
    
    register_function("remove_document", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            throw std::runtime_error("Retrieval system not available");
        }
        return retrieval_manager_->remove_document(params);
    });
    
    // Search functions
    register_function("internet_search", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            throw std::runtime_error("Search system not available");
        }
        return retrieval_manager_->internet_search(params);
    });
    
    register_function("research", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            throw std::runtime_error("Retrieval system not available");
        }
        return retrieval_manager_->combined_search(params);
    });
}

void Agent::configure_retrieval(const json& config) {
    if (!config.contains("retrieval")) {
        return;
    }
    
    auto retrieval_config = config["retrieval"];
    RetrievalManager::Config new_config;
    
    // Parse configuration
    if (retrieval_config.contains("vector_db_type")) {
        new_config.vector_db_type = retrieval_config["vector_db_type"];
    }
    if (retrieval_config.contains("db_host")) {
        new_config.db_host = retrieval_config["db_host"];
    }
    if (retrieval_config.contains("db_port")) {
        new_config.db_port = retrieval_config["db_port"];
    }
    if (retrieval_config.contains("search_enabled")) {
        new_config.search_enabled = retrieval_config["search_enabled"];
    }
    if (retrieval_config.contains("searxng_url")) {
        new_config.searxng_url = retrieval_config["searxng_url"];
    }
    
    // Recreate retrieval manager with new config
    retrieval_manager_ = std::make_unique<RetrievalManager>(new_config);
    
    // Add capabilities based on what's available
    if (retrieval_manager_->is_available()) {
        add_capability("document_management");
        add_capability("semantic_search");
        
        if (new_config.search_enabled) {
            add_capability("internet_search");
            add_capability("research");
        }
    }
}
#endif
