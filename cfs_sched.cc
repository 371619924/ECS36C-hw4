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
};

// 比较器：先比较 vruntime；若相同，则比较任务 id（按字母顺序）
// 这样就能保证同一时刻到达且 vruntime 相同时，以标识符小者优先。
struct TaskComparator {
  bool operator()(const Task* a, const Task* b) const {
    if (a == b) return false;  // 避免同指针比较出现歧义
    if (a->vruntime != b->vruntime) {
      return a->vruntime < b->vruntime;
    }
    // vruntime 相同则按字母顺序
    return a->id < b->id;
  }
};

int main(int argc, char* argv[]) {
  // 1) 参数检查：若无文件参数，则输出用法到 stdout 并返回 1
  if (argc != 2) {
    std::cout << "Usage: /autograder/source/tests/cfs_sched <task_file.dat>" << std::endl;
    return 1;
  }

  // 2) 尝试打开文件，不成功则输出错误到 stderr 并返回 1
  std::ifstream infile(argv[1]);
  if (!infile) {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    return 1;
  }

  // 读取任务：每行格式“<id> <start_time> <duration>”
  std::vector<Task*> pending;
  std::string line;
  while (std::getline(infile, line)) {
    if (line.empty()) continue;
    std::istringstream iss(line);
    Task* t = new Task;
    iss >> t->id >> t->start_time >> t->duration;
    t->remaining = t->duration;
    // 到达时才初始化 vruntime
    t->vruntime = 0;
    pending.push_back(t);
  }
  infile.close();

  // multiset 用于就绪队列，按照 (vruntime, id) 排序
  std::set<Task*, TaskComparator> ready;
  Task* current = nullptr;
  unsigned tick = 0;
  unsigned global_min_vruntime = 0;

  // 开始调度循环
  while (!pending.empty() || !ready.empty() || current) {
    // 把 “start_time == tick” 的任务放入就绪队列
    for (auto it = pending.begin(); it != pending.end();) {
      if ((*it)->start_time == tick) {
        // 初始化它们的 vruntime 为当前 global_min_vruntime
        (*it)->vruntime = global_min_vruntime;
        ready.insert(*it);
        it = pending.erase(it);
      } else {
        ++it;
      }
    }

    // 若有当前任务，检查是否需要抢占（包含相等时也抢占）
    if (current && !ready.empty()) {
      Task* candidate = *ready.begin();
      if (candidate->vruntime <= current->vruntime) {
        // 将当前任务放回就绪队列
        ready.insert(current);
        current = nullptr;
      }
    }

    // 若当前无任务，则从就绪队列选出 vruntime 最小者
    if (!current && !ready.empty()) {
      current = *ready.begin();
      ready.erase(ready.begin());
      global_min_vruntime = current->vruntime;
    }

    // 计算系统中（就绪+当前）任务数
    unsigned total_tasks = ready.size() + (current ? 1 : 0);

    // 输出：tick [total_tasks]: {task_id 或 _}
    // 若本 tick 运行后任务将结束，则加 *
    std::cout << tick << " [" << total_tasks << "]: ";
    if (!current) {
      std::cout << "_" << std::endl;
    } else {
      if (current->remaining == 1)
        std::cout << current->id << "*" << std::endl;
      else
        std::cout << current->id << std::endl;
    }

    // 运行当前任务 1 tick
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

  // 理论上不会剩余任务，但保险起见释放
  for (Task* t : pending) {
    delete t;
  }
  return 0;
}
