#include "scheduler.hpp"
using namespace std;

 int main() {
     int choice;
     cout << "Choose a scheduling algorithm:\n";
     cout << CHOICE_RM   << ". Rate-Monotonic (RM)\n";
     cout << CHOICE_DM   << ". Deadline-Monotonic (DM)\n";
     cout << CHOICE_EDF  << ". Earliest Deadline First (EDF)\n";
     cout << CHOICE_LST  << ". Least Slack Time (LST)\n";
     cout << CHOICE_PIP  << ". Priority Inheritance Protocol (PIP)\n";
     cout << CHOICE_OCPP << ". Original Ceiling Priority Protocol (OCPP)\n";
     cout << CHOICE_ICPP << ". Immediate Ceiling Priority Protocol (ICPP)\n";
     cout << "Enter your choice (1-7): ";
     cin >> choice;

     if (choice < CHOICE_RM || choice > CHOICE_ICPP) {
         cout << "Invalid Input\n";
         return 1;
     }

     int numTasks;
     cout << "Enter the number of tasks: ";
     cin >> numTasks;

     if (numTasks <= 0) {
         cout << "Invalid number of tasks. Exiting.\n";
         return 1;
     }

     //Vector for holding tasks
     vector<Task> tasks;
     for (int i = 0; i < numTasks; ++i) {
         Task task;
         task.id = i + 1;
         if(choice == CHOICE_PIP || choice == CHOICE_OCPP || choice == CHOICE_ICPP) {
             cout << "Enter WCET, period, deadline, priority, release time for Task " << task.id << ": ";
             cin >> task.WCET >> task.period >> task.deadline >> task.priority;
         } else {
             cout << "Enter WCET, period, deadline for Task " << task.id << ": ";
             cin >> task.WCET >> task.period >> task.deadline;
             task.priority = 0; // Default priority for non-resource sharing protocols
         }

         tasks.push_back(task);
     }

     Scheduler scheduler(tasks, choice);

     if (choice == CHOICE_RM || choice == CHOICE_DM) {
         if(scheduler.runRMDMTest()){
             scheduler.setPriority();
             scheduler.generateTimeline();
             scheduler.displayTimelineGraphic();
         }
     } else if (choice == CHOICE_EDF || choice == CHOICE_LST) {
         if(scheduler.runEDFLSTTest()){
             scheduler.generateTimeline();
             scheduler.displayTimelineGraphic();
         }
     } else if (choice == CHOICE_PIP) {

     } else if (choice == CHOICE_OCPP || choice == CHOICE_ICPP) {

     }
     else {
         cout << "Selected protocol with resource sharing not yet implemented.\n";
     }

     return 0;
 }