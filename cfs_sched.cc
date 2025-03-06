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
  unsigned order;  // 保留文件中的顺序
};

struct TaskComparator {
  bool operator()(const Task* a, const Task* b) const {
    if (a->vruntime != b->vruntime)
      return a->vruntime < b->vruntime;
    return a->order < b->order;
  }
};

int main(int argc, char* argv[]) {
  // 参数检查：输出正确的用法提示并返回 1
  if (argc != 2) {
    std::cout << "Usage: /autograder/source/tests/cfs_sched <task_file.dat>" << std::endl;
    return 1;
  }
  std::ifstream infile(argv[1]);
  if (!infile) {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    return 1;
  }

  // 读取任务
  std::vector<Task*> pending;
  std::string line;
  unsigned file_order = 0;
  while (std::getline(infile, line)) {
    if (line.empty()) continue;
    std::istringstream iss(line);
    Task* t = new Task;
    iss >> t->id >> t->start_time >> t->duration;
    t->remaining = t->duration;
    t->vruntime = 0; // 待启动时初始化为 global_min_vruntime
    t->order = file_order++;  // 按文件顺序赋值
    pending.push_back(t);
  }
  infile.close();

  // 就绪队列，使用 multiset 按 (vruntime, order) 排序
  std::set<Task*, TaskComparator> ready;
  Task* current = nullptr;
  unsigned tick = 0;
  unsigned global_min_vruntime = 0;

  // 模拟调度直到所有任务完成
  while (!pending.empty() || !ready.empty() || current) {
    // 将启动时间等于当前 tick 的任务加入就绪队列，并初始化 vruntime
    for (auto it = pending.begin(); it != pending.end(); ) {
      if ((*it)->start_time == tick) {
        (*it)->vruntime = global_min_vruntime;
        ready.insert(*it);
        it = pending.erase(it);
      } else {
        ++it;
      }
    }

    // 检查是否需要抢占：若就绪队列中最小 vruntime 的任务比当前任务的 vruntime 小，则抢占
    if (current && !ready.empty()) {
      Task* candidate = *ready.begin();
      if (candidate->vruntime < current->vruntime) {
        // 不更新 order，保持文件顺序
        ready.insert(current);
        current = nullptr;
      }
    }

    // 如果没有当前运行任务，则从就绪队列中选取一个
    if (!current && !ready.empty()) {
      current = *ready.begin();
      ready.erase(ready.begin());
      global_min_vruntime = current->vruntime;
    }

    // 计算系统中任务数（就绪队列中的任务 + 当前运行任务）
    unsigned total_tasks = ready.size() + (current ? 1 : 0);

    // 输出当前 tick 状态，若任务完成则在标识符后加 *
    std::cout << tick << " [" << total_tasks << "]: ";
    if (!current)
      std::cout << "_" << std::endl;
    else {
      if (current->remaining == 1)
        std::cout << current->id << "*" << std::endl;
      else
        std::cout << current->id << std::endl;
    }

    // 当前任务运行一个 tick
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

  // 清理 pending 中剩余的任务（理论上不可能发生）
  for (Task* t : pending)
    delete t;
  return 0;
}
