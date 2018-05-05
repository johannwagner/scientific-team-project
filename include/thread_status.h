#ifndef THREAD_POOL_STATUS_H
#define THREAD_POOL_STATUS_H

typedef enum thread_status_e
{
    thread_status_idle = 0,
    thread_status_working = 1,
    thread_status_aborted = 2,
    thread_status_finished = 3,
    thread_status_killed = 4,
    thread_status_created = 5,
    thread_status_will_terminate = 6,
    thread_status_empty = 99
} thread_status_e;

#endif //THREAD_POOL_STATUS_H
