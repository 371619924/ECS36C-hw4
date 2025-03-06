#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "multimap.h"

// Task class to represent a process
class Task {
private:
    char id;              // Task identifier
    unsigned int start;   // Start time in ticks
    unsigned int duration; // Total duration in ticks
    unsigned int remaining; // Remaining ticks
    unsigned int vruntime; // Virtual runtime

public:
    Task(char id, unsigned int start, unsigned int duration)
        : id(id), start(start), duration(duration), remaining(duration), vruntime(0) {}

    char getId() const { return id; }
    unsigned int getStart() const { return start; }
    unsigned int getDuration() const { return duration; }
    unsigned int getRemaining() const { return remaining; }
    unsigned int getVRuntime() const { return vruntime; }

    void setVRuntime(unsigned int vr) { vruntime = vr; }
    void incrementVRuntime() { vruntime++; }
    void decrementRemaining() { if (remaining > 0) remaining--; }
    bool isCompleted() const { return remaining == 0; }

    friend std::ostream& operator<<(std::ostream& os, const Task& task) {
        os << "Task " << task.id << " (Start: " << task.start 
           << ", Duration: " << task.duration 
           << ", Remaining: " << task.remaining
           << ", VRuntime: " << task.vruntime << ")";
        return os;
    }
};

// Scheduler class to manage tasks
class Scheduler {
private:
    unsigned int current_tick;
    unsigned int min_vruntime;
    std::vector<Task*> all_tasks;
    Multimap<unsigned int, Task*> timeline;
    Task* current_task;
    
    // Read tasks from file
    bool readTasksFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: cannot open file " << filename << std::endl;
            return false;
        }
        
        char id;
        unsigned int start, duration;
        
        while (file >> id >> start >> duration) {
            Task* task = new Task(id, start, duration);
            all_tasks.push_back(task);
        }
        
        file.close();
        return true;
    }

public:
    Scheduler() : current_tick(0), min_vruntime(0), current_task(nullptr) {}
    
    ~Scheduler() {
        // Clean up all tasks
        for (auto task : all_tasks) {
            delete task;
        }
    }
    
    bool initialize(const std::string& filename) {
        return readTasksFromFile(filename);
    }
    
    // Run the scheduler
    void run() {
        // Sort all_tasks by start time and then by ID
        std::sort(all_tasks.begin(), all_tasks.end(), [](const Task* a, const Task* b) {
            if (a->getStart() == b->getStart())
                return a->getId() < b->getId();
            return a->getStart() < b->getStart();
        });
        
        // Keep track of tasks that have not yet been added to the timeline
        std::vector<Task*> pending_tasks = all_tasks;
        // Count of currently active tasks
        int active_task_count = 0;
        
        // Main scheduling loop
        while (!pending_tasks.empty() || active_task_count > 0 || current_task != nullptr) {
            // 1. Launch tasks that should start at current tick
            auto it = pending_tasks.begin();
            while (it != pending_tasks.end()) {
                if ((*it)->getStart() == current_tick) {
                    Task* task = *it;
                    // Initialize virtual runtime to current min_vruntime
                    task->setVRuntime(min_vruntime);
                    // Add to timeline
                    timeline.Insert(min_vruntime, task);
                    active_task_count++;
                    // Remove from pending tasks
                    it = pending_tasks.erase(it);
                } else if ((*it)->getStart() > current_tick) {
                    // No more tasks to launch at current tick
                    break;
                } else {
                    ++it;
                }
            }
            
            // 2. Check if current task should yield
            if (current_task != nullptr) {
                if (current_task->getVRuntime() > min_vruntime && !current_task->isCompleted()) {
                    // Add back to timeline
                    timeline.Insert(current_task->getVRuntime(), current_task);
                }
            }
            
            // 3. Get next task if no current task or current task completed/yielded
            if (current_task == nullptr || current_task->isCompleted() || 
                current_task->getVRuntime() > min_vruntime) {
                if (timeline.Size() > 0) {
                    // Get task with minimum virtual runtime
                    current_task = timeline.Get(timeline.Min());
                    // Remove from timeline
                    timeline.Remove(timeline.Min());
                    // Update min_vruntime
                    min_vruntime = current_task->getVRuntime();
                } else {
                    current_task = nullptr;
                }
            }
            
            // 4. Print scheduling status
            std::cout << current_tick << " [" << active_task_count << "]: ";
            if (current_task != nullptr) {
                std::cout << current_task->getId();
                
                // 5. Run current task for one tick
                current_task->incrementVRuntime();
                current_task->decrementRemaining();
                
                // Check if task has completed
                if (current_task->isCompleted()) {
                    std::cout << "*";
                    active_task_count--;
                    current_task = nullptr;
                }
            } else {
                std::cout << "_";
            }
            std::cout << std::endl;
            
            // 6. Increment tick
            current_tick++;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <task_file.dat>" << std::endl;
        return 1;
    }
    
    Scheduler scheduler;
    if (!scheduler.initialize(argv[1])) {
        return 1;
    }
    
    scheduler.run();
    return 0;
}