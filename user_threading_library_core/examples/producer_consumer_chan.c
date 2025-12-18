#include "types.h"
#include "user.h"
#include "uthreads.h"

#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 10
#define CHANNEL_CAPACITY 5

channel_t *item_channel;

void* producer_chan(void *arg) {
    int id = (int)arg;
    int i;
    
    for (i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int *item = (int*)malloc(sizeof(int));
        *item = id * 100 + i;
        
        if (channel_send(item_channel, item) == 0) {
            printf(1, "Producer %d: sent item %d\n", id, *item);
        } else {
            printf(1, "Producer %d: channel closed, stopping\n", id);
            free(item);
            break;
        }
        
        thread_yield();
    }
    
    printf(1, "Producer %d: finished\n", id);
    return 0;
}

void* consumer_chan(void *arg) {
    int id = (int)arg;
    int items_consumed = 0;
    
    while (1) {
        void *data;
        int result = channel_recv(item_channel, &data);
        
        if (result == -1) {
            printf(1, "Consumer %d: channel closed, exiting\n", id);
            break;
        }
        
        int *item = (int*)data;
        printf(1, "Consumer %d: received item %d\n", id, *item);
        free(item);
        items_consumed++;
        
        thread_yield();
    }
    
    printf(1, "Consumer %d: finished (consumed %d items)\n", id, items_consumed);
    return 0;
}

int main(void) {
    int producer_tids[NUM_PRODUCERS];
    int consumer_tids[NUM_CONSUMERS];
    int i;
    
    printf(1, "=== Producer-Consumer Problem (Channels) ===\n\n");
    printf(1, "Configuration:\n");
    printf(1, "  Producers: %d\n", NUM_PRODUCERS);
    printf(1, "  Consumers: %d\n", NUM_CONSUMERS);
    printf(1, "  Items per producer: %d\n", ITEMS_PER_PRODUCER);
    printf(1, "  Channel capacity: %d\n", CHANNEL_CAPACITY);
    printf(1, "  Total items: %d\n\n", NUM_PRODUCERS * ITEMS_PER_PRODUCER);
    
    thread_init();
    
    item_channel = channel_create(CHANNEL_CAPACITY);
    
    for (i = 0; i < NUM_PRODUCERS; i++) {
        producer_tids[i] = thread_create(producer_chan, (void*)(i + 1));
    }
    
    for (i = 0; i < NUM_CONSUMERS; i++) {
        consumer_tids[i] = thread_create(consumer_chan, (void*)(i + 1));
    }
    
    for (i = 0; i < NUM_PRODUCERS; i++) {
        thread_join(producer_tids[i]);
    }
    
    printf(1, "\nAll producers finished. Closing channel...\n");
    
    channel_close(item_channel);
    
    for (i = 0; i < NUM_CONSUMERS; i++) {
        thread_join(consumer_tids[i]);
    }
    
    printf(1, "\nAll consumers finished.\n");
    printf(1, "SUCCESS! Channel-based producer-consumer complete.\n");
    
    channel_destroy(item_channel);
    
    exit();
}