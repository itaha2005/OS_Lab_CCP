#include "ProducerConsumer.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>

using namespace std;

Process generateRandomProcess(int processID, int numResources) {
    Process p;
    p.processID = processID;
    p.arrivalTime = 0;
    p.burstTime = (rand() % 10) + 1;
    p.priority = (rand() % 5) + 1;
    
    p.resourceRequirements.resize(numResources);
    for (int i = 0; i < numResources; i++) {
        p.resourceRequirements[i] = (rand() % 5) + 1;
    }
    
    p.remainingTime = p.burstTime;
    p.hasStarted = false;
    p.startTime = -1;
    p.isBlocked = false;
    
    return p;
}

void* producerThread(void* args) {
    ProducerArgs* pArgs = (ProducerArgs*)args;
    
    cout << "\n[PRODUCER " << pArgs->producerID << "] Started" << endl;
    
    for (int i = 0; i < pArgs->numProcesses; i++) {
        pthread_mutex_lock(pArgs->idMutex);
        int processID = (*pArgs->nextProcessID)++;
        pthread_mutex_unlock(pArgs->idMutex);
        
        Process process = generateRandomProcess(processID, pArgs->numResources);
        
        cout << "[PRODUCER " << pArgs->producerID << "] Generated Process P" 
             << process.processID << " (Priority: " << process.priority 
             << ", Burst: " << process.burstTime << ")" << endl;
        
        pArgs->buffer->insert(process);
        
        usleep((rand() % 500000) + 100000);
    }
    
    cout << "[PRODUCER " << pArgs->producerID << "] Finished producing" << endl;
    
    return NULL;
}

void* consumerThread(void* args) {
    ConsumerArgs* cArgs = (ConsumerArgs*)args;
    
    cout << "\n[CONSUMER] Started - waiting for processes..." << endl;
    
    int processesConsumed = 0;
    
    while (processesConsumed < cArgs->totalProcesses) {
        Process process = cArgs->buffer->remove();
        
        Process* heapProcess = new Process(process);
        cArgs->scheduler->addProcess(heapProcess);
        processesConsumed++;
        
        cout << "[CONSUMER] Added Process P" << process.processID 
             << " to scheduler (" << processesConsumed << "/" 
             << cArgs->totalProcesses << ")" << endl;
        
        usleep((rand() % 300000) + 50000);
    }
    
    pthread_mutex_lock(cArgs->finishMutex);
    *cArgs->finished = true;
    pthread_mutex_unlock(cArgs->finishMutex);
    
    cout << "[CONSUMER] Finished consuming all processes" << endl;
    
    return NULL;
}
