#include "thread_pool.h"
#include <stdlib.h>
#include <string.h>

// generate unique ids
static size_t counter_tasks = 0;

//
//  EXTERNAL METHODS
//

thread_pool* thread_pool_create(size_t num_threads) {
   thread_pool* pool = malloc(sizeof(thread_pool));
   pool->size = num_threads;
   pool->capacity = num_threads * 2;
   pool->waiting_tasks = calloc(1, sizeof(priority_queue_t));
//   pool->thread_status = malloc(sizeof(thread_status_e) * num_threads);
   pool->thread_tasks = calloc(num_threads, sizeof(thread_task*));
   pool->thread_infos = malloc(sizeof(__thread_information*) * pool->capacity);

   pthread_t* threads = malloc(sizeof(pthread_t) * pool->capacity);
   pool->pool = threads;

   for(size_t i = 0; i < pool->capacity; i++) {
    // one block per thread to reduce risk of two threads sharing the same cache line
    __thread_information* thread_info = malloc(sizeof(__thread_information));
    pool->thread_infos[i] = thread_info;
    thread_info->pool = pool;
    thread_info->id = i;
    thread_info->status = thread_status_empty;
  }
  for(size_t i = 0; i < num_threads; ++i)
    __create_thread(pool->thread_infos[i], &pool->pool[i]);

   return pool;
}

void thread_pool_free(thread_pool* pool) {
    // Update all status and wait until the threads finish
    for(size_t i=0; i < pool->size; ++i) {
      pool->thread_infos[i]->status = thread_status_will_terminate;
    }

    for(size_t i=0; i < pool->size; ++i) {
      pthread_join(pool->pool[i], NULL);
      free(pool->thread_infos[i]);
    }
    for(size_t i = pool->size; i < pool->capacity; ++i)
      free(pool->thread_infos[i]);

    priority_queue_free(pool->waiting_tasks);
    free(pool->pool);
    free(pool->waiting_tasks);
    free(pool->thread_tasks);
    free(pool->thread_infos);
    free(pool);
}

status_e thread_pool_resize(thread_pool* pool, size_t num_threads)
{
	if(num_threads > pool->size)
	{
    if(pool->size + num_threads > pool->capacity){
      return status_failed;
    }

    for(size_t i = pool->size; i < num_threads; ++i){
      //try to revive thread
      int will_terminate = thread_status_will_terminate;
      if(atomic_compare_exchange_strong(&pool->thread_infos[i]->status, &will_terminate, thread_status_idle));
      else{ // create a new
        __create_thread(pool->thread_infos[i], &pool->pool[i]);
      }
    }
	}
  else if(num_threads < pool->size){
    for(size_t i= num_threads; i < pool->size; ++i) {
      pool->thread_infos[i]->status = thread_status_will_terminate;
    }
  }
	return status_ok;
}

status_e gecko_pool_enqueue_tasks(thread_task* tasks, thread_pool* pool, size_t num_tasks) {
  size_t group_id = gecko_pool_create_group_id();
  
  for(size_t i= 0; i < num_tasks; i++) {
    //Assuming queue will conatin something like this, this will not fail
    tasks[i].group_id = group_id;
    priority_queue_push(pool->waiting_tasks, &tasks[i], tasks[i].priority); 
  }

//  gecko_pool_wait_for_id(group_id, pool);

  return status_ok;
}

status_e gecko_pool_enqueue_task(thread_task* task, thread_pool* pool) {
  return gecko_pool_enqueue_tasks(task, pool, 1);
}

size_t gecko_pool_create_group_id() {
  return counter_tasks++;
}

status_e gecko_pool_wait_for_id(size_t id, thread_pool* pool) {
  while(__check_for_group_queue(pool->waiting_tasks,pool->waiting_tasks->num_elements, id) == status_failed){ //TODO: set size
    //adjust this to not constantly check array for task_id contains instead check only when changes occur
  }

  while(__check_for_thread_tasks(pool,id) == status_failed){ //TODO: set size
    //adjust this to not constantly check array for task_id contains instead check only when changes occur
  }

  return status_ok;
}

//
//  INTERNAL METHODS
//

void *__thread_main(void* args) {
  __thread_information* thread_info = (__thread_information*)args;

  //structs for short waiting interval
  struct timespec waiting_time_start = {0 , 0};
  struct timespec waiting_time_end = {0 , 100};

  while(1){

    thread_task* next_task = __get_next_task(thread_info->pool, thread_info->id);
    if(next_task) {
      // the task has to be executed since it has been taken out of the queue
  //    __update_thread_status(thread_info->pool, thread_info->id, thread_status_working);
      // Execute task
      (*next_task->routine)(next_task->args);
    }

    // Check if this thread has to terminate, set the status and leave the loop
    int will_terminate = thread_status_will_terminate;
    if(atomic_compare_exchange_strong(&thread_info->status, &will_terminate, thread_status_killed)){
      break;
    }


    if(!next_task)
      nanosleep(&waiting_time_start, &waiting_time_end);
	}

  thread_info->status = thread_status_finished;

  // Be sure to free the passed thread_information since no other reference exists
 // free(thread_info);

  return (void*)0;
}

status_e __check_for_group_queue(priority_queue_t* waiting_tasks, size_t size, size_t task_id) {
  for(size_t i=0; i < size; i++) {
    __element_info_t* element_info = &waiting_tasks->data[i];
    if (((thread_task*)element_info->element)->group_id == task_id) { 
      //returns failed in case queue still contains specified task id
      return status_failed;
    }
  }

  //returns ok in case of no further waiting
  return status_ok;
}

status_e __check_for_thread_tasks(thread_pool* pool, size_t id) {
   for(size_t i=0; i < pool->size; i++) {
    thread_task* task = pool->thread_tasks[i];

      if(task && task->group_id == id)
        return status_failed;
   }
    
  //returns ok in case of no further waiting
  return status_ok;
}

//status_e __remove_from_queue(priority_queue_t* waiting_tasks, size_t task_id) {
  //OK here i need some more inview of the working of the queue and what methods will in the end exist
//  return status_ok;
//}

thread_task* __get_next_task(thread_pool *pool, size_t thread_id) {
  thread_task* next_task = priority_queue_pop(pool->waiting_tasks);
  if(next_task)
    pool->thread_tasks[thread_id] = next_task;
  
  return next_task;
}

status_e __create_thread(__thread_information* thread_info, pthread_t* pp){
  thread_info->status = thread_status_created;
  pthread_create(pp,NULL , &__thread_main, thread_info);

  return status_ok;
}


thread_status_e __update_thread_status(thread_pool* pool, size_t thread_id, thread_status_e thread_status) {
  // Check if free is called on the thread pool and tell the thread to finish
  if(pool->thread_infos[thread_id]->status == thread_status_will_terminate)
    return thread_status_will_terminate;

  if(thread_status == thread_status_completed)
    pool->thread_tasks[thread_id] = NULL;

  // Otherwise update thread status in pool
  pool->thread_infos[thread_id]->status = thread_status;
  return thread_status_empty;
}