#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <map>
#include <cstdlib>
#include <algorithm>

struct Task {
    char id;
    unsigned int start_time;
    unsigned int duration;
    unsigned int remaining;
    unsigned int vruntime;
};

// 将任务按开始时间排序
bool CompareStartTime(const Task &a, const Task &b) {
    return a.start_time < b.start_time;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <task_file.dat>" << std::endl;
        return 1;
    }
    
    std::ifstream infile(argv[1]);
    if (!infile) {
        std::cerr << "Error: cannot open file " << argv[1] << std::endl;
        return 1;
    }
    
    std::vector<Task> pending;
    std::string line;
    while (std::getline(infile, line)) {
        if(line.empty()) continue;
        std::istringstream iss(line);
        Task t;
        if (!(iss >> t.id >> t.start_time >> t.duration))
            continue;
        t.remaining = t.duration;
        t.vruntime = 0;
        pending.push_back(t);
    }
    infile.close();
    
    // 按任务开始时间排序
    std::sort(pending.begin(), pending.end(), CompareStartTime);
    
    // ready 队列：使用 multimap 按虚拟运行时间排序；键为 vruntime，值为指向 Task 的指针
    std::multimap<unsigned int, Task*> ready;
    
    Task* current = nullptr;
    unsigned int global_min = 0;
    unsigned int tick = 0;
    
    // 模拟直到所有任务均完成
    while (!pending.empty() || !ready.empty() || current != nullptr) {
        // 将所有在本 tick 应该启动的任务移入 ready 队列
        while (!pending.empty() && pending.front().start_time == tick) {
            // 新任务初始化 vruntime 为 global_min
            pending.front().vruntime = global_min;
            ready.insert({ pending.front().vruntime, &pending.front() });
            pending.erase(pending.begin());
        }
        
        // 如果当前没有任务，且就绪队列不空，则调度虚拟运行时间最小的任务
        if (current == nullptr && !ready.empty()) {
            auto it = ready.begin();
            current = it->second;
            global_min = current->vruntime;
            ready.erase(it);
        }
        
        // 计算系统中总任务数（当前运行的任务 + 就绪队列中任务数）
        int total_tasks = (current ? 1 : 0) + ready.size();
        
        bool finished_this_tick = false;
        // 运行当前任务 1 个 tick
        if (current) {
            current->vruntime += 1;
            current->remaining -= 1;
            if (current->remaining == 0) {
                finished_this_tick = true;
            }
        }
        
        // 调度抢占：如果当前任务未结束，且就绪队列中存在虚拟运行时间更低的任务，则抢占
        if (current && !finished_this_tick && !ready.empty()) {
            auto it = ready.begin();
            if (it->first < current->vruntime) {
                // 将当前任务重新放入就绪队列，使用更新后的 vruntime 作为键
                ready.insert({ current->vruntime, current });
                current = nullptr;
                if (!ready.empty()) {
                    auto new_it = ready.begin();
                    current = new_it->second;
                    global_min = current->vruntime;
                    ready.erase(new_it);
                }
            }
        }
        
        // 输出本 tick 状态：格式 "<tick> [<总任务数>]: <任务ID>"，若任务刚结束则在 ID 后追加 "*"
        std::cout << tick << " [" << total_tasks << "]: ";
        if (current)
            std::cout << current->id << (finished_this_tick ? "*" : "");
        else
            std::cout << "_";
        std::cout << std::endl;
        
        // 如果当前任务在本 tick 执行后完成，置空当前任务
        if (current && finished_this_tick) {
            current = nullptr;
        }
        
        tick++;
    }
    
    return 0;
}
