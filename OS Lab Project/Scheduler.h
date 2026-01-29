#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include "Process.h"
#include "BankersAlgorithm.h"

class Scheduler {
private:
    std::vector<Process*> processes;
    std::vector<GanttEntry> ganttChart;
    int timeQuantum;
    BankersAlgorithm* banker;
    
    std::vector<int> getReadyProcesses(int currentTime, std::vector<bool>& completed);
    void priorityScheduling();
    void roundRobinScheduling();
    
public:
    Scheduler();
    ~Scheduler();
    
    void addProcess(Process* process);
    void setBanker(BankersAlgorithm* bankerAlgo);
    void executeScheduling();
    void setTimeQuantum(int quantum);
    
    void displayProcessTable();
    void displayGanttChart();
    void displayStatistics();
    
    int getProcessCount();
    std::vector<Process*>& getProcesses();
};

#endif
