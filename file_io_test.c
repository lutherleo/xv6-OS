#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "uthreads.h"

mutex_t m;

void* simple_func(void *arg) {
    printf(1, "Thread running\n");
    mutex_lock(&m);
    printf(1, "Got mutex\n");
    mutex_unlock(&m);
    return 0;
}

int main(void) {
    printf(1, "Start\n");
    thread_init();
    printf(1, "Init done\n");
    mutex_init(&m);
    printf(1, "Mutex init done\n");
    
    int tid = thread_create(simple_func, 0);
    printf(1, "Created thread %d\n", tid);
    
    thread_join(tid);
    printf(1, "SUCCESS\n");
    exit();
}