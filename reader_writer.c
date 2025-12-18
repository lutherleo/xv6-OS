#include "types.h"
#include "user.h"
#include "uthreads.h"

#define NUM_READERS 3
#define NUM_WRITERS 2
#define READS_PER_READER 5
#define WRITES_PER_WRITER 3

int shared_data = 0;

int active_readers = 0;
int active_writers = 0;
int waiting_writers = 0;

mutex_t rw_mutex;
cond_t readers_ok;
cond_t writers_ok;

void reader_lock(void) {
    mutex_lock(&rw_mutex);
    
    while (active_writers > 0 || waiting_writers > 0) {
        cond_wait(&readers_ok, &rw_mutex);
    }
    
    active_readers++;
    mutex_unlock(&rw_mutex);
}

void reader_unlock(void) {
    mutex_lock(&rw_mutex);
    
    active_readers--;
    
    if (active_readers == 0 && waiting_writers > 0) {
        cond_signal(&writers_ok);
    }
    
    mutex_unlock(&rw_mutex);
}

void writer_lock(void) {
    mutex_lock(&rw_mutex);
    
    waiting_writers++;
    
    while (active_readers > 0 || active_writers > 0) {
        cond_wait(&writers_ok, &rw_mutex);
    }
    
    waiting_writers--;
    active_writers++;
    
    mutex_unlock(&rw_mutex);
}

void writer_unlock(void) {
    mutex_lock(&rw_mutex);
    
    active_writers--;
    
    if (waiting_writers > 0) {
        cond_signal(&writers_ok);
    } else {
        cond_broadcast(&readers_ok);
    }
    
    mutex_unlock(&rw_mutex);
}

void* reader(void *arg) {
    int id = (int)arg;
    int i;
    
    for (i = 0; i < READS_PER_READER; i++) {
        reader_lock();
        
        int value = shared_data;
        printf(1, "Reader %d: reading value = %d (active readers: %d)\n", 
               id, value, active_readers);
        
        thread_yield();
        
        reader_unlock();
        
        thread_yield();
    }
    
    printf(1, "Reader %d: finished\n", id);
    return 0;
}

void* writer(void *arg) {
    int id = (int)arg;
    int i;
    
    for (i = 0; i < WRITES_PER_WRITER; i++) {
        writer_lock();
        
        shared_data++;
        printf(1, "Writer %d: wrote new value = %d\n", id, shared_data);
        
        thread_yield();
        
        writer_unlock();
        
        thread_yield();
    }
    
    printf(1, "Writer %d: finished\n", id);
    return 0;
}

int main(void) {
    int reader_tids[NUM_READERS];
    int writer_tids[NUM_WRITERS];
    int i;
    
    printf(1, "=== Reader-Writer Lock with Writer Priority ===\n\n");
    printf(1, "Configuration:\n");
    printf(1, "  Readers: %d (each performs %d reads)\n", NUM_READERS, READS_PER_READER);
    printf(1, "  Writers: %d (each performs %d writes)\n", NUM_WRITERS, WRITES_PER_WRITER);
    printf(1, "  Writer Priority: Enabled\n\n");
    
    thread_init();
    
    mutex_init(&rw_mutex);
    cond_init(&readers_ok);
    cond_init(&writers_ok);
    
    for (i = 0; i < NUM_READERS; i++) {
        reader_tids[i] = thread_create(reader, (void*)(i + 1));
    }
    
    for (i = 0; i < NUM_WRITERS; i++) {
        writer_tids[i] = thread_create(writer, (void*)(i + 1));
    }
    
    for (i = 0; i < NUM_READERS; i++) {
        thread_join(reader_tids[i]);
    }
    
    for (i = 0; i < NUM_WRITERS; i++) {
        thread_join(writer_tids[i]);
    }
    
    printf(1, "\n=== Summary ===\n");
    printf(1, "Final shared_data value: %d\n", shared_data);
    printf(1, "Expected writes: %d\n", NUM_WRITERS * WRITES_PER_WRITER);
    
    if (shared_data == NUM_WRITERS * WRITES_PER_WRITER) {
        printf(1, "SUCCESS! All writes completed correctly.\n");
        printf(1, "Writer priority ensured writers were not starved.\n");
    } else {
        printf(1, "FAILED!\n");
    }
    
    exit();
}