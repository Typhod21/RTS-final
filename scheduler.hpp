// // A program that can take a task set of any size and then run schedubility tests relating to scheduling algorithms including RM, DM, EDF, LST, PIP, OCPP, and ICPP.
// // Then our program would return the results of the schedubility tests and a visual timeline displaying the tasks in a hyperperiod, like our homework.
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

#define CHOICE 0
#define CHOICE_RM 1
#define CHOICE_DM 2
#define CHOICE_EDF 3
#define CHOICE_LST 4
#define CHOICE_PIP 5
#define CHOICE_OCPP 6
#define CHOICE_ICPP 7
#define CHOICE_ARB_DEADLINE 8

struct Task
{
    int id;
    int WCET;
    int period;
    int deadline;
    int priority;
};

class Scheduler
{
public:
    Scheduler(); // Default constructor
    Scheduler(const std::vector<Task> &tasks, int choice = CHOICE);

    bool runRMDMTest(std::vector<Task> taskSet);
    bool runEDFLSTTest();
    bool runPIPTest();      // Optional for now
    bool runOCPPICPPTest(); // Optional for now
    bool runOPA();
    void setPriority();
    void generateTimeline(); // Optional for now
    double computeUtilization() const;
    int computeHyperperiod() const;
    std::vector<Task> tasks_;

private:
    // std::vector<Task> tasks_;
    int choice_;
};


//from chat

struct Resource {
    string id;
    int ceilingPriority = 0;
    bool isHeld = false;
    string heldBy;
	
};

struct ResourceRequest {
	string id;
	int duration;
    bool nested;
    bool isFinished = false;
};

struct Job
{
    string id;
    int releaseTime;
    int WCET;
    int basePriority;
	int period;
	int deadline;
    vector<ResourceRequest> resourceSequence;

    int RWCET;
    int currentPriority;
    int resourceIndex = 0;
    bool isBlocked = false;
    bool isFinished = false;
    string waitingFor;
};




class Inheritance {
    vector<Job> jobs;
    vector<Resource> resources;
	int numOfResource;
    int time = 0;

public:
    Inheritance(vector<Job>& taskList, int numOfResource);
    void simulatePIP();
    bool allTasksFinished();
    void updateResourceUsage(Job& job);
    Job* getNextRunnableTask();
    void runTask(Job& t);
    Job& getTaskById(const string& id);
	Resource& getResourceById(const string& id);

};

#endif // SCHEDULER_H
