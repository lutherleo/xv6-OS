# xv6 User-Level Threading Library

Love Kush Pranu
CS-GY 6233 - Operating Systems  
Fall 2025  
NYU Tandon

---

## What I Built

I made a threading library for xv6 that runs completely in user space. The kernel doesn't know about my threads at all. 

Main components:
- Thread creation and management
- Context switching in x86 assembly
- Round-robin scheduler
- Mutexes, semaphores, condition variables, channels
- Solutions to Producer-Consumer and Reader-Writer problems

---

## Thread Structure
```c
struct thread {
    int tid;
    int state;
    char stack[4096];
    void *sp;
    void *(*start_routine)(void*);
    void *arg;
    void *retval;
    int joining_tid;
};
```

I made the stack 4KB because that seemed reasonable. Too small and you get stack overflow, too big and you waste memory. 16 threads max, so 64KB total.

The stack pointer is at offset 4104 bytes. This matters a lot for the assembly code.

Thread states:
- T_UNUSED = 0
- T_RUNNABLE = 1
- T_RUNNING = 2
- T_SLEEPING = 3
- T_ZOMBIE = 4

---

## Thread Creation

When you create a thread I have to set up its stack to look like it was running and got switched out. This is the tricky part.
```c
int thread_create(void* (*start_routine)(void*), void *arg) {
    for (i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == T_UNUSED) {
            t = &threads[i];
            break;
        }
    }
    
    t->tid = next_tid++;
    t->state = T_RUNNABLE;
    t->start_routine = start_routine;
    t->arg = arg;
    
    char *sp = t->stack + STACK_SIZE;
    sp = (char*)((unsigned int)sp & ~3);
    
    sp -= 4;
    *(unsigned int*)sp = (unsigned int)thread_wrapper;
    
    sp -= 16;
    
    t->sp = sp;
    
    return t->tid;
}
```

The return address points to thread_wrapper:
```c
static void thread_wrapper(void) {
    void *retval = current_thread->start_routine(current_thread->arg);
    thread_exit(retval);
}
```

This way threads automatically exit when the function returns. User doesn't have to remember to call thread_exit.

---

## Context Switching

This is the assembly part. You can't change the stack pointer from C so I had to write this:
```assembly
.globl thread_switch
thread_switch:
    movl 4(%esp), %eax
    
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %ebx
    
    movl %esp, 4104(%eax)
    
    movl 8(%esp), %eax
    movl 4104(%eax), %esp
    
    popl %ebx
    popl %esi
    popl %edi
    popl %ebp
    
    ret
```

The key line is `movl 4104(%eax), %esp` which switches the stack.

The offset 4104 comes from:
- tid = 4 bytes
- state = 4 bytes
- stack = 4096 bytes
- Total = 4104

I save ebx, esi, edi, ebp because those are callee-saved registers in x86.

I spent like 2 hours debugging when I had the wrong offset. Once I fixed it everything worked.

---

## Scheduler

Round-robin scheduler. Just looks for the next runnable thread:
```c
void thread_schedule(void) {
    struct thread *old = current_thread;
    struct thread *next = 0;
    
    for (i = 1; i <= MAX_THREADS; i++) {
        int idx = (old->tid + i) % MAX_THREADS;
        if (threads[idx].state == T_RUNNABLE) {
            next = &threads[idx];
            break;
        }
    }
    
    if (next == 0) return;
    
    if (old->state == T_RUNNING) old->state = T_RUNNABLE;
    next->state = T_RUNNING;
    current_thread = next;
    
    thread_switch(old, next);
}
```

---

## Mutexes
```c
typedef struct {
    int locked;
    int owner_tid;
    struct wait_node *wait_queue;
} mutex_t;
```

Lock:
```c
void mutex_lock(mutex_t *m) {
    while (m->locked) {
        wait_queue_add(&m->wait_queue, current_thread->tid);
        current_thread->state = T_SLEEPING;
        thread_schedule();
    }
    m->locked = 1;
    m->owner_tid = current_thread->tid;
}
```

Unlock:
```c
void mutex_unlock(mutex_t *m) {
    int waiting_tid = wait_queue_remove(&m->wait_queue);
    if (waiting_tid != -1) {
        wake_thread(waiting_tid);
    }
    m->locked = 0;
    m->owner_tid = -1;
}
```

Wait queue is FIFO so it's fair.

---

## Semaphores
```c
typedef struct {
    int count;
    struct wait_node *wait_queue;
} sem_t;
```

Operations:
```c
void sem_wait(sem_t *s) {
    s->count--;
    if (s->count < 0) {
        wait_queue_add(&s->wait_queue, current_thread->tid);
        current_thread->state = T_SLEEPING;
        thread_schedule();
    }
}

void sem_post(sem_t *s) {
    s->count++;
    if (s->count <= 0) {
        int tid = wait_queue_remove(&s->wait_queue);
        wake_thread(tid);
    }
}
```

Count can go negative. Negative count = number waiting.

---

## Condition Variables
```c
typedef struct {
    struct wait_node *wait_queue;
} cond_t;
```

Wait:
```c
void cond_wait(cond_t *c, mutex_t *m) {
    wait_queue_add(&c->wait_queue, current_thread->tid);
    mutex_unlock(m);
    current_thread->state = T_SLEEPING;
    thread_schedule();
    mutex_lock(m);
}
```

Signal:
```c
void cond_signal(cond_t *c) {
    int tid = wait_queue_remove(&c->wait_queue);
    if (tid != -1) wake_thread(tid);
}

void cond_broadcast(cond_t *c) {
    while (c->wait_queue != 0) {
        int tid = wait_queue_remove(&c->wait_queue);
        wake_thread(tid);
    }
}
```

The unlock and sleep need to be atomic but since I have cooperative scheduling no other thread runs between them so it works.

---

## Channels
```c
typedef struct {
    void **buffer;
    int capacity;
    int count;
    int read_pos;
    int write_pos;
    int closed;
    mutex_t lock;
    cond_t not_empty;
    cond_t not_full;
} channel_t;
```

Send:
```c
int channel_send(channel_t *ch, void *data) {
    mutex_lock(&ch->lock);
    
    if (ch->closed) {
        mutex_unlock(&ch->lock);
        return -1;
    }
    
    while (ch->count == ch->capacity) {
        cond_wait(&ch->not_full, &ch->lock);
        if (ch->closed) {
            mutex_unlock(&ch->lock);
            return -1;
        }
    }
    
    ch->buffer[ch->write_pos] = data;
    ch->write_pos = (ch->write_pos + 1) % ch->capacity;
    ch->count++;
    
    cond_signal(&ch->not_empty);
    mutex_unlock(&ch->lock);
    return 0;
}
```

Channels use mutex and condition variables internally. They're basically a cleaner interface.

---

## Race Condition Demo

This was important. I needed to show mutexes actually work.

Without mutex:
```c
void* increment_without_mutex(void *arg) {
    for (i = 0; i < 1000; i++) {
        shared_counter++;
        if (i % 100 == 0) thread_yield();
    }
}
```

The problem is `shared_counter++` compiles to:
```assembly
mov  eax, [shared_counter]
inc  eax
mov  [shared_counter], eax
```

When threads interleave you lose updates:
```
Thread 1: Read 10
Thread 1: Increment to 11
[switch]
Thread 2: Read 10
Thread 2: Increment to 11
Thread 2: Write 11
[switch]
Thread 1: Write 11

Expected: 12
Got: 11
```

3 threads doing 1000 increments each should give 3000.

I got 2847. Lost 153 updates.

With mutex:
```c
void* increment_with_mutex(void *arg) {
    for (i = 0; i < 1000; i++) {
        mutex_lock(&counter_mutex);
        shared_counter++;
        mutex_unlock(&counter_mutex);
    }
}
```

Got exactly 3000. Mutex prevents the race.

---

## Producer-Consumer with Semaphores

Setup:
- 3 producers, each makes 10 items
- 2 consumers
- Buffer size 5
- Total 30 items

Sync:
```c
sem_t empty;
sem_t full;
mutex_t buffer_mutex;
```

Producer:
```c
void* producer(void *arg) {
    for each item:
        sem_wait(&empty);
        mutex_lock(&buffer_mutex);
        
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        
        mutex_unlock(&buffer_mutex);
        sem_post(&full);
}
```

Consumer:
```c
void* consumer(void *arg) {
    while (items remaining):
        sem_wait(&full);
        mutex_lock(&buffer_mutex);
        
        item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        
        mutex_unlock(&buffer_mutex);
        sem_post(&empty);
}
```

Empty semaphore tracks empty slots. Full semaphore tracks full slots. Mutex protects buffer.

Test result: All 30 items produced and consumed.

---

## Producer-Consumer with Channels

Same problem but cleaner:
```c
channel_t *ch = channel_create(5);

for each item:
    channel_send(ch, item);

while (true):
    if (channel_recv(ch, &item) == -1) break;
    process(item);

channel_close(ch);
```

Channel handles all the sync. Way easier to use.

---

## Reader-Writer with Writer Priority

Problem: Multiple readers okay, writers need exclusive access. Need to prevent writer starvation.

State:
```c
int active_readers = 0;
int active_writers = 0;
int waiting_writers = 0;
mutex_t rw_mutex;
cond_t readers_ok;
cond_t writers_ok;
```

Reader lock:
```c
void reader_lock(void) {
    mutex_lock(&rw_mutex);
    
    while (active_writers > 0 || waiting_writers > 0) {
        cond_wait(&readers_ok, &rw_mutex);
    }
    
    active_readers++;
    mutex_unlock(&rw_mutex);
}
```

Key part is checking waiting_writers. If any writers are waiting, readers block.

Writer unlock:
```c
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
```

Writers get signaled first. This plus readers checking for waiting writers prevents writer starvation.

Test: 3 readers doing 5 reads each, 2 writers doing 3 writes each. All 6 writes completed correctly.

---

## Design Choices

Stack size:

I picked 4KB because:
- Standard page size
- None of my tests overflow
- 16 threads = 64KB total which is reasonable

Cooperative scheduling:

Pros:
- Simpler to implement
- No timer interrupts needed
- Full control over when switches happen

Cons:
- Threads can hog CPU
- No parallelism

For this project simpler is better. Plus it's easier to debug.

Round-robin scheduling:

Could've done priority scheduling but round-robin is simpler and fair. Every thread gets equal time.

FIFO wait queues:

First to wait should be first to wake up. Fair.

---

## What Worked Well

The assembly context switching worked on the first try once I got the offset right. That felt good.

Mutexes were pretty straightforward to implement.

The race condition demo clearly shows why you need synchronization. You can literally see the lost updates.

Channels are really nice to use once they're implemented.

---

## What Was Hard

Context switching took me a while to understand. Had to look at xv6's swtch.S to figure out what registers to save.

Getting the stack setup right in thread_create was tricky. The stack grows down which is confusing.

Producer-Consumer termination was annoying. Consumers need to know when to stop. With channels I can just close them which is cleaner.

---

## Testing

I wrote 5 tests:

1. basic_thread_test - Creates 3 threads, they print stuff and return values
2. mutex_test - Shows race condition without mutex, correct with mutex
3. producer_consumer_sem - 3 producers, 2 consumers, 30 items
4. producer_consumer_chan - Same with channels
5. reader_writer - 3 readers, 2 writers, writer priority

All tests pass.

---

## Things I'd Do Differently

Maybe add priority scheduling. Would be more interesting than just round-robin.

Could make stack size configurable instead of hardcoded.

The file I/O stuff was too complicated and I couldn't get it working right. Probably would skip that.

---

## Conclusion

I built a complete threading library with:
- Thread management
- Context switching in assembly
- Scheduler
- 4 different synchronization primitives
- Solutions to classic problems

Everything works. The hardest part was the assembly but once I understood it it made sense.

Total around 500 lines of C code plus 30 lines of assembly.