#include "scheduler.hpp"
#include <numeric> // Add this include for std::lcm
#include <algorithm>

using namespace std;
Scheduler::Scheduler()
{
    // Default constructor
}

Scheduler::Scheduler(const vector<Task> &tasks, int choice)
    : tasks_(tasks), choice_(choice) {}

double Scheduler::computeUtilization() const
{
    double utilization = 0.0;
    for (const auto &task : tasks_)
    {
        utilization += static_cast<double>(task.WCET) / task.deadline;
    }
    return utilization;
}

int Scheduler::computeHyperperiod() const
{
    int h = 1;
    for (const auto &task : tasks_)
    {
        h = lcm(h, task.period);
    }
    return h;
}

void Scheduler::setPriority()
{
    int numTasks = tasks_.size();
    int assigned = 0;
    int priority = tasks_.size();

    while (assigned < numTasks)
    {
        int shortestValue = INT_MAX;
        int shortestIndex = -1;

        for (size_t i = 0; i < tasks_.size(); ++i)
        {
            if (tasks_[i].priority != 0)
                continue;

            int compareValue = (choice_ == CHOICE_RM) ? tasks_[i].period : tasks_[i].deadline;
            if (compareValue < shortestValue)
            {
                shortestValue = compareValue;
                shortestIndex = i;
            }
        }

        if (shortestIndex != -1)
        {
            tasks_[shortestIndex].priority = priority--;
            assigned++;
        }
        else
        {
            std::cerr << "Error: No unassigned tasks found in priority assignment.\n";
            break;
        }
    }

    for (const auto &task : tasks_)
    {
        std::cout << "Task " << task.id << " has priority " << task.priority << '\n';
    }
}

bool Scheduler::runRMDMTest(std::vector<Task> taskSet)
{
    cout << "\nRunning RM/DM schedulability tests...\n";
    double utilization = computeUtilization();
    double bound = taskSet.size() * (pow(2, 1.0 / taskSet.size()) - 1);

    if (utilization <= bound)
    {
        cout << "Schedulable: " << utilization << " <= " << bound << endl;
        return true;
    }
    else
    {
        cout << utilization << " >= " << bound << endl;
        cout << "Inconclusive using utilization. Checking response time analysis...\n";
    }

    bool schedulable = true;
    for (const auto &task : taskSet)
    {
        cout << "Task " << task.id << " response time analysis:" << endl;
        int previousTime = 0;
        int responseTime = task.WCET;
        int constantTime = 0;
        while (previousTime <= task.deadline)
        {
            constantTime = responseTime;
            responseTime = task.WCET;
            cout << "Task " << task.id << " response time:";
            for (const auto &otherTask : taskSet)
            {
                if (otherTask.id != task.id && otherTask.deadline <= task.deadline)
                {
                    cout << " ceil(" << constantTime << " / " << otherTask.period << ") * " << otherTask.WCET;
                    responseTime += ceil(static_cast<double>(constantTime) / otherTask.period) * otherTask.WCET;
                }
            }
            cout << " = " << responseTime << endl;
            if (previousTime == responseTime)
                break;
            previousTime = responseTime;
        }
        if (responseTime > task.deadline)
        {
            cout << "Task " << task.id << " is not schedulable. \n\n";
            schedulable = false;
        }
        else
        {
            cout << "Task " << task.id << " is schedulable with response time: " << responseTime << "\n\n";
        }
    }

    return schedulable;
}

bool Scheduler::runEDFLSTTest()
{
    cout << "\nRunning EDF/LST schedulability test...\n";
    double utilization = 0.0;
    bool usesDeadline = false;
    int hyper = computeHyperperiod();
    vector<int> L;

    for (const auto &task : tasks_)
    {
        utilization += static_cast<double>(task.WCET) / task.deadline;
        if (task.deadline < task.period)
        {
            usesDeadline = true;
        }
    }

    if (utilization <= 1.0)
    {
        cout << "Schedulable: " << utilization << " <= 1\n";
        return true;
    }
    else if (!usesDeadline)
    {
        cout << "Unschedulable: " << utilization << " > 1 with D == T\n";
        return false;
    }
    else
    {
        cout << "Inconclusive using utilization, applying processor demand criterion...\n";
    }

    for (const auto &task : tasks_)
    {
        for (int i = 0; i <= hyper; i += task.period)
        {
            int deadlinePoint = i + task.deadline;
            if (deadlinePoint <= hyper && find(L.begin(), L.end(), deadlinePoint) == L.end())
            {
                L.push_back(deadlinePoint);
            }
        }
    }

    sort(L.begin(), L.end());
    for (const auto &l : L)
    {
        int demand = 0;
        for (const auto &task : tasks_)
        {
            demand += floor((l + task.period - task.deadline) / task.period) * task.WCET;
        }

        if (demand > l)
        {
            cout << "Unschedulable at time " << l << ": " << demand << " > " << l << "\n";
            return false;
        }
        else
        {
            cout << "Schedulable at time " << l << ": " << demand << " <= " << l << "\n";
        }
    }

    return true;
}
bool Scheduler::runPIPTest()
{
    // Optional PIP feature
    cout << "PIP test not implemented.\n";
    return true;
}
bool Scheduler::runOCPPICPPTest()
{
    // Optional OCPP/ICPP feature
    cout << "OCPP/ICPP test not implemented.\n";
    return true;
}

void Scheduler::generateTimeline()
{
    // Optional timeline feature
    int hyperperiod = computeHyperperiod();
    vector<int> remaining(tasks_.size(), 0);
    vector<int> nextRelease(tasks_.size(), 0);
    vector<int> nextDeadline(tasks_.size(), 0);

    std::cout << "\nTimeline (0 to " << hyperperiod << "):\n";

    for (int t = 0; t < hyperperiod; ++t)
    {
        // Release tasks
        for (size_t i = 0; i < tasks_.size(); ++i)
        {
            if (t == nextRelease[i])
            {
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
        for (size_t i = 0; i < tasks_.size(); ++i)
        {
            if (choice_ == CHOICE_RM || choice_ == CHOICE_DM)
            {
                if (remaining[i] > 0 && tasks_[i].priority > priority)
                {
                    priority = tasks_[i].priority;
                    runningTask = i;
                }
            }
            else if (choice_ == CHOICE_EDF)
            {
                if (remaining[i] > 0)
                {
                    if (nextDeadline[i] < minDeadline)
                    {
                        minDeadline = nextDeadline[i];
                        runningTask = i;
                    }
                    else if (nextDeadline[i] == minDeadline)
                    {
                        runningTask = previousTask;
                    }
                }
            }
            else if (choice_ == CHOICE_LST)
            {
                int slack = (nextDeadline[i] - t) - remaining[i];
                if (remaining[i] > 0)
                {
                    if (slack < minSlack)
                    {
                        minSlack = slack;
                        runningTask = i;
                    }
                    else if (slack == minSlack)
                    {
                        runningTask = previousTask;
                    }
                }
            }
        }

        // Print which task runs
        if (runningTask != -1)
        {
            std::cout << "|T" << tasks_[runningTask].id;
            remaining[runningTask]--;
            previousTask = runningTask;
        }
        else
        {
            std::cout << "|ID";
        }
    }
    std::cout << "|\n";
}

bool Scheduler::runOPA() {
    bool unassigned = false;
    std::vector<Task> unassignedTasks(tasks_);
    std::vector<Task> tasks;

    cout << "\nAssigning priorities and checking schedulability...\n";
    for (int j = tasks_.size(); j > 0; j--)
    {
        unassigned = true;
        for (int k = 0; k < unassignedTasks.size(); k++)
        {
            tasks.clear();

            for (const Task& t : tasks_)
            {
                if (t.priority != 0)
                { // Assume 0 means no priority has been assigned
                    tasks.push_back(t);
                }
            }

            Task newTask = unassignedTasks[k];
            newTask.priority = j;
            tasks.push_back(newTask);

            if (runRMDMTest(tasks))
            {
                unassignedTasks[k].priority = j; // tasks_.size() - newTask.priority + 1;

                for (int i = 0; i < tasks_.size(); i++)
                {
                    if (tasks_[i].id == unassignedTasks[k].id)
                    {
                        tasks_[i] = unassignedTasks[k];
                        break;
                    }
                }

                unassignedTasks.erase(unassignedTasks.begin() + k);
                unassigned = false;
            }
        }
        if (unassigned)
        {
            return false;
        }
    }
    return true;
}


Inheritance::Inheritance(vector<Job>& taskList, int numOfResource) : jobs(taskList), numOfResource(numOfResource) {
	for (int i = 0; i < numOfResource; ++i) {
		Resource res;
        res.id = "S" + to_string(i + 1);
		resources.push_back(res);
	}
    for (auto& task : jobs) {
        task.RWCET = task.WCET;
        task.currentPriority = task.basePriority;
		for (auto& resource : task.resourceSequence) {
			Resource& res = getResourceById(resource.id);
			if (res.ceilingPriority < task.basePriority)
				res.ceilingPriority = task.basePriority;
        }
		cout << "Task " << task.id << " ceiling priority: " << task.basePriority << endl;
		cout << "WCET: " << task.WCET << ", Release Time: " << task.releaseTime << ", Deadline: " << task.deadline << endl;
		cout << "RWECET: " << task.RWCET << ", Current Priority: " << task.currentPriority << endl;
    }
	for (auto& resource : resources) {
		cout << "Resource " << resource.id << " ceiling priority: " << resource.ceilingPriority << endl;
	}
}

void Inheritance::simulatePIP() {
    cout << "Starting PIP Simulation\n";
	Job* prevTask = nullptr;

    while (!allTasksFinished()) {
        cout << "Time: " << time << "\n";
        //updateResourceUsage(*prevTask);

        Job* nextTask = getNextRunnableTask();

        if (nextTask) {
            runTask(*nextTask);
			prevTask = nextTask;
        }
        else {
            cout << "  CPU Idle\n";
			prevTask = nullptr;
        }
        time++;
    }
    cout << "Simulation complete.\n";
}



void Inheritance::updateResourceUsage(Job& job) {
    if (&job == nullptr)
        return;
    for (auto& resourceRequest : job.resourceSequence) {
        Resource& resource = getResourceById(resourceRequest.id);
        if (resource.heldBy == job.id)
            resourceRequest.duration--;
        if (resourceRequest.duration == 0) {
            resource.isHeld = false;
            resource.heldBy = "";
            cout << "  " << job.id << " released " << resourceRequest.id << "\n";
            job.currentPriority = job.basePriority;
        }
    }
}



Job* Inheritance::getNextRunnableTask() {
    Job* selected = nullptr;

	//find the next task with the highest priority
	for (auto& t : jobs) {
		if (t.isFinished || t.releaseTime > time || t.isBlocked)
			continue;
		if (!selected || t.currentPriority > selected->currentPriority)
			selected = &t;
	}

    if (!selected) {
		cout << "  No runnable tasks\n";
		return nullptr;
    }

	for (auto& resourceRequest : selected->resourceSequence) {
		bool here = false;
		Resource& resource = getResourceById(resourceRequest.id);
        if (resource.isHeld && resource.heldBy != selected->id) {
			cout << "  " << selected->id << " is blocked by " << resource.heldBy << "\n";
            Job& prev = getTaskById(resource.heldBy);
            prev.currentPriority = selected->basePriority;
			selected->isBlocked = true;
			selected->waitingFor = resourceRequest.id;
			return getNextRunnableTask();
        }
        else if (!resource.isHeld && !resourceRequest.isFinished) {
			resource.isHeld = true;
			resource.heldBy = selected->id;
			cout << "  " << selected->id << " acquired " << resource.id << "\n";
            break;
        }

	}

   	return selected;
}

bool Inheritance::allTasksFinished() {
    return all_of(jobs.begin(), jobs.end(), [](const Job& t) { return t.isFinished; });
}

void Inheritance::runTask(Job& t) {
    cout << "  Running " << t.id << "\n";
    t.RWCET--;
    if (t.RWCET == 0) {
        t.isFinished = true;
        cout << "  " << t.id << " finished execution\n";
    }
    for (auto& resourceRequest : t.resourceSequence) {
        Resource& resource = getResourceById(resourceRequest.id);
        if (resource.heldBy == t.id)
            resourceRequest.duration--;
        if (resourceRequest.duration == 0 && resource.isHeld) {
            resource.isHeld = false;
            resource.heldBy = "";
            t.currentPriority = t.basePriority;
            cout << "  " << t.id << " released " << resourceRequest.id << "\n";
			resourceRequest.isFinished = true;
            for (auto& job : jobs) {
                if (job.isBlocked && job.waitingFor == resourceRequest.id)
                    job.isBlocked = false;
            }

        }
    }
    
}

Job& Inheritance::getTaskById(const string& id) {
    for (auto& t : jobs) {
        if (t.id == id)
            return t;
    }
    throw runtime_error("Invalid Task ID");
}

Resource& Inheritance::getResourceById(const string& id) {
    for (auto& r : resources) {
        if (r.id == id)
            return r;
    }
	cout << "Invalid Resource ID: " << id << endl;
    throw runtime_error("Invalid Resource ID");
}