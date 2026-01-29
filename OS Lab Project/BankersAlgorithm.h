#ifndef BANKERS_ALGORITHM_H
#define BANKERS_ALGORITHM_H

#include <vector>
#include <pthread.h>
#include "Process.h"

class BankersAlgorithm {
private:
    int numResources;
    std::vector<int> available;      // Available resources
    std::vector<int> maxResources;   // Total resources in system
    std::vector<Process*> processes; // All processes
    std::vector<int> safeSequence;   // Last computed safe sequence
    std::vector<int> blockedProcesses; // Blocked process IDs
    
    pthread_mutex_t resourceMutex;
    
    // Helper functions
    bool isSafe(const std::vector<int>& tempAvailable);
    bool canAllocate(const Process& p, const std::vector<int>& tempAvailable);
    
public:
    BankersAlgorithm(int numResourceTypes, const std::vector<int>& totalResources);
    ~BankersAlgorithm();
    
    // Check if resource allocation is safe
    bool requestResources(Process* process);
    
    // Release resources when process completes
    void releaseResources(Process* process);
    
    // Display system state
    void displaySystemState();
    
    // Get safe sequence
    std::vector<int> getSafeSequence() const;
    
    // Get blocked processes
    std::vector<int> getBlockedProcesses() const;
    
    // Add process to system
    void addProcess(Process* process);
    
    // Remove process from system
    void removeProcess(Process* process);
};

#endif
