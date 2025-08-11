/**
 * @file task_job_manager.hpp
 * @brief Task and job scheduling and execution
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_TASK_JOB_MANAGER_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_TASK_JOB_MANAGER_HPP_INCLUDED

#include "export.hpp"
#include "function_execution_manager.hpp"
#include "agent/core/agent_data.hpp"
#include <queue>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

namespace kolosal::agents {

enum class JobStatus {
    PENDING, RUNNING, COMPLETED, FAILED, CANCELLED
};

struct KOLOSAL_SERVER_API Job {
    std::string id;
    std::string function_name;
    AgentData parameters;
    JobStatus status = JobStatus::PENDING;
    FunctionResult result;
    std::string requester;
    int priority = 0;
    Job(const std::string& func_name, const AgentData& parameters);
};

/**
 * @brief Manages job queue and execution for agents
 */
class KOLOSAL_SERVER_API JobManager {
private:
    std::queue<std::shared_ptr<Job>> job_queue;
    std::unordered_map<std::string, std::shared_ptr<Job>> all_jobs;
    std::shared_ptr<FunctionManager> function_manager;
    std::shared_ptr<Logger> logger;
    mutable std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::atomic<bool> running {false};
    std::thread worker_thread;

public:
    JobManager(std::shared_ptr<FunctionManager> func_mgr, std::shared_ptr<Logger> log);
    ~JobManager();

    void start();
    void stop();

    std::string submit_job(const std::string& function_name, const AgentData& parameters, 
                          const int priority = 0, const std::string& requester = "");
    JobStatus get__job_status(const std::string& job_id) const;
    FunctionResult get__job_result(const std::string& job_id) const;
    bool cancel_job(const std::string& job_id);
    std::map<std::string, int> get__stats() const;

private:
    void worker_loop();
};

} // namespace kolosal::agents

#endif // KOLOSAL_AGENT_INCLUDE_TASK_JOB_MANAGER_HPP_INCLUDED
