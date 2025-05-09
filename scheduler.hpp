// // A program that can take a task set of any size and then run schedulability tests relating to scheduling algorithms including RM, DM, EDF, LST, PIP, OCPP, and ICPP.
// // Then our program would return the results of the schedulability tests and a visual timeline displaying the tasks in a hyperperiod, like our homework.
// // Also, deadlines can be arbitrary. Meaning that the deadline of a task can be greater than its period.
// // For the resource sharing protocol PIP etc., you may assume they are used together with fixed priority scheduling.

// // Task set up
// // task|WCET|Period|Deadline| for non resource sharing protocols
// // task|ReleaseTime|WCET|Period|Deadline|Priority|Critical Section| for resource sharing protocols


#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
using namespace std;
#include <iostream>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <iomanip>
#include <queue>
#include <unordered_map>

#define CHOICE      0
#define CHOICE_RM   1
#define CHOICE_DM   2
#define CHOICE_EDF  3
#define CHOICE_LST  4
#define CHOICE_PIP  5
#define CHOICE_OCPP 6
#define CHOICE_ICPP 7

struct Task {
    int id;
    int WCET;
    int period;
    int deadline;
    int priority;
};


class Scheduler {
public:
	Scheduler(); // Default constructor
    Scheduler(const std::vector<Task>& tasks, int choice = CHOICE);

    bool runRMDMTest();
    bool runEDFLSTTest();
    bool runPIPTest(); // Optional for now
    bool runOCPPICPPTest(); // Optional for now
    void setPriority();
    void generateTimeline(); // Optional for now
    double computeUtilization() const;
    int computeHyperperiod() const;
private:
    std::vector<Task> tasks_;
    int choice_;

    
};

//from chat
struct ResourceRequest {
    string resource;
    int duration;
    bool nested;
};

struct Job {
    string id;
    int releaseTime;
    int executionTime;
    int remainingTime;
    int basePriority;
    int currentPriority;
    vector<ResourceRequest> resourceSequence;
    int resourceIndex = 0;
    bool isBlocked = false;
    bool isFinished = false;
    string waitingFor;
};

struct Resource {
    string id;
    bool isHeld = false;
    string heldBy;
    int timeLeft = 0;
    queue<string> waitingQueue;
};

class Inheritance {
    vector<Job> tasks;
    unordered_map<string, Resource> resources;
    int time = 0;

public:
    Inheritance(const vector<Job>& taskList) : tasks(taskList) {
        for (const auto& task : taskList) {
            for (const auto& req : task.resourceSequence)
                resources[req.resource] = Resource{ req.resource };
        }
    }

    void simulatePIP() {
        cout << "Starting PIP Simulation\n";

        while (!allTasksFinished()) {
            cout << "Time: " << time << "\n";
            updateResourceUsage();

            Job* nextTask = getNextRunnableTask();

            if (nextTask) {
                runTask(*nextTask);
            }
            else {
                cout << "  CPU Idle\n";
            }

            time++;
        }

        cout << "Simulation complete.\n";
    }

private:
    bool allTasksFinished() {
        return all_of(tasks.begin(), tasks.end(), [](const Job& t) { return t.isFinished; });
    }

    void updateResourceUsage() {
        for (auto it = resources.begin(); it != resources.end(); ++it) {
            const std::string& rid = it->first;
            Resource& res = it->second;
            if (res.isHeld) {
                res.timeLeft--;
                if (res.timeLeft == 0) {
                    cout << "  Resource " << rid << " released by " << res.heldBy << "\n";
                    Job& t = getTaskById(res.heldBy);
                    t.resourceIndex++;
                    res.isHeld = false;
                    res.heldBy = "";

                    // Restore priority
                    t.currentPriority = t.basePriority;

                    // Unblock waiting tasks
                    if (!res.waitingQueue.empty()) {
                        string waitingTaskId = res.waitingQueue.front(); res.waitingQueue.pop();
                        Job& wt = getTaskById(waitingTaskId);
                        wt.isBlocked = false;
                    }
                }
            }
        }
    }

    Job* getNextRunnableTask() {
        Job* selected = nullptr;

        for (auto& t : tasks) {
            if (t.isFinished || t.releaseTime > time || t.isBlocked)
                continue;

            // Check if resource needed
            if (t.resourceIndex < t.resourceSequence.size()) {
                const auto& req = t.resourceSequence[t.resourceIndex];
                Resource& r = resources[req.resource];

                if (!r.isHeld) {
                    // Acquire it
                    r.isHeld = true;
                    r.heldBy = t.id;
                    r.timeLeft = req.duration;
                    cout << "  " << t.id << " acquired " << r.id << " for " << req.duration << " units";
                }
                else if (r.heldBy != t.id) {
                    // Blocked
                    t.isBlocked = true;
                    t.waitingFor = r.id;
                    r.waitingQueue.push(t.id);

                    // Priority Inheritance
                    Job& holder = getTaskById(r.heldBy);
                    if (t.currentPriority < holder.currentPriority) {
                        cout << "  " << holder.id << " inherits priority from " << t.id << "\n";
                        holder.currentPriority = t.currentPriority;
                    }

                    continue;
                }
            }

            if (!selected || t.currentPriority < selected->currentPriority)
                selected = &t;
        }

        return selected;
    }

    void runTask(Job& t) {
        cout << "  Running " << t.id << "\n";
        t.remainingTime--;
        if (t.remainingTime == 0) {
            t.isFinished = true;
            cout << "  " << t.id << " finished execution\n";
        }
    }

    Job& getTaskById(const string& id) {
        for (auto& t : tasks) {
            if (t.id == id)
                return t;
        }
        throw runtime_error("Invalid Task ID");
    }
};

#endif // SCHEDULER_H
