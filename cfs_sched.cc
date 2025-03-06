#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "multimap.h"  // include multimap header

// Composite key: (virtual runtime, last run tick)
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

  // Runs the scheduling loop until no tasks remain pending or active.
  void Run() {
    while (!pending_tasks.empty() || timeline.Size() != 0 || current_task != nullptr) {
      ProcessTick();
    }
  }

 private:
  // Comparator for pending tasks based on start time and then task id.
  struct PendingComparator {
    bool operator()(const Task* a, const Task* b) const {
      if (a->start_time != b->start_time)
        return a->start_time > b->start_time;
      else
        return a->id > b->id;
    }
  };

  std::priority_queue<Task*, std::vector<Task*>, PendingComparator> pending_tasks;  // tasks waiting to be launched
  // Timeline now uses a composite key for tie-breaking.
  Multimap<Key, Task*> timeline;
  Task* current_task = nullptr;      // currently running task
  unsigned int min_v_runtime = 0;      // global minimum virtual runtime
  unsigned int current_tick = 0;       // current tick counter
  std::vector<std::unique_ptr<Task>> all_tasks;  // storage for tasks

  // Helper to construct the composite key for a task.
  Key makeKey(Task* task) {
    return std::make_pair(task->v_runtime, task->last_run_tick);
  }

  // Processes a single tick.
  void ProcessTick() {
    // 1. Insert new tasks whose start time equals current_tick.
    while (!pending_tasks.empty() && pending_tasks.top()->start_time == current_tick) {
      Task* task = pending_tasks.top();
      pending_tasks.pop();
      task->v_runtime = min_v_runtime;  // initialize v_runtime to global min
      timeline.Insert(makeKey(task), task);
    }

    // 2. Preemption check:
    // If a task is running and the timeline is not empty, compare the composite keys.
    if (current_task != nullptr && timeline.Size() != 0) {
      Key current_key = makeKey(current_task);
      Key min_key = timeline.Min();
      if (current_key > min_key) {  // lexicographic order: if current task is "worse"
        timeline.Insert(current_key, current_task);
        current_task = nullptr;
      }
    }

    // 3. If no task is running, select the next task from the timeline.
    if (current_task == nullptr && timeline.Size() != 0) {
      Key min_key = timeline.Min();
      current_task = timeline.Get(min_key);
      timeline.Remove(min_key);
      min_v_runtime = current_task->v_runtime;  // update global min
    }

    // 4. Run the current task for one tick.
    bool finished = false;
    if (current_task != nullptr) {
      current_task->remaining_duration--;
      current_task->v_runtime++;  // increment virtual runtime
      finished = (current_task->remaining_duration == 0);
      current_task->last_run_tick = current_tick;  // update last run tick for tie-breaking
    }

    // 5. Output the current tick status.
    unsigned int active_count = timeline.Size() + (current_task != nullptr ? 1 : 0);
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
        if (it != all_tasks.end()) {
          all_tasks.erase(it);
        }
        current_task = nullptr;
      }
    } else {
      std::cout << "_";
    }
    std::cout << std::endl;

    // 6. Increment tick counter.
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
