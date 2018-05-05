#include "../include/thread_pool.h"
#include <stdlib.h>
#include <string.h>

//
//  EXTERNAL METHODS
//

thread_pool* thread_pool_create(size_t num_threads) {
   thread_pool* pool = malloc(sizeof(thread_pool));
   pool->size = num_threads;
   pool->waiting_tasks = calloc(1, sizeof(priority_queue_t));
   pool->thread_status = malloc(sizeof(thread_status_e) * num_threads);

   pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
   pool->pool = threads;

   //create empty threads which themselves catch their own tasks from the created queue
   for(size_t i = 0; i < num_threads; i++) {
     __thread_information* thread_info = malloc(sizeof(__thread_information));
     thread_info->pool = pool;
     thread_info->id = i;
     pthread_create(&threads[i],NULL , &__thread_main, thread_info);
     
     /*if ( ret == 0) {
       creation_pool->pool[i] = counter_threads;
     }
     else {
       --i;
     }
     counter_threads++; */
   }
   
   memset(pool->thread_status, thread_status_idle, sizeof(thread_status_e) * num_threads);
   return pool;
}

void thread_pool_free(thread_pool* pool) {
    // Update all status and wait until the threads finish
    for(size_t i=0; i < pool->size; ++i) {
      pool->thread_status[i] = thread_status_will_terminate;
    }

    for(size_t i=0; i < pool->size; ++i) {
      pthread_join(pool->pool[i], NULL);
    }

    free(pool->pool);
    free(pool->waiting_tasks);
    free(pool);
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
  __thread_information* thread_info = (__thread_information*)args;


  while(1){

    __enqueued_task* next_task = __get_next_task(thread_info->pool);
    if(next_task) {

      // Check if this thread has to terminate
      if(thread_status_will_terminate == __update_thread_status(thread_info->pool, thread_info->id, thread_status_working))
        break;

      // Execute task
      (*next_task->thread->routine)(next_task->thread->args);
    }

    // Check if this thread has to terminate
    if(thread_status_will_terminate == __update_thread_status(thread_info->pool, thread_info->id, thread_status_idle))
      break;

    // TODO: idle

	}

  __update_thread_status(thread_info->pool, thread_info->id, thread_status_finished);

  // Be sure to free the passed thread_information since no other reference exists
  free(thread_info);

  return (void*)0;
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


thread_status_e __update_thread_status(thread_pool* pool, size_t thread_id, thread_status_e thread_status) {
  // Check if free is called on the thread pool and tell the thread to finish
  if(pool->thread_status[thread_id] == thread_status_will_terminate)
    return thread_status_will_terminate;

  // Otherwise update thread status in pool
  pool->thread_status[thread_id] = thread_status;
  return thread_status_empty;
}