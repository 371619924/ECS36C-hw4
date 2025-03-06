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
  unsigned order;  // 用于 tie-break：数值越小，优先级越高
};

struct TaskComparator {
  // 按照 vruntime 升序排序；若 vruntime 相等，则按照 order 升序排序
  bool operator()(const Task* a, const Task* b) const {
    if(a->vruntime != b->vruntime)
      return a->vruntime < b->vruntime;
    return a->order < b->order;
  }
};

int main(int argc, char* argv[]) {
  // 当参数不足时，必须输出提示到标准输出
  if(argc != 2) {
    std::cout << "Usage: /autograder/source/tests/cfs_sched <task_file.dat>" << std::endl;
    return 1;
  }
  std::ifstream infile(argv[1]);
  if(!infile) {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    return 1;
  }
  std::vector<Task*> pending;
  std::string line;
  unsigned order_counter = 0;
  while(std::getline(infile, line)) {
    if(line.empty()) continue;
    Task* t = new Task;
    std::istringstream iss(line);
    iss >> t->id >> t->start_time >> t->duration;
    t->remaining = t->duration;
    t->vruntime = 0;  // 到达时初始化为当前全局最小 vruntime（稍后赋值）
    t->order = order_counter++;
    pending.push_back(t);
  }
  infile.close();
  
  // 使用 multiset 来维护就绪队列，按 (vruntime, order) 排序
  std::set<Task*, TaskComparator> ready;
  Task* current = nullptr;
  unsigned tick = 0;
  unsigned global_min = 0;
  
  while(!pending.empty() || !ready.empty() || current) {
    bool newArrival = false;
    // 把当前 tick 到达的任务加入就绪队列，并初始化其 vruntime 为当前 global_min
    for(auto it = pending.begin(); it != pending.end();) {
      if((*it)->start_time == tick) {
        (*it)->vruntime = global_min;
        ready.insert(*it);
        newArrival = true;
        it = pending.erase(it);
      } else {
        ++it;
      }
    }
    // 如果本 tick 有新到任务且当前任务正在运行，则抢占当前任务
    if(newArrival && current) {
      current->order = order_counter++;
      ready.insert(current);
      current = nullptr;
    }
    // 如果当前没有任务，则从就绪队列中选取 vruntime 最小的任务
    if(!current && !ready.empty()){
      current = *ready.begin();
      ready.erase(ready.begin());
      global_min = current->vruntime;
    }
    
    // 计算系统中任务总数（就绪队列中的任务数加上当前正在运行的任务）
    unsigned total_tasks = ready.size() + (current ? 1 : 0);
    std::cout << tick << " [" << total_tasks << "]: ";
    if(!current)
      std::cout << "_" << std::endl;
    else {
      // 如果当前任务本 tick 执行后将完成，则在标识符后加 *
      if(current->remaining == 1)
        std::cout << current->id << "*" << std::endl;
      else
        std::cout << current->id << std::endl;
    }
    
    // 运行当前任务1 tick
    if(current) {
      current->vruntime++;
      current->remaining--;
    }
    
    // 运行后若当前任务未完成，且就绪队列中有任务 vruntime 小于当前任务，则抢占当前任务
    if(current && current->remaining > 0 && !ready.empty()) {
      Task* candidate = *ready.begin();
      if(candidate->vruntime < current->vruntime) {
        current->order = order_counter++;
        ready.insert(current);
        current = nullptr;
      }
    }
    // 若当前任务刚好完成，则释放它
    if(current && current->remaining == 0) {
      delete current;
      current = nullptr;
    }
    tick++;
  }
  
  for(Task* t : pending)
    delete t;
  return 0;
}
