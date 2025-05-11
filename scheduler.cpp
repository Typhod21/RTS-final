#include "scheduler.hpp"
#include <numeric> // Add this include for std::lcm
#include <algorithm>

#include <filesystem>

//graphics:
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <map>
#include <vector>

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
    int priority = tasks_.size();

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
            tasks_[shortestIndex].priority = priority--;
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
    timeline.assign(hyperperiod, 0); //tracks the timeline for graphic printing 

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
            timeline[t] = tasks_[runningTask].id; //record in timeline
            std::cout << "|T" << tasks_[runningTask].id;
            remaining[runningTask]--;
            previousTask = runningTask;
        } else {
            timeline[t] = 0;//record in timeline
            std::cout << "|ID";
        }
    }
    std::cout << "|\n";

    // print the timeline array
    std::cout << "Timeline array:\n[ ";
    for (int taskId : timeline) {
        std::cout << taskId << " ";
    }
    std::cout << "]\n";


}



void Scheduler::displayTimelineGraphic() {
    const int boxWidth = 20;
    const int boxHeight = 50;
    const int spacing = 2;
    const int arrowSize = 10;
    const int textOffsetY = 80;

    const int hyperperiod = timeline.size();
    unsigned int width = static_cast<unsigned int>((boxWidth + spacing) * hyperperiod + 100);
    unsigned int height = static_cast<unsigned int>(boxHeight + 100);

    sf::VideoMode videoMode(width, height);
    sf::RenderWindow window(videoMode, "Task Timeline", sf::Style::Titlebar | sf::Style::Close);

    sf::Font font;


    std::string fontPath = "arial.ttf";  // fully initialized, safe
    std::cout << "Loading font from: " << fontPath << "\n";

    if (!font.loadFromFile(fontPath)) {
        std::cerr << "Error loading font\n";
        return;
    }
    //font.loadFromFile("C:/Windows/Fonts/arial");

    // Assign colors to tasks
    std::map<int, sf::Color> taskColors;
    taskColors[0] = sf::Color::Black; // Idle
    int colorIndex = 0;
    std::vector<sf::Color> colors = {
        sf::Color::Red, sf::Color::Green, sf::Color::Blue,
        sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan,
        sf::Color(255, 165, 0),   // Orange
        sf::Color(128, 0, 128),   // Purple
        sf::Color(0, 255, 127),   // Spring green
        sf::Color(255, 192, 203)  // Pink
    };

    for (const auto& task : tasks_) {
        if (taskColors.count(task.id) == 0) {
            taskColors[task.id] = colors[colorIndex++ % colors.size()];
        }
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);

        // Draw timeline boxes and colored spacings
        for (int t = 0; t < hyperperiod; ++t) {
            float x = t * (boxWidth + spacing) + 50;
            sf::RectangleShape box(sf::Vector2f(boxWidth, boxHeight));
            box.setPosition(x, 25);
            box.setFillColor(taskColors[timeline[t]]);
            box.setOutlineColor(sf::Color::Black);
            box.setOutlineThickness(1);
            window.draw(box);

            // Draw colored spacing if next task is the same
            if (t < hyperperiod - 1 && timeline[t] == timeline[t + 1]) {
                sf::RectangleShape join(sf::Vector2f(spacing, boxHeight));
                join.setPosition(x + boxWidth, 25);
                join.setFillColor(taskColors[timeline[t]]);
                window.draw(join);
            }
        }

        // Draw timeline arrows
        sf::ConvexShape startArrow;
        startArrow.setPointCount(3);
        startArrow.setPoint(0, sf::Vector2f(40, boxHeight / 2 + 25));
        startArrow.setPoint(1, sf::Vector2f(50, 25));
        startArrow.setPoint(2, sf::Vector2f(50, boxHeight + 25));
        startArrow.setFillColor(sf::Color::Black);
        window.draw(startArrow);

        sf::ConvexShape endArrow;
        float endX = 50 + hyperperiod * (boxWidth + spacing);
        endArrow.setPointCount(3);
        endArrow.setPoint(0, sf::Vector2f(endX + 10, boxHeight / 2 + 25));
        endArrow.setPoint(1, sf::Vector2f(endX, 25));
        endArrow.setPoint(2, sf::Vector2f(endX, boxHeight + 25));
        endArrow.setFillColor(sf::Color::Black);
        window.draw(endArrow);

        // Add numbering every 5 steps
        for (int t = 0; t < hyperperiod; t += 5) {
            sf::Text label;
            label.setFont(font);
            label.setString(std::to_string(t));
            label.setCharacterSize(12);
            label.setFillColor(sf::Color::Black);
            label.setPosition(t * (boxWidth + spacing) + 50 + boxWidth / 4, textOffsetY);
            window.draw(label);
        }

        window.display();
    }
}


