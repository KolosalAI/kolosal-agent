/**
 * @file test_deep_research_agent_performance.cpp
 * @brief Performance benchmarks for DeepResearchAgent
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 */

#include <gtest/gtest.h>
#include <benchmark/benchmark.h>
#include "examples/deep_research_agent.hpp"
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

using namespace kolosal::agents::examples;
using namespace kolosal::agents;

class DeepResearchAgentPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Check if performance tests should run
        const char* env = std::getenv("KOLOSAL_PERFORMANCE_TESTS");
        performance_enabled = env && std::string(env) == "1";
        
        if (!performance_enabled) {
            GTEST_SKIP() << "Performance tests disabled. Set KOLOSAL_PERFORMANCE_TESTS=1 to enable.";
        }
        
        // Get server URL from environment or use default
        const char* server_env = std::getenv("KOLOSAL_SERVER_URL");
        server_url = server_env ? server_env : "http://localhost:8080";
        
        // Setup base configuration
        base_config.methodology = "systematic";
        base_config.max_sources = 10;
        base_config.max_web_results = 5;
        base_config.relevance_threshold = 0.7;
        base_config.include_academic = true;
        base_config.include_news = true;
        base_config.include_documents = false; // Disable for consistent performance
        base_config.output_format = "comprehensive_report";
        base_config.language = "en";
    }

    void TearDown() override {
        // Cleanup any agents
        for (auto& agent : test_agents) {
            if (agent) {
                agent->stop();
            }
        }
        test_agents.clear();
    }

    std::unique_ptr<DeepResearchAgent> create_test_agent(bool enable_server = true) {
        auto agent = std::make_unique<DeepResearchAgent>(
            "PerformanceTestAgent_" + std::to_string(test_agents.size()),
            server_url,
            enable_server
        );
        
        test_agents.push_back(agent.get());
        return agent;
    }

    bool performance_enabled = false;
    std::string server_url;
    ResearchConfig base_config;
    std::vector<DeepResearchAgent*> test_agents;
};

// Initialization Performance Tests
TEST_F(DeepResearchAgentPerformanceTest, InitializationTime) {
    const int num_iterations = 10;
    std::vector<double> init_times;
    
    for (int i = 0; i < num_iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto agent = create_test_agent(false); // No server for consistent timing
        bool success = agent->initialize();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        EXPECT_TRUE(success) << "Initialization failed on iteration " << i;
        init_times.push_back(duration.count());
    }
    
    // Calculate statistics
    double total = 0;
    for (double time : init_times) {
        total += time;
    }
    double average = total / num_iterations;
    
    // Performance expectations
    EXPECT_LT(average, 1000.0) << "Average initialization time too high: " << average << "ms";
    
    // Log performance metrics
    std::cout << "Initialization Performance:" << std::endl;
    std::cout << "  Average time: " << average << "ms" << std::endl;
    std::cout << "  Min time: " << *std::min_element(init_times.begin(), init_times.end()) << "ms" << std::endl;
    std::cout << "  Max time: " << *std::max_element(init_times.begin(), init_times.end()) << "ms" << std::endl;
}

TEST_F(DeepResearchAgentPerformanceTest, StartupTime) {
    const int num_iterations = 5;
    std::vector<double> startup_times;
    
    for (int i = 0; i < num_iterations; ++i) {
        auto agent = create_test_agent(false);
        
        auto start = std::chrono::high_resolution_clock::now();
        bool success = agent->start();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        EXPECT_TRUE(success) << "Startup failed on iteration " << i;
        startup_times.push_back(duration.count());
        
        agent->stop();
    }
    
    double average = std::accumulate(startup_times.begin(), startup_times.end(), 0.0) / num_iterations;
    
    EXPECT_LT(average, 2000.0) << "Average startup time too high: " << average << "ms";
    
    std::cout << "Startup Performance:" << std::endl;
    std::cout << "  Average time: " << average << "ms" << std::endl;
}

// Research Performance Tests
TEST_F(DeepResearchAgentPerformanceTest, BasicResearchTiming) {
    auto agent = create_test_agent(false);
    ASSERT_TRUE(agent->start());
    
    const std::string research_question = "Artificial intelligence trends";
    const int num_iterations = 3;
    std::vector<double> research_times;
    
    for (int i = 0; i < num_iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto result = agent->conduct_research(research_question, base_config);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        EXPECT_FALSE(result.full_report.empty()) << "Research failed on iteration " << i;
        research_times.push_back(duration.count());
    }
    
    double average = std::accumulate(research_times.begin(), research_times.end(), 0.0) / num_iterations;
    
    // Basic research should complete quickly without server
    EXPECT_LT(average, 5000.0) << "Average research time too high: " << average << "ms";
    
    std::cout << "Basic Research Performance:" << std::endl;
    std::cout << "  Average time: " << average << "ms" << std::endl;
    std::cout << "  Min time: " << *std::min_element(research_times.begin(), research_times.end()) << "ms" << std::endl;
    std::cout << "  Max time: " << *std::max_element(research_times.begin(), research_times.end()) << "ms" << std::endl;
}

TEST_F(DeepResearchAgentPerformanceTest, ScalabilityWithSourceLimits) {
    auto agent = create_test_agent(false);
    ASSERT_TRUE(agent->start());
    
    const std::string research_question = "Machine learning applications";
    const std::vector<int> source_limits = {5, 10, 15, 20, 25};
    
    std::cout << "Scalability with Source Limits:" << std::endl;
    
    for (int limit : source_limits) {
        ResearchConfig config = base_config;
        config.max_sources = limit;
        config.max_web_results = limit / 2;
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = agent->conduct_research(research_question, config);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        EXPECT_FALSE(result.full_report.empty()) << "Research failed for limit " << limit;
        
        std::cout << "  " << limit << " sources: " << duration.count() << "ms" << std::endl;
        
        // Time should scale reasonably with source count
        EXPECT_LT(duration.count(), limit * 200) << "Time scaling is too poor for " << limit << " sources";
    }
}

// Concurrency Performance Tests
TEST_F(DeepResearchAgentPerformanceTest, ConcurrentResearchCapacity) {
    const int num_threads = 3;
    const std::string base_question = "Technology trends ";
    
    auto agent = create_test_agent(false);
    ASSERT_TRUE(agent->start());
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<ResearchResult>> futures;
    
    for (int i = 0; i < num_threads; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            return agent->conduct_research(
                base_question + std::to_string(i), 
                base_config
            );
        }));
    }
    
    // Wait for all to complete
    std::vector<ResearchResult> results;
    for (auto& future : futures) {
        results.push_back(future.get());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Verify all succeeded
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_FALSE(results[i].full_report.empty()) << "Concurrent research " << i << " failed";
        EXPECT_EQ(results[i].research_question, base_question + std::to_string(i));
    }
    
    std::cout << "Concurrent Research Performance:" << std::endl;
    std::cout << "  " << num_threads << " threads total time: " << total_duration.count() << "ms" << std::endl;
    std::cout << "  Average per thread: " << total_duration.count() / num_threads << "ms" << std::endl;
    
    // Concurrent execution should be efficient
    EXPECT_LT(total_duration.count(), num_threads * 5000) << "Concurrent execution inefficient";
}

TEST_F(DeepResearchAgentPerformanceTest, MultipleAgentInstances) {
    const int num_agents = 3;
    const std::string research_question = "Renewable energy developments";
    
    std::vector<std::unique_ptr<DeepResearchAgent>> agents;
    
    // Create multiple agent instances
    auto creation_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_agents; ++i) {
        auto agent = create_test_agent(false);
        ASSERT_TRUE(agent->start());
        agents.push_back(std::move(agent));
    }
    auto creation_end = std::chrono::high_resolution_clock::now();
    
    auto creation_time = std::chrono::duration_cast<std::chrono::milliseconds>(creation_end - creation_start);
    
    // Execute research on all agents
    auto research_start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<ResearchResult>> futures;
    for (auto& agent : agents) {
        futures.push_back(std::async(std::launch::async, [&]() {
            return agent->conduct_research(research_question, base_config);
        }));
    }
    
    std::vector<ResearchResult> results;
    for (auto& future : futures) {
        results.push_back(future.get());
    }
    
    auto research_end = std::chrono::high_resolution_clock::now();
    auto research_time = std::chrono::duration_cast<std::chrono::milliseconds>(research_end - research_start);
    
    // Verify all agents worked
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_FALSE(results[i].full_report.empty()) << "Agent " << i << " research failed";
    }
    
    std::cout << "Multiple Agent Performance:" << std::endl;
    std::cout << "  " << num_agents << " agents creation time: " << creation_time.count() << "ms" << std::endl;
    std::cout << "  " << num_agents << " agents research time: " << research_time.count() << "ms" << std::endl;
    
    // Creation should be reasonable
    EXPECT_LT(creation_time.count(), num_agents * 2000) << "Agent creation too slow";
}

// Memory Performance Tests
TEST_F(DeepResearchAgentPerformanceTest, MemoryUsagePattern) {
    // This test requires manual monitoring or memory profiling tools
    // We'll do basic functionality testing here
    
    auto agent = create_test_agent(false);
    ASSERT_TRUE(agent->start());
    
    const int num_iterations = 10;
    const std::string base_question = "Memory test query ";
    
    std::cout << "Memory Usage Pattern Test:" << std::endl;
    
    for (int i = 0; i < num_iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto result = agent->conduct_research(
            base_question + std::to_string(i), 
            base_config
        );
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        EXPECT_FALSE(result.full_report.empty()) << "Memory test iteration " << i << " failed";
        
        if (i % 5 == 0) {
            std::cout << "  Iteration " << i << ": " << duration.count() << "ms" << std::endl;
        }
    }
    
    std::cout << "  Completed " << num_iterations << " iterations without crash" << std::endl;
}

// Workflow Performance Tests
TEST_F(DeepResearchAgentPerformanceTest, WorkflowCreationPerformance) {
    auto agent = create_test_agent(false);
    ASSERT_TRUE(agent->start());
    
    const int num_workflows = 10;
    std::vector<double> creation_times;
    
    for (int i = 0; i < num_workflows; ++i) {
        std::vector<std::string> steps = {
            "step1_" + std::to_string(i),
            "step2_" + std::to_string(i),
            "step3_" + std::to_string(i)
        };
        
        auto start = std::chrono::high_resolution_clock::now();
        
        bool success = agent->create_research_workflow(
            "perf_workflow_" + std::to_string(i),
            "Performance Workflow " + std::to_string(i),
            steps
        );
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        EXPECT_TRUE(success) << "Workflow creation " << i << " failed";
        creation_times.push_back(duration.count());
    }
    
    double average = std::accumulate(creation_times.begin(), creation_times.end(), 0.0) / num_workflows;
    
    std::cout << "Workflow Creation Performance:" << std::endl;
    std::cout << "  Average creation time: " << average << "μs" << std::endl;
    std::cout << "  Total workflows created: " << num_workflows << std::endl;
    
    // Workflow creation should be fast
    EXPECT_LT(average, 10000.0) << "Workflow creation too slow: " << average << "μs";
    
    // Verify workflows are available
    auto available = agent->get_available_workflows();
    EXPECT_GE(available.size(), num_workflows) << "Not all workflows were created";
}

// Configuration Performance Tests
TEST_F(DeepResearchAgentPerformanceTest, ConfigurationChangeImpact) {
    auto agent = create_test_agent(false);
    ASSERT_TRUE(agent->start());
    
    const std::string research_question = "Configuration impact test";
    
    // Test different configuration settings
    std::vector<std::pair<std::string, ResearchConfig>> configs;
    
    // Minimal config
    ResearchConfig minimal = base_config;
    minimal.max_sources = 1;
    minimal.max_web_results = 1;
    configs.push_back({"Minimal", minimal});
    
    // Standard config
    configs.push_back({"Standard", base_config});
    
    // Heavy config
    ResearchConfig heavy = base_config;
    heavy.max_sources = 30;
    heavy.max_web_results = 20;
    configs.push_back({"Heavy", heavy});
    
    std::cout << "Configuration Impact Performance:" << std::endl;
    
    for (const auto& config_pair : configs) {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto result = agent->conduct_research(research_question, config_pair.second);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        EXPECT_FALSE(result.full_report.empty()) << config_pair.first << " config research failed";
        
        std::cout << "  " << config_pair.first << " config: " << duration.count() << "ms" << std::endl;
    }
}

// Stress Tests
TEST_F(DeepResearchAgentPerformanceTest, StressTestContinuousOperations) {
    auto agent = create_test_agent(false);
    ASSERT_TRUE(agent->start());
    
    const int stress_duration_seconds = 30; // Run for 30 seconds
    const std::string base_question = "Stress test query ";
    
    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::seconds(stress_duration_seconds);
    
    int operations_completed = 0;
    std::vector<double> operation_times;
    
    std::cout << "Stress Test - Continuous Operations:" << std::endl;
    
    while (std::chrono::steady_clock::now() < end_time) {
        auto op_start = std::chrono::high_resolution_clock::now();
        
        auto result = agent->conduct_research(
            base_question + std::to_string(operations_completed), 
            base_config
        );
        
        auto op_end = std::chrono::high_resolution_clock::now();
        auto op_duration = std::chrono::duration_cast<std::chrono::milliseconds>(op_end - op_start);
        
        if (!result.full_report.empty()) {
            operations_completed++;
            operation_times.push_back(op_duration.count());
        }
        
        // Small delay to prevent overwhelming
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (!operation_times.empty()) {
        double average_time = std::accumulate(operation_times.begin(), operation_times.end(), 0.0) / operation_times.size();
        
        std::cout << "  Duration: " << stress_duration_seconds << " seconds" << std::endl;
        std::cout << "  Operations completed: " << operations_completed << std::endl;
        std::cout << "  Average operation time: " << average_time << "ms" << std::endl;
        std::cout << "  Operations per second: " << (double)operations_completed / stress_duration_seconds << std::endl;
        
        // Should maintain reasonable performance under stress
        EXPECT_GT(operations_completed, stress_duration_seconds / 10) << "Too few operations completed under stress";
        EXPECT_LT(average_time, 10000.0) << "Performance degraded too much under stress: " << average_time << "ms";
    } else {
        FAIL() << "No operations completed during stress test";
    }
}

// Benchmark Functions (for integration with Google Benchmark if available)
#ifdef BENCHMARK_FOUND

static void BM_AgentInitialization(benchmark::State& state) {
    for (auto _ : state) {
        DeepResearchAgent agent("BenchmarkAgent", "http://localhost:8080", false);
        benchmark::DoNotOptimize(agent.initialize());
    }
}
BENCHMARK(BM_AgentInitialization);

static void BM_BasicResearch(benchmark::State& state) {
    DeepResearchAgent agent("BenchmarkAgent", "http://localhost:8080", false);
    agent.start();
    
    ResearchConfig config;
    config.max_sources = 5;
    config.max_web_results = 3;
    
    for (auto _ : state) {
        auto result = agent.conduct_research("Benchmark test query", config);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_BasicResearch);

static void BM_WorkflowCreation(benchmark::State& state) {
    DeepResearchAgent agent("BenchmarkAgent", "http://localhost:8080", false);
    agent.start();
    
    std::vector<std::string> steps = {"step1", "step2", "step3"};
    
    int counter = 0;
    for (auto _ : state) {
        bool success = agent.create_research_workflow(
            "benchmark_workflow_" + std::to_string(counter++),
            "Benchmark Workflow",
            steps
        );
        benchmark::DoNotOptimize(success);
    }
}
BENCHMARK(BM_WorkflowCreation);

#endif // BENCHMARK_FOUND
