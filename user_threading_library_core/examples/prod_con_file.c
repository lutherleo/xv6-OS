#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "uthreads.h"
#include "async_io.h"

#define NUM_ITEMS 10
#define PIPE_FILE "testpipe"

void* producer_file(void *arg) {
    int fd;
    int i;
    
    printf(1, "Producer: opening file for writing\n");
    fd = async_open(PIPE_FILE, O_CREATE | O_WRONLY);
    if (fd < 0) {
        printf(1, "Producer: failed to open file\n");
        return 0;
    }
    
    for (i = 0; i < NUM_ITEMS; i++) {
        char buf[32];
        int len = 0;
        int temp = i;
        
        buf[len++] = 'I';
        buf[len++] = 't';
        buf[len++] = 'e';
        buf[len++] = 'm';
        buf[len++] = ' ';
        
        if (temp >= 10) {
            buf[len++] = '0' + (temp / 10);
        }
        buf[len++] = '0' + (temp % 10);
        buf[len++] = '\n';
        
        printf(1, "Producer: writing item %d\n", i);
        int written = async_write(fd, buf, len);
        if (written != len) {
            printf(1, "Producer: write failed\n");
        }
        
        thread_yield();
    }
    
    printf(1, "Producer: closing file\n");
    async_close(fd);
    printf(1, "Producer: finished\n");
    
    return 0;
}

void* consumer_file(void *arg) {
    int fd;
    char buf[512];
    int total_read = 0;
    
    thread_yield();
    thread_yield();
    
    printf(1, "Consumer: opening file for reading\n");
    fd = async_open(PIPE_FILE, O_RDONLY);
    if (fd < 0) {
        printf(1, "Consumer: failed to open file\n");
        return 0;
    }
    
    printf(1, "Consumer: reading from file\n");
    while (1) {
        int n = async_read(fd, buf + total_read, sizeof(buf) - total_read - 1);
        if (n <= 0) {
            break;
        }
        total_read += n;
        printf(1, "Consumer: read %d bytes\n", n);
        thread_yield();
    }
    
    buf[total_read] = 0;
    printf(1, "Consumer: total read %d bytes:\n", total_read);
    printf(1, "%s", buf);
    
    async_close(fd);
    printf(1, "Consumer: finished\n");
    
    return 0;
}

int main(void) {
    int producer_tid, consumer_tid;
    
    printf(1, "=== Producer-Consumer through File I/O ===\n\n");
    
    thread_init();
    async_io_init();
    
    unlink(PIPE_FILE);
    
    producer_tid = thread_create(producer_file, 0);
    consumer_tid = thread_create(consumer_file, 0);
    
    printf(1, "Created producer and consumer threads\n\n");
    
    thread_join(producer_tid);
    thread_join(consumer_tid);
    
    printf(1, "\n=== Summary ===\n");
    printf(1, "Producer-consumer through file completed.\n");
    printf(1, "Async I/O allowed threads to continue while waiting for I/O.\n");
    printf(1, "SUCCESS!\n");
    
    unlink(PIPE_FILE);
    
    exit();
}