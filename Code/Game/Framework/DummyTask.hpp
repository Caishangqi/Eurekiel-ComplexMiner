#pragma once
#include "Engine/Core/Schedule/RunnableTask.hpp"
#include <string>

//-----------------------------------------------------------------------------------------------
// DummyTask: Simple test task for validating the Schedule Subsystem
//
// PURPOSE:
// - Demonstrates how to create a custom task by inheriting from RunnableTask
// - Used for Phase 1 testing and validation
// - Shows proper task lifecycle: construction -> execution -> completion
//
// USAGE EXAMPLE:
//   DummyTask* task = new DummyTask("Generic", "Test Task", 100);
//   g_theSchedule->AddTask(task);
//   // ... later, after task completes ...
//   delete task;
//
// EDUCATIONAL NOTE:
// This is a minimal task implementation showing the essential pattern:
// 1. Constructor: Initialize task type and custom data
// 2. Execute(): Perform the actual work (here: simulate work with sleep)
// 3. Destructor: Clean up (here: just logging)
//-----------------------------------------------------------------------------------------------
class DummyTask : public enigma::core::RunnableTask
{
public:
    //-------------------------------------------------------------------------------------------
    // Constructor: Create a dummy task with specified type and work duration
    //
    // Parameters:
    //   taskType     - Task type string (e.g., "Generic", "FileIO")
    //   taskName     - Human-readable task name for logging
    //   workDuration - Simulated work time in milliseconds
    //-------------------------------------------------------------------------------------------
    DummyTask(const std::string& taskName, int workDurationMs);

    ~DummyTask() override;

    //-------------------------------------------------------------------------------------------
    // Execute: Perform the task's work (pure virtual from RunnableTask)
    //
    // IMPLEMENTATION:
    // - Logs start of execution
    // - Simulates work by sleeping for specified duration
    // - Logs completion
    //
    // NOTE: This is called by worker threads, not the main thread!
    // - Must be thread-safe if accessing shared data
    // - State changes are handled by ScheduleSubsystem
    //-------------------------------------------------------------------------------------------
    void Execute() override;

    // Accessors for task information
    const std::string& GetTaskName() const { return m_taskName; }
    int                GetWorkDuration() const { return m_workDurationMs; }

private:
    std::string m_taskName = "DummyTask"; // Human-readable task name
    int         m_workDurationMs; // Simulated work duration in milliseconds
};
