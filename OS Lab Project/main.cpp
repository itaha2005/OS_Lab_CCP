#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include "BoundedBuffer.h"
#include "Scheduler.h"
#include "ProducerConsumer.h"
#include "BankersAlgorithm.h"

using namespace std;

// Global variables for the system
Scheduler* globalScheduler = nullptr;
BankersAlgorithm* globalBanker = nullptr;
int numResourceTypes = 3;
vector<int> totalResources = {10, 5, 7};

void displayMenu() {
    cout << "\n========================================" << endl;
    cout << "  CPU SCHEDULING SIMULATOR - MAIN MENU" << endl;
    cout << "========================================" << endl;
    cout << "1. Start Simulation (Producer-Consumer)" << endl;
    cout << "2. Add Process Manually" << endl;
    cout << "3. Display System State" << endl;
    cout << "4. Exit" << endl;
    cout << "========================================" << endl;
    cout << "Enter your choice: ";
}

void startSimulation() {
    srand(time(NULL));
    
    cout << "\n========================================" << endl;
    cout << "  PRODUCER-CONSUMER SIMULATION" << endl;
    cout << "========================================\n" << endl;
    
    int numProducers, bufferSize, totalProcesses, timeQuantum;
    
    cout << "Enter number of producer threads (minimum 2): ";
    cin >> numProducers;
    if (numProducers < 2) {
        cout << "Warning: At least 2 producers required. Setting to 2." << endl;
        numProducers = 2;
    }
    
    cout << "Enter buffer size: ";
    cin >> bufferSize;
    
    cout << "Enter total number of processes to generate: ";
    cin >> totalProcesses;
    
    // Determine if we need time quantum (only for Round Robin when >5 processes)
    timeQuantum = 2; // Default value
    if (totalProcesses > 5) {
        cout << "Enter time quantum for Round Robin: ";
        cin >> timeQuantum;
    }
    
    // Initialize Banker's Algorithm
    if (globalBanker) delete globalBanker;
    globalBanker = new BankersAlgorithm(numResourceTypes, totalResources);
    
    // Initialize Scheduler
    if (globalScheduler) delete globalScheduler;
    globalScheduler = new Scheduler();
    globalScheduler->setTimeQuantum(timeQuantum);
    globalScheduler->setBanker(globalBanker);
    
    BoundedBuffer buffer(bufferSize);
    
    int nextProcessID = 1;
    pthread_mutex_t idMutex;
    pthread_mutex_init(&idMutex, NULL);
    
    bool consumerFinished = false;
    pthread_mutex_t finishMutex;
    pthread_mutex_init(&finishMutex, NULL);
    
    int processesPerProducer = totalProcesses / numProducers;
    int remainingProcesses = totalProcesses % numProducers;
    
    pthread_t* producers = new pthread_t[numProducers];
    ProducerArgs* producerArgs = new ProducerArgs[numProducers];
    
    cout << "\n========================================" << endl;
    cout << "STARTING THREADS" << endl;
    cout << "========================================" << endl;
    
    for (int i = 0; i < numProducers; i++) {
        producerArgs[i].producerID = i + 1;
        producerArgs[i].numProcesses = processesPerProducer;
        if (i == 0) producerArgs[i].numProcesses += remainingProcesses;
        producerArgs[i].buffer = &buffer;
        producerArgs[i].nextProcessID = &nextProcessID;
        producerArgs[i].idMutex = &idMutex;
        producerArgs[i].numResources = numResourceTypes;
        
        pthread_create(&producers[i], NULL, producerThread, &producerArgs[i]);
    }
    
    pthread_t consumer;
    ConsumerArgs consumerArgs;
    consumerArgs.buffer = &buffer;
    consumerArgs.scheduler = globalScheduler;
    consumerArgs.totalProcesses = totalProcesses;
    consumerArgs.finished = &consumerFinished;
    consumerArgs.finishMutex = &finishMutex;
    
    pthread_create(&consumer, NULL, consumerThread, &consumerArgs);
    
    for (int i = 0; i < numProducers; i++) {
        pthread_join(producers[i], NULL);
    }
    pthread_join(consumer, NULL);
    
    pthread_mutex_destroy(&idMutex);
    pthread_mutex_destroy(&finishMutex);
    delete[] producers;
    delete[] producerArgs;
    
    cout << "\n========================================" << endl;
    cout << "ALL THREADS COMPLETED" << endl;
    cout << "========================================" << endl;
    
    // Display results
    globalScheduler->displayProcessTable();
    globalBanker->displaySystemState();
    globalScheduler->executeScheduling();
    globalScheduler->displayGanttChart();
    globalScheduler->displayStatistics();
    globalBanker->displaySystemState();
}

void addProcessManually() {
    if (!globalScheduler) {
        globalScheduler = new Scheduler();
        globalScheduler->setTimeQuantum(2);
    }
    
    if (!globalBanker) {
        globalBanker = new BankersAlgorithm(numResourceTypes, totalResources);
        globalScheduler->setBanker(globalBanker);
    }
    
    cout << "\n========================================" << endl;
    cout << "  ADD PROCESS MANUALLY" << endl;
    cout << "========================================\n" << endl;
    
    Process* p = new Process();
    
    cout << "Enter Process ID: ";
    cin >> p->processID;
    
    cout << "Enter Arrival Time: ";
    cin >> p->arrivalTime;
    
    cout << "Enter Burst Time: ";
    cin >> p->burstTime;
    
    cout << "Enter Priority (lower = higher priority): ";
    cin >> p->priority;
    
    cout << "Enter resource requirements [" << numResourceTypes << " types]:" << endl;
    p->resourceRequirements.resize(numResourceTypes);
    for (int i = 0; i < numResourceTypes; i++) {
        cout << "  Resource R" << (i + 1) << ": ";
        cin >> p->resourceRequirements[i];
    }
    
    p->remainingTime = p->burstTime;
    p->hasStarted = false;
    p->startTime = -1;
    p->isBlocked = false;
    
    globalScheduler->addProcess(p);
    
    // Check if resources can be allocated
    if (globalBanker->requestResources(p)) {
        cout << "\n[SUCCESS] Process P" << p->processID 
             << " added and resources allocated safely." << endl;
        globalBanker->releaseResources(p);  // Release for now, will allocate during execution
    } else {
        cout << "\n[WARNING] Process P" << p->processID 
             << " added but would cause unsafe state!" << endl;
        cout << "Process will be blocked during execution." << endl;
    }
}

void displaySystemState() {
    if (!globalScheduler || globalScheduler->getProcessCount() == 0) {
        cout << "\nNo processes in the system yet." << endl;
        return;
    }
    
    globalScheduler->displayProcessTable();
    
    if (globalBanker) {
        globalBanker->displaySystemState();
    }
}

int main() {
    cout << "========================================" << endl;
    cout << "  COMPREHENSIVE CPU SCHEDULING SYSTEM" << endl;
    cout << "  Parts A, B, C Integration" << endl;
    cout << "========================================" << endl;
    cout << "\nResource Configuration:" << endl;
    cout << "Number of Resource Types: " << numResourceTypes << endl;
    cout << "Total Resources: [";
    for (int i = 0; i < numResourceTypes; i++) {
        cout << totalResources[i];
        if (i < numResourceTypes - 1) cout << ", ";
    }
    cout << "]" << endl;
    
    int choice;
    bool running = true;
    
    while (running) {
        displayMenu();
        cin >> choice;
        
        switch (choice) {
            case 1:
                startSimulation();
                break;
            case 2:
                addProcessManually();
                break;
            case 3:
                displaySystemState();
                break;
            case 4:
                cout << "\nExiting system..." << endl;
                running = false;
                break;
            default:
                cout << "\nInvalid choice! Please try again." << endl;
        }
    }
    
    // Cleanup
    if (globalScheduler) delete globalScheduler;
    if (globalBanker) delete globalBanker;
    
    cout << "\n========================================" << endl;
    cout << "  SYSTEM SHUTDOWN COMPLETE" << endl;
    cout << "========================================" << endl;
    
    return 0;
}