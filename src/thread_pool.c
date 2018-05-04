#include "thread_pool.h"
#include <stdlib.h>

//
//  EXTERNAL METHODS
//

thread_pool* thread_pool_create(size_t num_threads) {
   thread_pool* creation_pool = (thread_pool*)malloc(sizeof(thread_pool));

   creation_pool->size = num_threads;
   //create empty threads which themselves catch their own tasks from the created queue
   for(size_t i = 0; i < num_threads; i++) {
     int ret = pthread_create(&(counter_threads),NULL , &__thread_main, creation_pool);
     if ( ret == 0) {
       creation_pool->pool[i] = counter_threads;
     }
     else {
       --i;
     }
     counter_threads++;
   }

   return creation_pool;
}

status_e gecko_pool_enqueue_tasks(thread_task* tasks, thread_pool* pool, size_t num_threads) {
  for(size_t i= 0; i < num_threads; i++) {
    //Assuming queue will conatin something like this, this will not fail
    priority_queue_push(pool->waiting_tasks, &tasks[i], 1);  //TODO: set prio
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
  while(__check_for_group_queue(pool->waiting_tasks,0, id) == status_failed){ //TODO: set size
    //adjust this to not constantly check array for task_id contains instead check only when changes occur
  }
  return status_ok;
}

//
//  INTERNAL METHODS
//

void *__thread_main(void* args) {
  //pthread_attr_t* attr;
  __thread_information thread_info;
  thread_info.is_active = 0;
  thread_pool* pool = ((thread_pool*)args);

  while(1){
		if(thread_info.is_active == 1) {
            (*thread_info.routine)(thread_info.args);
			thread_info.is_active = 0;
		}
		else {
			__enqueued_task* next_task = __get_next_task(pool);
			thread_info.routine = next_task->thread->routine;
      thread_info.args = next_task->thread->args;
      //attr = next_task->thread->attr;
      thread_info.is_active = 1;
		}
	}
}

status_e __check_for_group_queue(priority_queue_t* waiting_tasks, size_t size, size_t task_id) {
  for(size_t i=0; i < size; i++) {
    if (((__enqueued_task*)waiting_tasks[i].data->element)->id == task_id) { //TODO: id?
      //returns failed in case queue still contains specified task id
      return status_failed;
    }
  }
  //returns ok in case of no further waiting
  return status_ok;
}

//status_e __remove_from_queue(priority_queue_t* waiting_tasks, size_t task_id) {
  //OK here i need some more inview of the working of the queue and what methods will in the end exist
//  return status_ok;
//}

__enqueued_task* __get_next_task(thread_pool *pool) {
  if(!priority_queue_is_empty(pool->waiting_tasks))
    return (__enqueued_task*) priority_queue_pop(pool->waiting_tasks);
  else
    return NULL;
}