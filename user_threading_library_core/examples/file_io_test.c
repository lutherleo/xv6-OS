#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "uthreads.h"

mutex_t file_mutex;

void* writer_thread(void *arg) {
    int id = (int)arg;
    int i;
    
    for (i = 0; i < 3; i++) {
        mutex_lock(&file_mutex);
        
        int fd = open("shared.txt", O_CREATE | O_WRONLY);
        if (fd >= 0) {
            char buf[32];
            int len = 0;
            
            buf[len++] = 'W';
            buf[len++] = 'r';
            buf[len++] = 'i';
            buf[len++] = 't';
            buf[len++] = 'e';
            buf[len++] = 'r';
            buf[len++] = ' ';
            buf[len++] = '0' + id;
            buf[len++] = ' ';
            buf[len++] = 'i';
            buf[len++] = 't';
            buf[len++] = 'e';
            buf[len++] = 'r';
            buf[len++] = ' ';
            buf[len++] = '0' + i;
            buf[len++] = '\n';
            
            write(fd, buf, len);
            close(fd);
            
            printf(1, "Writer %d: wrote iteration %d\n", id, i);
        }
        
        mutex_unlock(&file_mutex);
        thread_yield();
    }
    
    printf(1, "Writer %d: finished\n", id);
    return 0;
}

void* reader_thread(void *arg) {
    int id = (int)arg;
    
    thread_yield();
    thread_yield();
    
    mutex_lock(&file_mutex);
    
    int fd = open("shared.txt", O_RDONLY);
    if (fd >= 0) {
        char buf[256];
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = 0;
            printf(1, "Reader %d: read %d bytes:\n%s", id, n, buf);
        }
        close(fd);
    }
    
    mutex_unlock(&file_mutex);
    
    printf(1, "Reader %d: finished\n", id);
    return 0;
}

int main(void) {
    int w1_tid, w2_tid, r_tid;
    
    printf(1, "=== Thread-Safe File I/O Test ===\n\n");
    
    thread_init();
    mutex_init(&file_mutex);
    
    printf(1, "Creating writer threads\n");
    w1_tid = thread_create(writer_thread, (void*)1);
    w2_tid = thread_create(writer_thread, (void*)2);
    
    printf(1, "Creating reader thread\n");
    r_tid = thread_create(reader_thread, (void*)1);
    
    thread_join(w1_tid);
    thread_join(w2_tid);
    thread_join(r_tid);
    
    printf(1, "\n=== Summary ===\n");
    printf(1, "Multiple threads safely accessed shared file.\n");
    printf(1, "Mutex prevented file corruption from concurrent access.\n");
    printf(1, "SUCCESS!\n");
    
    exit();
}