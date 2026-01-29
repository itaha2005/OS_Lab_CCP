#ifndef BOUNDED_BUFFER_H
#define BOUNDED_BUFFER_H

#include <queue>
#include <semaphore.h>
#include <pthread.h>
#include "Process.h"

class BoundedBuffer {
private:
    std::queue<Process> buffer;
    int capacity;
    
    // Semaphores for synchronization
    sem_t empty;  // Counts empty slots
    sem_t full;   // Counts full slots
    
    // Mutex for mutual exclusion
    pthread_mutex_t mutex;
    
public:
    BoundedBuffer(int size);
    ~BoundedBuffer();
    
    // Producer operation
    void insert(const Process& process);
    
    // Consumer operation
    Process remove();
    
    // Check if buffer is empty
    bool isEmpty();
    
    // Get current buffer size
    int size();
};

#endif
