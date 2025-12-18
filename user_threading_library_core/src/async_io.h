#ifndef ASYNC_IO_H
#define ASYNC_IO_H

#define MAX_IO_REQUESTS 16

typedef enum {
    IO_READ,
    IO_WRITE,
    IO_OPEN,
    IO_CLOSE
} io_op_type;

typedef struct {
    io_op_type op;
    int fd;
    void *buf;
    int n;
    char *path;
    int flags;
    int result;
    int complete;
    int requester_tid;
} io_request_t;

void async_io_init(void);
int async_open(char *path, int flags);
int async_read(int fd, void *buf, int n);
int async_write(int fd, void *buf, int n);
int async_close(int fd);

#endif