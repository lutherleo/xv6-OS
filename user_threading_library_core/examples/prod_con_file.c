#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "uthreads.h"
#include "async_io.h"

#define NUM_ITEMS 5

void* producer_file(void *arg) {
    int fd;
    int i;
    
    printf(1, "Producer: opening file\n");
    fd = async_open("testfile", O_CREATE | O_WRONLY);
    if (fd < 0) {
        printf(1, "Producer: open failed\n");
        return 0;
    }
    
    printf(1, "Producer: writing items\n");
    for (i = 0; i < NUM_ITEMS; i++) {
        char buf[16];
        buf[0] = 'A' + i;
        buf[1] = '\n';
        
        int written = async_write(fd, buf, 2);
        printf(1, "Producer: wrote item %d (result: %d)\n", i, written);
        thread_yield();
    }
    
    async_close(fd);
    printf(1, "Producer: done\n");
    
    return 0;
}

void* consumer_file(void *arg) {
    int fd;
    char buf[64];
    
    thread_yield();
    thread_yield();
    
    printf(1, "Consumer: opening file\n");
    fd = async_open("testfile", O_RDONLY);
    if (fd < 0) {
        printf(1, "Consumer: open failed\n");
        return 0;
    }
    
    printf(1, "Consumer: reading\n");
    int n = async_read(fd, buf, sizeof(buf));
    if (n > 0) {
        printf(1, "Consumer: read %d bytes\n", n);
        buf[n] = 0;
        printf(1, "Content: %s", buf);
    }
    
    async_close(fd);
    printf(1, "Consumer: done\n");
    
    return 0;
}

int main(void) {
    int p_tid, c_tid;
    
    printf(1, "=== Async File I/O Test ===\n\n");
    
    thread_init();
    async_io_init();
    
    printf(1, "Creating threads\n");
    
    p_tid = thread_create(producer_file, 0);
    c_tid = thread_create(consumer_file, 0);
    
    thread_join(p_tid);
    thread_join(c_tid);
    
    printf(1, "\nSUCCESS! Async I/O completed.\n");
    
    exit();
}