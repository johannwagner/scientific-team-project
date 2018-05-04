#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#ifndef noop
#define noop (void)0
#endif //noop

#include "status.h"

//
//	GLOBAL VARIABLES
//

atomic_uint counter_threads = 0;
atomic_uint counter_tasks = 0;

//
//	STRUCTS
//

struct thread_task {
	void* arg;
	pthread_attr_t *attr;
	void *(*routine);
};

struct thread_pool {
	pthread_t* pool;
	queue* waiting_tasks;
	size_t size;
};

struct __enqueued_task {
	struct thread_task* thread;
	size_t priority;
	size_t id;
	size_t is_activated;
};

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

void __thread_main(void *(*routine) (void *), thread_pool *pool);
status_e __task_finished(__enqueued_task* task);
void __thread_idle();


#endif //THREAD_POOL_H
