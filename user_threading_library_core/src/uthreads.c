#include "types.h"
#include "user.h"
#include "uthreads.h"

struct thread threads[MAX_THREADS];
struct thread *current_thread = 0;
int next_tid = 1;

void wait_queue_add(struct wait_node **queue, int tid) {
    struct wait_node *node = (struct wait_node*)malloc(sizeof(struct wait_node));
    node->tid = tid;
    node->next = 0;
    
    if (*queue == 0) {
        *queue = node;
    } else {
        struct wait_node *curr = *queue;
        while (curr->next != 0) {
            curr = curr->next;
        }
        curr->next = node;
    }
}

int wait_queue_remove(struct wait_node **queue) {
    if (*queue == 0) {
        return -1;
    }
    
    struct wait_node *node = *queue;
    int tid = node->tid;
    *queue = node->next;
    free(node);
    return tid;
}

int wait_queue_remove_all(struct wait_node **queue, int *tids, int max_tids) {
    int count = 0;
    while (*queue != 0 && count < max_tids) {
        tids[count++] = wait_queue_remove(queue);
    }
    return count;
}

static void thread_wrapper(void) {
    void *retval = current_thread->start_routine(current_thread->arg);
    thread_exit(retval);
}

void thread_init(void) {
    int i;
    
    for (i = 0; i < MAX_THREADS; i++) {
        threads[i].tid = i;
        threads[i].state = T_UNUSED;
        threads[i].joining_tid = -1;
    }
    
    threads[0].state = T_RUNNING;
    threads[0].tid = 0;
    threads[0].joining_tid = -1;
    current_thread = &threads[0];
    next_tid = 1;
}

int thread_create(void* (*start_routine)(void*), void *arg) {
    int i;
    struct thread *t = 0;
    
    for (i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == T_UNUSED) {
            t = &threads[i];
            break;
        }
    }
    
    if (t == 0) {
        return -1;
    }
    
    t->tid = next_tid++;
    t->state = T_RUNNABLE;
    t->start_routine = start_routine;
    t->arg = arg;
    t->retval = 0;
    t->joining_tid = -1;
    
    char *sp = t->stack + STACK_SIZE;
    sp = (char*)((unsigned int)sp & ~3);
    
    sp -= 4;
    *(unsigned int*)sp = (unsigned int)thread_wrapper;
    
    sp -= 4 * 4;
    
    t->sp = sp;
    
    return t->tid;
}

void *thread_join(int tid) {
    struct thread *t = 0;
    int i;
    
    for (i = 0; i < MAX_THREADS; i++) {
        if (threads[i].tid == tid && threads[i].state != T_UNUSED) {
            t = &threads[i];
            break;
        }
    }
    
    if (t == 0) {
        return 0;
    }
    
    while (t->state != T_ZOMBIE) {
        current_thread->state = T_SLEEPING;
        t->joining_tid = current_thread->tid;
        thread_schedule();
    }
    
    void *retval = t->retval;
    
    t->state = T_UNUSED;
    t->joining_tid = -1;
    
    return retval;
}

void thread_exit(void *retval) {
    current_thread->retval = retval;
    current_thread->state = T_ZOMBIE;
    
    if (current_thread->joining_tid != -1) {
        int i;
        for (i = 0; i < MAX_THREADS; i++) {
            if (threads[i].tid == current_thread->joining_tid) {
                threads[i].state = T_RUNNABLE;
                break;
            }
        }
    }
    
    thread_schedule();
    
    while(1);
}

int thread_self(void) {
    return current_thread->tid;
}

void thread_yield(void) {
    current_thread->state = T_RUNNABLE;
    thread_schedule();
}

void thread_schedule(void) {
    struct thread *old = current_thread;
    struct thread *next = 0;
    int i;
    
    for (i = 1; i <= MAX_THREADS; i++) {
        int idx = (old->tid + i) % MAX_THREADS;
        if (threads[idx].state == T_RUNNABLE) {
            next = &threads[idx];
            break;
        }
    }
    
    if (next == 0) {
        if (old->state == T_RUNNABLE) {
            old->state = T_RUNNING;
        }
        return;
    }
    
    if (old->state == T_RUNNING) {
        old->state = T_RUNNABLE;
    }
    next->state = T_RUNNING;
    current_thread = next;
    
    thread_switch(old, next);
}

void mutex_init(mutex_t *m) {
    m->locked = 0;
    m->owner_tid = -1;
    m->wait_queue = 0;
}

void mutex_lock(mutex_t *m) {
    while (m->locked) {
        wait_queue_add(&m->wait_queue, current_thread->tid);
        current_thread->state = T_SLEEPING;
        thread_schedule();
    }
    
    m->locked = 1;
    m->owner_tid = current_thread->tid;
}

void mutex_unlock(mutex_t *m) {
    if (m->owner_tid != current_thread->tid) {
        return;
    }
    
    int waiting_tid = wait_queue_remove(&m->wait_queue);
    
    if (waiting_tid == -1) {
        m->locked = 0;
        m->owner_tid = -1;
    } else {
        int i;
        for (i = 0; i < MAX_THREADS; i++) {
            if (threads[i].tid == waiting_tid) {
                threads[i].state = T_RUNNABLE;
                break;
            }
        }
        
        m->locked = 0;
        m->owner_tid = -1;
    }
}

void sem_init(sem_t *s, int value) {
    s->count = value;
    s->wait_queue = 0;
}

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
        int waiting_tid = wait_queue_remove(&s->wait_queue);
        if (waiting_tid != -1) {
            int i;
            for (i = 0; i < MAX_THREADS; i++) {
                if (threads[i].tid == waiting_tid) {
                    threads[i].state = T_RUNNABLE;
                    break;
                }
            }
        }
    }
}

void cond_init(cond_t *c) {
    c->wait_queue = 0;
}

void cond_wait(cond_t *c, mutex_t *m) {
    wait_queue_add(&c->wait_queue, current_thread->tid);
    mutex_unlock(m);
    current_thread->state = T_SLEEPING;
    thread_schedule();
    mutex_lock(m);
}

void cond_signal(cond_t *c) {
    int waiting_tid = wait_queue_remove(&c->wait_queue);
    
    if (waiting_tid != -1) {
        int i;
        for (i = 0; i < MAX_THREADS; i++) {
            if (threads[i].tid == waiting_tid) {
                threads[i].state = T_RUNNABLE;
                break;
            }
        }
    }
}

void cond_broadcast(cond_t *c) {
    int tids[MAX_THREADS];
    int count = wait_queue_remove_all(&c->wait_queue, tids, MAX_THREADS);
    int i, j;
    
    for (i = 0; i < count; i++) {
        for (j = 0; j < MAX_THREADS; j++) {
            if (threads[j].tid == tids[i]) {
                threads[j].state = T_RUNNABLE;
                break;
            }
        }
    }
}

channel_t* channel_create(int capacity) {
    channel_t *ch = (channel_t*)malloc(sizeof(channel_t));
    
    ch->buffer = (void**)malloc(sizeof(void*) * capacity);
    ch->capacity = capacity;
    ch->count = 0;
    ch->read_pos = 0;
    ch->write_pos = 0;
    ch->closed = 0;
    
    mutex_init(&ch->lock);
    cond_init(&ch->not_empty);
    cond_init(&ch->not_full);
    
    return ch;
}

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

int channel_recv(channel_t *ch, void **data) {
    mutex_lock(&ch->lock);
    
    while (ch->count == 0) {
        if (ch->closed) {
            mutex_unlock(&ch->lock);
            return -1;
        }
        
        cond_wait(&ch->not_empty, &ch->lock);
    }
    
    *data = ch->buffer[ch->read_pos];
    ch->read_pos = (ch->read_pos + 1) % ch->capacity;
    ch->count--;
    
    cond_signal(&ch->not_full);
    
    mutex_unlock(&ch->lock);
    return 0;
}

void channel_close(channel_t *ch) {
    mutex_lock(&ch->lock);
    
    ch->closed = 1;
    
    cond_broadcast(&ch->not_empty);
    cond_broadcast(&ch->not_full);
    
    mutex_unlock(&ch->lock);
}

void channel_destroy(channel_t *ch) {
    free(ch->buffer);
    free(ch);
}