#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "multimap.h"

// Class representing a task
class Task {
 public:
  // Constructor for Task
  Task(char id, unsigned int start_time, unsigned int duration)
      : id(id),
        start_time(start_time),
        remaining_duration(duration),
        v_runtime(0) {}

  char id;                     // task identifier
  unsigned int start_time;     // tick at which task should start
  unsigned int remaining_duration;  // ticks remaining for completion
  unsigned int v_runtime;      // virtual runtime of the task
};

// Class representing the Completely Fair Scheduler (CFS)
class Scheduler {
 public:
  // Adds a new task to the scheduler
  void AddTask(char id, unsigned int start_time, unsigned int duration) {
    all_tasks.push_back(
        std::unique_ptr<Task>(new Task(id, start_time, duration)));
    Task* raw_ptr = all_tasks.back().get();
    pending_tasks.push(raw_ptr);
  }

  // Runs the scheduling loop until all tasks are completed
  void Run() {
    while (!pending_tasks.empty() || timeline.Size() > 0 ||
           current_task != nullptr) {
      ProcessTick();
    }
  }

 private:
  // Comparator for pending tasks based on start time and task identifier
  struct PendingComparator {
    bool operator()(const Task* a, const Task* b) const {
      if (a->start_time != b->start_time) {
        return a->start_time > b->start_time;
      } else {
        return a->id > b->id;
      }
    }
  };

  std::priority_queue<Task*, std::vector<Task*>, PendingComparator> pending_tasks;
  Multimap<unsigned int, Task*> timeline;
  Task* current_task = nullptr;
  unsigned int min_v_runtime = 0;
  unsigned int current_tick = 0;
  std::vector<std::unique_ptr<Task>> all_tasks;

  // Processes a single tick of the scheduler
  void ProcessTick() {
    // 1. Add tasks that are due to start at the current tick
    while (!pending_tasks.empty() &&
           pending_tasks.top()->start_time <= current_tick) {
      Task* task = pending_tasks.top();
      pending_tasks.pop();
      // Initialize with current minimum virtual runtime
      task->v_runtime = min_v_runtime;
      timeline.Insert(task->v_runtime, task);
    }

    // 2. If a task is currently running, add it back to the timeline
    if (current_task != nullptr) {
      // Only if the task is not completed
      if (current_task->remaining_duration > 0) {
        timeline.Insert(current_task->v_runtime, current_task);
      }
      current_task = nullptr;
    }

    // 3. Get the task with the minimum virtual runtime
    if (timeline.Size() > 0) {
      unsigned int min_key = timeline.Min();
      current_task = timeline.Get(min_key);
      timeline.Remove(min_key);
      min_v_runtime = current_task->v_runtime;
    }

    // 4. Calculate number of active tasks
    unsigned int active_count = timeline.Size() + (current_task != nullptr ? 1 : 0);

    // 5. Output current scheduling status
    std::cout << current_tick << " [" << active_count << "]: ";
    
    if (current_task != nullptr) {
      std::cout << current_task->id;
      
      // Run the task for one tick
      current_task->v_runtime++;
      current_task->remaining_duration--;
      
      // Check if task has completed
      if (current_task->remaining_duration == 0) {
        std::cout << "*";
        current_task = nullptr;
      }
    } else {
      std::cout << "_";
    }
    std::cout << std::endl;

    // 6. Increment tick counter
    current_tick++;
  }
};

int main(int argc, char* argv[]) {
  // Check for correct command line arguments
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <task_file.dat>" << std::endl;
    return 1;
  }

  // Open input file
  std::ifstream file(argv[1]);
  if (!file.is_open()) {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    return 1;
  }

  Scheduler scheduler;
  char id;
  unsigned int start_time, duration;
  
  // Read tasks from file
  while (file >> id >> start_time >> duration) {
    scheduler.AddTask(id, start_time, duration);
  }

  // Run the scheduler
  scheduler.Run();

  return 0;
}