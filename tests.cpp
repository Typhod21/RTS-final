#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "scheduler.hpp"
using namespace std;

TEST_CASE("Scheduler Tests RM")
{
    // id WCET period deadline priority;
    vector<Task> tasks = {
        {1, 21, 80, 80},
        {2, 9, 25, 25},
        {3, 4, 20, 20}};

    Scheduler scheduler(tasks, CHOICE_RM);

    double utilization = scheduler.computeUtilization();
    REQUIRE(utilization == Approx(0.823).epsilon(0.01));
    REQUIRE(scheduler.runRMDMTest(scheduler.tasks_) == true);
    scheduler.setPriority();
    scheduler.generateTimeline();
    scheduler.displayTimeline();

}
TEST_CASE("Scheduler Tests DM")
{
    // id WCET period deadline priority;
    vector<Task> tasks = {
        {1, 13, 60, 45},
        {2, 11, 50, 50},
        {3, 6, 20, 15}};

    Scheduler scheduler(tasks, CHOICE_DM);

    double utilization = scheduler.computeUtilization();
    REQUIRE(utilization == Approx(0.909).epsilon(0.01));
    REQUIRE(scheduler.runRMDMTest(scheduler.tasks_) == true);
    scheduler.setPriority();
    scheduler.generateTimeline();
}

TEST_CASE("Scheduler Tests EDF")
{
    // id WCET period deadline priority;
    vector<Task> tasks = {
        {1, 22, 60, 60},
        {2, 14, 50, 50},
        {3, 5, 20, 20}};

    vector<Task> tasks2 = {
        {1, 21, 50, 40},
        {2, 8, 20, 15}};

    Scheduler scheduler(tasks, CHOICE_EDF);
    double utilization = scheduler.computeUtilization();
    REQUIRE(utilization == Approx(0.897).epsilon(0.01));
    REQUIRE(scheduler.runEDFLSTTest() == true);
    scheduler.generateTimeline();

    Scheduler scheduler2(tasks2, CHOICE_EDF);
    double utilization2 = scheduler2.computeUtilization();
    REQUIRE(utilization2 == Approx(1.058).epsilon(0.01));
    REQUIRE(scheduler2.runEDFLSTTest() == true);
    scheduler2.generateTimeline();
}

TEST_CASE("Scheduler Tests LST")
{
    // id WCET period deadline priority;
    vector<Task> tasks = {
        {1, 3, 20, 7},
        {2, 2, 5, 4},
        {3, 2, 10, 8}};

    Scheduler scheduler(tasks, CHOICE_LST);

    double utilization = scheduler.computeUtilization();
    REQUIRE(utilization == Approx(1.178).epsilon(0.01));
    REQUIRE(scheduler.runEDFLSTTest() == true);

    scheduler.generateTimeline();
}

TEST_CASE("Scheduler Tests PIP")
{
	cout << "Testing PIP\n";
    int numOfResources = 2;
    // id, release time, WCET, priority, period, deadline, {id,duration}
    vector<Job> taskList = {

       {1, 10, 4, 5, 23, 23, {{1, 3}}},
       {2, 8,  3, 4, 23, 23, {{2, 2}}},
       {3, 6,  3, 3, 23, 23, {{1, 2}}},
       {4, 3,  7, 2, 23, 23, {{1, 4}, {2, 2}}},
       {5, 0,  6, 1, 23, 23, {{2, 3}}}
    };


    Inheritance Inheritance(taskList, numOfResources, CHOICE_PIP);
    Inheritance.simulateResource();
}

TEST_CASE("Scheduler Tests ICPP")
{
	cout << "Testing ICPP\n";
    int numOfResources = 2;
    // id, release time, WCET, priority, period, deadline, {id,duration, nested}
    vector<Job> taskList = {

       {1, 10, 4, 5, 23, 23, {{1, 3}}},
       {2, 8,  3, 4, 23, 23, {{2, 2}}},
       {3, 6,  3, 3, 23, 23, {{1, 2}}},
       {4, 3,  7, 2, 23, 23, {{1, 4}, {2, 2}}},
       {5, 0,  6, 1, 23, 23, {{2, 3}}}
    };

    Inheritance Inheritance(taskList, numOfResources, CHOICE_ICPP);
    Inheritance.simulateResource();
}


TEST_CASE("Scheduler Tests OCPP") {
	cout << "Testing OCPP\n";
    int numOfResources = 2;
    // id, release time, WCET, priority, period, deadline, {id,duration, nested}
    vector<Job> taskList = {
       {1, 10, 4, 5, 23, 23, {{1, 3}}},
       {2, 8,  3, 4, 23, 23, {{2, 2}}},
       {3, 6,  3, 3, 23, 23, {{2, 2}}},
       {4, 3,  7, 2, 23, 23, {{1, 4}, {2, 2}}},
       {5, 0,  6, 1, 23, 23, {{2, 3}}}
    };


    Inheritance Inheritance(taskList, numOfResources, CHOICE_OCPP);
    Inheritance.simulateResource();
}

TEST_CASE("Scheduler Tests OCPP2") {
    cout << "Testing OCPP2\n";
    int numOfResources = 2;
    // id, release time, WCET, priority, period, deadline, {id,duration, nested}
    vector<Job> taskList = {
       {1, 10, 4, 5, 23, 23, {{1, 3}}},
       {2, 8,  3, 4, 23, 23, {{2, 2}}},
       {3, 6,  3, 3, 23, 23, {{1, 2}}},
       {4, 3,  7, 2, 23, 23, {{1, 4}, {2, 2}}},
       {5, 0,  6, 1, 23, 23, {{2, 5}}}
    };


    Inheritance Inheritance(taskList, numOfResources, CHOICE_OCPP);
    Inheritance.simulateResource();
}

