#include "scheduler.hpp"
#include <numeric> // Add this include for std::lcm
#include <algorithm>

//graphics
#include <SFML/Graphics.hpp>
#include <filesystem> 


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
	setPriority();
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

void Scheduler::generateTimeline()
{
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
            if (choice_ == CHOICE_RM || choice_ == CHOICE_DM || choice_ == CHOICE_ARB_DEADLINE)
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
			timeline.push_back("|T" + std::to_string(tasks_[runningTask].id));
            std::cout << "|T" << tasks_[runningTask].id;
            remaining[runningTask]--;
            previousTask = runningTask;
        }
        else
        {
			timeline.push_back("|ID");
            std::cout << "|ID";
        }
    }
    std::cout << "|\n";
}

void Scheduler::displayTimeline() {
    std::string fontPath = "C:\\Fonts\\arial.ttf";
    if (!std::filesystem::exists(fontPath)) {
        std::cerr << "Font file does not exist at: " << fontPath << std::endl;
        return;
    }

    sf::Font font;
    if (!font.loadFromFile(fontPath)) {
        std::cerr << "Failed to load font from: " << fontPath << std::endl;
        return;
    }

    const int blockWidth = 20;
    const int blockHeight = 50;
    const int spacing = 0;
    const int maxTimelineSteps = std::min<size_t>(1000, timeline.size());
    const int stepsPerLine = 100;
    const int marginLeft = 50;
    const int marginTop = 80;

    const int blocksPerLine = std::min(stepsPerLine, static_cast<int>(maxTimelineSteps));
    const int lines = (maxTimelineSteps + stepsPerLine - 1) / stepsPerLine;

    const int windowWidth = blocksPerLine * (blockWidth + spacing) + marginLeft * 2;
    const int windowHeight = lines * (blockHeight + 60) + marginTop * 2;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Scheduler Timeline");

    std::map<std::string, sf::Color> taskColors;
    std::vector<sf::Color> colors = {
        sf::Color::Red, sf::Color::Green, sf::Color::Blue,
        sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan,
        sf::Color(255, 165, 0),   // Orange
        sf::Color(128, 0, 128),   // Purple
        sf::Color(0, 128, 128),   // Teal
        sf::Color(210, 105, 30),  // DBrown
        sf::Color(75, 0, 130),    // Indigo
        sf::Color(60, 179, 113),  // LGreen
        sf::Color(255, 105, 180), // HotPink
        sf::Color(47, 79, 79),    // DGray
        sf::Color(255, 215, 0)    // Gold
    };
    int colorIndex = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);

        for (int i = 0; i < maxTimelineSteps; ++i) {
            int row = i / stepsPerLine;
            int col = i % stepsPerLine;

            std::string entry = timeline[i].substr(1); // remove '|'

            float x = marginLeft + col * (blockWidth + spacing);
            float y = marginTop + row * (blockHeight + 60);

            sf::RectangleShape block(sf::Vector2f(blockWidth, blockHeight));
            block.setPosition(x, y);

            if (entry == "ID") {
                block.setFillColor(sf::Color::Black);
            }
            else {
                if (taskColors.find(entry) == taskColors.end()) {
                    taskColors[entry] = colors[colorIndex++ % colors.size()];
                }
                block.setFillColor(taskColors[entry]);
            }

            window.draw(block);

            sf::Text text(entry, font, 12);
            text.setFillColor(sf::Color::Black);
            text.setPosition(x, y + blockHeight + 2);
            window.draw(text);
        }

        for (int row = 0; row < lines; ++row) {
            float y = marginTop + row * (blockHeight + 60) + blockHeight + 25;
            float startX = marginLeft - 10;
            float endX = marginLeft + blocksPerLine * (blockWidth + spacing);

            // Draw horizontal timeline line
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(startX, y), sf::Color::Black),
                sf::Vertex(sf::Vector2f(endX, y), sf::Color::Black)
            };
            window.draw(line, 2, sf::Lines);

            //Only draw left arrow for first row
            if (row == 0) {
                sf::CircleShape leftArrow(5, 3);
                leftArrow.setRotation(270);
                leftArrow.setPosition(startX, y +5);
                leftArrow.setFillColor(sf::Color::Black);
                window.draw(leftArrow);
            }

            //Only draw right arrow for the last row
            if (row == lines - 1) {
                sf::CircleShape rightArrow(5, 3);
                rightArrow.setRotation(90);
                rightArrow.setPosition(endX + 8, y - 5);
                rightArrow.setFillColor(sf::Color::Black);
                window.draw(rightArrow);
            }

            for (int step = 0; step <= stepsPerLine && (row * stepsPerLine + step) <= maxTimelineSteps; ++step) {
                float tickX = marginLeft + step * (blockWidth + spacing);
                sf::Vertex tick[] = {
                    sf::Vertex(sf::Vector2f(tickX, y - 5), sf::Color::Black),
                    sf::Vertex(sf::Vector2f(tickX, y + 5), sf::Color::Black)
                };
                window.draw(tick, 2, sf::Lines);

                if ((step + row * stepsPerLine) % 5 == 0) {
                    sf::Text label(std::to_string(step + row * stepsPerLine), font, 12);
                    label.setFillColor(sf::Color::Black);
                    label.setPosition(tickX - 5, y + 10);
                    window.draw(label);
                }
            }
        }

        //-----------------------------------------
        // Draw color key at bottom
        float legendStartY = marginTop + lines * (blockHeight + 60) + 30;
        float legendX = marginLeft;
        float legendBlockSize = 15;
        float legendSpacing = 10;
        float textOffsetX = legendBlockSize + 5;

        int count = 0;
        for (const auto& [taskName, color] : taskColors) {
            float x = legendX + (count % 5) * 150; // max 5 entries per row
            float y = legendStartY + (count / 5) * 25;

            sf::RectangleShape colorBox(sf::Vector2f(legendBlockSize, legendBlockSize));
            colorBox.setPosition(x, y);
            colorBox.setFillColor(color);
            window.draw(colorBox);

            sf::Text label(taskName, font, 14);
            label.setFillColor(sf::Color::Black);
            label.setPosition(x + textOffsetX, y - 2);
            window.draw(label);

            count++;
        }
        //--------------------------------------------

        window.display();
    }
}



bool Scheduler::runOPA()
{
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

            for (const Task &t : tasks_)
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

    for (int i = 0; i < tasks.size(); i++)
    {
        tasks[i].priority = tasks.size() - tasks[i].priority + 1;
        cout << tasks[i].id << " priority: " << tasks[i].priority << '\n';
    }
    tasks_ = tasks;

    return true;
}


Inheritance::Inheritance(vector<Job>& taskList, int numOfResource, int choice) : jobs(taskList), numOfResource(numOfResource), choice_(choice) {
    for (int i = 1; i <= numOfResource; ++i) {
        Resource res;
        res.id = i;
        resources.push_back(res);
    }
    for (auto& task : jobs) {
        task.RWCET = task.WCET;
        task.currentPriority = task.basePriority;
        for (auto& resource : task.resourceSequence)
        {
            Resource& res = getResourceById(resource.id);
            if (res.ceilingPriority < task.basePriority)
                res.ceilingPriority = task.basePriority;
        }
    }
}

void Inheritance::simulateResource()
{
    cout << "Starting Simulation\n";
    Job* prevTask = nullptr;

    while (!allTasksFinished())
    {


        while (!allTasksFinished()) {
            for (auto& job : jobs) {
                if (job.deadline < time || job.period < time) {
                    cout << " T" << job.id << " exceeds its period or missed its deadline at time:  " << time << "\n";
                    return;
                }
            }
            cout << "Time: " << time << "\n";
            updateResourceUsage(*prevTask);
            Job* nextTask = getNextRunnableTask();

            if (nextTask)
            {
                runTask(*nextTask);
                prevTask = nextTask;
            }
            else
            {
                cout << "  CPU Idle\n";
                prevTask = nullptr;
            }
            time++;
        }
        cout << "Simulation complete.\n";
    }
}

void Inheritance::updateResourceUsage(Job &job)
{
    if (&job == nullptr)
        return;
    if (job.RWCET == 0)
    {
        job.isFinished = true;
        cout << "  " << job.id << " finished execution\n";
    }
    for (auto &resourceRequest : job.resourceSequence)
    {
        Resource &resource = getResourceById(resourceRequest.id);
        if (resource.heldBy == job.id)
            resourceRequest.duration--;

        if (resourceRequest.duration == 0 && resource.isHeld && resource.heldBy == job.id) {

            resource.isHeld = false;
            resource.heldBy = 0;
            job.currentPriority = job.basePriority;

            cout << " T" << job.id << " released R" << resourceRequest.id << "\n";
            if (choice_ == CHOICE_ICPP || choice_ == CHOICE_OCPP)
            {
                for (auto &res : job.resourceSequence)
                {
                    Resource &reso = getResourceById(res.id);

                    if (reso.ceilingPriority > job.currentPriority && reso.heldBy == job.id)
                        job.currentPriority = reso.ceilingPriority;
                }
            }
            resourceRequest.isFinished = true;
            for (auto &j : jobs)
            {
                if (choice_ == CHOICE_OCPP)
                {
                    if (j.isBlocked)
                    {
                        j.isBlocked = false;
                    }
                }
                else
                {
                    if (j.isBlocked && j.waitingFor == resourceRequest.id)
                        j.isBlocked = false;
                }
            }
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
		Resource& resource = getResourceById(resourceRequest.id);
        if (resource.isHeld && resource.heldBy != selected->id) {
			cout << " T" << selected->id << " is blocked by T" << resource.heldBy << "\n";
            Job& prev = getTaskById(resource.heldBy);

            prev.currentPriority = selected->basePriority;
            selected->isBlocked = true;
            selected->waitingFor = resourceRequest.id;
            return getNextRunnableTask();
        }

        else if (!resource.isHeld && !resourceRequest.isFinished)
        {

            if (choice_ == CHOICE_OCPP)
            {
                int lockedCeiling = 0;
                string resourceID = "";
                for (int i = 0; i < resources.size(); i++)
                {
                    if (resources[i].ceilingPriority > lockedCeiling && resources[i].heldBy != selected->id && resources[i].isHeld)
                    {
                        lockedCeiling = resources[i].ceilingPriority;
                    }
                }
                if (selected->currentPriority > lockedCeiling)
                {
                    resource.isHeld = true;
                    resource.heldBy = selected->id;
                    cout << "  T" << selected->id << " acquired R" << resource.id << "\n";
                }
                else
                {

                    for (int i = 0; i < resources.size(); i++)
                    {
                        for (int j = 0; j < jobs.size(); j++)
                        {
                            if (resources[i].isHeld && jobs[j].id != selected->id &&
                                jobs[j].id == resources[i].heldBy && jobs[j].currentPriority < selected->currentPriority)
                            {
                                jobs[j].currentPriority = selected->currentPriority;
                                selected->isBlocked = true;
                                selected->waitingFor = resourceRequest.id;
                                return getNextRunnableTask();
                            }
                        }
                    }
                }
            }
            else
            {
                resource.isHeld = true;
                resource.heldBy = selected->id;
                cout << "  T" << selected->id << " acquired R" << resource.id << "\n";
                if (choice_ == CHOICE_ICPP)
                {
                    if (selected->currentPriority < resource.ceilingPriority)
                        selected->currentPriority = resource.ceilingPriority;
                }

            }

            break;
        }
    }

    return selected;
}

bool Inheritance::allTasksFinished()
{
    return all_of(jobs.begin(), jobs.end(), [](const Job &t)
                  { return t.isFinished; });
}


void Inheritance::runTask(Job& job) {
    cout << "  Running T" << job.id << "\n";
	vector<Resource> res;
    for (auto& resource : resources) {
		if (resource.heldBy == job.id) {
			res.push_back(resource);
		}
    }
	timeline.push_back({ "T" + to_string(job.id),res , time });
    job.RWCET--;  
}

Job& Inheritance::getTaskById(const int& id) {
    for (auto& job : jobs) {
        if (job.id == id)
            return job;
    }
    throw runtime_error("Invalid Task ID");
}


Resource& Inheritance::getResourceById(const int& id) {
    for (auto& resource : resources) {
        if (resource.id == id)
            return resource;

    }
    cout << "Invalid Resource ID: " << id << endl;
    throw runtime_error("Invalid Resource ID");
}