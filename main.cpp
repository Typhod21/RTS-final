#include "scheduler.hpp"
using namespace std;

int main(){
    int choice;
    cout << "Choose a scheduling algorithm:\n";
    cout << CHOICE_RM << ". Rate-Monotonic (RM)\n";
    cout << CHOICE_DM << ". Deadline-Monotonic (DM)\n";
    cout << CHOICE_EDF << ". Earliest Deadline First (EDF)\n";
    cout << CHOICE_LST << ". Least Slack Time (LST)\n";
    cout << CHOICE_PIP << ". Priority Inheritance Protocol (PIP)\n";
    cout << CHOICE_OCPP << ". Original Ceiling Priority Protocol (OCPP)\n";
    cout << CHOICE_ICPP << ". Immediate Ceiling Priority Protocol (ICPP)\n";
    cout << CHOICE_ARB_DEADLINE << ". Arbitrary Deadlines\n";
    cout << "Enter your choice (1-8): ";
    cin >> choice;

    if (choice < CHOICE_RM || choice > CHOICE_ARB_DEADLINE){
        cout << "Invalid Input\n";
        return 1;
    }
    int numTasks;
    cout << "Enter the number of tasks: ";
    cin >> numTasks;
    if (numTasks <= 0){
        cout << "Invalid number of tasks. Exiting.\n";
        return 1;
    }
    int numResources;
    if (choice == CHOICE_PIP || choice == CHOICE_OCPP || choice == CHOICE_ICPP) {
		cout << "Enter the number of resources: ";
		cin >> numResources;
        if (numResources <= 0) {
            cout << "Invalid number of resources. Exiting.\n";
            return 1;
        }
    }
	
    // Vector for holding tasks
    vector<Task> tasks;
	vector<Job> jobs;
    for (int i = 0; i < numTasks; ++i){
        if (choice == CHOICE_PIP || choice == CHOICE_OCPP || choice == CHOICE_ICPP){
			int numRes;
			Job job;
			job.id = i + 1;
            cout << "Enter release time, WCET, priority, period, deadline for Task " << job.id << ": ";
            cin >> job.releaseTime >> job.WCET >> job.basePriority >> job.period >> job.deadline;
			cout << "Enter number of resources for Task " << job.id << ": ";
			cin >> numRes;
			for (int j = 0; j < numRes; ++j){
				ResourceRequest resourceRequest;
				cout << "Enter resource ID and duration for Task " << job.id << ": ";
				cin >> resourceRequest.id >> resourceRequest.duration;
                job.resourceSequence.push_back(resourceRequest);
            }
            jobs.push_back(job);
        }
        else{
            Task task;
            task.id = i + 1;
            cout << "Enter WCET, period, deadline for Task " << task.id << ": ";
            cin >> task.WCET >> task.period >> task.deadline;
            task.priority = 0; // Default priority for non-resource sharing protocols
            tasks.push_back(task);
        }
    }
    
    if (choice == CHOICE_RM || choice == CHOICE_DM || choice == CHOICE_EDF || choice == CHOICE_LST || choice == CHOICE_ARB_DEADLINE) {
        Scheduler scheduler(tasks, choice);
        if (choice == CHOICE_RM || choice == CHOICE_DM){
            if (scheduler.runRMDMTest(scheduler.tasks_))
                scheduler.generateTimeline();
        }
        else if (choice == CHOICE_EDF || choice == CHOICE_LST){
            if (scheduler.runEDFLSTTest())
                scheduler.generateTimeline();
        }
        else if (choice == CHOICE_ARB_DEADLINE){
            if (scheduler.runOPA())
                scheduler.generateTimeline();
        }
        scheduler.displayTimeline();// display the timeline
    }
    else if (choice == CHOICE_PIP || choice == CHOICE_OCPP || choice == CHOICE_ICPP){
		Inheritance inheritance(jobs, numResources, choice);
		inheritance.simulateResource();
        if (inheritance.allTasksFinished()) {
            cout << "All tasks finished successfully.\n";
        }
        else {
            cout << "Some tasks are still running.\n";
        }
       // inheritance.displayTimeline(); //display visual
    }
    else
        cout << "Selected protocol with resource sharing not yet implemented.\n";
    return 0;
}