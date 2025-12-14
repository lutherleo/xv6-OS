#ifndef UTHREADS_H
#define UTHREADS_H

#define MAX_THREADS 16
#define STACK_SIZE 4096

#define T_UNUSED   0
#define T_RUNNABLE 1
#define T_RUNNING  2
#define T_SLEEPING 3
#define T_ZOMBIE   4

struct wait_node {
    int tid;
    struct wait_node *next;
};

struct thread {
    int tid;
    int state;
    char stack[STACK_SIZE];
    void *sp;
    void *(*start_routine)(void*);
    void *arg;
    void *retval;
    int joining_tid;
};

typedef struct {
    int locked;
    int owner_tid;
    struct wait_node *wait_queue;
} mutex_t;

typedef struct {
    int count;
    struct wait_node *wait_queue;
} sem_t;

typedef struct {
    struct wait_node *wait_queue;
} cond_t;

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



void thread_init(void);
int thread_create(void* (*start_routine)(void*), void *arg);
void *thread_join(int tid);
void thread_exit(void *retval) __attribute__((noreturn));
int thread_self(void);
void thread_yield(void);
void thread_schedule(void);
void thread_switch(struct thread *old, struct thread *next);

void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);

void sem_init(sem_t *s, int value);
void sem_wait(sem_t *s);
void sem_post(sem_t *s);

void cond_init(cond_t *c);
void cond_wait(cond_t *c, mutex_t *m);
void cond_signal(cond_t *c);
void cond_broadcast(cond_t *c);

channel_t* channel_create(int capacity);
int channel_send(channel_t *ch, void *data);
int channel_recv(channel_t *ch, void **data);
void channel_close(channel_t *ch);
void channel_destroy(channel_t *ch);

void wait_queue_add(struct wait_node **queue, int tid);
int wait_queue_remove(struct wait_node **queue);
int wait_queue_remove_all(struct wait_node **queue, int *tids, int max_tids);

#endif