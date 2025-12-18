#include "types.h"
#include "user.h"
#include "uthreads.h"

#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 10
#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

sem_t empty;
sem_t full;
mutex_t buffer_mutex;

int total_produced = 0;
int total_consumed = 0;
mutex_t stats_mutex;

void* producer(void *arg) {
    int id = (int)arg;
    int i;
    
    for (i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = id * 100 + i;
        
        sem_wait(&empty);
        mutex_lock(&buffer_mutex);
        
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        
        mutex_lock(&stats_mutex);
        total_produced++;
        mutex_unlock(&stats_mutex);
        
        printf(1, "Producer %d: produced item %d (total: %d)\n", 
               id, item, total_produced);
        
        mutex_unlock(&buffer_mutex);
        sem_post(&full);
        
        thread_yield();
    }
    
    printf(1, "Producer %d: finished\n", id);
    return 0;
}

void* consumer(void *arg) {
    int id = (int)arg;
    int items_consumed = 0;
    
    while (1) {
        mutex_lock(&stats_mutex);
        int current_total = total_consumed;
        mutex_unlock(&stats_mutex);
        
        if (current_total >= NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
            break;
        }
        
        sem_wait(&full);
        mutex_lock(&buffer_mutex);
        
        mutex_lock(&stats_mutex);
        if (total_consumed >= NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
            mutex_unlock(&stats_mutex);
            mutex_unlock(&buffer_mutex);
            sem_post(&full);
            break;
        }
        
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        
        total_consumed++;
        items_consumed++;
        
        printf(1, "Consumer %d: consumed item %d (total: %d)\n", 
               id, item, total_consumed);
        
        mutex_unlock(&stats_mutex);
        mutex_unlock(&buffer_mutex);
        sem_post(&empty);
        
        thread_yield();
    }
    
    printf(1, "Consumer %d: finished (consumed %d items)\n", id, items_consumed);
    return 0;
}

int main(void) {
    int producer_tids[NUM_PRODUCERS];
    int consumer_tids[NUM_CONSUMERS];
    int i;
    
    printf(1, "=== Producer-Consumer Problem (Semaphores) ===\n\n");
    printf(1, "Configuration:\n");
    printf(1, "  Producers: %d\n", NUM_PRODUCERS);
    printf(1, "  Consumers: %d\n", NUM_CONSUMERS);
    printf(1, "  Items per producer: %d\n", ITEMS_PER_PRODUCER);
    printf(1, "  Buffer size: %d\n", BUFFER_SIZE);
    printf(1, "  Total items: %d\n\n", NUM_PRODUCERS * ITEMS_PER_PRODUCER);
    
    thread_init();
    
    sem_init(&empty, BUFFER_SIZE);
    sem_init(&full, 0);
    mutex_init(&buffer_mutex);
    mutex_init(&stats_mutex);
    
    for (i = 0; i < NUM_PRODUCERS; i++) {
        producer_tids[i] = thread_create(producer, (void*)(i + 1));
    }
    
    for (i = 0; i < NUM_CONSUMERS; i++) {
        consumer_tids[i] = thread_create(consumer, (void*)(i + 1));
    }
    
    for (i = 0; i < NUM_PRODUCERS; i++) {
        thread_join(producer_tids[i]);
    }
    
    for (i = 0; i < NUM_CONSUMERS; i++) {
        thread_join(consumer_tids[i]);
    }
    
    printf(1, "\n=== Summary ===\n");
    printf(1, "Total produced: %d\n", total_produced);
    printf(1, "Total consumed: %d\n", total_consumed);
    printf(1, "Expected: %d\n", NUM_PRODUCERS * ITEMS_PER_PRODUCER);
    
    if (total_produced == NUM_PRODUCERS * ITEMS_PER_PRODUCER &&
        total_consumed == NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
        printf(1, "SUCCESS! All items produced and consumed.\n");
    } else {
        printf(1, "FAILED!\n");
    }
    
    exit();
}