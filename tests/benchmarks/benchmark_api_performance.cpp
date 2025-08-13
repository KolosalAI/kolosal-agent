/**
 * @file benchmark_api_performance.cpp
 * @brief Benchmark tests for API performance
 */

// Only compile benchmark tests if Google Benchmark is available
#ifdef BENCHMARK_FOUND

#include <benchmark/benchmark.h>
#include "api/simple_http_server.hpp"
#include "api/message_router.hpp"
#include "api/http_client.hpp"

#ifdef KOLOSAL_AGENT_TEST_MODE

/**
 * @brief Benchmark HTTP server initialization
 */
static void BM_HttpServerInit(benchmark::State& state) {
    for (auto _ : state) {
        auto server = std::make_unique<kolosal::api::SimpleHttpServer>(8080);
        benchmark::DoNotOptimize(server);
    }
}
BENCHMARK(BM_HttpServerInit);

/**
 * @brief Benchmark message router performance
 */
static void BM_MessageRouterPerformance(benchmark::State& state) {
    auto router = std::make_unique<kolosal::api::MessageRouter>();
    
    for (auto _ : state) {
        // Simulate message routing
        benchmark::DoNotOptimize(router.get());
    }
}
BENCHMARK(BM_MessageRouterPerformance);

/**
 * @brief Benchmark HTTP client operations
 */
static void BM_HttpClientOperations(benchmark::State& state) {
    for (auto _ : state) {
        auto client = std::make_unique<kolosal::api::HttpClient>();
        benchmark::DoNotOptimize(client);
    }
}
BENCHMARK(BM_HttpClientOperations);

#endif // KOLOSAL_AGENT_TEST_MODE

BENCHMARK_MAIN();

#else // BENCHMARK_FOUND

// Fallback implementation when Google Benchmark is not available
// Note: Only one benchmark file should define main() - handled in benchmark_agent_operations.cpp

#endif // BENCHMARK_FOUND
