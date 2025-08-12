/**
 * @file benchmark_agent_operations.cpp
 * @brief Benchmark tests for core agent operations
 * 
 * This file contains benchmark tests to measure the performance of core
 * agent operations including initialization, task execution, and cleanup.
 */

#include <benchmark/benchmark.h>
#include "agent/core/agent_core.hpp"
#include "agent/core/multi_agent_system.hpp"
#include "config/yaml_configuration_parser.hpp"

#ifdef KOLOSAL_AGENT_TEST_MODE

/**
 * @brief Benchmark agent creation and initialization
 */
static void BM_AgentCreation(benchmark::State& state) {
    for (auto _ : state) {
        // Create and initialize agent
        auto agent = std::make_unique<kolosal::agent::AgentCore>();
        benchmark::DoNotOptimize(agent);
    }
}
BENCHMARK(BM_AgentCreation);

/**
 * @brief Benchmark multi-agent system initialization
 */
static void BM_MultiAgentSystemInit(benchmark::State& state) {
    for (auto _ : state) {
        auto system = std::make_unique<kolosal::agent::MultiAgentSystem>();
        benchmark::DoNotOptimize(system);
    }
}
BENCHMARK(BM_MultiAgentSystemInit);

/**
 * @brief Benchmark configuration loading
 */
static void BM_ConfigurationLoading(benchmark::State& state) {
    for (auto _ : state) {
        kolosal::config::YamlConfigurationParser parser;
        // Use test configuration file
        auto config = parser.parseConfiguration(TEST_DATA_DIR "/test_config.yaml");
        benchmark::DoNotOptimize(config);
    }
}
BENCHMARK(BM_ConfigurationLoading);

/**
 * @brief Benchmark agent task processing with varying number of tasks
 */
static void BM_AgentTaskProcessing(benchmark::State& state) {
    auto agent = std::make_unique<kolosal::agent::AgentCore>();
    
    for (auto _ : state) {
        // Simulate task processing
        for (int i = 0; i < state.range(0); ++i) {
            // Process a simple task
            benchmark::DoNotOptimize(agent.get());
        }
    }
}
BENCHMARK(BM_AgentTaskProcessing)->Range(1, 1000);

/**
 * @brief Benchmark memory allocation patterns
 */
static void BM_MemoryAllocation(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<std::unique_ptr<kolosal::agent::AgentCore>> agents;
        agents.reserve(state.range(0));
        
        for (int i = 0; i < state.range(0); ++i) {
            agents.emplace_back(std::make_unique<kolosal::agent::AgentCore>());
        }
        
        benchmark::DoNotOptimize(agents);
    }
}
BENCHMARK(BM_MemoryAllocation)->Range(1, 100);

#endif // KOLOSAL_AGENT_TEST_MODE

BENCHMARK_MAIN();
