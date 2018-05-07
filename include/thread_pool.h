#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#ifndef noop
#define noop (void)0
#endif //noop

#include <stdlib.h>
#include <pthread.h>

#include "status.h"
#include "priority_queue.h"
#include "thread_status.h"
#include <time.h>

//
//	GLOBAL VARIABLES
//

//static pthread_t counter_threads = 0;
static size_t counter_tasks = 0;

//
//	STRUCTS
//

typedef void (*task_routine)(void* routine);

typedef struct thread_task {
	void* args;
	pthread_attr_t *attr;
	task_routine routine;
	size_t group_id;
	size_t priority;
} thread_task;

typedef struct thread_pool {
	pthread_t* pool;
	priority_queue_t* waiting_tasks;
	size_t size;
    thread_status_e* thread_status;
	thread_task** thread_tasks;
} thread_pool;

/*typedef struct __enqueued_task {
	struct thread_task* thread;
	size_t priority;
	size_t id;
	size_t is_activated;
} __enqueued_task;
*/
typedef struct __thread_information {
	thread_pool* pool;
	size_t id;
} __thread_information;

//
//	EXTERNAL METHODS
//

thread_pool* thread_pool_create(size_t num_threads);
void thread_pool_free(thread_pool* pool);

status_e gecko_pool_enqueue_tasks(thread_task* task, thread_pool* pool, size_t num_threads);
status_e gecko_pool_enqueue_task(thread_task* task, thread_pool* pool);
size_t gecko_pool_create_group_id();
status_e gecko_pool_wait_for_id(size_t id, thread_pool* pool);

//
//	INTERNAL METHODS
//

void *__thread_main(void *args);
void __thread_idle();
status_e __check_for_group_queue(priority_queue_t* waiting_tasks, size_t size, size_t task_id);
status_e __check_for_thread_tasks(thread_pool* pool, size_t id);
//status_e __remove_from_queue(priority_queue_t* waiting_tasks, size_t task_id);

thread_task* __get_next_task(thread_pool *pool, size_t thread_id);

thread_status_e __update_thread_status(thread_pool* pool, size_t thread_id, thread_status_e thread_status);

#endif //THREAD_POOL_H
