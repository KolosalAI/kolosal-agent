/**
 * @file benchmark_memory_operations.cpp
 * @brief Benchmark tests for memory management operations
 */

#include <benchmark/benchmark.h>
#include "agent/memory/agent_memory_manager.hpp"

#ifdef KOLOSAL_AGENT_TEST_MODE

/**
 * @brief Benchmark memory manager creation
 */
static void BM_MemoryManagerCreation(benchmark::State& state) {
    for (auto _ : state) {
        auto manager = std::make_unique<kolosal::agent::AgentMemoryManager>();
        benchmark::DoNotOptimize(manager);
    }
}
BENCHMARK(BM_MemoryManagerCreation);

/**
 * @brief Benchmark memory allocation and deallocation
 */
static void BM_MemoryAllocationDeallocation(benchmark::State& state) {
    auto manager = std::make_unique<kolosal::agent::AgentMemoryManager>();
    
    for (auto _ : state) {
        // Simulate memory operations
        std::vector<std::unique_ptr<char[]>> allocations;
        for (int i = 0; i < state.range(0); ++i) {
            allocations.emplace_back(std::make_unique<char[]>(1024));
        }
        benchmark::DoNotOptimize(allocations);
    }
}
BENCHMARK(BM_MemoryAllocationDeallocation)->Range(1, 1000);

#endif // KOLOSAL_AGENT_TEST_MODE

BENCHMARK_MAIN();
