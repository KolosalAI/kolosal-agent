/**
 * @file benchmark_workflow_execution.cpp
 * @brief Benchmark tests for workflow execution performance
 */

#include <benchmark/benchmark.h>
#include "workflow/workflow_engine.hpp"
#include "workflow/sequential_workflow.hpp"

#ifdef KOLOSAL_AGENT_TEST_MODE

/**
 * @brief Benchmark workflow engine initialization
 */
static void BM_WorkflowEngineInit(benchmark::State& state) {
    for (auto _ : state) {
        auto engine = std::make_unique<kolosal::workflow::WorkflowEngine>();
        benchmark::DoNotOptimize(engine);
    }
}
BENCHMARK(BM_WorkflowEngineInit);

/**
 * @brief Benchmark sequential workflow execution
 */
static void BM_SequentialWorkflowExecution(benchmark::State& state) {
    auto workflow = std::make_unique<kolosal::workflow::SequentialWorkflow>();
    
    for (auto _ : state) {
        // Simulate workflow execution
        benchmark::DoNotOptimize(workflow.get());
    }
}
BENCHMARK(BM_SequentialWorkflowExecution);

/**
 * @brief Benchmark workflow with varying number of steps
 */
static void BM_WorkflowScaling(benchmark::State& state) {
    for (auto _ : state) {
        auto workflow = std::make_unique<kolosal::workflow::SequentialWorkflow>();
        
        // Add varying number of steps
        for (int i = 0; i < state.range(0); ++i) {
            // Add step simulation
            benchmark::DoNotOptimize(workflow.get());
        }
    }
}
BENCHMARK(BM_WorkflowScaling)->Range(1, 100);

#endif // KOLOSAL_AGENT_TEST_MODE

BENCHMARK_MAIN();
