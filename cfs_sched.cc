#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <cstdlib>

struct Task {
  char id;
  unsigned start_time;
  unsigned duration;
  unsigned remaining;
  unsigned vruntime;
  unsigned order;  // 调度顺序，初始时取文件顺序，抢占时更新
};

struct TaskComparator {
  bool operator()(const Task* a, const Task* b) const {
    if (a->vruntime != b->vruntime)
      return a->vruntime < b->vruntime;
    return a->order < b->order;
  }
};

int main(int argc, char* argv[]) {
  // 参数检查：如果参数个数不等于 2，则输出用法提示到 stderr，并返回 1
  if (argc != 2) {
    std::cerr << "Usage: /autograder/source/tests/cfs_sched <task_file.dat>" << std::endl;
    return 1;
  }
  std::ifstream infile(argv[1]);
  if (!infile) {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    return 1;
  }

  // 读取任务描述，将每行任务转换为 Task 对象，初始 order 取文件顺序
  std::vector<Task*> pending;
  std::string line;
  unsigned file_order = 0;
  while (std::getline(infile, line)) {
    if(line.empty()) continue;
    std::istringstream iss(line);
    Task* t = new Task;
    iss >> t->id >> t->start_time >> t->duration;
    t->remaining = t->duration;
    t->vruntime = 0; // 待启动时设为全局最小 vruntime
    t->order = file_order++;
    pending.push_back(t);
  }
  infile.close();

  // 使用 multiset 按 (vruntime, order) 排序来维护就绪队列
  std::set<Task*, TaskComparator> ready;
  Task* current = nullptr;
  unsigned tick = 0;
  unsigned global_min_vruntime = 0;
  // 用于抢占时更新 order，初始值设为文件任务数
  unsigned order_counter = file_order;

  // 模拟调度直到所有任务完成
  while (!pending.empty() || !ready.empty() || current) {
    // 将启动时间等于当前 tick 的任务加入就绪队列，初始化它们的 vruntime 为 global_min_vruntime
    for (auto it = pending.begin(); it != pending.end(); ) {
      if ((*it)->start_time == tick) {
        (*it)->vruntime = global_min_vruntime;
        ready.insert(*it);
        it = pending.erase(it);
      } else {
        ++it;
      }
    }

    // 检查是否需要抢占：若就绪队列中 vruntime 最小的任务比当前任务的 vruntime小，则抢占当前任务
    if (current && !ready.empty()) {
      Task* candidate = *ready.begin();
      if (candidate->vruntime < current->vruntime) {
        current->order = order_counter++;
        ready.insert(current);
        current = nullptr;
      }
    }

    // 若当前无任务，则从就绪队列中选取 vruntime 最小的任务，并更新 global_min_vruntime
    if (!current && !ready.empty()) {
      current = *ready.begin();
      ready.erase(ready.begin());
      global_min_vruntime = current->vruntime;
    }

    // 计算系统中任务总数：就绪队列中任务数 + 当前正在运行的任务（不包括未到启动时间的 pending 任务）
    unsigned total_tasks = ready.size() + (current ? 1 : 0);
    std::cout << tick << " [" << total_tasks << "]: ";
    if (!current)
      std::cout << "_" << std::endl;
    else {
      if (current->remaining == 1)
        std::cout << current->id << "*" << std::endl;
      else
        std::cout << current->id << std::endl;
    }

    // 运行当前任务 1 tick：增加 vruntime 并减少 remaining
    if (current) {
      current->vruntime++;
      current->remaining--;
      if (current->remaining == 0) {
        delete current;
        current = nullptr;
      }
    }
    tick++;
  }

  // 清理 pending 中可能残留的任务
  for (Task* t : pending)
    delete t;
  return 0;
}
