#include "../include/agent.hpp"
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
    : id_(generate_uuid()), name_(name.empty() ? "Agent-" + id_.substr(0, 8) : name) {
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

json Agent::get_info() const {
    json info;
    info["id"] = id_;
    info["name"] = name_;
    info["running"] = running_.load();
    info["capabilities"] = capabilities_;
    
    json available_functions = json::array();
    for (const auto& [func_name, _] : functions_) {
        available_functions.push_back(func_name);
    }
    info["functions"] = available_functions;
    info["created_at"] = get_timestamp();
    
    return info;
}

void Agent::setup_builtin_functions() {
    // Basic chat function
    register_function("chat", [this](const json& params) -> json {
        std::string message = params.value("message", "");
        if (message.empty()) {
            throw std::runtime_error("Missing 'message' parameter");
        }
        
        // Check if context from tool execution is provided
        std::string context = params.value("context", "");
        json tool_results = params.value("tool_results", json::object());
        
        json response;
        response["agent"] = name_;
        response["timestamp"] = get_timestamp();
        
        if (!context.empty()) {
            // Enhanced response with tool context
            std::string enhanced_response = "Based on the tool execution results, I can provide you with the following information:\n\n";
            
            // Analyze tool results
            if (tool_results.contains("analyze")) {
                auto analyze_result = tool_results["analyze"];
                if (!analyze_result.contains("error")) {
                    enhanced_response += "Analysis: ";
                    enhanced_response += "Text contains " + std::to_string(analyze_result.value("word_count", 0)) + " words. ";
                }
            }
            
            if (tool_results.contains("search_documents") || tool_results.contains("internet_search") || tool_results.contains("research")) {
                enhanced_response += "Search results were retrieved from available sources. ";
            }
            
            if (tool_results.contains("list_documents")) {
                enhanced_response += "Document repository was accessed. ";
            }
            
            enhanced_response += "\n\nRegarding your message: \"" + message + "\"\n";
            enhanced_response += "I have executed multiple tool functions to gather relevant information. ";
            enhanced_response += "The detailed results are available in the tool_results section of this response.";
            
            response["response"] = enhanced_response;
            response["context_used"] = true;
            response["tool_results_summary"] = tool_results;
        } else {
            // Simple response without context
            response["response"] = "Hello! I'm " + name_ + ". You said: " + message;
            response["context_used"] = false;
        }
        
        return response;
    });
    
    // Analysis function
    register_function("analyze", [this](const json& params) -> json {
        std::string text = params.value("text", "");
        if (text.empty()) {
            throw std::runtime_error("Missing 'text' parameter");
        }
        
        json analysis;
        analysis["agent"] = name_;
        analysis["text_length"] = text.length();
        analysis["word_count"] = std::count(text.begin(), text.end(), ' ') + 1;
        analysis["char_count"] = text.length();
        analysis["analysis_time"] = get_timestamp();
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
