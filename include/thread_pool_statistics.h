#ifndef THREAD_POOL_STATISTICS_H
#define THREAD_POOL_STATISTICS_H

#include <time.h>
#include <stdio.h>
//
//	STRUCTS
//
typedef struct thread_pool_stats {
	struct timespec creation_time;
	unsigned int task_enqueued_count;
	unsigned int task_complete_count; 
	long long complete_time;
	long long wait_time;
	long long avg_complete_time;
	long long avg_wait_time;
} thread_pool_stats;

typedef struct thread_stats {
	struct timespec creation_time;
	long long idle_time;
	long long busy_time;
	size_t task_count;
} thread_stats;

typedef struct task_stats {
	struct timespec enqueue_time;
	struct timespec execution_time;
	struct timespec complete_time;
} task_stats;

//
// INTERNAL METHODS
//

static inline double __get_time_diff(struct timespec* begin, struct timespec* end) {
	
	return (end->tv_sec - begin->tv_sec) * 1000000000L + (end->tv_nsec - begin->tv_nsec); /// 1000000000.0;
}

#endif //THREAD_POOL_STATISTICS_H
