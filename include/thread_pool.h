#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#ifndef noop
#define noop (void)0
#endif //noop

#include <stdatomic.h>
#include <stdlib.h>
#include <pthread.h>

#include "status.h"
#include "priority_queue.h"

//
//	GLOBAL VARIABLES
//

pthread_t counter_threads = 0;
pthread_t counter_tasks = 0;

//
//	STRUCTS
//

typedef struct thread_task {
	void* args;
	pthread_attr_t *attr;
	void *(*routine);
} thread_task;

typedef struct thread_pool {
	pthread_t* pool;
	priority_queue_t* waiting_tasks;
	size_t size;

} thread_pool;

typedef struct __enqueued_task {
	struct thread_task* thread;
	size_t priority;
	size_t id;
	size_t is_activated;
} __enqueued_task;

//
//	EXTERNAL METHODS
//

thread_pool* thread_pool_create(size_t num_threads);
status_e gecko_pool_enqueue_tasks(thread_task* task, thread_pool* pool, size_t num_threads);
status_e gecko_pool_enqueue_task(thread_task* task, thread_pool* pool);
size_t gecko_pool_create_group_id();
status_e gecko_pool_wait_for_id(size_t id, thread_pool* pool);

//
//	INTERNAL METHODS
//

void __thread_main(void *(*routine) (void *, void *), thread_pool *pool);
status_e __task_finished(__enqueued_task* task);
void __thread_idle();
status_e __check__for_ready_queue(priority_queue_t* waiting_tasks, size_t size, size_t task_id);
status_e __remove_from_queue(priority_queue_t* waiting_tasks, size_t task_id);

#endif //THREAD_POOL_H
