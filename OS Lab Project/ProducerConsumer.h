#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include <pthread.h>
#include "BoundedBuffer.h"
#include "Scheduler.h"

struct ProducerArgs {
    int producerID;
    int numProcesses;
    BoundedBuffer* buffer;
    int* nextProcessID;
    pthread_mutex_t* idMutex;
    int numResources;
};

struct ConsumerArgs {
    BoundedBuffer* buffer;
    Scheduler* scheduler;
    int totalProcesses;
    bool* finished;
    pthread_mutex_t* finishMutex;
};

void* producerThread(void* args);
void* consumerThread(void* args);
Process generateRandomProcess(int processID, int numResources);

#endif
