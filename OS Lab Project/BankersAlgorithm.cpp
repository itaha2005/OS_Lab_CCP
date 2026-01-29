#include "BankersAlgorithm.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

BankersAlgorithm::BankersAlgorithm(int numResourceTypes, const vector<int>& totalResources) 
    : numResources(numResourceTypes), maxResources(totalResources), available(totalResources) {
    pthread_mutex_init(&resourceMutex, NULL);
}

BankersAlgorithm::~BankersAlgorithm() {
    pthread_mutex_destroy(&resourceMutex);
}

void BankersAlgorithm::addProcess(Process* process) {
    pthread_mutex_lock(&resourceMutex);
    processes.push_back(process);
    
    // Initialize allocated resources to 0
    process->allocatedResources.resize(numResources, 0);
    
    // Ensure resource requirements match number of resource types
    if (process->resourceRequirements.size() < numResources) {
        process->resourceRequirements.resize(numResources, 0);
    }
    
    pthread_mutex_unlock(&resourceMutex);
}

void BankersAlgorithm::removeProcess(Process* process) {
    pthread_mutex_lock(&resourceMutex);
    
    auto it = find(processes.begin(), processes.end(), process);
    if (it != processes.end()) {
        processes.erase(it);
    }
    
    // Remove from blocked list if present
    auto bit = find(blockedProcesses.begin(), blockedProcesses.end(), process->processID);
    if (bit != blockedProcesses.end()) {
        blockedProcesses.erase(bit);
    }
    
    pthread_mutex_unlock(&resourceMutex);
}

bool BankersAlgorithm::canAllocate(const Process& p, const vector<int>& tempAvailable) {
    for (int i = 0; i < numResources; i++) {
        int need = p.resourceRequirements[i] - p.allocatedResources[i];
        if (need > tempAvailable[i]) {
            return false;
        }
    }
    return true;
}

bool BankersAlgorithm::isSafe(const vector<int>& tempAvailable) {
    vector<bool> finished(processes.size(), false);
    vector<int> work = tempAvailable;
    vector<int> sequence;
    
    int count = 0;
    while (count < processes.size()) {
        bool found = false;
        
        for (int i = 0; i < processes.size(); i++) {
            if (!finished[i]) {
                bool canProceed = true;
                
                // Check if process can finish with available resources
                for (int j = 0; j < numResources; j++) {
                    int need = processes[i]->resourceRequirements[j] - 
                               processes[i]->allocatedResources[j];
                    if (need > work[j]) {
                        canProceed = false;
                        break;
                    }
                }
                
                if (canProceed) {
                    // Process can finish, release its resources
                    for (int j = 0; j < numResources; j++) {
                        work[j] += processes[i]->allocatedResources[j];
                    }
                    
                    sequence.push_back(processes[i]->processID);
                    finished[i] = true;
                    found = true;
                    count++;
                }
            }
        }
        
        if (!found) {
            // No process can proceed - unsafe state
            return false;
        }
    }
    
    // System is safe, update safe sequence
    safeSequence = sequence;
    return true;
}

bool BankersAlgorithm::requestResources(Process* process) {
    pthread_mutex_lock(&resourceMutex);
    
    // Create temporary available vector for testing
    vector<int> tempAvailable = available;
    
    // Simulate allocation
    for (int i = 0; i < numResources; i++) {
        int request = process->resourceRequirements[i] - process->allocatedResources[i];
        
        // Check if request exceeds available
        if (request > tempAvailable[i]) {
            process->isBlocked = true;
            if (find(blockedProcesses.begin(), blockedProcesses.end(), 
                     process->processID) == blockedProcesses.end()) {
                blockedProcesses.push_back(process->processID);
            }
            pthread_mutex_unlock(&resourceMutex);
            return false;
        }
        
        tempAvailable[i] -= request;
    }
    
    // Temporarily allocate resources to test safety
    vector<int> oldAllocated = process->allocatedResources;
    process->allocatedResources = process->resourceRequirements;
    
    // Check if system remains safe
    if (isSafe(tempAvailable)) {
        // Safe - commit the allocation
        available = tempAvailable;
        process->isBlocked = false;
        
        // Remove from blocked list
        auto it = find(blockedProcesses.begin(), blockedProcesses.end(), process->processID);
        if (it != blockedProcesses.end()) {
            blockedProcesses.erase(it);
        }
        
        pthread_mutex_unlock(&resourceMutex);
        return true;
    } else {
        // Unsafe - rollback and block process
        process->allocatedResources = oldAllocated;
        process->isBlocked = true;
        
        if (find(blockedProcesses.begin(), blockedProcesses.end(), 
                 process->processID) == blockedProcesses.end()) {
            blockedProcesses.push_back(process->processID);
        }
        
        pthread_mutex_unlock(&resourceMutex);
        return false;
    }
}

void BankersAlgorithm::releaseResources(Process* process) {
    pthread_mutex_lock(&resourceMutex);
    
    // Release all allocated resources
    for (int i = 0; i < numResources; i++) {
        available[i] += process->allocatedResources[i];
        process->allocatedResources[i] = 0;
    }
    
    pthread_mutex_unlock(&resourceMutex);
}

void BankersAlgorithm::displaySystemState() {
    pthread_mutex_lock(&resourceMutex);
    
    cout << "\n========================================" << endl;
    cout << "     RESOURCE MANAGEMENT STATE" << endl;
    cout << "========================================\n" << endl;
    
    // Display available resources
    cout << "Available Resources: [";
    for (int i = 0; i < numResources; i++) {
        cout << available[i];
        if (i < numResources - 1) cout << ", ";
    }
    cout << "]\n" << endl;
    
    // Display process resource allocation
    if (!processes.empty()) {
        cout << "Process Resource Table:" << endl;
        cout << left << setw(8) << "PID" 
             << setw(20) << "Max" 
             << setw(20) << "Allocated" 
             << setw(20) << "Need"
             << setw(10) << "Status" << endl;
        cout << string(78, '-') << endl;
        
        for (const auto& p : processes) {
            cout << left << setw(8) << p->processID;
            
            // Max
            cout << "[";
            for (int i = 0; i < numResources; i++) {
                cout << p->resourceRequirements[i];
                if (i < numResources - 1) cout << ",";
            }
            cout << "]" << setw(20 - numResources * 2) << " ";
            
            // Allocated
            cout << "[";
            for (int i = 0; i < numResources; i++) {
                cout << p->allocatedResources[i];
                if (i < numResources - 1) cout << ",";
            }
            cout << "]" << setw(20 - numResources * 2) << " ";
            
            // Need
            cout << "[";
            for (int i = 0; i < numResources; i++) {
                cout << (p->resourceRequirements[i] - p->allocatedResources[i]);
                if (i < numResources - 1) cout << ",";
            }
            cout << "]" << setw(20 - numResources * 2) << " ";
            
            // Status
            cout << (p->isBlocked ? "BLOCKED" : "READY") << endl;
        }
        cout << endl;
    }
    
    // Display safe sequence
    if (!safeSequence.empty()) {
        cout << "Safe Sequence: <";
        for (int i = 0; i < safeSequence.size(); i++) {
            cout << "P" << safeSequence[i];
            if (i < safeSequence.size() - 1) cout << ", ";
        }
        cout << ">" << endl;
    } else {
        cout << "Safe Sequence: Not yet computed" << endl;
    }
    
    // Display blocked processes
    if (!blockedProcesses.empty()) {
        cout << "Blocked Processes: ";
        for (int i = 0; i < blockedProcesses.size(); i++) {
            cout << "P" << blockedProcesses[i];
            if (i < blockedProcesses.size() - 1) cout << ", ";
        }
        cout << endl;
    } else {
        cout << "Blocked Processes: None" << endl;
    }
    
    cout << "========================================\n" << endl;
    
    pthread_mutex_unlock(&resourceMutex);
}

vector<int> BankersAlgorithm::getSafeSequence() const {
    return safeSequence;
}

vector<int> BankersAlgorithm::getBlockedProcesses() const {
    return blockedProcesses;
}
