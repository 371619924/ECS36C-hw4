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

// 比较器：按照 (vruntime) 递增排序；若 vruntime 相同，则按标识符字母顺序。
// （可以根据测试要求微调这里的 tie-break 规则）
struct TaskComparator {
  bool operator()(const Task* a, const Task* b) const {
    if (a == b) return false;  
    if (a->vruntime != b->vruntime)
      return a->vruntime < b->vruntime;
    return a->id < b->id;
  }
};

int main(int argc, char* argv[]) {
  // 当参数不足时，输出提示到“标准输出”并返回退出码1
  if (argc != 2) {
    std::cout << "Usage: /autograder/source/tests/cfs_sched <task_file.dat>" << std::endl;
    return 1;
  }

  // 若无法打开文件，输出错误到stderr并返回1
  std::ifstream infile(argv[1]);
  if (!infile) {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    return 1;
  }

  // 读取所有任务到 pending 容器
  std::vector<Task*> pending;
  {
    std::string line;
    while (std::getline(infile, line)) {
      if (line.empty()) continue;
      std::istringstream iss(line);
      Task* t = new Task;
      iss >> t->id >> t->start_time >> t->duration;
      t->remaining = t->duration;
      t->vruntime = 0;  // 等真正开始调度时再设置为 global_min_vruntime
      pending.push_back(t);
    }
  }
  infile.close();

  // 就绪队列：按 (vruntime, id) 排序
  std::set<Task*, TaskComparator> ready;
  // 当前正在运行的任务
  Task* current = nullptr;
  // 全局最小虚拟时间
  unsigned global_min_vruntime = 0;
  // 全局时钟
  unsigned tick = 0;

  // 主调度循环
  while (!pending.empty() || !ready.empty() || current) {
    // 1) 把此刻到达的任务加入就绪队列，并将其 vruntime 初始化为 global_min_vruntime
    for (auto it = pending.begin(); it != pending.end();) {
      if ((*it)->start_time == tick) {
        (*it)->vruntime = global_min_vruntime;
        ready.insert(*it);
        it = pending.erase(it);
      } else {
        ++it;
      }
    }

    // 2) 若当前有任务，则判断是否有就绪队列中任务 vruntime 更小 => 抢占
    if (current && !ready.empty()) {
      Task* candidate = *ready.begin();  // 就绪队列中vruntime最小者
      // 只在 candidate->vruntime < current->vruntime 时才抢占
      if (candidate->vruntime < current->vruntime) {
        // 将 current 重新放回就绪队列
        ready.insert(current);
        current = nullptr;
      }
    }

    // 3) 若当前没有任务，则从就绪队列中选取 vruntime 最小的任务
    if (!current && !ready.empty()) {
      current = *ready.begin();
      ready.erase(ready.begin());
      global_min_vruntime = current->vruntime;  
    }

    // 4) 计算当前系统中任务数（就绪队列 + 正在运行的任务）
    unsigned total_tasks = ready.size() + (current ? 1 : 0);

    // 5) 输出：若当前任务下一刻就会结束，则在标识符后加“*”
    std::cout << tick << " [" << total_tasks << "]: ";
    if (!current) {
      std::cout << "_" << std::endl;
    } else {
      if (current->remaining == 1) {
        std::cout << current->id << "*" << std::endl;
      } else {
        std::cout << current->id << std::endl;
      }
    }

    // 6) 让当前任务执行 1 tick
    if (current) {
      current->vruntime++;
      current->remaining--;
      if (current->remaining == 0) {
        delete current;
        current = nullptr;
      }
    }

    // 7) 时间向前推进
    tick++;
  }

  // 理论上这里不会剩下任何 pending，但以防万一做个清理
  for (Task* t : pending) {
    delete t;
  }

  return 0;
}
