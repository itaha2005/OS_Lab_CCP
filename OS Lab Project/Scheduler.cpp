#include "Scheduler.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <queue>
#include <climits>

using namespace std;

Scheduler::Scheduler() : timeQuantum(2), banker(nullptr) {}

Scheduler::~Scheduler() {
    for (auto p : processes) {
        delete p;
    }
}

void Scheduler::addProcess(Process* process) {
    processes.push_back(process);
    if (banker) {
        banker->addProcess(process);
    }
}

void Scheduler::setBanker(BankersAlgorithm* bankerAlgo) {
    banker = bankerAlgo;
}

void Scheduler::setTimeQuantum(int quantum) {
    timeQuantum = quantum;
}

int Scheduler::getProcessCount() {
    return processes.size();
}

vector<Process*>& Scheduler::getProcesses() {
    return processes;
}

vector<int> Scheduler::getReadyProcesses(int currentTime, vector<bool>& completed) {
    vector<int> readyQueue;
    for (int i = 0; i < processes.size(); i++) {
        if (!completed[i] && processes[i]->arrivalTime <= currentTime && 
            processes[i]->remainingTime > 0 && !processes[i]->isBlocked) {
            readyQueue.push_back(i);
        }
    }
    return readyQueue;
}

void Scheduler::priorityScheduling() {
    cout << "\n========================================" << endl;
    cout << "EXECUTING: PRIORITY SCHEDULING (Non-preemptive)" << endl;
    cout << "========================================\n" << endl;
    
    vector<bool> completed(processes.size(), false);
    int currentTime = 0;
    int completedCount = 0;
    ganttChart.clear();
    
    while (completedCount < processes.size()) {
        vector<int> readyQueue = getReadyProcesses(currentTime, completed);
        
        if (readyQueue.empty()) {
            int nextArrival = INT_MAX;
            for (size_t i = 0; i < processes.size(); i++) {
                if (!completed[i] && processes[i]->arrivalTime > currentTime) {
                    nextArrival = min(nextArrival, processes[i]->arrivalTime);
                }
            }
            if (nextArrival != INT_MAX) {
                currentTime = nextArrival;
                continue;
            } else {
                // No more processes can arrive, but some are blocked
                // Execute without Banker's check (skip blocked processes)
                break;
            }
        }
        
        int selectedIdx = -1;
        
        // Try to find a process that can be allocated resources
        for (int idx : readyQueue) {
            // First, try to select by priority
            if (selectedIdx == -1) {
                selectedIdx = idx;
            } else if (processes[idx]->priority < processes[selectedIdx]->priority) {
                selectedIdx = idx;
            } else if (processes[idx]->priority == processes[selectedIdx]->priority) {
                if (processes[idx]->arrivalTime < processes[selectedIdx]->arrivalTime) {
                    selectedIdx = idx;
                }
            }
        }
        
        if (selectedIdx == -1) {
            break; // No process can be selected
        }
        
        Process* p = processes[selectedIdx];
        
        // Check resource allocation with Banker's Algorithm
        if (banker && !banker->requestResources(p)) {
            cout << "[BLOCKED] Process P" << p->processID 
                 << " blocked - unsafe state" << endl;
            
            // Mark as blocked and try to find another process
            p->isBlocked = true;
            
            // Try to find an unblocked process from ready queue
            bool foundUnblocked = false;
            for (int idx : readyQueue) {
                if (idx != selectedIdx && !processes[idx]->isBlocked) {
                    if (banker->requestResources(processes[idx])) {
                        selectedIdx = idx;
                        p = processes[idx];
                        foundUnblocked = true;
                        break;
                    } else {
                        processes[idx]->isBlocked = true;
                    }
                }
            }
            
            if (!foundUnblocked) {
                // All ready processes are blocked, advance time or exit
                cout << "[WARNING] All ready processes blocked. ";
                
                // Check if any process can eventually run
                bool canContinue = false;
                for (size_t i = 0; i < processes.size(); i++) {
                    if (!completed[i] && !processes[i]->isBlocked) {
                        canContinue = true;
                        break;
                    }
                }
                
                if (!canContinue) {
                    cout << "Cannot proceed safely. Skipping blocked processes." << endl;
                    // Mark all blocked processes as "completed" to exit
                    for (size_t i = 0; i < processes.size(); i++) {
                        if (processes[i]->isBlocked && !completed[i]) {
                            completed[i] = true;
                            completedCount++;
                        }
                    }
                    break;
                }
                
                currentTime++;
                continue;
            }
        }
        
        if (!p->hasStarted) {
            p->startTime = currentTime;
            p->hasStarted = true;
        }
        
        GanttEntry entry;
        entry.processID = p->processID;
        entry.startTime = currentTime;
        currentTime += p->burstTime;
        entry.endTime = currentTime;
        ganttChart.push_back(entry);
        
        p->completionTime = currentTime;
        p->turnaroundTime = p->completionTime - p->arrivalTime;
        p->waitingTime = p->turnaroundTime - p->burstTime;
        p->remainingTime = 0;
        completed[selectedIdx] = true;
        completedCount++;
        
        // Release resources
        if (banker) {
            banker->releaseResources(p);
        }
    }
}

void Scheduler::roundRobinScheduling() {
    cout << "\n========================================" << endl;
    cout << "EXECUTING: ROUND ROBIN SCHEDULING (Preemptive)" << endl;
    cout << "Time Quantum: " << timeQuantum << endl;
    cout << "========================================\n" << endl;
    
    queue<int> readyQueue;
    vector<bool> inQueue(processes.size(), false);
    vector<bool> completed(processes.size(), false);
    int currentTime = 0;
    int completedCount = 0;
    ganttChart.clear();
    int consecutiveBlocks = 0; // Track consecutive blocked attempts
    
    for (size_t i = 0; i < processes.size(); i++) {
        if (processes[i]->arrivalTime <= currentTime && !processes[i]->isBlocked) {
            readyQueue.push(i);
            inQueue[i] = true;
        }
    }
    
    while (completedCount < processes.size()) {
        if (readyQueue.empty()) {
            int nextArrival = INT_MAX;
            for (size_t i = 0; i < processes.size(); i++) {
                if (!completed[i] && processes[i]->arrivalTime > currentTime) {
                    nextArrival = min(nextArrival, processes[i]->arrivalTime);
                }
            }
            if (nextArrival != INT_MAX) {
                currentTime = nextArrival;
                for (size_t i = 0; i < processes.size(); i++) {
                    if (!inQueue[i] && !completed[i] && 
                        processes[i]->arrivalTime <= currentTime && 
                        !processes[i]->isBlocked) {
                        readyQueue.push(i);
                        inQueue[i] = true;
                    }
                }
                consecutiveBlocks = 0;
            } else {
                // No more arrivals and queue empty - all remaining must be blocked
                cout << "[WARNING] All remaining processes blocked. Terminating." << endl;
                for (size_t i = 0; i < processes.size(); i++) {
                    if (!completed[i]) {
                        completed[i] = true;
                        completedCount++;
                    }
                }
                break;
            }
            continue;
        }
        
        // Check if we're in an infinite loop (all processes blocked)
        if (consecutiveBlocks > processes.size()) {
            cout << "[WARNING] Deadlock detected - all processes blocked. Terminating." << endl;
            for (size_t i = 0; i < processes.size(); i++) {
                if (!completed[i]) {
                    completed[i] = true;
                    completedCount++;
                }
            }
            break;
        }
        
        int idx = readyQueue.front();
        readyQueue.pop();
        inQueue[idx] = false;
        
        Process* p = processes[idx];
        
        // Check resources with Banker's Algorithm
        if (banker && !p->hasStarted && !banker->requestResources(p)) {
            cout << "[BLOCKED] Process P" << p->processID 
                 << " blocked - unsafe state" << endl;
            p->isBlocked = true;
            consecutiveBlocks++;
            
            // Don't re-add to queue if blocked
            continue;
        }
        
        consecutiveBlocks = 0; // Reset counter when a process executes
        
        if (!p->hasStarted) {
            p->startTime = currentTime;
            p->hasStarted = true;
        }
        
        int executionTime = min(timeQuantum, p->remainingTime);
        
        GanttEntry entry;
        entry.processID = p->processID;
        entry.startTime = currentTime;
        currentTime += executionTime;
        entry.endTime = currentTime;
        ganttChart.push_back(entry);
        
        p->remainingTime -= executionTime;
        
        for (size_t i = 0; i < processes.size(); i++) {
            if (!inQueue[i] && !completed[i] && i != idx &&
                processes[i]->arrivalTime <= currentTime && 
                processes[i]->remainingTime > 0 &&
                !processes[i]->isBlocked) {
                readyQueue.push(i);
                inQueue[i] = true;
            }
        }
        
        if (p->remainingTime == 0) {
            p->completionTime = currentTime;
            p->turnaroundTime = p->completionTime - p->arrivalTime;
            p->waitingTime = p->turnaroundTime - p->burstTime;
            completed[idx] = true;
            completedCount++;
            
            // Release resources
            if (banker) {
                banker->releaseResources(p);
            }
        } else {
            readyQueue.push(idx);
            inQueue[idx] = true;
        }
    }
}

void Scheduler::executeScheduling() {
    int readyProcessCount = 0;
    for (const auto& p : processes) {
        if (p->arrivalTime == 0) {
            readyProcessCount++;
        }
    }
    
    cout << "\n========================================" << endl;
    cout << "SCHEDULER SELECTION" << endl;
    cout << "========================================" << endl;
    cout << "Ready processes at time 0: " << readyProcessCount << endl;
    
    for (auto& p : processes) {
        p->remainingTime = p->burstTime;
        p->hasStarted = false;
        p->startTime = -1;
    }
    
    if (readyProcessCount <= 5) {
        cout << "Condition: <= 5 ready processes" << endl;
        cout << "Selected: PRIORITY SCHEDULING" << endl;
        priorityScheduling();
    } else {
        cout << "Condition: > 5 ready processes" << endl;
        cout << "Selected: ROUND ROBIN SCHEDULING" << endl;
        roundRobinScheduling();
    }
}

void Scheduler::displayProcessTable() {
    cout << "\n========================================" << endl;
    cout << "          PROCESS TABLE" << endl;
    cout << "========================================" << endl;
    cout << left << setw(6) << "PID" 
         << setw(12) << "Arrival" 
         << setw(12) << "Burst" 
         << setw(12) << "Priority"
         << "Resources" << endl;
    cout << "----------------------------------------" << endl;
    
    for (const auto& p : processes) {
        cout << left << setw(6) << p->processID
             << setw(12) << p->arrivalTime
             << setw(12) << p->burstTime
             << setw(12) << p->priority
             << "[";
        for (size_t i = 0; i < p->resourceRequirements.size(); i++) {
            cout << p->resourceRequirements[i];
            if (i < p->resourceRequirements.size() - 1) cout << ", ";
        }
        cout << "]" << endl;
    }
    cout << endl;
}

void Scheduler::displayGanttChart() {
    cout << "\n========================================" << endl;
    cout << "          GANTT CHART" << endl;
    cout << "========================================\n" << endl;
    
    cout << "|";
    for (const auto& entry : ganttChart) {
        cout << " P" << entry.processID << " |";
    }
    cout << "\n";
    
    if (!ganttChart.empty()) {
        cout << ganttChart[0].startTime;
        for (const auto& entry : ganttChart) {
            int width = 4;
            cout << string(width, ' ') << entry.endTime;
        }
    }
    cout << "\n" << endl;
}

void Scheduler::displayStatistics() {
    cout << "========================================" << endl;
    cout << "       PROCESS STATISTICS" << endl;
    cout << "========================================" << endl;
    cout << left << setw(6) << "PID"
         << setw(15) << "Arrival"
         << setw(15) << "Burst"
         << setw(15) << "Completion"
         << setw(15) << "Waiting"
         << setw(15) << "Turnaround" << endl;
    cout << "----------------------------------------" << endl;
    
    double totalWaitingTime = 0;
    double totalTurnaroundTime = 0;
    
    for (const auto& p : processes) {
        cout << left << setw(6) << p->processID
             << setw(15) << p->arrivalTime
             << setw(15) << p->burstTime
             << setw(15) << p->completionTime
             << setw(15) << p->waitingTime
             << setw(15) << p->turnaroundTime << endl;
        
        totalWaitingTime += p->waitingTime;
        totalTurnaroundTime += p->turnaroundTime;
    }
    
    cout << "\n========================================" << endl;
    cout << "       AVERAGE STATISTICS" << endl;
    cout << "========================================" << endl;
    cout << fixed << setprecision(2);
    cout << "Average Waiting Time: " << (totalWaitingTime / processes.size()) << endl;
    cout << "Average Turnaround Time: " << (totalTurnaroundTime / processes.size()) << endl;
    cout << "========================================\n" << endl;
}