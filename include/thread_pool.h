#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#ifndef noop
#define noop (void)0
#endif //noop

#include <stdlib.h>
#include <pthread.h>

// Required due to bug in gcc, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60932
// stdatomic.h must no be included in GTest
#ifndef CPP_TEST 
	#include <stdatomic.h>
#endif

#include "status.h"
#include "priority_queue.h"
#include "thread_status.h"
#include "thread_pool_statistics.h"

//
//	GLOBAL VARIABLES
//

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
	task_stats* statistics; 
} thread_task;

struct __thread_information;


typedef struct __task_state{
	atomic_int task_count; //< remaining tasks in this group
	unsigned generation;
} __task_state;

typedef struct task_handle{
	size_t index;
	unsigned generation;
} task_handle;

typedef struct thread_pool {
	char* name;
	pthread_t* pool;
	priority_queue_t* waiting_tasks;
	__task_state* task_group_states;
	size_t task_state_capacity; // number of tasks that can be tracked
	size_t size;
	size_t capacity;
	struct __thread_information** thread_infos;
	thread_task** thread_tasks;
	thread_pool_stats* statistics; 
} thread_pool;

typedef struct __thread_information {
	char name[12];
	thread_pool* pool;
	size_t id;
	atomic_int status;
	struct timespec creation; // cannot be removed, leads to segfault
	thread_stats* statistics;
} __thread_information;

//
//	EXTERNAL METHODS
//

thread_pool* thread_pool_create(size_t num_threads, int enable_monitoring);
thread_pool* thread_pool_create_named(size_t num_threads, const char* name, int enable_monitoring);
// Releases all resources hold by the threadpool. 
// Currently working threads may finish but tasks left in the queue will be discarded.
void thread_pool_free(thread_pool* pool);

void thread_pool_set_name(thread_pool* pool, const char* name);
const char* thread_pool_get_name(thread_pool* pool);

// Sets the number of active threads to num_threads.
// Currently working threads are terminated after there task is completed.
status_e thread_pool_resize(thread_pool* pool, size_t num_threads);

// Add multiple tasks to be executed. Their progress is tracked by a single handle.
// hndl can be a nullptr.
status_e thread_pool_enqueue_tasks(thread_task* task, thread_pool* pool, size_t num_tasks, task_handle* hndl);
status_e thread_pool_enqueue_task(thread_task* task, thread_pool* pool, task_handle* hndl);

// Add multiple tasks to be executed. Waits until all passed tasks are finished. 
// The main thread also participates in task execution
status_e thread_pool_enqueue_tasks_wait(thread_task* task, thread_pool* pool, size_t num_tasks);

// Waits until the tasks referenced by hndl are completed.
status_e thread_pool_wait_for_task(thread_pool* pool, task_handle* hndl);

// Waits until all tasks currently in the queue are executed.
// The main thread also participates in task execution.
status_e thread_pool_wait_for_all(thread_pool* pool);

//
//	INTERNAL METHODS
//

void *__thread_main(void *args);
thread_task* __get_next_task(thread_pool *pool);

status_e __create_thread(__thread_information* thread_info, pthread_t* pp);
thread_status_e __update_thread_status(thread_pool* pool, size_t thread_id, thread_status_e thread_status);

#endif //THREAD_POOL_H
