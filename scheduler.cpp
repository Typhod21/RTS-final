#include "scheduler.hpp"
#include <numeric> // Add this include for std::lcm
#include <algorithm>

using namespace std;
Scheduler::Scheduler() {
	// Default constructor
}

Scheduler::Scheduler(const vector<Task>& tasks, int choice)
    : tasks_(tasks), choice_(choice) {}

double Scheduler::computeUtilization() const {
    double utilization = 0.0;
    for (const auto& task : tasks_) {
        utilization += static_cast<double>(task.WCET) / task.deadline;
    }
    return utilization;
}

int Scheduler::computeHyperperiod() const {
  int h = 1;
  for (const auto& task : tasks_) {
      h = lcm(h, task.period);
  }
  return h;
}

void Scheduler::setPriority() {
    int numTasks = tasks_.size();
    int assigned = 0;
    int priority = 1; // 1 is highest priority

    while (assigned < numTasks) {
        int shortestValue = INT_MAX;
        int shortestIndex = -1;

        for (size_t i = 0; i < tasks_.size(); ++i) {
            if (tasks_[i].priority != 0) continue;

            int compareValue = (choice_ == CHOICE_RM) ? tasks_[i].period : tasks_[i].deadline;
            if (compareValue < shortestValue) {
                shortestValue = compareValue;
                shortestIndex = i;
            }
        }

        if (shortestIndex != -1) {
            tasks_[shortestIndex].priority = priority++;
            assigned++;
        }
        else {
            std::cerr << "Error: No unassigned tasks found in priority assignment.\n";
            break;
        }
    }

    for (const auto& task : tasks_) {
        std::cout << "Task " << task.id << " has priority " << task.priority << '\n';
    }
}


bool Scheduler::runRMDMTest() {
    cout << "\nRunning RM/DM schedulability tests...\n";
    double utilization = computeUtilization();
    double bound = tasks_.size() * (pow(2, 1.0 / tasks_.size()) - 1);

    if (utilization <= bound) {
        cout << "Schedulable: " << utilization << " <= " << bound << endl;
        return true;
    } else {
        cout << utilization << " >= " << bound << endl;
        cout << "Inconclusive using utilization. Checking response time analysis...\n";
    }

    bool schedulable = true;
    for( const auto &task : tasks_){
        cout << "Task " << task.id << " response time analysis:" << endl;
        int previousTime = 0;
        int responseTime = task.WCET;
        int constantTime = 0;
        while(previousTime <= task.deadline){
            constantTime = responseTime;
            responseTime = task.WCET;
            cout << "Task " << task.id << " response time:";
            for (const auto &otherTask : tasks_){
                if (otherTask.id != task.id && otherTask.deadline <= task.deadline){
                    cout << " ceil(" << constantTime << " / " << otherTask.period << ") * " << otherTask.WCET;
                    responseTime += ceil(static_cast<double>(constantTime) / otherTask.period) * otherTask.WCET;
                }
            }
            cout << " = " << responseTime << endl;
            if(previousTime == responseTime)
                break;
            previousTime = responseTime;
        }
        if (responseTime > task.deadline){
            cout << "Task " << task.id << " is not schedulable. \n\n";
            schedulable = false;
        }
        else{
            cout << "Task " << task.id << " is schedulable with response time: " << responseTime << "\n\n";
        }
    }
    
    return schedulable;
}

bool Scheduler::runEDFLSTTest() {
    cout << "\nRunning EDF/LST schedulability test...\n";
    double utilization = 0.0;
    bool usesDeadline = false;
    int hyper = computeHyperperiod();
    vector<int> L;

    for (const auto& task : tasks_) {
        utilization += static_cast<double>(task.WCET) / task.deadline;
        if (task.deadline < task.period) {
            usesDeadline = true;
        }
    }

    if (utilization <= 1.0) {
        cout << "Schedulable: " << utilization << " <= 1\n";
        return true;
    } else if (!usesDeadline) {
        cout << "Unschedulable: " << utilization <<  " > 1 with D == T\n";
        return false;
    }
    else{
        cout << "Inconclusive using utilization, applying processor demand criterion...\n";
    }

    for (const auto& task : tasks_) {
        for (int i = 0; i <= hyper; i += task.period) {
            int deadlinePoint = i + task.deadline;
            if (deadlinePoint <= hyper && find(L.begin(), L.end(), deadlinePoint) == L.end()) {
                L.push_back(deadlinePoint);
            }
        }
    }

    sort(L.begin(), L.end());
    for (const auto& l : L) {
        int demand = 0;
        for (const auto& task : tasks_) {
            demand += floor((l + task.period - task.deadline) / task.period) * task.WCET;
        }

        if (demand > l) {
            cout << "Unschedulable at time " << l << ": " << demand << " > " << l << "\n";
            return false;
        } else {
            cout << "Schedulable at time " << l << ": " << demand << " <= " << l << "\n";
        }
    }

    return true;
}
bool Scheduler::runPIPTest() {
    // Optional PIP feature
    cout << "PIP test not implemented.\n";
    return true;
}
bool Scheduler::runOCPPICPPTest() {
    // Optional OCPP/ICPP feature
    cout << "OCPP/ICPP test not implemented.\n";
    return true;
}

void Scheduler::generateTimeline() {
    // Optional timeline feature
    int hyperperiod = computeHyperperiod();
    vector<int> remaining(tasks_.size(), 0);
    vector<int> nextRelease(tasks_.size(), 0);
    vector<int> nextDeadline(tasks_.size(), 0);

    std::cout << "\nTimeline (0 to " << hyperperiod << "):\n";

    for (int t = 0; t < hyperperiod; ++t) {
        // Release tasks
        for (size_t i = 0; i < tasks_.size(); ++i) {
            if (t == nextRelease[i]) {
                remaining[i] += tasks_[i].WCET;
                nextDeadline[i] = tasks_[i].deadline + nextRelease[i];
                nextRelease[i] += tasks_[i].period;
            }
        }
        int runningTask = -1;
        static int previousTask = -1;
        int priority = 0;
        int minDeadline = INT_MAX;
        int minSlack = INT_MAX;
        for(size_t i = 0; i < tasks_.size(); ++i) {
            if(choice_ == CHOICE_RM || choice_ == CHOICE_DM){
                if(remaining[i] > 0 && tasks_[i].priority > priority){
                    priority = tasks_[i].priority;
                    runningTask = i;
                }
            }
            else if(choice_ == CHOICE_EDF){
                if(remaining[i] > 0){
                    if(nextDeadline[i] < minDeadline){
                        minDeadline = nextDeadline[i];
                        runningTask = i;
                    }
                    else if(nextDeadline[i] == minDeadline){
                        runningTask = previousTask;
                    }  
                }
            }
            else if(choice_ == CHOICE_LST){
                int slack = (nextDeadline[i] - t) - remaining[i];
                if(remaining[i] > 0){
                    if(slack < minSlack){
                        minSlack = slack;
                        runningTask = i;
                    }
                    else if(slack == minSlack){
                        runningTask = previousTask;
                    }
                }
            }
        }

        // Print which task runs
        if (runningTask != -1) {
            std::cout << "|T" << tasks_[runningTask].id;
            remaining[runningTask]--;
            previousTask = runningTask;
        } else {
            std::cout << "|ID";
        }
    }
    std::cout << "|\n";


}
