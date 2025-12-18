#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "uthreads.h"
#include "async_io.h"

extern struct thread threads[MAX_THREADS];
extern struct thread *current_thread;

io_request_t io_requests[MAX_IO_REQUESTS];
mutex_t io_mutex;
int io_thread_running = 0;
int next_request_id = 0;

void* io_worker_thread(void *arg) {
    int i;
    
    while (io_thread_running) {
        mutex_lock(&io_mutex);
        
        for (i = 0; i < MAX_IO_REQUESTS; i++) {
            if (io_requests[i].requester_tid != -1 && !io_requests[i].complete) {
                io_request_t *req = &io_requests[i];
                
                mutex_unlock(&io_mutex);
                
                switch (req->op) {
                    case IO_OPEN:
                        req->result = open(req->path, req->flags);
                        break;
                    case IO_READ:
                        req->result = read(req->fd, req->buf, req->n);
                        break;
                    case IO_WRITE:
                        req->result = write(req->fd, req->buf, req->n);
                        break;
                    case IO_CLOSE:
                        req->result = close(req->fd);
                        break;
                }
                
                mutex_lock(&io_mutex);
                req->complete = 1;
                
                int tid = req->requester_tid;
                int j;
                for (j = 0; j < MAX_THREADS; j++) {
                    if (threads[j].tid == tid && threads[j].state == T_SLEEPING) {
                        threads[j].state = T_RUNNABLE;
                        break;
                    }
                }
                mutex_unlock(&io_mutex);
                
                thread_yield();
                mutex_lock(&io_mutex);
            }
        }
        
        mutex_unlock(&io_mutex);
        thread_yield();
    }
    
    return 0;
}

void async_io_init(void) {
    int i;
    
    mutex_init(&io_mutex);
    
    for (i = 0; i < MAX_IO_REQUESTS; i++) {
        io_requests[i].requester_tid = -1;
        io_requests[i].complete = 0;
    }
    
    io_thread_running = 1;
    thread_create(io_worker_thread, 0);
}

static int submit_io_request(io_request_t *req) {
    int slot = -1;
    int i;
    
    mutex_lock(&io_mutex);
    
    for (i = 0; i < MAX_IO_REQUESTS; i++) {
        if (io_requests[i].requester_tid == -1) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        mutex_unlock(&io_mutex);
        return -1;
    }
    
    io_requests[slot] = *req;
    io_requests[slot].requester_tid = thread_self();
    io_requests[slot].complete = 0;
    
    mutex_unlock(&io_mutex);
    
    while (!io_requests[slot].complete) {
        current_thread->state = T_SLEEPING;
        thread_schedule();
    }
    
    int result = io_requests[slot].result;
    
    mutex_lock(&io_mutex);
    io_requests[slot].requester_tid = -1;
    mutex_unlock(&io_mutex);
    
    return result;
}

int async_open(char *path, int flags) {
    io_request_t req;
    req.op = IO_OPEN;
    req.path = path;
    req.flags = flags;
    return submit_io_request(&req);
}

int async_read(int fd, void *buf, int n) {
    io_request_t req;
    req.op = IO_READ;
    req.fd = fd;
    req.buf = buf;
    req.n = n;
    return submit_io_request(&req);
}

int async_write(int fd, void *buf, int n) {
    io_request_t req;
    req.op = IO_WRITE;
    req.fd = fd;
    req.buf = buf;
    req.n = n;
    return submit_io_request(&req);
}

int async_close(int fd) {
    io_request_t req;
    req.op = IO_CLOSE;
    req.fd = fd;
    return submit_io_request(&req);
}