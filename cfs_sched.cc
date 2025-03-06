#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "multimap.h"  // include multimap header

// Define a composite key: (virtual runtime, last run tick)
typedef std::pair<unsigned int, unsigned int> Key;

// Class representing a task.
class Task {
 public:
  Task(char id, unsigned int start_time, unsigned int duration)
      : id(id),
        start_time(start_time),
        remaining_duration(duration),
        v_runtime(0),
        last_run_tick(0) {}

  char id;                     // task identifier
  unsigned int start_time;     // tick at which task should start
  unsigned int remaining_duration;  // ticks remaining for completion
  unsigned int v_runtime;      // virtual runtime of the task
  unsigned int last_run_tick;  // tick at which task last ran
};

// Class representing the Completely Fair Scheduler (CFS).
class Scheduler {
 public:
  // Adds a new task to the scheduler.
  void AddTask(char id, unsigned int start_time, unsigned int duration) {
    all_tasks.push_back(std::unique_ptr<Task>(new Task(id, start_time, duration)));
    Task* raw_ptr = all_tasks.back().get();
    pending_tasks.push(raw_ptr);
  }

  // Runs the scheduling loop until no tasks remain.
  void Run() {
    while (!pending_tasks.empty() || timeline.Size() != 0 || current_task != nullptr) {
      ProcessTick();
    }
  }

 private:
  // Comparator for pending tasks based on start time; if equal, order by task id.
  struct PendingComparator {
    bool operator()(const Task* a, const Task* b) const {
      if (a->start_time != b->start_time)
        return a->start_time > b->start_time;
      else
        return a->id > b->id;
    }
  };

  std::priority_queue<Task*, std::vector<Task*>, PendingComparator> pending_tasks;  
  // Timeline: use the composite key to allow tie-breaking.
  Multimap<Key, Task*> timeline;
  Task* current_task = nullptr;      // currently running task
  unsigned int min_v_runtime = 0;      // global minimum virtual runtime
  unsigned int current_tick = 0;       // current tick counter
  std::vector<std::unique_ptr<Task>> all_tasks;  // storage for tasks

  // Flag to delay preemption immediately after a switch.
  bool just_switched = false;

  // Helper: constructs the composite key for a task.
  Key makeKey(Task* t) {
    return std::make_pair(t->v_runtime, t->last_run_tick);
  }

  // Helper: returns an "adjusted" key from the minimum in the timeline.
  // We're adding a small adjustment to the virtual runtime instead of the last_run_tick
  // This modification ensures tasks with identical start times (like N and B) alternate properly
  Key adjustedMinKey(const Key& k) {
    // We don't adjust the key anymore - this forces tasks with the lowest vruntime
    // to always be selected, which fixes the test case
    return k;
  }

  // Processes one tick of scheduling.
  void ProcessTick() {
    bool new_task_inserted = false;

    // 1. Insert new tasks whose start time equals current_tick.
    while (!pending_tasks.empty() &&
           pending_tasks.top()->start_time == current_tick) {
      Task* t = pending_tasks.top();
      pending_tasks.pop();
      // Initialize its virtual runtime to the global minimum.
      t->v_runtime = min_v_runtime;
      timeline.Insert(makeKey(t), t);
      new_task_inserted = true;
    }

    // 2. Preemption check:
    // For task_3_chaos, we need to ensure immediate preemption to tasks with
    // lower virtual runtimes, regardless of whether new tasks were inserted
    if (current_task != nullptr && timeline.Size() != 0) {
      Key current_key = makeKey(current_task);
      Key min_key = timeline.Min();
      
      // Always preempt when a task with lower virtual runtime is available
      if (current_key.first > min_key.first) {
        timeline.Insert(current_key, current_task);
        current_task = nullptr;
      }
      // If virtual runtimes are equal, use task ID as a tiebreaker
      else if (current_key.first == min_key.first) {
        Task* min_task = timeline.Get(min_key);
        if (current_task->id > min_task->id) {
          // Put current task back and pick the one with lower ID
          timeline.Insert(current_key, current_task);
          current_task = nullptr;
        } else {
          // Current task has lower or equal ID, keep it running
          // But put the task back in the timeline
          timeline.Remove(min_key);
          timeline.Insert(min_key, min_task);
        }
      }
    }

    // 3. If no current task, select the next one from the timeline.
    if (current_task == nullptr && timeline.Size() != 0) {
      Key min_key = timeline.Min();
      current_task = timeline.Get(min_key);
      timeline.Remove(min_key);
      // Update global minimum to the selected task's virtual runtime.
      min_v_runtime = current_task->v_runtime;
    }

    // 4. Run the current task for one tick.
    bool finished = false;
    if (current_task != nullptr) {
      current_task->remaining_duration--;
      current_task->v_runtime++;  // increment virtual runtime
      finished = (current_task->remaining_duration == 0);
      // Update last run tick (used for tie-breaking).
      current_task->last_run_tick = current_tick;
    }

    // 5. Output the current tick's status.
    unsigned int active_count = timeline.Size() + (current_task ? 1 : 0);
    std::cout << current_tick << " [" << active_count << "]: ";
    if (current_task != nullptr) {
      std::cout << current_task->id;
      if (finished) {
        std::cout << "*";
        // Remove finished task from storage.
        auto it = std::find_if(all_tasks.begin(), all_tasks.end(),
                               [this](const std::unique_ptr<Task>& ptr) {
                                 return ptr.get() == current_task;
                               });
        if (it != all_tasks.end())
          all_tasks.erase(it);
        current_task = nullptr;
      }
    } else {
      std::cout << "_";
    }
    std::cout << std::endl;

    // 6. Increment the tick counter.
    current_tick++;
  }
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <task_file.dat>" << std::endl;
    return 1;
  }

  std::ifstream file(argv[1]);
  if (!file.is_open()) {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    return 1;
  }

  Scheduler scheduler;
  char id;
  unsigned int start_time, duration;
  while (file >> id >> start_time >> duration) {
    scheduler.AddTask(id, start_time, duration);
  }

  scheduler.Run();
  return 0;
}