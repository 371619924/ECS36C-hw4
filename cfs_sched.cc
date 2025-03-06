#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "multimap.h"  // include multimap header

// Class representing a task
class Task {
 public:
  // Constructor for Task; initializes id, start time, duration,
  // and sets initial virtual runtime to 0
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

// Class representing the Completely Fair Scheduler (CFS)
class Scheduler {
 public:
  // Adds a new task to the scheduler, creates a new Task object,
  // stores its pointer in all_tasks and adds it to the pending queue
  void AddTask(char id, unsigned int start_time, unsigned int duration) {
    all_tasks.push_back(
        std::unique_ptr<Task>(new Task(id, start_time, duration)));
          // create and add new task using new (std::make_unique not in C++11)
    Task* raw_ptr = all_tasks.back().get();  // get raw pointer
    pending_tasks.push(raw_ptr);             // add task to pending queue
  }

  // Runs the scheduling loop until no tasks remain pending or active
  void Run() {
    while (!pending_tasks.empty() || timeline.Size() != 0 ||
           current_task != nullptr) {
      ProcessTick();
    }
  }

 private:
  // Comparator for pending tasks based on start time
  // If start times are equal, order by task identifier
  struct PendingComparator {
    bool operator()(const Task* a, const Task* b) const {
      if (a->start_time != b->start_time) {
        return a->start_time > b->start_time;
      } else {
        return a->id > b->id;
      }
    }
  };

  std::priority_queue<Task*, std::vector<Task*>, PendingComparator>
      pending_tasks;               // tasks waiting to be launched
  Multimap<unsigned int, Task*> timeline;  // timeline for scheduling
  Task* current_task = nullptr;  // task currently running
  unsigned int min_v_runtime = 0;  // global minimum virtual runtime
  unsigned int current_tick = 0;   // current tick counter
  std::vector<std::unique_ptr<Task>> all_tasks;  // storage for tasks

  // Processes a single tick of the scheduler
  void ProcessTick() {
    // 1. Insert new tasks whose start time equals current_tick
    while (!pending_tasks.empty() &&
           pending_tasks.top()->start_time == current_tick) {
      Task* task = pending_tasks.top();  // get pending task
      pending_tasks.pop();               // remove it from pending queue
      // initialize its virtual runtime to current global value
      task->v_runtime = min_v_runtime;
      timeline.Insert(task->v_runtime, task);  // insert into timeline
    }

    // 2. Preemption check:
    // If a task is running and the timeline is not empty, and the running
    // task's virtual runtime is greater than the smallest in timeline,
    // then preempt it
    if (current_task != nullptr && timeline.Size() != 0 &&
        current_task->v_runtime > timeline.Min()) {
      timeline.Insert(current_task->v_runtime, current_task);
          // reinsert current task into timeline
      current_task = nullptr;  // yield current task
    }

    // 3. If no current task is running, select the next task from timeline
    if (current_task == nullptr && timeline.Size() != 0) {
      unsigned int min_key = timeline.Min();  // get smallest vruntime key
      current_task = timeline.Get(min_key);    // select next task
      timeline.Remove(min_key);                // remove it from timeline
      min_v_runtime = current_task->v_runtime;   // update global min_v_runtime
    }

    // 4. Run the current task for one tick
    bool finished = false;
    if (current_task != nullptr) {
      current_task->remaining_duration--;  // decrement duration
      current_task->v_runtime++;             // increment virtual runtime
      finished = (current_task->remaining_duration == 0);
      // update last run tick for tie-breaking if needed
      current_task->last_run_tick = current_tick;
    }

    // 5. Calculate number of active tasks (in timeline plus current task)
    unsigned int active_count = timeline.Size() +
        (current_task != nullptr ? 1 : 0);

    // 6. Output current tick status
    // Format: <tick> [<#tasks>]: <task_id> with '*' if finished
    std::cout << current_tick << " [" << active_count << "]: ";
    if (current_task != nullptr) {
      std::cout << current_task->id;
      if (finished) {
        std::cout << "*";
        // After printing, remove the finished task from all_tasks
        auto it = std::find_if(all_tasks.begin(), all_tasks.end(),
            [this](const std::unique_ptr<Task>& ptr) {
              return ptr.get() == current_task;
            });
        if (it != all_tasks.end()) {
          all_tasks.erase(it);
        }
        current_task = nullptr;  // clear finished task
      }
    } else {
      std::cout << '_';
    }
    std::cout << std::endl;

    // 7. Increment tick counter
    current_tick++;
  }
};

int main(int argc, char* argv[]) {
  // Check for correct number of command-line arguments
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0]
              << " <task_file.dat>" << std::endl;
    return 1;
  }

  // Open the input file containing task descriptions
  std::ifstream file(argv[1]);
  if (!file.is_open()) {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    return 1;
  }

  Scheduler scheduler;  // create a scheduler instance
  char id;
  unsigned int start_time, duration;
  // Read tasks from file and add them to the scheduler
  while (file >> id >> start_time >> duration) {
    scheduler.AddTask(id, start_time, duration);
  }

  scheduler.Run();  // run the scheduler

  return 0;
}
