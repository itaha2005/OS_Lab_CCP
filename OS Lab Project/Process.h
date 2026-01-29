#ifndef PROCESS_H
#define PROCESS_H

#include <vector>

// Structure to represent a Process
struct Process {
    int processID;
    int arrivalTime;
    int burstTime;
    int priority;
    std::vector<int> resourceRequirements;  // Max resources needed
    std::vector<int> allocatedResources;     // Currently allocated
    
    // For scheduling calculations
    int remainingTime;
    int completionTime;
    int waitingTime;
    int turnaroundTime;
    int startTime;
    bool hasStarted;
    bool isBlocked;  // For deadlock prevention
    
    Process() : processID(0), arrivalTime(0), burstTime(0), priority(0),
                remainingTime(0), completionTime(0), waitingTime(0),
                turnaroundTime(0), startTime(-1), hasStarted(false), 
                isBlocked(false) {}
};

// Structure for Gantt Chart entry
struct GanttEntry {
    int processID;
    int startTime;
    int endTime;
};

#endif
