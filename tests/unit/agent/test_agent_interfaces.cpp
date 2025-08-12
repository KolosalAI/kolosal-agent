/**
 * @file test_agent_interfaces.cpp
 * @brief Unit tests for agent interface implementations
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "agent/core/agent_interfaces.hpp"
#include "../fixtures/test_fixtures.hpp"

using namespace testing;
using namespace kolosal::agents;
using namespace kolosal::agents::test;

class AgentInterfacesTest : public KolosalAgentTestFixture {
protected:
    void SetUp() override {
        KolosalAgentTestFixture::SetUp();
    }
};

TEST_F(AgentInterfacesTest, AgentDataOperations) {
    AgentData data;
    
    // Test empty data
    EXPECT_TRUE(data.empty());
    EXPECT_EQ(data.size(), 0);
    
    // Test adding data
    data["string_key"] = "test_value";
    data["int_key"] = 42;
    data["bool_key"] = true;
    data["double_key"] = 3.14159;
    
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(data.size(), 4);
    
    // Test retrieving data
    EXPECT_EQ(data["string_key"], "test_value");
    EXPECT_EQ(data["int_key"], 42);
    EXPECT_EQ(data["bool_key"], true);
    EXPECT_EQ(data["double_key"], 3.14159);
    
    // Test key existence
    EXPECT_TRUE(data.contains("string_key"));
    EXPECT_FALSE(data.contains("nonexistent_key"));
}

TEST_F(AgentInterfacesTest, FunctionResultOperations) {
    FunctionResult result;
    
    // Test default state
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.result_data.empty());
    EXPECT_TRUE(result.error_message.empty());
    EXPECT_GT(result.execution_time_ms, 0); // Should have some default time
    
    // Test successful result
    result.success = true;
    result.result_data = AgentData{{"output", "success"}};
    result.execution_time_ms = 150.5;
    
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.result_data.empty());
    EXPECT_EQ(result.result_data["output"], "success");
    EXPECT_DOUBLE_EQ(result.execution_time_ms, 150.5);
    
    // Test error result
    FunctionResult error_result;
    error_result.success = false;
    error_result.error_message = "Test error occurred";
    error_result.error_code = -1;
    
    EXPECT_FALSE(error_result.success);
    EXPECT_EQ(error_result.error_message, "Test error occurred");
    EXPECT_EQ(error_result.error_code, -1);
}

TEST_F(AgentInterfacesTest, FunctionSchemaValidation) {
    FunctionSchema schema;
    schema.name = "test_function";
    schema.description = "A test function";
    schema.async_capable = true;
    
    // Add parameters
    ParameterSchema param1;
    param1.name = "input";
    param1.type = "string";
    param1.required = true;
    param1.description = "Input string";
    
    ParameterSchema param2;
    param2.name = "count";
    param2.type = "integer";
    param2.required = false;
    param2.default_value = "1";
    
    schema.parameters = {param1, param2};
    
    // Test schema properties
    EXPECT_EQ(schema.name, "test_function");
    EXPECT_EQ(schema.description, "A test function");
    EXPECT_TRUE(schema.async_capable);
    EXPECT_EQ(schema.parameters.size(), 2);
    
    // Test parameter properties
    EXPECT_EQ(schema.parameters[0].name, "input");
    EXPECT_TRUE(schema.parameters[0].required);
    EXPECT_EQ(schema.parameters[1].name, "count");
    EXPECT_FALSE(schema.parameters[1].required);
    EXPECT_EQ(schema.parameters[1].default_value, "1");
}

TEST_F(AgentInterfacesTest, AgentMessageOperations) {
    AgentMessage message;
    message.from_agent = "sender_agent";
    message.to_agent = "receiver_agent";
    message.message_type = "data_request";
    message.payload = AgentData{{"request_type", "analysis"}, {"priority", "high"}};
    message.correlation_id = "msg_001";
    message.requires_response = true;
    
    EXPECT_EQ(message.from_agent, "sender_agent");
    EXPECT_EQ(message.to_agent, "receiver_agent");
    EXPECT_EQ(message.message_type, "data_request");
    EXPECT_EQ(message.payload["request_type"], "analysis");
    EXPECT_EQ(message.payload["priority"], "high");
    EXPECT_EQ(message.correlation_id, "msg_001");
    EXPECT_TRUE(message.requires_response);
    EXPECT_GT(message.timestamp.time_since_epoch().count(), 0);
}

TEST_F(AgentInterfacesTest, JobOperations) {
    Job job;
    job.job_id = "test_job_001";
    job.function_name = "process_data";
    job.parameters = AgentData{{"input", "test_data"}, {"format", "json"}};
    job.priority = 5;
    job.timeout_ms = 30000;
    job.retry_count = 3;
    job.created_by = "test_agent";
    
    EXPECT_EQ(job.job_id, "test_job_001");
    EXPECT_EQ(job.function_name, "process_data");
    EXPECT_EQ(job.parameters["input"], "test_data");
    EXPECT_EQ(job.priority, 5);
    EXPECT_EQ(job.timeout_ms, 30000);
    EXPECT_EQ(job.retry_count, 3);
    EXPECT_EQ(job.created_by, "test_agent");
    EXPECT_EQ(job.status, JobStatus::PENDING);
}

TEST_F(AgentInterfacesTest, JobStatusTransitions) {
    Job job;
    
    // Test initial status
    EXPECT_EQ(job.status, JobStatus::PENDING);
    
    // Test status transitions
    job.status = JobStatus::RUNNING;
    EXPECT_EQ(job.status, JobStatus::RUNNING);
    
    job.status = JobStatus::COMPLETED;
    EXPECT_EQ(job.status, JobStatus::COMPLETED);
    
    // Test other statuses
    job.status = JobStatus::FAILED;
    EXPECT_EQ(job.status, JobStatus::FAILED);
    
    job.status = JobStatus::CANCELLED;
    EXPECT_EQ(job.status, JobStatus::CANCELLED);
    
    job.status = JobStatus::TIMEOUT;
    EXPECT_EQ(job.status, JobStatus::TIMEOUT);
}

TEST_F(AgentInterfacesTest, MemoryEntryOperations) {
    MemoryEntry entry;
    entry.memory_id = "mem_001";
    entry.content = "This is a test memory entry";
    entry.memory_type = "test";
    entry.importance_score = 0.8;
    entry.access_count = 5;
    entry.metadata = AgentData{{"category", "testing"}, {"source", "unit_test"}};
    
    EXPECT_EQ(entry.memory_id, "mem_001");
    EXPECT_EQ(entry.content, "This is a test memory entry");
    EXPECT_EQ(entry.memory_type, "test");
    EXPECT_DOUBLE_EQ(entry.importance_score, 0.8);
    EXPECT_EQ(entry.access_count, 5);
    EXPECT_EQ(entry.metadata["category"], "testing");
    EXPECT_EQ(entry.metadata["source"], "unit_test");
    EXPECT_GT(entry.created_time.time_since_epoch().count(), 0);
    EXPECT_GT(entry.last_accessed.time_since_epoch().count(), 0);
}

TEST_F(AgentInterfacesTest, EventOperations) {
    Event event;
    event.event_id = "evt_001";
    event.event_type = "agent_started";
    event.source_agent = "test_agent";
    event.event_data = AgentData{{"agent_name", "test"}, {"startup_time", 150.5}};
    event.priority = EventPriority::NORMAL;
    
    EXPECT_EQ(event.event_id, "evt_001");
    EXPECT_EQ(event.event_type, "agent_started");
    EXPECT_EQ(event.source_agent, "test_agent");
    EXPECT_EQ(event.event_data["agent_name"], "test");
    EXPECT_EQ(event.event_data["startup_time"], 150.5);
    EXPECT_EQ(event.priority, EventPriority::NORMAL);
    EXPECT_GT(event.timestamp.time_since_epoch().count(), 0);
}

TEST_F(AgentInterfacesTest, ExecutionPlanOperations) {
    ExecutionPlan plan;
    plan.plan_id = "plan_001";
    plan.goal = "Complete data analysis task";
    plan.context = "User requested sales data analysis";
    plan.status = PlanStatus::CREATED;
    plan.estimated_duration_ms = 60000; // 1 minute
    
    // Add execution steps
    ExecutionStep step1;
    step1.step_id = "step_001";
    step1.description = "Load data";
    step1.function_name = "load_data";
    step1.parameters = AgentData{{"source", "database"}};
    step1.estimated_duration_ms = 10000;
    
    ExecutionStep step2;
    step2.step_id = "step_002";
    step2.description = "Analyze data";
    step2.function_name = "analyze";
    step2.parameters = AgentData{{"method", "statistical"}};
    step2.dependencies = {"step_001"};
    step2.estimated_duration_ms = 30000;
    
    plan.steps = {step1, step2};
    
    EXPECT_EQ(plan.plan_id, "plan_001");
    EXPECT_EQ(plan.goal, "Complete data analysis task");
    EXPECT_EQ(plan.status, PlanStatus::CREATED);
    EXPECT_EQ(plan.steps.size(), 2);
    EXPECT_EQ(plan.steps[0].step_id, "step_001");
    EXPECT_EQ(plan.steps[1].dependencies.size(), 1);
    EXPECT_EQ(plan.steps[1].dependencies[0], "step_001");
}

TEST_F(AgentInterfacesTest, ToolOperations) {
    ToolSchema schema;
    schema.name = "calculator";
    schema.description = "A calculator tool";
    schema.tool_type = ToolType::BUILTIN;
    
    ParameterSchema param;
    param.name = "expression";
    param.type = "string";
    param.required = true;
    param.description = "Mathematical expression to evaluate";
    
    schema.parameters = {param};
    
    EXPECT_EQ(schema.name, "calculator");
    EXPECT_EQ(schema.description, "A calculator tool");
    EXPECT_EQ(schema.tool_type, ToolType::BUILTIN);
    EXPECT_EQ(schema.parameters.size(), 1);
    EXPECT_EQ(schema.parameters[0].name, "expression");
    EXPECT_TRUE(schema.parameters[0].required);
}

TEST_F(AgentInterfacesTest, ToolFilterOperations) {
    ToolFilter filter;
    filter.tool_types = {ToolType::BUILTIN, ToolType::EXTERNAL_API};
    filter.name_pattern = "calc*";
    filter.include_async_only = false;
    filter.max_results = 10;
    
    EXPECT_EQ(filter.tool_types.size(), 2);
    EXPECT_THAT(filter.tool_types, Contains(ToolType::BUILTIN));
    EXPECT_THAT(filter.tool_types, Contains(ToolType::EXTERNAL_API));
    EXPECT_EQ(filter.name_pattern, "calc*");
    EXPECT_FALSE(filter.include_async_only);
    EXPECT_EQ(filter.max_results, 10);
}

TEST_F(AgentInterfacesTest, ComplexAgentDataOperations) {
    AgentData complex_data;
    
    // Test nested objects
    AgentData nested_obj;
    nested_obj["inner_key"] = "inner_value";
    nested_obj["inner_number"] = 123;
    
    complex_data["nested"] = nested_obj;
    complex_data["array"] = std::vector<int>{1, 2, 3, 4, 5};
    complex_data["mixed_array"] = std::vector<nlohmann::json>{"string", 42, true, 3.14};
    
    // Test accessing nested data
    EXPECT_TRUE(complex_data.contains("nested"));
    EXPECT_TRUE(complex_data["nested"].is_object());
    
    if (complex_data["nested"].is_object()) {
        auto nested = complex_data["nested"];
        EXPECT_EQ(nested["inner_key"], "inner_value");
        EXPECT_EQ(nested["inner_number"], 123);
    }
    
    // Test array operations
    EXPECT_TRUE(complex_data["array"].is_array());
    if (complex_data["array"].is_array()) {
        auto arr = complex_data["array"];
        EXPECT_EQ(arr.size(), 5);
        EXPECT_EQ(arr[0], 1);
        EXPECT_EQ(arr[4], 5);
    }
}
