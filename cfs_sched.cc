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
  unsigned order;  // 用于 tie-break
};

struct TaskComparator {
  bool operator()(const Task* a, const Task* b) const {
    if (a->vruntime != b->vruntime)
      return a->vruntime < b->vruntime;
    return a->order < b->order;
  }
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: ./cfs_sched <task_file.dat>" << std::endl;
    return 0;
  }
  std::ifstream infile(argv[1]);
  if (!infile) {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    return 1;
  }

  // 读取任务
  std::vector<Task*> pending;
  std::string line;
  while (std::getline(infile, line)) {
    if(line.empty()) continue;
    std::istringstream iss(line);
    Task* t = new Task;
    iss >> t->id >> t->start_time >> t->duration;
    t->remaining = t->duration;
    t->vruntime = 0; // 待启动时初始化为 global_min_vruntime
    pending.push_back(t);
  }
  infile.close();

  // 就绪队列，用 multiset 保持任务按照 (vruntime, order) 排序
  std::set<Task*, TaskComparator> ready;
  Task* current = nullptr;
  unsigned tick = 0;
  unsigned global_min_vruntime = 0;
  unsigned order_counter = 0;

  // 模拟调度直至所有任务完成
  while (!pending.empty() || !ready.empty() || current) {
    // 启动新任务：将启动时间等于当前 tick 的任务加入就绪队列，并初始化 vruntime = global_min_vruntime
    for (auto it = pending.begin(); it != pending.end(); ) {
      if ((*it)->start_time == tick) {
        (*it)->vruntime = global_min_vruntime;
        (*it)->order = order_counter++;
        ready.insert(*it);
        it = pending.erase(it);
      } else {
        ++it;
      }
    }

    // 检查是否需要抢占：若就绪队列中最小 vruntime 的任务比当前任务（若存在）的 vruntime 小，则抢占
    if (current && !ready.empty()) {
      Task* candidate = *ready.begin();
      if (candidate->vruntime < current->vruntime) {
        current->order = order_counter++;
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

    // 计算系统中任务总数（就绪队列中的任务数 + 当前运行任务是否存在）
    unsigned total_tasks = ready.size() + (current ? 1 : 0);

    // 输出当前 tick 状态
    std::cout << tick << " [" << total_tasks << "]: ";
    if (!current)
      std::cout << "_" << std::endl;
    else {
      // 若本 tick 运行后任务完成，则在标识符后加上 *
      if (current->remaining == 1) {
        std::cout << current->id << "*" << std::endl;
      } else {
        std::cout << current->id << std::endl;
      }
    }

    // 运行当前任务一个 tick
    if (current) {
      current->vruntime++;
      current->remaining--;
      // 若任务完成，删除 current
      if (current->remaining == 0) {
        delete current;
        current = nullptr;
      }
    }

    tick++;
  }

  // 释放 pending 中剩余的任务（理论上不可能）
  for (Task* t : pending) {
    delete t;
  }
  return 0;
}
