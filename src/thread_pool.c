#include "../include/thread_pool.h"

//
//  EXTERNAL METHODS
//

thread_pool* thread_pool_create(size_t num_threads) {
   thread_pool* creation_pool = (thread_pool*)malloc(sizeof(thread_pool));

   creation_pool->size = num_threads;
   //create empty threads which themselves catch their own tasks from the created queue
   for(size_t i = 0; i < num_threads; i++) {
     creation_pool->pool[i] = pthread_create(counter_threads++ ,NULL ,(__thread_main)*, NULL);
   }
}

status_e gecko_pool_enqueue_tasks(thread_task* tasks, thread_pool* pool, size_t num_threads) {
  for(size_t i= 0; i < num_threads; i++) {
    //Assuming queue will conatin something like this, this will not fail
    push(pool->waiting_tasks, thread_task[i]);
  }
  return status_ok;
}

status_e gecko_pool_enqueue_task(thread_task* task, thread_pool* pool) {
  return gecko_pool_enqueue_tasks(task, pool, 1);
}

size_t gecko_pool_create_group_id() {
  return counter_tasks++;
}

status_e gecko_pool_wait_for_id(size_t id, thread_pool* pool) {
  while(__check_queue(pool->waiting_tasks) == status_failed){
    //adjust this to not constantly check array for task_id contains instead check only when changes occur
  }
  return status_ok;
}

//
//  INTERNAL METHODS
//

void __thread_main(void *(*routine) (void *, void *), thread_pool *pool) {
	_Bool has_task = 0;
	void* args;
	pthread_attr_t* attr;
	while(1){
		if(has_task == 1) {
			routine(args, attr);
			has_task = 0;
		}
		else {
			__enqueued_task* next_task = check_queue(pool);
			routine = next_task->thread->routine;
			args = next_task->thread->args;
			attr = next_task->thread->attr;
			has_task = 1;
		}
	}
}

status_e __check_queue(priority_queue_t* waiting_tasks, size_t size, size_t task_id) {
  for(size_t i=0; i < size; i++) {
    if (waiting_tasks->content[i]->id == task_id) {
      //returns failed in case queue still contains specified task id
      return status_failed;
    }
  }
  //returns ok in case of no further waiting
  return status_ok;
}

status_e __remove_from_queue(priority_queue_t* waiting_tasks, size_t task_id) {
  //OK here i need some more inview of the working of the queue and what methods will in the end exist
}
