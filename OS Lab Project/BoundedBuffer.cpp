#include "BoundedBuffer.h"
#include <iostream>

BoundedBuffer::BoundedBuffer(int size) : capacity(size) {
    sem_init(&empty, 0, size);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);
}

BoundedBuffer::~BoundedBuffer() {
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
}

void BoundedBuffer::insert(const Process& process) {
    sem_wait(&empty);
    
    pthread_mutex_lock(&mutex);
    buffer.push(process);
    std::cout << "[PRODUCER] Inserted Process P" << process.processID 
              << " (Priority: " << process.priority 
              << ", Burst: " << process.burstTime << ")" << std::endl;
    pthread_mutex_unlock(&mutex);
    
    sem_post(&full);
}

Process BoundedBuffer::remove() {
    sem_wait(&full);
    
    pthread_mutex_lock(&mutex);
    Process process = buffer.front();
    buffer.pop();
    std::cout << "[CONSUMER] Removed Process P" << process.processID 
              << " from buffer" << std::endl;
    pthread_mutex_unlock(&mutex);
    
    sem_post(&empty);
    
    return process;
}

bool BoundedBuffer::isEmpty() {
    pthread_mutex_lock(&mutex);
    bool empty = buffer.empty();
    pthread_mutex_unlock(&mutex);
    return empty;
}

int BoundedBuffer::size() {
    pthread_mutex_lock(&mutex);
    int sz = buffer.size();
    pthread_mutex_unlock(&mutex);
    return sz;
}
