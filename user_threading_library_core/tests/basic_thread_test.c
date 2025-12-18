#include "types.h"
#include "user.h"
#include "uthreads.h"

void* simple_thread(void *arg) {
    int id = (int)arg;
    int i;
    
    printf(1, "Thread %d: starting\n", id);
    
    for (i = 0; i < 3; i++) {
        printf(1, "Thread %d: iteration %d\n", id, i);
        thread_yield();
    }
    
    printf(1, "Thread %d: finishing\n", id);
    return (void*)(id * 10);
}

int main(void) {
    int tid1, tid2, tid3;
    void *ret1, *ret2, *ret3;
    
    printf(1, "=== Basic Threading Test ===\n\n");
    
    thread_init();
    printf(1, "Threading system initialized\n");
    
    printf(1, "Main thread TID: %d\n\n", thread_self());
    
    tid1 = thread_create(simple_thread, (void*)1);
    printf(1, "Created thread with TID: %d\n", tid1);
    
    tid2 = thread_create(simple_thread, (void*)2);
    printf(1, "Created thread with TID: %d\n", tid2);
    
    tid3 = thread_create(simple_thread, (void*)3);
    printf(1, "Created thread with TID: %d\n\n", tid3);
    
    printf(1, "Main thread: yielding to let threads run\n");
    thread_yield();
    
    printf(1, "\nMain thread: joining threads\n");
    
    ret1 = thread_join(tid1);
    printf(1, "Thread %d returned: %d\n", tid1, (int)ret1);
    
    ret2 = thread_join(tid2);
    printf(1, "Thread %d returned: %d\n", tid2, (int)ret2);
    
    ret3 = thread_join(tid3);
    printf(1, "Thread %d returned: %d\n", tid3, (int)ret3);
    
    printf(1, "\nAll threads completed successfully!\n");
    printf(1, "SUCCESS!\n");
    
    exit();
}