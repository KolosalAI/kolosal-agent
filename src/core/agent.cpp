#include "../include/agent.hpp"
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
        
        json response;
        response["agent"] = name_;
        response["response"] = "Hello! I'm " + name_ + ". You said: " + message;
        response["timestamp"] = get_timestamp();
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
        return status;
    });
}
