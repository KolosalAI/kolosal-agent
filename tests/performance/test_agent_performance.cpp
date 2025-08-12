/**
 * @file test_agent_performance.cpp
 * @brief Performance tests for agent operations
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "agent/core/agent_core.hpp"
#include "../fixtures/test_fixtures.hpp"
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <future>

using namespace testing;
using namespace kolosal::agents;
using namespace kolosal::agents::test;

class AgentPerformanceTest : public AgentTestFixture {
protected:
    void SetUp() override {
        AgentTestFixture::SetUp();
    }
    
    // Helper to measure execution time
    template<typename Func>
    double measureExecutionTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return duration.count() / 1000.0; // Return milliseconds
    }
};

TEST_F(AgentPerformanceTest, AgentStartupTime) {
    const int num_trials = 10;
    std::vector<double> startup_times;
    
    for (int i = 0; i < num_trials; ++i) {
        auto agent = std::make_shared<AgentCore>("perf_agent_" + std::to_string(i), "test", AgentRole::ASSISTANT);
        
        double startup_time = measureExecutionTime([&agent]() {
            agent->start();
        });
        
        startup_times.push_back(startup_time);
        agent->stop();
    }
    
    // Calculate statistics
    double total_time = 0.0;
    double max_time = 0.0;
    double min_time = startup_times[0];
    
    for (double time : startup_times) {
        total_time += time;
        max_time = std::max(max_time, time);
        min_time = std::min(min_time, time);
    }
    
    double avg_time = total_time / num_trials;
    
    // Log performance metrics
    std::cout << "Agent Startup Performance:" << std::endl;
    std::cout << "  Average: " << avg_time << " ms" << std::endl;
    std::cout << "  Min: " << min_time << " ms" << std::endl;
    std::cout << "  Max: " << max_time << " ms" << std::endl;
    
    // Performance expectations (these can be adjusted based on requirements)
    EXPECT_LT(avg_time, 100.0); // Average startup should be under 100ms
    EXPECT_LT(max_time, 500.0); // Max startup should be under 500ms
}

TEST_F(AgentPerformanceTest, ConcurrentFunctionExecution) {
    test_agent_->start();
    
    const int num_concurrent_operations = 100;
    const int batch_size = 10;
    std::atomic<int> completed_operations{0};
    std::atomic<int> successful_operations{0};
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<void>> futures;
    
    // Launch concurrent operations in batches to avoid overwhelming the system
    for (int batch = 0; batch < num_concurrent_operations / batch_size; ++batch) {
        std::vector<std::future<void>> batch_futures;
        
        for (int i = 0; i < batch_size; ++i) {
            int operation_id = batch * batch_size + i;
            
            batch_futures.push_back(std::async(std::launch::async, [this, operation_id, &completed_operations, &successful_operations]() {
                try {
                    AgentData params{{"operation_id", operation_id}, {"data", "test_data_" + std::to_string(operation_id)}};
                    std::string job_id = test_agent_->execute_function_async("echo", params);
                    
                    if (!job_id.empty()) {
                        successful_operations++;
                    }
                    completed_operations++;
                } catch (...) {
                    completed_operations++;
                }
            }));
        }
        
        // Wait for this batch to complete before starting the next
        for (auto& future : batch_futures) {
            future.wait();
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    double throughput = (double)successful_operations.load() / (total_time.count() / 1000.0);
    
    std::cout << "Concurrent Function Execution Performance:" << std::endl;
    std::cout << "  Total operations: " << num_concurrent_operations << std::endl;
    std::cout << "  Completed operations: " << completed_operations.load() << std::endl;
    std::cout << "  Successful operations: " << successful_operations.load() << std::endl;
    std::cout << "  Total time: " << total_time.count() << " ms" << std::endl;
    std::cout << "  Throughput: " << throughput << " ops/sec" << std::endl;
    
    EXPECT_EQ(completed_operations.load(), num_concurrent_operations);
    EXPECT_GT(successful_operations.load(), num_concurrent_operations * 0.8); // At least 80% success rate
    EXPECT_GT(throughput, 10.0); // At least 10 operations per second
}

TEST_F(AgentPerformanceTest, MemoryOperationPerformance) {
    test_agent_->start();
    
    const int num_memory_operations = 1000;
    
    // Test memory storage performance
    auto storage_time = measureExecutionTime([this, num_memory_operations]() {
        for (int i = 0; i < num_memory_operations; ++i) {
            std::string content = "Performance test memory entry " + std::to_string(i) + 
                                " with some additional content to make it more realistic";
            test_agent_->store_memory(content, "performance_test");
        }
    });
    
    double storage_throughput = num_memory_operations / (storage_time / 1000.0);
    
    // Test memory recall performance
    const int num_recall_operations = 100;
    auto recall_time = measureExecutionTime([this, num_recall_operations]() {
        for (int i = 0; i < num_recall_operations; ++i) {
            auto memories = test_agent_->recall_memories("performance test " + std::to_string(i % 10), 5);
        }
    });
    
    double recall_throughput = num_recall_operations / (recall_time / 1000.0);
    
    std::cout << "Memory Operation Performance:" << std::endl;
    std::cout << "  Storage: " << storage_throughput << " ops/sec (" << storage_time << " ms total)" << std::endl;
    std::cout << "  Recall: " << recall_throughput << " ops/sec (" << recall_time << " ms total)" << std::endl;
    
    // Performance expectations
    EXPECT_GT(storage_throughput, 100.0); // At least 100 storage ops/sec
    EXPECT_GT(recall_throughput, 50.0);   // At least 50 recall ops/sec
}

TEST_F(AgentPerformanceTest, WorkingContextPerformance) {
    test_agent_->start();
    
    const int num_context_operations = 5000;
    
    // Test context setting performance
    auto set_time = measureExecutionTime([this, num_context_operations]() {
        for (int i = 0; i < num_context_operations; ++i) {
            AgentData data{
                {"key_" + std::to_string(i % 100), "value_" + std::to_string(i)},
                {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()},
                {"iteration", i}
            };
            test_agent_->set_working_context("context_" + std::to_string(i % 500), data);
        }
    });
    
    // Test context getting performance
    auto get_time = measureExecutionTime([this, num_context_operations]() {
        for (int i = 0; i < num_context_operations; ++i) {
            auto data = test_agent_->get_working_context("context_" + std::to_string(i % 500));
        }
    });
    
    double set_throughput = num_context_operations / (set_time / 1000.0);
    double get_throughput = num_context_operations / (get_time / 1000.0);
    
    std::cout << "Working Context Performance:" << std::endl;
    std::cout << "  Set operations: " << set_throughput << " ops/sec" << std::endl;
    std::cout << "  Get operations: " << get_throughput << " ops/sec" << std::endl;
    
    EXPECT_GT(set_throughput, 1000.0); // At least 1000 set ops/sec
    EXPECT_GT(get_throughput, 2000.0); // At least 2000 get ops/sec
}

TEST_F(AgentPerformanceTest, MessageSendingPerformance) {
    test_agent_->start();
    
    const int num_messages = 1000;
    
    auto message_time = measureExecutionTime([this, num_messages]() {
        for (int i = 0; i < num_messages; ++i) {
            AgentData payload{
                {"message_id", i},
                {"content", "Performance test message " + std::to_string(i)},
                {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
            };
            
            if (i % 2 == 0) {
                test_agent_->send_message("target_agent", "test_message", payload);
            } else {
                test_agent_->broadcast_message("test_broadcast", payload);
            }
        }
    });
    
    double message_throughput = num_messages / (message_time / 1000.0);
    
    std::cout << "Message Sending Performance:" << std::endl;
    std::cout << "  Throughput: " << message_throughput << " messages/sec" << std::endl;
    std::cout << "  Total time: " << message_time << " ms" << std::endl;
    
    EXPECT_GT(message_throughput, 500.0); // At least 500 messages/sec
}

TEST_F(AgentPerformanceTest, StatisticsRetrievalPerformance) {
    test_agent_->start();
    
    // Generate some activity first
    for (int i = 0; i < 100; ++i) {
        AgentData params{{"data", "stats_test_" + std::to_string(i)}};
        test_agent_->execute_function_async("echo", params);
        test_agent_->store_memory("Stats test memory " + std::to_string(i), "stats_test");
    }
    
    // Allow some processing time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    const int num_stats_calls = 1000;
    auto stats_time = measureExecutionTime([this, num_stats_calls]() {
        for (int i = 0; i < num_stats_calls; ++i) {
            auto stats = test_agent_->get_statistics();
        }
    });
    
    double stats_throughput = num_stats_calls / (stats_time / 1000.0);
    
    std::cout << "Statistics Retrieval Performance:" << std::endl;
    std::cout << "  Throughput: " << stats_throughput << " calls/sec" << std::endl;
    
    EXPECT_GT(stats_throughput, 1000.0); // At least 1000 stats calls/sec
}

TEST_F(AgentPerformanceTest, MultiAgentConcurrency) {
    const int num_agents = 10;
    const int operations_per_agent = 100;
    
    std::vector<std::shared_ptr<AgentCore>> agents;
    
    // Create agents
    auto creation_time = measureExecutionTime([&]() {
        for (int i = 0; i < num_agents; ++i) {
            auto agent = std::make_shared<AgentCore>(
                "perf_agent_" + std::to_string(i), 
                "performance_test", 
                AgentRole::WORKER
            );
            agent->start();
            agents.push_back(agent);
        }
    });
    
    // Perform concurrent operations
    std::atomic<int> total_operations{0};
    std::atomic<int> successful_operations{0};
    
    auto operation_time = measureExecutionTime([&]() {
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_agents; ++i) {
            threads.emplace_back([&, i]() {
                auto& agent = agents[i];
                
                for (int j = 0; j < operations_per_agent; ++j) {
                    try {
                        // Mixed operations
                        if (j % 4 == 0) {
                            agent->store_memory("Multi-agent test " + std::to_string(j), "multi_test");
                        } else if (j % 4 == 1) {
                            AgentData context{{"agent", i}, {"op", j}};
                            agent->set_working_context("op_" + std::to_string(j), context);
                        } else if (j % 4 == 2) {
                            AgentData params{{"agent_id", i}, {"operation", j}};
                            std::string job = agent->execute_function_async("process", params);
                            if (!job.empty()) successful_operations++;
                        } else {
                            auto memories = agent->recall_memories("test", 3);
                        }
                        
                        total_operations++;
                        
                    } catch (...) {
                        // Handle errors gracefully
                    }
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    });
    
    // Cleanup
    auto cleanup_time = measureExecutionTime([&]() {
        for (auto& agent : agents) {
            agent->stop();
        }
    });
    
    double total_throughput = total_operations.load() / (operation_time / 1000.0);
    
    std::cout << "Multi-Agent Concurrency Performance:" << std::endl;
    std::cout << "  Number of agents: " << num_agents << std::endl;
    std::cout << "  Operations per agent: " << operations_per_agent << std::endl;
    std::cout << "  Total operations: " << total_operations.load() << std::endl;
    std::cout << "  Successful async operations: " << successful_operations.load() << std::endl;
    std::cout << "  Creation time: " << creation_time << " ms" << std::endl;
    std::cout << "  Operation time: " << operation_time << " ms" << std::endl;
    std::cout << "  Cleanup time: " << cleanup_time << " ms" << std::endl;
    std::cout << "  Total throughput: " << total_throughput << " ops/sec" << std::endl;
    
    // Performance expectations
    EXPECT_LT(creation_time, 5000.0); // Agent creation should be under 5 seconds
    EXPECT_LT(cleanup_time, 2000.0);  // Cleanup should be under 2 seconds
    EXPECT_GT(total_throughput, 100.0); // At least 100 ops/sec total
    EXPECT_EQ(total_operations.load(), num_agents * operations_per_agent);
}

TEST_F(AgentPerformanceTest, LongRunningAgentStability) {
    test_agent_->start();
    
    const int runtime_seconds = 10;  // Run for 10 seconds
    const int operation_interval_ms = 50; // Operation every 50ms
    
    std::atomic<bool> keep_running{true};
    std::atomic<int> operations_completed{0};
    std::atomic<int> errors_encountered{0};
    
    // Start background operations
    std::thread worker_thread([&]() {
        int operation_id = 0;
        while (keep_running.load()) {
            try {
                // Rotate between different operation types
                switch (operation_id % 4) {
                    case 0: {
                        AgentData params{{"id", operation_id}};
                        test_agent_->execute_function_async("echo", params);
                        break;
                    }
                    case 1: {
                        test_agent_->store_memory(
                            "Long running test memory " + std::to_string(operation_id), 
                            "stability_test"
                        );
                        break;
                    }
                    case 2: {
                        AgentData context{{"operation_id", operation_id}};
                        test_agent_->set_working_context("stable_context", context);
                        break;
                    }
                    case 3: {
                        auto memories = test_agent_->recall_memories("stability", 2);
                        break;
                    }
                }
                
                operations_completed++;
                operation_id++;
                
            } catch (...) {
                errors_encountered++;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(operation_interval_ms));
        }
    });
    
    // Monitor memory usage and performance
    std::vector<double> response_times;
    std::thread monitor_thread([&]() {
        while (keep_running.load()) {
            auto start = std::chrono::high_resolution_clock::now();
            auto stats = test_agent_->get_statistics();
            auto end = std::chrono::high_resolution_clock::now();
            
            auto response_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
            response_times.push_back(response_time);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    
    // Let it run for the specified time
    std::this_thread::sleep_for(std::chrono::seconds(runtime_seconds));
    
    keep_running.store(false);
    worker_thread.join();
    monitor_thread.join();
    
    // Calculate statistics
    double avg_response_time = 0.0;
    if (!response_times.empty()) {
        double total_time = 0.0;
        for (double time : response_times) {
            total_time += time;
        }
        avg_response_time = total_time / response_times.size();
    }
    
    double operations_per_second = operations_completed.load() / (double)runtime_seconds;
    double error_rate = (double)errors_encountered.load() / operations_completed.load();
    
    std::cout << "Long Running Agent Stability:" << std::endl;
    std::cout << "  Runtime: " << runtime_seconds << " seconds" << std::endl;
    std::cout << "  Operations completed: " << operations_completed.load() << std::endl;
    std::cout << "  Errors encountered: " << errors_encountered.load() << std::endl;
    std::cout << "  Operations per second: " << operations_per_second << std::endl;
    std::cout << "  Error rate: " << (error_rate * 100.0) << "%" << std::endl;
    std::cout << "  Average response time: " << avg_response_time << " ms" << std::endl;
    
    // Stability expectations
    EXPECT_GT(operations_completed.load(), runtime_seconds * 10); // At least 10 ops/sec
    EXPECT_LT(error_rate, 0.05); // Less than 5% error rate
    EXPECT_LT(avg_response_time, 50.0); // Average response time under 50ms
}
