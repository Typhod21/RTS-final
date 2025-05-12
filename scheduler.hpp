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
    bool runOPA();
    void setPriority();
    void generateTimeline();
    double computeUtilization() const;
    int computeHyperperiod() const;

    void displayTimeline();

    std::vector<Task> tasks_;
	std::vector<string> timeline;

private:
    // std::vector<Task> tasks_;
    int choice_;
};


struct Resource {
    int id;
    int ceilingPriority = 0;
    bool isHeld = false;
    int heldBy;
};

struct ResourceRequest {
	int id;
	int duration;
    bool isFinished = false;
};

struct Job
{
    int id;
    int releaseTime;
    int WCET;
    int basePriority;
	int period;
	int deadline;
    vector<ResourceRequest> resourceSequence;

    int RWCET;
    int currentPriority;
    bool isBlocked = false;
    bool isFinished = false;
    int waitingFor;
};

struct simulate {
    string job;
	vector<Resource> resource;
    int time;
};

class Inheritance {
    vector<Job> jobs;
    vector<Resource> resources;
	vector<simulate> timeline;
	int numOfResource;
    int time = 0;

public:
    Inheritance(vector<Job>& taskList, int numOfResource, int choice = CHOICE);
    void simulateResource();
    bool allTasksFinished();
    void updateResourceUsage(Job& job);
    Job* getNextRunnableTask();
    void runTask(Job& t);
    Job& getTaskById(const int& id);
	Resource& getResourceById(const int& id);
    void displayTimeline();
private:
    int choice_;
};

#endif // SCHEDULER_H
