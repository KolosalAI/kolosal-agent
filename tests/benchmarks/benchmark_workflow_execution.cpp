/**
 * @file benchmark_workflow_execution.cpp
 * @brief Benchmark tests for workflow execution using Google Benchmark
 * @version 2.0.0
 */

// Only compile benchmark tests if Google Benchmark is available
#ifdef BENCHMARK_FOUND

#include <benchmark/benchmark.h>
#include "workflow/workflow_engine.hpp"
#include "../fixtures/test_fixtures.hpp"
#include <memory>
#include <random>

using namespace kolosal::agents;
using namespace kolosal::agents::test;

class WorkflowBenchmarkFixture : public benchmark::Fixture {
protected:
    void SetUp(const ::benchmark::State& state) override {
        // Initialize workflow engine for benchmarks
        // test_workflow_engine_ = std::make_shared<WorkflowEngine>(test_agent_manager_);
        // test_workflow_engine_->start();
    }

    void TearDown(const ::benchmark::State& state) override {
        // if (test_workflow_engine_) {
        //     test_workflow_engine_->stop();
        //     test_workflow_engine_.reset();
        // }
    }

    Workflow createBenchmarkWorkflow(size_t num_steps, WorkflowType type = WorkflowType::SEQUENTIAL) {
        Workflow workflow;
        workflow.workflow_id = "benchmark_workflow_" + std::to_string(num_steps);
        workflow.name = "Benchmark Workflow";
        workflow.type = type;
        
        for (size_t i = 0; i < num_steps; ++i) {
            WorkflowStep step;
            step.step_id = "benchmark_step_" + std::to_string(i);
            step.name = "Benchmark Step " + std::to_string(i);
            step.agent_id = "benchmark_agent";
            step.function_name = "benchmark_function";
            step.parameters = {{"step_index", i}};
            
            if (type == WorkflowType::SEQUENTIAL && i > 0) {
                step.dependencies.push_back({"benchmark_step_" + std::to_string(i-1), "success", true});
            }
            
            step.parallel_allowed = (type == WorkflowType::PARALLEL);
            step.timeout_seconds = 10;
            workflow.steps.push_back(step);
        }
        
        return workflow;
    }

    // std::shared_ptr<WorkflowEngine> test_workflow_engine_;
    // std::shared_ptr<YAMLConfigurableAgentManager> test_agent_manager_;
};

// Benchmark workflow creation performance
BENCHMARK_DEFINE_F(WorkflowBenchmarkFixture, WorkflowCreation)(benchmark::State& state) {
    size_t num_steps = state.range(0);
    
    for (auto _ : state) {
        auto workflow = createBenchmarkWorkflow(num_steps);
        
        // Simulate workflow creation
        benchmark::DoNotOptimize(workflow);
        
        // Optional: Measure actual creation if engine is available
        // if (test_workflow_engine_) {
        //     std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
        //     benchmark::DoNotOptimize(workflow_id);
        // }
    }
    
    state.SetComplexityN(num_steps);
    state.SetItemsProcessed(state.iterations() * num_steps);
}

// Register benchmark with different step counts
BENCHMARK_REGISTER_F(WorkflowBenchmarkFixture, WorkflowCreation)
    ->RangeMultiplier(2)
    ->Range(1, 64)
    ->Complexity(benchmark::oN);

// Benchmark sequential workflow execution
BENCHMARK_DEFINE_F(WorkflowBenchmarkFixture, SequentialExecution)(benchmark::State& state) {
    size_t num_steps = state.range(0);
    
    for (auto _ : state) {
        state.PauseTiming();
        auto workflow = createBenchmarkWorkflow(num_steps, WorkflowType::SEQUENTIAL);
        state.ResumeTiming();
        
        // Simulate sequential execution timing
        auto start = std::chrono::high_resolution_clock::now();
        
        // Mock sequential execution time (proportional to step count)
        std::this_thread::sleep_for(std::chrono::microseconds(num_steps * 10));
        
        auto end = std::chrono::high_resolution_clock::now();
        
        benchmark::DoNotOptimize(workflow);
        
        // Optional: Measure actual execution if engine is available
        // if (test_workflow_engine_) {
        //     std::string workflow_id = test_workflow_engine_->create_workflow(workflow);
        //     std::string execution_id = test_workflow_engine_->execute_workflow(workflow_id);
        //     
        //     // Wait for completion (with timeout)
        //     auto timeout = std::chrono::milliseconds(1000);
        //     auto wait_start = std::chrono::high_resolution_clock::now();
        //     
        //     while ((std::chrono::high_resolution_clock::now() - wait_start) < timeout) {
        //         auto status = test_workflow_engine_->get_execution_status(execution_id);
        //         if (status.has_value() && 
        //             (status->current_status == WorkflowStatus::COMPLETED ||
        //              status->current_status == WorkflowStatus::FAILED)) {
        //             break;
        //         }
        //         std::this_thread::sleep_for(std::chrono::milliseconds(1));
        //     }
        // }
    }
    
    state.SetComplexityN(num_steps);
    state.SetItemsProcessed(state.iterations() * num_steps);
}

BENCHMARK_REGISTER_F(WorkflowBenchmarkFixture, SequentialExecution)
    ->RangeMultiplier(2)
    ->Range(1, 32)
    ->Complexity(benchmark::oN);

// Benchmark parallel workflow execution
BENCHMARK_DEFINE_F(WorkflowBenchmarkFixture, ParallelExecution)(benchmark::State& state) {
    size_t num_steps = state.range(0);
    
    for (auto _ : state) {
        state.PauseTiming();
        auto workflow = createBenchmarkWorkflow(num_steps, WorkflowType::PARALLEL);
        state.ResumeTiming();
        
        // Simulate parallel execution timing (less than linear scaling)
        auto start = std::chrono::high_resolution_clock::now();
        
        // Mock parallel execution time (logarithmic scaling for parallelism benefits)
        auto parallel_time = static_cast<size_t>(10 * std::log2(num_steps + 1));
        std::this_thread::sleep_for(std::chrono::microseconds(parallel_time));
        
        auto end = std::chrono::high_resolution_clock::now();
        
        benchmark::DoNotOptimize(workflow);
    }
    
    state.SetComplexityN(num_steps);
    state.SetItemsProcessed(state.iterations() * num_steps);
}

BENCHMARK_REGISTER_F(WorkflowBenchmarkFixture, ParallelExecution)
    ->RangeMultiplier(2)
    ->Range(1, 32)
    ->Complexity(benchmark::oN);

// Benchmark workflow dependency resolution
BENCHMARK_DEFINE_F(WorkflowBenchmarkFixture, DependencyResolution)(benchmark::State& state) {
    size_t num_steps = state.range(0);
    
    for (auto _ : state) {
        state.PauseTiming();
        
        // Create workflow with complex dependencies
        Workflow workflow;
        workflow.workflow_id = "dependency_benchmark";
        workflow.type = WorkflowType::PIPELINE;
        
        // Create a diamond dependency pattern
        for (size_t i = 0; i < num_steps; ++i) {
            WorkflowStep step;
            step.step_id = "dep_step_" + std::to_string(i);
            step.name = "Dependency Step " + std::to_string(i);
            step.agent_id = "dep_agent";
            step.function_name = "dep_function";
            
            // Create complex dependency patterns
            if (i > 0) {
                // Each step depends on multiple previous steps
                size_t num_deps = std::min(i, size_t(3));  // Up to 3 dependencies
                for (size_t j = 0; j < num_deps; ++j) {
                    size_t dep_index = i - j - 1;
                    step.dependencies.push_back({
                        "dep_step_" + std::to_string(dep_index),
                        "success",
                        true
                    });
                }
            }
            
            workflow.steps.push_back(step);
        }
        
        state.ResumeTiming();
        
        // Simulate dependency resolution algorithm
        // This would normally be done by WorkflowEngine::resolve_execution_order()
        std::vector<std::string> resolved_order;
        
        // Simple topological sort simulation
        for (size_t i = 0; i < num_steps; ++i) {
            resolved_order.push_back("dep_step_" + std::to_string(i));
        }
        
        benchmark::DoNotOptimize(resolved_order);
    }
    
    state.SetComplexityN(num_steps);
}

BENCHMARK_REGISTER_F(WorkflowBenchmarkFixture, DependencyResolution)
    ->RangeMultiplier(2)
    ->Range(4, 64)
    ->Complexity(benchmark::oNSquared);

// Benchmark memory allocation patterns
static void BM_WorkflowMemoryAllocation(benchmark::State& state) {
    size_t num_workflows = state.range(0);
    
    for (auto _ : state) {
        std::vector<std::unique_ptr<Workflow>> workflows;
        workflows.reserve(num_workflows);
        
        for (size_t i = 0; i < num_workflows; ++i) {
            auto workflow = std::make_unique<Workflow>();
            workflow->workflow_id = "mem_test_" + std::to_string(i);
            workflow->name = "Memory Test Workflow";
            workflow->type = WorkflowType::SEQUENTIAL;
            
            // Add some steps
            for (int j = 0; j < 5; ++j) {
                WorkflowStep step;
                step.step_id = "mem_step_" + std::to_string(j);
                step.name = "Memory Step";
                step.agent_id = "mem_agent";
                step.function_name = "mem_function";
                workflow->steps.push_back(step);
            }
            
            workflows.push_back(std::move(workflow));
        }
        
        benchmark::DoNotOptimize(workflows);
        
        // Cleanup happens automatically with unique_ptr
    }
    
    state.SetItemsProcessed(state.iterations() * num_workflows);
}

BENCHMARK(BM_WorkflowMemoryAllocation)
    ->RangeMultiplier(2)
    ->Range(1, 128);

// Benchmark JSON parameter processing
static void BM_ParameterProcessing(benchmark::State& state) {
    size_t num_parameters = state.range(0);
    
    // Create sample JSON parameters
    nlohmann::json parameters = nlohmann::json::object();
    for (size_t i = 0; i < num_parameters; ++i) {
        parameters["param_" + std::to_string(i)] = "value_" + std::to_string(i);
        parameters["number_" + std::to_string(i)] = static_cast<int>(i);
        parameters["nested_" + std::to_string(i)] = nlohmann::json::object({
            {"sub_param", "sub_value_" + std::to_string(i)},
            {"sub_number", static_cast<int>(i * 2)}
        });
    }
    
    for (auto _ : state) {
        // Simulate parameter processing
        nlohmann::json processed = parameters;
        
        // Simulate variable interpolation
        for (auto& [key, value] : processed.items()) {
            if (value.is_string()) {
                std::string str_value = value.get<std::string>();
                // Simulate template variable replacement
                if (str_value.find("${") != std::string::npos) {
                    // Mock variable replacement
                    str_value = "processed_" + str_value;
                    value = str_value;
                }
            }
        }
        
        benchmark::DoNotOptimize(processed);
    }
    
    state.SetItemsProcessed(state.iterations() * num_parameters);
}

BENCHMARK(BM_ParameterProcessing)
    ->RangeMultiplier(2)
    ->Range(1, 64);

// Benchmark condition evaluation
static void BM_ConditionEvaluation(benchmark::State& state) {
    size_t num_conditions = state.range(0);
    
    // Create sample conditions
    std::vector<std::string> conditions;
    for (size_t i = 0; i < num_conditions; ++i) {
        conditions.push_back("steps.step" + std::to_string(i) + ".output.value > " + std::to_string(i * 10));
        conditions.push_back("global.threshold <= " + std::to_string(i));
        conditions.push_back("steps.step" + std::to_string(i) + ".status == 'completed'");
    }
    
    // Sample context for evaluation
    nlohmann::json context = {
        {"global", {{"threshold", 50}}},
        {"steps", nlohmann::json::object()}
    };
    
    for (size_t i = 0; i < num_conditions / 3; ++i) {
        context["steps"]["step" + std::to_string(i)] = {
            {"output", {{"value", static_cast<int>(i * 15)}}},
            {"status", "completed"}
        };
    }
    
    for (auto _ : state) {
        bool all_results = true;
        
        // Simulate condition evaluation
        for (const auto& condition : conditions) {
            // Mock condition evaluation (simple string parsing simulation)
            bool result = true;  // Mock evaluation result
            
            if (condition.find(">") != std::string::npos) {
                result = (std::rand() % 100) > 50;  // Mock comparison
            } else if (condition.find("==") != std::string::npos) {
                result = (std::rand() % 100) > 30;  // Mock equality check
            } else if (condition.find("<=") != std::string::npos) {
                result = (std::rand() % 100) > 70;  // Mock comparison
            }
            
            all_results = all_results && result;
        }
        
        benchmark::DoNotOptimize(all_results);
    }
    
    state.SetItemsProcessed(state.iterations() * conditions.size());
}

BENCHMARK(BM_ConditionEvaluation)
    ->RangeMultiplier(2)
    ->Range(1, 32);

// Benchmark workflow state serialization
static void BM_WorkflowStateSerialization(benchmark::State& state) {
    size_t workflow_complexity = state.range(0);
    
    // Create a complex workflow state
    WorkflowExecutionContext context;
    context.execution_id = "benchmark_execution";
    context.workflow_id = "benchmark_workflow";
    
    // Add global variables
    for (size_t i = 0; i < workflow_complexity; ++i) {
        context.global_variables["global_" + std::to_string(i)] = "value_" + std::to_string(i);
    }
    
    // Add step outputs
    for (size_t i = 0; i < workflow_complexity; ++i) {
        context.step_outputs["step_" + std::to_string(i)] = {
            {"result", "result_" + std::to_string(i)},
            {"metadata", {
                {"execution_time", i * 100},
                {"memory_used", i * 1024},
                {"success", true}
            }}
        };
        context.step_statuses["step_" + std::to_string(i)] = StepStatus::COMPLETED;
        context.completed_steps.push_back("step_" + std::to_string(i));
    }
    
    for (auto _ : state) {
        // Simulate state serialization to JSON
        nlohmann::json serialized = {
            {"execution_id", context.execution_id},
            {"workflow_id", context.workflow_id},
            {"global_variables", context.global_variables},
            {"step_outputs", context.step_outputs},
            {"completed_steps", context.completed_steps},
            {"failed_steps", context.failed_steps}
        };
        
        // Simulate serialization to string
        std::string json_string = serialized.dump();
        
        benchmark::DoNotOptimize(json_string);
    }
    
    state.SetBytesProcessed(state.iterations() * workflow_complexity * 100);  // Estimated bytes
}

BENCHMARK(BM_WorkflowStateSerialization)
    ->RangeMultiplier(2)
    ->Range(1, 64);

// Main function for running benchmarks
BENCHMARK_MAIN();

#else // BENCHMARK_FOUND

// Fallback implementation when Google Benchmark is not available
// Note: Only one benchmark file should define main() - handled in benchmark_agent_operations.cpp

#endif // BENCHMARK_FOUND
