#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <tuple>   // 用于 composite key

#include "multimap.h"  // 引入 multimap 头文件

// 定义 composite key: (虚拟运行时间, 上次运行时刻, 任务 id)
// 加入任务 id 作为 tie-breaking 的依据
typedef std::tuple<unsigned int, unsigned int, char> Key;

// 表示任务的类
class Task {
 public:
  Task(char id, unsigned int start_time, unsigned int duration)
      : id(id),
        start_time(start_time),
        remaining_duration(duration),
        v_runtime(0),
        last_run_tick(0) {}

  char id;                     // 任务标识符
  unsigned int start_time;     // 任务开始的时刻（tick）
  unsigned int remaining_duration;  // 剩余运行时长（tick）
  unsigned int v_runtime;      // 任务的虚拟运行时间
  unsigned int last_run_tick;  // 上次运行时刻（用于 tie-breaking）
};

// 表示完全公平调度器（CFS）的类
class Scheduler {
 public:
  // 添加新任务到调度器
  void AddTask(char id, unsigned int start_time, unsigned int duration) {
    all_tasks.push_back(std::unique_ptr<Task>(new Task(id, start_time, duration)));
    Task* raw_ptr = all_tasks.back().get();
    pending_tasks.push(raw_ptr);
  }

  // 运行调度循环，直到没有任务
  void Run() {
    while (!pending_tasks.empty() || timeline.Size() != 0 || current_task != nullptr) {
      ProcessTick();
    }
  }

 private:
  // 用于待启动任务的比较器：先比较开始时间；相同则比较任务 id（保证稳定性）
  struct PendingComparator {
    bool operator()(const Task* a, const Task* b) const {
      if (a->start_time != b->start_time)
        return a->start_time > b->start_time;
      else
        return a->id > b->id;
    }
  };

  std::priority_queue<Task*, std::vector<Task*>, PendingComparator> pending_tasks;
  // 使用 composite key 来存储任务以实现 tie-breaking
  Multimap<Key, Task*> timeline;
  Task* current_task = nullptr;         // 当前正在运行的任务
  unsigned int min_v_runtime = 0;         // 全局最小虚拟运行时间
  unsigned int current_tick = 0;          // 当前 tick 计数器
  std::vector<std::unique_ptr<Task>> all_tasks;  // 保存所有任务的存储容器

  // 用于切换任务后延迟抢占的标志
  bool just_switched = false;

  // 构造任务的 composite key
  Key makeKey(Task* t) {
    return std::make_tuple(t->v_runtime, t->last_run_tick, t->id);
  }

  // 处理一个 tick 的调度
  void ProcessTick() {
    bool new_task_inserted = false;

    // 1. 插入所有在当前 tick 到达的任务
    while (!pending_tasks.empty() &&
           pending_tasks.top()->start_time == current_tick) {
      Task* t = pending_tasks.top();
      pending_tasks.pop();
      // 初始化任务的虚拟运行时间为全局最小值
      t->v_runtime = min_v_runtime;
      timeline.Insert(makeKey(t), t);
      new_task_inserted = true;
    }

    // 2. 抢占检查：如果当前任务存在且 timeline 非空，
    //    当当前任务的 composite key大于 timeline 中最小 key 时，抢占当前任务
    if (current_task != nullptr && timeline.Size() != 0) {
      Key current_key = makeKey(current_task);
      Key min_key = timeline.Min();
      if (current_key > min_key) {
        timeline.Insert(current_key, current_task);
        current_task = nullptr;
      }
    }

    // 3. 如果当前没有任务，则从 timeline 中选择下一个任务
    if (current_task == nullptr && timeline.Size() != 0) {
      Key min_key = timeline.Min();
      current_task = timeline.Get(min_key);
      timeline.Remove(min_key);
      // 更新全局最小虚拟运行时间
      min_v_runtime = current_task->v_runtime;
      just_switched = true;
    }

    // 4. 运行当前任务一个 tick
    bool finished = false;
    if (current_task != nullptr) {
      current_task->remaining_duration--;
      current_task->v_runtime++;  // 虚拟运行时间递增
      finished = (current_task->remaining_duration == 0);
      // 更新上次运行 tick
      current_task->last_run_tick = current_tick;
    }

    // 5. 输出当前 tick 的状态
    unsigned int active_count = timeline.Size() + (current_task ? 1 : 0);
    std::cout << current_tick << " [" << active_count << "]: ";
    if (current_task != nullptr) {
      std::cout << current_task->id;
      if (finished) {
        std::cout << "*";
        // 完成任务后，从存储中移除该任务
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

    if (just_switched)
      just_switched = false;

    // 6. tick 计数器自增
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
