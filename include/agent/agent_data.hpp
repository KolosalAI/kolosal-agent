/**
 * @file agent_data.hpp
 * @brief Core functionality for agent data
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#ifndef KOLOSAL_AGENTS_AGENT_DATA_HPP
#define KOLOSAL_AGENTS_AGENT_DATA_HPP

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "json.hpp"

namespace kolosal {
namespace agents {

struct AgentDataValue {
    enum Type {
        NONE,
        STRING,
        INT,
        DOUBLE,
        BOOL,
        ARRAY_STRING,
        OBJECT_DATA
    };

    // value storage (non-const to allow assignment/copy)
    Type type = NONE;
    std::string s_val;
    int i_val = 0;
    double d_val = 0.0;
    bool b_val = false;
    std::vector<std::string> arr_s_val;
    std::unique_ptr<std::map<std::string, AgentDataValue>> obj_val;

    AgentDataValue() = default;
    AgentDataValue(const std::string& val);
    AgentDataValue(int val);
    AgentDataValue(double val);
    AgentDataValue(bool val);
    AgentDataValue(const std::vector<std::string>& val);
    AgentDataValue(const class AgentData& val);
    AgentDataValue(const AgentDataValue& other);
    AgentDataValue& operator=(const AgentDataValue& other);
};

/**
 * @brief Represents agent data functionality
 */
class AgentData {
public:
    AgentData() = default;
    ~AgentData() = default;

    // Set methods for different types
    /**
     * @brief Set 
     * @return void Description of return value
     */
    void set(const std::string& key, const std::string& value) {
        data[key] = AgentDataValue(value);
    }
    
    /**
     * @brief Set 
     * @return void Description of return value
     */
    void set(const std::string& key, int value) {
        data[key] = AgentDataValue(value);
    }
    
    /**
     * @brief Set 
     * @return void Description of return value
     */
    void set(const std::string& key, double value) {
        data[key] = AgentDataValue(value);
    }
    
    /**
     * @brief Set 
     * @return void Description of return value
     */
    void set(const std::string& key, bool value) {
        data[key] = AgentDataValue(value);
    }
    
    /**
     * @brief Set 
     * @return void Description of return value
     */
    void set(const std::string& key, const std::vector<std::string>& value) {
        data[key] = AgentDataValue(value);
    }
    
    /**
     * @brief Set 
     * @return void Description of return value
     */
    void set(const std::string& key, const AgentData& value) {
        data[key] = AgentDataValue(value);
    }
    
    /**
     * @brief Set 
     * @return void Description of return value
     */
    void set(const std::string& key, const AgentDataValue& value) {
        data[key] = value;
    }

    // Get methods with defaults
    std::string get__string(const std::string& key, const std::string& default_val = "") const;
    int get__int(const std::string& key, int default_val = 0) const;
    double get__double(const std::string& key, double default_val = 0.0) const;
    bool get__bool(const std::string& key, bool default_val = false) const;
    std::vector<std::string> get__array_string(const std::string& key) const;

    // Compatibility wrapper methods (used by some modules)
    std::string get_string(const std::string& key, const std::string& default_val = "") const { return get__string(key, default_val); }
    int get_int(const std::string& key, int default_val = 0) const { return get__int(key, default_val); }
    double get_double(const std::string& key, double default_val = 0.0) const { return get__double(key, default_val); }
    bool get_bool(const std::string& key, bool default_val = false) const { return get__bool(key, default_val); }
    std::vector<std::string> get_array_string(const std::string& key) const { return get__array_string(key); }

    // Utility methods
    bool has__key(const std::string& key) const;
    // wrapper
    bool has_key(const std::string& key) const { return has__key(key); }
    void clear();
    std::vector<std::string> get__all_keys() const;
    std::vector<std::string> get__keys() const { return get__all_keys(); }
    // wrapper
    std::vector<std::string> get_keys() const { return get__all_keys(); }
    std::string to_string() const;
    
    // Get the underlying data
    const std::map<std::string, AgentDataValue>& get__data() const { return data; }
    // wrapper
    const std::map<std::string, AgentDataValue>& get_data() const { return data; }
    // additional convenience wrapper
    std::vector<std::string> get_all_keys() const { return get__all_keys(); }

    // JSON conversion
    nlohmann::json to_json() const;
    void from_json(const nlohmann::json& json_data);

private:
    std::map<std::string, AgentDataValue> data;
};

/**
 * @brief Represents u u i d generator functionality
 */
class UUIDGenerator {
public:
    static std::string generate();
};

/**
 * @brief Represents agent functionality
 */
class Agent {
public:
    Agent(const std::string& id, const std::string& name, const std::string& type)
        : agent_id_(id), agent_name_(name), agent_type_(type), running_(false) {}

    virtual ~Agent() = default;

    std::string get__agent_id() const { return agent_id_; }
    std::string get__agent_name() const { return agent_name_; }
    std::string get__agent_type() const { return agent_type_; }
    bool is__running() const { return running_; }

    void set__running(bool running) { running_ = running; }

    std::vector<std::string> get__capabilities() const {
        return {"text_processing", "data_analysis", "task_execution"};
    }

private:
    std::string agent_id_;
    std::string agent_name_;
    std::string agent_type_;
    bool running_;
};

struct CommandResult {
    bool success;
    std::string message;
    std::string data;
    std::string error_message;
    long total_execution_time_ms = 0;
    std::map<std::string, CommandResult> step_results;
};

struct WorkflowResult {
    std::string workflow_id;
    bool success = false;
    std::string error_message;
    std::string final_output;
    std::vector<std::string> step_outputs;
    long total_execution_time_ms = 0;
    std::map<std::string, CommandResult> step_results;
};

struct CollaborationGroup {
    std::string group_id;
    std::vector<std::string> agent_ids;
    std::string pattern_type;
};

} // namespace agents
} // namespace kolosal

#endif // KOLOSAL_AGENTS_AGENT_DATA_HPP
