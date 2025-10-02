#include "DummyTask.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <thread>
#include <chrono>

#include "Engine/Core/Logger/LoggerAPI.hpp"
using namespace enigma::core;
//-----------------------------------------------------------------------------------------------
// Constructor: Initialize task with type, name, and work duration
//
// EDUCATIONAL NOTE:
// - Calls parent constructor with task type string
// - Parent (RunnableTask) sets initial state to Queued
// - Worker threads will later change state to Executing -> Completed
//-----------------------------------------------------------------------------------------------
DummyTask::DummyTask(const std::string& taskName, int workDurationMs)
    : m_taskName(taskName)
      , m_workDurationMs(workDurationMs)
{
    m_type = "Generic"; //Rendering
    LogInfo("DummyTask", "DummyTask created: '%s' (type=%s, duration=%dms)", m_taskName.c_str(), m_type.c_str(), workDurationMs);
}

//-----------------------------------------------------------------------------------------------
// Destructor: Clean up task
//
// NOTE: Called by main thread after RetrieveCompletedTasks()
// - Task should be in Completed state by this point
// - Worker threads no longer touch this task
//-----------------------------------------------------------------------------------------------
DummyTask::~DummyTask()
{
    LogInfo("DummyTask", "DummyTask destroyed: '%s'", m_taskName.c_str());
}

//-----------------------------------------------------------------------------------------------
// Execute: Perform the task's work
//
// CALLED BY: Worker thread (NOT main thread!)
//
// IMPLEMENTATION:
// 1. Log start of work
// 2. Simulate work by sleeping (replace with real work in actual tasks)
// 3. Log completion
//
// THREAD SAFETY:
// - This function runs on a worker thread
// - Multiple tasks execute concurrently on different threads
// - Accessing shared data requires synchronization (mutex, atomic, etc.)
// - Here we only access task-local data (m_taskName, m_workDurationMs)
//
// EDUCATIONAL NOTE:
// Real tasks would do actual work here:
// - File I/O: Load texture, read config file
// - Computation: Generate chunk mesh, pathfinding
// - Network: Send HTTP request, download asset
//-----------------------------------------------------------------------------------------------
void DummyTask::Execute()
{
    LogInfo("DummyTask", "DummyTask '%s' starting execution...", m_taskName.c_str());
    // Simulate work by sleeping
    std::this_thread::sleep_for(std::chrono::milliseconds(m_workDurationMs));
    LogInfo("DummyTask", "DummyTask '%s' completed execution", m_taskName.c_str());
}
