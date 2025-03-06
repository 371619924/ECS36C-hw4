#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "multimap.h" // include multimap header

// Class representing a task
class Task
{
public:
  // Constructor for Task; initializes id, start time, duration,
  // and sets initial virtual runtime to 0
  Task(char id, unsigned int start_time, unsigned int duration)
      : id(id),
        start_time(start_time),
        remaining_duration(duration),
        v_runtime(0),
        last_run_tick(0) {}

  char id;                         // task identifier
  unsigned int start_time;         // tick at which task should start
  unsigned int remaining_duration; // ticks remaining for completion
  unsigned int v_runtime;          // virtual runtime of the task
  unsigned int last_run_tick;      // tick at which task last ran
};

// Class representing the Completely Fair Scheduler (CFS)
class Scheduler
{
public:
  // Adds a new task to the scheduler, creates a new Task object,
  // stores its pointer in all_tasks and adds it to the pending queue
  void AddTask(char id, unsigned int start_time, unsigned int duration)
  {
    all_tasks.push_back(
        std::unique_ptr<Task>(new Task(id, start_time, duration)));
    // create and add new task using new (std::make_unique not in C++11)
    Task *raw_ptr = all_tasks.back().get(); // get raw pointer
    pending_tasks.push(raw_ptr);            // add task to pending queue
  }

  // Runs the scheduling loop until no tasks remain pending or active
  void Run()
  {
    while (!pending_tasks.empty() || timeline.Size() != 0 ||
           current_task != nullptr)
    {
      ProcessTick();
    }
  }

private:
  // Comparator for pending tasks based on start time
  // If start times are equal, order by task identifier
  struct PendingComparator
  {
    bool operator()(const Task *a, const Task *b) const
    {
      if (a->start_time != b->start_time)
      {
        return a->start_time > b->start_time;
      }
      else
      {
        return a->id > b->id;
      }
    }
  };

  std::priority_queue<Task *, std::vector<Task *>, PendingComparator>
      pending_tasks;                            // tasks waiting to be launched
  Multimap<unsigned int, Task *> timeline;      // timeline for scheduling
  Task *current_task = nullptr;                 // task currently running
  unsigned int min_v_runtime = 0;               // global minimum virtual runtime
  unsigned int current_tick = 0;                // current tick counter
  std::vector<std::unique_ptr<Task>> all_tasks; // storage for tasks

  // Processes a single tick of the scheduler
  void ProcessTick() {
    // (1) 将当前 tick 到达的任务从 pending 队列移入 timeline
    while (!pending_tasks.empty() &&
           pending_tasks.top()->start_time == current_tick) {
      Task* t = pending_tasks.top();
      pending_tasks.pop();
      // 为新任务设置初始 v_runtime = min_v_runtime（常见做法）
      // 或者你也可以做一些微调，例如 min_v_runtime - 1 等，但要注意不要破坏其他测试
      t->v_runtime = min_v_runtime;
      timeline.Insert(t->v_runtime, t);
    }
  
    // (2) 如果当前没有运行的任务，就从 timeline 选取下一个
    if (current_task == nullptr && timeline.Size() > 0) {
      unsigned int key = timeline.Min();       // 拿到最小的 v_runtime
      current_task = timeline.Get(key);        // 获取对应任务
      timeline.Remove(key);                    // 从 timeline 移除它
      min_v_runtime = current_task->v_runtime; // 更新全局 min_v_runtime
    }
  
    // (3) **若有当前任务，让它运行一个 tick（先增量，再做抢占检查）**
    bool finished = false;
    if (current_task != nullptr) {
      // 3.1 运行一 tick
      current_task->remaining_duration -= 1;
      current_task->v_runtime += 1; // 先增加 v_runtime
      finished = (current_task->remaining_duration == 0);
  
      // 3.2 若尚未完成，则做抢占检查（此时 v_runtime 已更新）
      if (!finished && timeline.Size() > 0) {
        // 如果当前任务 v_runtime 大于 timeline 中最小的 key，则抢占
        if (current_task->v_runtime > timeline.Min()) {
          timeline.Insert(current_task->v_runtime, current_task);
          current_task = nullptr;  // 表示本 tick 被抢占
        }
      }
    }
  
    // (4) 如果当前任务在运行完毕后退出，则移除之
    if (finished && current_task != nullptr) {
      // 输出时标记 *
      // 从 all_tasks 中 erase
      auto it = std::find_if(all_tasks.begin(), all_tasks.end(),
                             [this](const std::unique_ptr<Task> &ptr) {
                               return ptr.get() == current_task;
                             });
      if (it != all_tasks.end()) {
        all_tasks.erase(it);
      }
      current_task = nullptr;
    }
  
    // (5) 如果当前任务为空并且 timeline 中有任务，就马上取出新的当前任务
    //     （这是为了处理 “同一个 tick 内被抢占/任务结束后，立刻换下一个任务” 的场景）
    if (current_task == nullptr && timeline.Size() > 0) {
      unsigned int key = timeline.Min();
      current_task = timeline.Get(key);
      timeline.Remove(key);
      min_v_runtime = current_task->v_runtime; 
    }
  
    // (6) 输出当前 tick 状态
    unsigned int active_count = timeline.Size() + (current_task ? 1 : 0);
    std::cout << current_tick << " [" << active_count << "]: ";
    if (current_task != nullptr) {
      std::cout << current_task->id;
      if (finished) {
        // 在上面 (4) 已经做了清理，这里只是打印 *
        std::cout << "*";
      }
    } else {
      std::cout << '_';
    }
    std::cout << std::endl;
  
    // (7) 递增全局时间
    current_tick++;
  }
  
};

int main(int argc, char *argv[])
{
  // Check for correct number of command-line arguments
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0]
              << " <task_file.dat>" << std::endl;
    return 1;
  }

  // Open the input file containing task descriptions
  std::ifstream file(argv[1]);
  if (!file.is_open())
  {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    return 1;
  }

  Scheduler scheduler; // create a scheduler instance
  char id;
  unsigned int start_time, duration;
  // Read tasks from file and add them to the scheduler
  while (file >> id >> start_time >> duration)
  {
    scheduler.AddTask(id, start_time, duration);
  }

  scheduler.Run(); // run the scheduler

  return 0;
}
