#include "agent.hpp"
#include "model_interface.hpp"
#include "logger.hpp"
#ifdef BUILD_WITH_RETRIEVAL
#include "retrieval_manager.hpp"
#include "../include/functions/research.hpp"
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
    TRACE_FUNCTION();
    
    // Validate agent name
    if (name.empty()) {
        LOG_ERROR("Agent name cannot be empty");
        throw std::invalid_argument("Agent name cannot be empty");
    }
    
    LOG_DEBUG_F("Creating agent '%s' with ID: %s", name.c_str(), id_.c_str());
    
    // Set default system instruction
    system_instruction_ = "You are a helpful AI assistant. Be accurate, helpful, and professional in your responses.";
    
    // Initialize model interface with correct server URL
    LOG_DEBUG("Initializing model interface");
    model_interface_ = std::make_unique<ModelInterface>("http://127.0.0.1:8081");
    
    LOG_DEBUG("Setting up builtin functions");
    setup_builtin_functions();
    LOG_DEBUG("Setting up research brief functions");
    setup_research_brief_functions();
#ifdef BUILD_WITH_RETRIEVAL
    LOG_DEBUG("Setting up retrieval functions");
    setup_retrieval_functions();
    LOG_DEBUG("Setting up deep research functions");
    setup_deep_research_functions();
#endif
    
    LOG_INFO_F("Agent '%s' created successfully with %zu functions", name.c_str(), functions_.size());
}

bool Agent::start() {
    TRACE_FUNCTION();
    
    if (running_.load()) {
        LOG_DEBUG_F("Agent '%s' is already running", name_.c_str());
        return true;
    }
    
    running_.store(true);
    LOG_INFO_F("Agent '%s' (%s) started", name_.c_str(), id_.c_str());
    return true;
}

void Agent::stop() {
    TRACE_FUNCTION();
    
    if (!running_.load()) {
        LOG_DEBUG_F("Agent '%s' is already stopped", name_.c_str());
        return;
    }
    
    running_.store(false);
    LOG_INFO_F("Agent '%s' (%s) stopped", name_.c_str(), id_.c_str());
}

json Agent::execute_function(const std::string& function_name, const json& params) {
    TRACE_FUNCTION();
    SCOPED_TIMER("function_execution_" + function_name);
    
    if (!running_.load()) {
        LOG_ERROR_F("Agent '%s' is not running, cannot execute function '%s'", name_.c_str(), function_name.c_str());
        throw std::runtime_error("Agent is not running");
    }
    
    auto it = functions_.find(function_name);
    if (it == functions_.end()) {
        LOG_ERROR_F("Function '%s' not found in agent '%s'", function_name.c_str(), name_.c_str());
        throw std::runtime_error("Function '" + function_name + "' not found");
    }
    
    try {
        LOG_INFO_F("Agent '%s' executing function: %s", name_.c_str(), function_name.c_str());
        LOG_DEBUG_F("Function parameters: %s", params.dump().c_str());
        
        auto result = it->second(params);
        
        LOG_INFO_F("Function '%s' completed successfully", function_name.c_str());
        LOG_DEBUG_F("Function result size: %zu bytes", result.dump().size());
        
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR_F("Function '%s' failed: %s", function_name.c_str(), e.what());
        throw;
    }
}

void Agent::register_function(const std::string& name, std::function<json(const json&)> func) {
    TRACE_FUNCTION();
    
    functions_[name] = std::move(func);
    LOG_INFO_F("Function '%s' registered for agent '%s'", name.c_str(), name_.c_str());
}

void Agent::add_capability(const std::string& capability) {
    TRACE_FUNCTION();
    
    if (std::find(capabilities_.begin(), capabilities_.end(), capability) == capabilities_.end()) {
        capabilities_.push_back(capability);
        LOG_INFO_F("Capability '%s' added to agent '%s'", capability.c_str(), name_.c_str());
    } else {
        LOG_DEBUG_F("Capability '%s' already exists for agent '%s'", capability.c_str(), name_.c_str());
    }
}

void Agent::set_system_instruction(const std::string& instruction) {
    TRACE_FUNCTION();
    
    system_instruction_ = instruction;
    LOG_INFO_F("System instruction updated for agent '%s' (length: %zu)", name_.c_str(), instruction.length());
    LOG_DEBUG_F("System instruction content: %s", instruction.c_str());
}

void Agent::set_agent_specific_prompt(const std::string& prompt) {
    TRACE_FUNCTION();
    
    agent_specific_prompt_ = prompt;
    LOG_INFO_F("Agent-specific prompt updated for agent '%s' (length: %zu)", name_.c_str(), prompt.length());
    LOG_DEBUG_F("Agent-specific prompt content: %s", prompt.c_str());
}

void Agent::configure_models(const json& model_configs) {
    TRACE_FUNCTION();
    
    if (!model_interface_) {
        LOG_ERROR_F("Model interface not initialized for agent '%s'", name_.c_str());
        return;
    }
    
    try {
        // Pass model configurations to the model interface
        model_interface_->configure_models(model_configs);
        LOG_INFO_F("Model configurations loaded for agent '%s'", name_.c_str());
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to configure models for agent '%s': %s", name_.c_str(), e.what());
    }
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

json Agent::create_research_function_response(const std::string& function_name, const json& params, const std::string& task_description) {
    std::string model_name = params.value("model", "gemma3-1b");
    
    json response;
    response["agent"] = name_;
    response["function"] = function_name;
    response["timestamp"] = get_timestamp();
    response["model_used"] = model_name;
    
    try {
        if (!model_interface_->is_model_available(model_name)) {
            throw std::runtime_error("Model '" + model_name + "' is not available");
        }
        
        // Create a comprehensive prompt based on the parameters and function
        std::string prompt = task_description + "\n\nInput parameters:\n" + params.dump(2) + 
                           "\n\nPlease provide a detailed response for this " + function_name + " task.";
        
        std::string ai_result = model_interface_->chat_with_model(
            model_name,
            prompt,
            get_combined_prompt() + "\n\nYou are an expert researcher. Provide thorough, accurate, and well-structured responses."
        );
        
        response["result"] = ai_result;
        response["status"] = "success";
        
    } catch (const std::exception& e) {
        response["error"] = e.what();
        response["status"] = "error";
        response["result"] = "Function " + function_name + " failed: " + std::string(e.what());
    }
    
    return response;
}

void Agent::setup_builtin_functions() {
    TRACE_FUNCTION();
    
    LOG_DEBUG("Setting up builtin functions");
    
    // Basic chat function with model parameter support
    register_function("chat", [this](const json& params) -> json {
        SCOPED_TIMER("chat_function");
        
        std::string message = params.value("message", "");
        if (message.empty()) {
            LOG_ERROR("Missing 'message' parameter in chat function");
            throw std::runtime_error("Missing 'message' parameter");
        }
        
        std::string model_name = params.value("model", "");
        if (model_name.empty()) {
            LOG_ERROR("Missing 'model' parameter in chat function");
            throw std::runtime_error("Missing 'model' parameter. Please specify which model to use.");
        }
        
        LOG_DEBUG_F("Chat function called with message: %s, model: %s", message.c_str(), model_name.c_str());
        
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
            LOG_DEBUG_F("Checking model availability: %s", model_name.c_str());
            if (!model_interface_->is_model_available(model_name)) {
                // Get available models for better error message
                json available_models = model_interface_->get_available_models();
                std::string error_msg = "Model '" + model_name + "' is not available. ";
                
                if (available_models.is_array() && !available_models.empty()) {
                    error_msg += "Available models: ";
                    for (const auto& model : available_models) {
                        if (model.contains("model_id")) {
                            error_msg += model["model_id"].get<std::string>() + " ";
                        }
                    }
                } else {
                    error_msg += "No models are currently available.";
                }
                
                LOG_WARN_F("Model not available: %s", error_msg.c_str());
                
                // Instead of throwing an error, provide a fallback response
                response["response"] = "I apologize, but the specified model '" + model_name + "' is not currently available. " + error_msg;
                response["status"] = "fallback";
                response["error"] = error_msg;
                return response;
            }
            
            std::string ai_response;
            
            if (!context.empty()) {
                LOG_DEBUG("Using enhanced context for AI response");
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
                LOG_DEBUG("Direct chat with model");
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
            LOG_DEBUG_F("Chat response generated successfully (length: %zu)", ai_response.length());
            
        } catch (const std::exception& e) {
            LOG_ERROR_F("Chat function error: %s", e.what());
            // Improved fallback response if model communication fails
            std::string fallback_response = "I apologize, but I'm currently unable to connect to the specified model '" + model_name + "'. ";
            fallback_response += "Error: " + std::string(e.what()) + "\n\n";
            
            if (!context.empty()) {
                fallback_response += "However, I can provide information based on the tool execution results:\n" + context;
            } else {
                fallback_response += "You requested: " + message + "\n";
                fallback_response += "While I cannot process this with the AI model right now, please check if the model is loaded and available, or try using a different model.";
            }
            
            response["response"] = fallback_response;
            response["status"] = "fallback_success";  // Changed to indicate this is a fallback but still functional
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
    
    // Research function for agents that don't have retrieval capabilities
    register_function("research", [this](const json& params) -> json {
        std::string query = params.value("query", "");
        if (query.empty()) {
            throw std::runtime_error("Missing 'query' parameter");
        }
        
        std::string depth = params.value("depth", "basic");
        std::string model_name = params.value("model", "gemma3-1b");
        
        json response;
        response["agent"] = name_;
        response["query"] = query;
        response["depth"] = depth;
        response["model_used"] = model_name;
        response["timestamp"] = get_timestamp();
        
        try {
            // Check if model is available
            if (!model_interface_->is_model_available(model_name)) {
                throw std::runtime_error("Model '" + model_name + "' is not available");
            }
            
            // Create research prompt based on depth
            std::string research_prompt;
            if (depth == "basic") {
                research_prompt = "Please provide a basic overview and key facts about: " + query;
            } else if (depth == "detailed") {
                research_prompt = "Please provide a detailed analysis and comprehensive information about: " + query + 
                                ". Include key facts, context, implications, and relevant details.";
            } else if (depth == "comprehensive") {
                research_prompt = "Please provide a comprehensive research analysis on: " + query + 
                                ". Include detailed background, current state, key findings, different perspectives, " +
                                "implications, and future considerations. Be thorough and analytical.";
            } else {
                research_prompt = "Please research and provide information about: " + query;
            }
            
            std::string research_result = model_interface_->chat_with_model(
                model_name,
                research_prompt,
                get_combined_prompt() + "\n\nYou are conducting research. Provide accurate, well-structured, and informative responses."
            );
            
            response["research_result"] = research_result;
            response["status"] = "success";
            response["depth_level"] = depth;
            
        } catch (const std::exception& e) {
            response["error"] = e.what();
            response["status"] = "error";
            response["research_result"] = "Research failed: " + std::string(e.what());
        }
        
        return response;
    });
    
    // Add missing deep research functions for compatibility
    register_function("plan_research", [this](const json& params) -> json {
        std::string query = params.value("query", "");
        if (query.empty()) {
            throw std::runtime_error("Missing 'query' parameter");
        }
        
        std::string research_scope = params.value("research_scope", "comprehensive");
        std::string depth_level = params.value("depth_level", "advanced");
        std::string model_name = params.value("model", "gemma3-1b");
        
        json response;
        response["agent"] = name_;
        response["query"] = query;
        response["research_scope"] = research_scope;
        response["depth_level"] = depth_level;
        response["timestamp"] = get_timestamp();
        
        try {
            if (!model_interface_->is_model_available(model_name)) {
                throw std::runtime_error("Model '" + model_name + "' is not available");
            }
            
            std::string planning_prompt = "Create a comprehensive research plan for the following query: " + query + 
                                        "\n\nResearch scope: " + research_scope + 
                                        "\nDepth level: " + depth_level + 
                                        "\n\nPlease provide:\n1. Research objectives\n2. Key areas to investigate\n3. Methodology\n4. Expected outcomes\n5. Timeline estimates";
            
            std::string plan_result = model_interface_->chat_with_model(
                model_name,
                planning_prompt,
                get_combined_prompt() + "\n\nYou are a research planning expert. Create detailed, structured research plans."
            );
            
            response["research_plan"] = plan_result;
            response["status"] = "success";
            
        } catch (const std::exception& e) {
            response["error"] = e.what();
            response["status"] = "error";
            response["research_plan"] = "Research planning failed: " + std::string(e.what());
        }
        
        return response;
    });
    
    // Add more research functions for deep research workflow compatibility
    register_function("targeted_research", [this](const json& params) -> json {
        return create_research_function_response("targeted_research", params, 
            "Conduct targeted research on specific gaps and topics");
    });
    
    register_function("verify_facts", [this](const json& params) -> json {
        return create_research_function_response("verify_facts", params,
            "Verify and cross-check the provided facts and findings");
    });
    
    register_function("synthesize_research", [this](const json& params) -> json {
        return create_research_function_response("synthesize_research", params,
            "Synthesize and integrate research data from multiple sources");
    });
    
    register_function("generate_research_report", [this](const json& params) -> json {
        return create_research_function_response("generate_research_report", params,
            "Generate a comprehensive research report with citations");
    });
    
    register_function("internet_search", [this](const json& params) -> json {
        std::string query = params.value("query", "");
        if (query.empty()) {
            throw std::runtime_error("Missing 'query' parameter");
        }
        
        int results = params.value("results", 10);
        std::string language = params.value("language", "en");
        std::string model_name = params.value("model", "gemma3-1b");
        
        json response;
        response["agent"] = name_;
        response["query"] = query;
        response["results_requested"] = results;
        response["language"] = language;
        response["timestamp"] = get_timestamp();
        
        try {
            if (!model_interface_->is_model_available(model_name)) {
                throw std::runtime_error("Model '" + model_name + "' is not available");
            }
            
            // Simulate internet search with AI-generated content
            std::string search_prompt = "Based on your knowledge, provide comprehensive search results for the query: " + query + 
                                      "\n\nPlease structure your response as if these were search results from the internet, " +
                                      "including relevant information, facts, and insights about this topic. " +
                                      "Provide up to " + std::to_string(results) + " relevant pieces of information.";
            
            std::string search_results = model_interface_->chat_with_model(
                model_name,
                search_prompt,
                get_combined_prompt() + "\n\nYou are simulating internet search results. Provide comprehensive, factual information."
            );
            
            response["search_results"] = search_results;
            response["status"] = "success";
            response["note"] = "Simulated search results based on AI knowledge";
            
        } catch (const std::exception& e) {
            response["error"] = e.what();
            response["status"] = "error";
            response["search_results"] = "Search failed: " + std::string(e.what());
        }
        
        return response;
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
            LOG_WARN("Retrieval system not available - skipping document addition");
            json response;
            response["status"] = "skipped";
            response["message"] = "Retrieval system not available";
            response["reason"] = "Vector database (Qdrant) not running";
            return response;
        }
        
        try {
            return retrieval_manager_->add_document(params);
        } catch (const std::exception& e) {
            LOG_WARN_F("Failed to add document: %s", e.what());
            json response;
            response["status"] = "failed";
            response["message"] = e.what();
            response["reason"] = "Document service initialization failed";
            return response;
        }
    });
    
    register_function("search_documents", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            LOG_WARN("Retrieval system not available - skipping document search");
            json response;
            response["status"] = "skipped";
            response["message"] = "Retrieval system not available";
            response["reason"] = "Vector database (Qdrant) not running";
            response["results"] = json::array();
            return response;
        }
        
        try {
            return retrieval_manager_->search_documents(params);
        } catch (const std::exception& e) {
            LOG_WARN_F("Failed to search documents: %s", e.what());
            json response;
            response["status"] = "failed";
            response["message"] = e.what();
            response["reason"] = "Document service initialization failed";
            response["results"] = json::array();
            return response;
        }
    });
    
    register_function("list_documents", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            LOG_WARN("Retrieval system not available - skipping document listing");
            json response;
            response["status"] = "skipped";
            response["message"] = "Retrieval system not available";
            response["reason"] = "Vector database (Qdrant) not running";
            response["documents"] = json::array();
            return response;
        }
        
        try {
            return retrieval_manager_->list_documents(params);
        } catch (const std::exception& e) {
            LOG_WARN_F("Failed to list documents: %s", e.what());
            json response;
            response["status"] = "failed";
            response["message"] = e.what();
            response["reason"] = "Document service initialization failed";
            response["documents"] = json::array();
            return response;
        }
    });
    
    register_function("remove_document", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            LOG_WARN("Retrieval system not available - skipping document removal");
            json response;
            response["status"] = "skipped";
            response["message"] = "Retrieval system not available";
            response["reason"] = "Vector database (Qdrant) not running";
            return response;
        }
        
        try {
            return retrieval_manager_->remove_document(params);
        } catch (const std::exception& e) {
            LOG_WARN_F("Failed to remove document: %s", e.what());
            json response;
            response["status"] = "failed";
            response["message"] = e.what();
            response["reason"] = "Document service initialization failed";
            return response;
        }
    });
    
    // Search functions
    register_function("internet_search", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            json fallback_response;
            fallback_response["status"] = "unavailable";
            fallback_response["message"] = "Search system not available - retrieval manager not initialized";
            fallback_response["query"] = params.value("query", "");
            fallback_response["results"] = json::array();
            fallback_response["suggestions"] = json::array({
                "Verify that the Kolosal server is running",
                "Check retrieval system configuration",
                "Use alternative research methods"
            });
            return fallback_response;
        }
        
        try {
            auto result = retrieval_manager_->internet_search(params);
            return result;
        } catch (const std::exception& e) {
            LOG_WARN_F("Internet search failed, returning graceful fallback: %s", e.what());
            json fallback_response;
            fallback_response["status"] = "error";
            fallback_response["message"] = std::string("Internet search failed: ") + e.what();
            fallback_response["query"] = params.value("query", "");
            fallback_response["results"] = json::array();
            fallback_response["suggestions"] = json::array({
                "Try rephrasing your search query",
                "Check if the search service is available",
                "Use document management functions for local searches"
            });
            return fallback_response;
        }
    });
    
    register_function("research", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            json fallback_response;
            fallback_response["status"] = "unavailable";
            fallback_response["message"] = "Retrieval system not available";
            fallback_response["query"] = params.value("query", "");
            fallback_response["results"] = json::array();
            fallback_response["suggestions"] = json::array({
                "Verify that the Kolosal server is running",
                "Check retrieval system configuration",
                "Use alternative research methods"
            });
            return fallback_response;
        }
        
        try {
            return retrieval_manager_->combined_search(params);
        } catch (const std::exception& e) {
            LOG_WARN_F("Research function failed, returning graceful fallback: %s", e.what());
            json fallback_response;
            fallback_response["status"] = "error";
            fallback_response["message"] = std::string("Research failed: ") + e.what();
            fallback_response["query"] = params.value("query", "");
            fallback_response["results"] = json::array();
            fallback_response["suggestions"] = json::array({
                "Try using document management functions",
                "Check if search services are available",
                "Consider breaking down the research into smaller queries"
            });
            return fallback_response;
        }
    });
    
    // Enhanced retrieval function that combines document search with AI-generated answers
    register_function("retrieve_and_answer", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            throw std::runtime_error("Retrieval system not available");
        }
        
        std::string question = params.value("question", "");
        if (question.empty()) {
            throw std::runtime_error("Missing 'question' parameter");
        }
        
        std::string model_name = params.value("model", "");
        if (model_name.empty()) {
            throw std::runtime_error("Missing 'model' parameter");
        }
        
        int max_docs = params.value("max_docs", 5);
        bool include_sources = params.value("include_sources", true);
        
        json result;
        result["question"] = question;
        result["model_used"] = model_name;
        result["timestamp"] = get_timestamp();
        
        try {
            // Search for relevant documents
            json search_params;
            search_params["query"] = question;
            search_params["limit"] = max_docs;
            
            json search_results = retrieval_manager_->search_documents(search_params);
            result["retrieved_documents"] = search_results;
            
            // Build context from retrieved documents
            std::string context = "Based on the following retrieved documents, please answer the user's question:\n\n";
            
            if (search_results.contains("results") && search_results["results"].is_array()) {
                int doc_count = 0;
                for (const auto& doc : search_results["results"]) {
                    if (doc.contains("content")) {
                        context += "Document " + std::to_string(++doc_count) + ":\n";
                        context += doc["content"].get<std::string>() + "\n\n";
                        
                        if (include_sources && doc.contains("metadata")) {
                            auto metadata = doc["metadata"];
                            if (metadata.contains("title")) {
                                context += "Source: " + metadata["title"].get<std::string>() + "\n";
                            }
                            if (metadata.contains("author")) {
                                context += "Author: " + metadata["author"].get<std::string>() + "\n";
                            }
                        }
                        context += "---\n\n";
                    }
                }
            }
            
            context += "Question: " + question + "\n\n";
            context += "Please provide a comprehensive answer based on the retrieved documents above. ";
            if (include_sources) {
                context += "Include references to the sources where applicable.";
            }
            
            // Generate AI response using the model
            if (!model_interface_->is_model_available(model_name)) {
                throw std::runtime_error("Model '" + model_name + "' is not available");
            }
            
            std::string ai_response = model_interface_->chat_with_model(
                model_name,
                context,
                "You are an expert information analyst. Provide accurate, well-structured answers based on the provided documents."
            );
            
            result["answer"] = ai_response;
            result["context_length"] = context.length();
            result["documents_used"] = max_docs;
            result["sources_included"] = include_sources;
            result["status"] = "success";
            
        } catch (const std::exception& e) {
            result["error"] = e.what();
            result["status"] = "error";
            result["answer"] = "I apologize, but I encountered an error while retrieving and processing the information: " + std::string(e.what());
        }
        
        return result;
    });
    
    // Enhanced document analysis function
    register_function("analyze_document", [this](const json& params) -> json {
        std::string content = params.value("content", "");
        if (content.empty()) {
            throw std::runtime_error("Missing 'content' parameter");
        }
        
        return RetrievalFunctions::analyze_document_structure(content);
    });
    
    // Batch document processing
    register_function("batch_add_documents", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            throw std::runtime_error("Retrieval system not available");
        }
        
        if (!params.contains("documents")) {
            throw std::runtime_error("Missing 'documents' parameter");
        }
        
        return RetrievalFunctions::batch_add_documents(params["documents"]);
    });
    
    // Document clustering and organization
    register_function("organize_documents", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            throw std::runtime_error("Retrieval system not available");
        }
        
        return RetrievalFunctions::organize_documents_by_similarity(params);
    });
    
    // Knowledge graph extraction
    register_function("extract_knowledge_graph", [this](const json& params) -> json {
        if (!retrieval_manager_ || !retrieval_manager_->is_available()) {
            throw std::runtime_error("Retrieval system not available");
        }
        
        if (!params.contains("documents")) {
            throw std::runtime_error("Missing 'documents' parameter");
        }
        
        return RetrievalFunctions::extract_knowledge_graph(params["documents"]);
    });
    
    // Search suggestions
    register_function("get_search_suggestions", [this](const json& params) -> json {
        std::string query = params.value("query", "");
        if (query.empty()) {
            throw std::runtime_error("Missing 'query' parameter");
        }
        
        auto suggestions = RetrievalFunctions::generate_search_suggestions(query);
        
        json result;
        result["query"] = query;
        result["suggestions"] = suggestions;
        result["count"] = suggestions.size();
        
        return result;
    });
}

void Agent::configure_retrieval(const json& config) {
    if (!config.contains("retrieval") || config["retrieval"].is_null()) {
        // If no retrieval config provided, use defaults if agent has retrieval capabilities
        bool has_retrieval_capabilities = false;
        for (const auto& capability : capabilities_) {
            if (capability == "retrieval" || capability == "document_management" || 
                capability == "semantic_search" || capability == "knowledge_base" || 
                capability == "vector_search") {
                has_retrieval_capabilities = true;
                break;
            }
        }
        
        if (has_retrieval_capabilities) {
            // Use default configuration for retrieval
            RetrievalManager::Config new_config; // Uses defaults from header
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
        return;
    }
    
    auto retrieval_config = config["retrieval"];
    RetrievalManager::Config new_config;
    
    // Parse configuration - only use fields that exist in new Config
    if (retrieval_config.contains("server_url")) {
        std::string url = retrieval_config["server_url"];
        if (!url.empty()) {
            new_config.server_url = url;
        }
        // If empty, keep the default value from new_config constructor
    }
    if (retrieval_config.contains("timeout_seconds")) {
        new_config.timeout_seconds = retrieval_config["timeout_seconds"];
    }
    if (retrieval_config.contains("max_retries")) {
        new_config.max_retries = retrieval_config["max_retries"];
    }
    if (retrieval_config.contains("search_enabled")) {
        new_config.search_enabled = retrieval_config["search_enabled"];
    }
    if (retrieval_config.contains("max_results")) {
        new_config.max_results = retrieval_config["max_results"];
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

void Agent::setup_deep_research_functions() {
    TRACE_FUNCTION();
    
    LOG_DEBUG("Setting up deep research functions");
    
    // Research planning function
    register_function("plan_research", [this](const json& params) -> json {
        try {
            auto plan = DeepResearchFunctions::plan_research(params);
            
            json result;
            result["query"] = plan.query;
            result["scope"] = plan.scope;
            result["depth_level"] = plan.depth_level;
            result["research_phases"] = plan.research_phases;
            result["key_questions"] = plan.key_questions;
            result["required_sources"] = plan.required_sources;
            result["metadata"] = plan.metadata;
            result["status"] = "completed";
            
            return result;
        } catch (const std::exception& e) {
            json error_result;
            error_result["error"] = e.what();
            error_result["status"] = "failed";
            return error_result;
        }
    });
    
    // Targeted research function
    register_function("targeted_research", [this](const json& params) -> json {
        try {
            return DeepResearchFunctions::targeted_research(params);
        } catch (const std::exception& e) {
            json error_result;
            error_result["error"] = e.what();
            error_result["status"] = "failed";
            return error_result;
        }
    });
    
    // Fact verification function
    register_function("verify_facts", [this](const json& params) -> json {
        try {
            return DeepResearchFunctions::verify_facts(params);
        } catch (const std::exception& e) {
            json error_result;
            error_result["error"] = e.what();
            error_result["status"] = "failed";
            return error_result;
        }
    });
    
    // Research synthesis function
    register_function("synthesize_research", [this](const json& params) -> json {
        try {
            auto synthesis = DeepResearchFunctions::synthesize_research(params);
            
            json result;
            result["summary"] = synthesis.summary;
            result["key_insights"] = synthesis.key_insights;
            result["research_gaps"] = synthesis.research_gaps;
            result["conflicting_information"] = synthesis.conflicting_information;
            result["metadata"] = synthesis.metadata;
            result["status"] = "completed";
            
            return result;
        } catch (const std::exception& e) {
            json error_result;
            error_result["error"] = e.what();
            error_result["status"] = "failed";
            return error_result;
        }
    });
    
    // Research report generation function
    register_function("generate_research_report", [this](const json& params) -> json {
        try {
            return DeepResearchFunctions::generate_research_report(params);
        } catch (const std::exception& e) {
            json error_result;
            error_result["error"] = e.what();
            error_result["status"] = "failed";
            return error_result;
        }
    });
    
    // Cross-reference search function
    register_function("cross_reference_search", [this](const json& params) -> json {
        try {
            return DeepResearchFunctions::cross_reference_search(params);
        } catch (const std::exception& e) {
            json error_result;
            error_result["error"] = e.what();
            error_result["status"] = "failed";
            return error_result;
        }
    });
    
    // Iterative search refinement function
    register_function("iterative_search_refinement", [this](const json& params) -> json {
        try {
            return DeepResearchFunctions::iterative_search_refinement(params);
        } catch (const std::exception& e) {
            json error_result;
            error_result["error"] = e.what();
            error_result["status"] = "failed";
            return error_result;
        }
    });
    
    // Source credibility analysis function
    register_function("source_credibility_analysis", [this](const json& params) -> json {
        try {
            return DeepResearchFunctions::source_credibility_analysis(params);
        } catch (const std::exception& e) {
            json error_result;
            error_result["error"] = e.what();
            error_result["status"] = "failed";
            return error_result;
        }
    });
    
    // Add deep research capabilities
    add_capability("deep_research");
    add_capability("iterative_search");
    add_capability("fact_verification");
    add_capability("research_planning");
    add_capability("synthesis");
    
    LOG_INFO("Deep research functions registered successfully");
}
#endif
