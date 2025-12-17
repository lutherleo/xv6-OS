#include "types.h"
#include "user.h"
#include "uthreads.h"

#define NUM_THREADS 3
#define INCREMENTS_PER_THREAD 1000

int shared_counter = 0;
mutex_t counter_mutex;

void* increment_with_mutex(void *arg) {
    int i;
    for (i = 0; i < INCREMENTS_PER_THREAD; i++) {
        mutex_lock(&counter_mutex);
        shared_counter++;
        mutex_unlock(&counter_mutex);
    }
    return 0;
}

void* increment_without_mutex(void *arg) {
    int i;
    for (i = 0; i < INCREMENTS_PER_THREAD; i++) {
        // Simulating a race condition in cooperative threading:
        // We yield BETWEEN read and write to allow interleaving
        int temp = shared_counter;   // READ
        thread_yield();               // Other threads run and modify counter
        shared_counter = temp + 1;    // WRITE (now stale!)
    }
    return 0;
}

int main(void) {
    int tids[NUM_THREADS];
    int i;
    
    printf(1, "=== Shared Counter Test ===\n\n");
    
    printf(1, "Test 1: WITHOUT mutex (demonstrating race condition)\n");
    thread_init();
    shared_counter = 0;
    
    for (i = 0; i < NUM_THREADS; i++) {
        tids[i] = thread_create(increment_without_mutex, 0);
        printf(1, "Created thread %d\n", tids[i]);
    }
    
    for (i = 0; i < NUM_THREADS; i++) {
        thread_join(tids[i]);
        printf(1, "Thread %d finished\n", tids[i]);
    }
    
    printf(1, "Final counter value WITHOUT mutex: %d\n", shared_counter);
    printf(1, "Expected value: %d\n", NUM_THREADS * INCREMENTS_PER_THREAD);
    
    if (shared_counter != NUM_THREADS * INCREMENTS_PER_THREAD) {
        printf(1, "RACE CONDITION DETECTED! Lost %d updates\n", 
               NUM_THREADS * INCREMENTS_PER_THREAD - shared_counter);
    }
    
    printf(1, "\n");
    
    printf(1, "Test 2: WITH mutex (correct synchronization)\n");
    thread_init();
    shared_counter = 0;
    mutex_init(&counter_mutex);
    
    for (i = 0; i < NUM_THREADS; i++) {
        tids[i] = thread_create(increment_with_mutex, 0);
        printf(1, "Created thread %d\n", tids[i]);
    }
    
    for (i = 0; i < NUM_THREADS; i++) {
        thread_join(tids[i]);
        printf(1, "Thread %d finished\n", tids[i]);
    }
    
    printf(1, "Final counter value WITH mutex: %d\n", shared_counter);
    printf(1, "Expected value: %d\n", NUM_THREADS * INCREMENTS_PER_THREAD);
    
    if (shared_counter == NUM_THREADS * INCREMENTS_PER_THREAD) {
        printf(1, "SUCCESS! All updates preserved.\n");
    } else {
        printf(1, "FAILED! Lost %d updates\n", 
               NUM_THREADS * INCREMENTS_PER_THREAD - shared_counter);
    }
    
    exit();
}
