#include "thread_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static inline void __execute_task();

//
//  EXTERNAL METHODS
//

thread_pool* thread_pool_create(size_t num_threads, int enable_monitoring) {
  thread_pool* pool = malloc(sizeof(thread_pool));
  pool->name = NULL;
  pool->size = num_threads;
  pool->capacity = num_threads * 2;
  pool->waiting_tasks = calloc(1, sizeof(priority_queue_t));
//   pool->thread_status = malloc(sizeof(thread_status_e) * num_threads);
  pool->thread_tasks = calloc(num_threads, sizeof(thread_task*));
  pool->thread_infos = malloc(sizeof(__thread_information*) * pool->capacity);

  pool->task_state_capacity = 4096;
  pool->task_group_states = calloc(pool->task_state_capacity, sizeof(__task_state));

   pthread_t* threads = malloc(sizeof(pthread_t) * pool->capacity);
   pool->pool = threads;

   if(enable_monitoring) {
     pool->statistics = malloc(sizeof(thread_pool_stats));
   }

   for(size_t i = 0; i < pool->capacity; i++) {
    // one block per thread to reduce risk of two threads sharing the same cache line
    __thread_information* thread_info = malloc(sizeof(__thread_information));
    pool->thread_infos[i] = thread_info;
    thread_info->pool = pool;
    thread_info->id = i;
    thread_info->status = thread_status_empty;
    sprintf(thread_info->name, "worker-%zu", i); // "worker%I64d" lead to segfault on linux

    if(enable_monitoring) {
     thread_info->statistics = malloc(sizeof(thread_stats));
   }
    
  }
  for(size_t i = 0; i < num_threads; ++i)
    __create_thread(pool->thread_infos[i], &pool->pool[i]);

   return pool;
}

thread_pool* thread_pool_create_named(size_t num_threads, const char* name, int enable_monitoring) {
  thread_pool* pool = thread_pool_create(num_threads, enable_monitoring);

  if(name) {
    thread_pool_set_name(pool, name);
  }

  return pool;
}

void thread_pool_free(thread_pool* pool) {
    // Update all status
    for(size_t i=0; i < pool->size; ++i) {
      pool->thread_infos[i]->status = thread_status_will_terminate;
    }
    // wait for threads to finish
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
    free(pool->task_group_states);
    if(pool->name) free(pool->name);
    free(pool);
}

void thread_pool_set_name(thread_pool* pool, const char* name){
  if(pool->name) free(pool->name);
  size_t s = strlen(name);
  char* str = malloc(s+1);
  strcpy(str, name);
  pool->name = str;
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
      // mark threads for termination
      pool->thread_infos[i]->status = thread_status_will_terminate;
    }
  }
	return status_ok;
}

status_e thread_pool_enqueue_tasks(thread_task* tasks, thread_pool* pool, size_t num_tasks, task_handle* hndl) {
  
  // find unused slot
  size_t ind = 0;
  for(; ind < pool->task_state_capacity && pool->task_group_states[ind].task_count; ++ind);
  if(ind == pool->task_state_capacity) return status_failed;

  // increment generation first to always be identifiable as finished
  ++pool->task_group_states[ind].generation;
  pool->task_group_states[ind].task_count = num_tasks;
  
  for(size_t i= 0; i < num_tasks; i++) {
    tasks[i].group_id = ind;
    priority_queue_push(pool->waiting_tasks, &tasks[i], tasks[i].priority); 
  }

  if(hndl){
    hndl->index = ind;
    hndl->generation = pool->task_group_states[ind].generation;
  }

  return status_ok;
}

status_e thread_pool_enqueue_task(thread_task* task, thread_pool* pool, task_handle* hndl) {
  return thread_pool_enqueue_tasks(task, pool, 1, hndl);
}

status_e thread_pool_enqueue_tasks_wait(thread_task* tasks, thread_pool* pool, size_t num_tasks) {
  // Pass all tasks except the last one to the queue
  task_handle hndl;
  thread_pool_enqueue_tasks(tasks, pool, num_tasks - 1, &hndl);

  // Execute the last tasks in the calling thread
  thread_task* main_task = &tasks[num_tasks - 1];
  (*main_task->routine)(main_task->args);

  return thread_pool_wait_for_task(pool, &hndl);
}

status_e thread_pool_wait_for_task(thread_pool* pool, task_handle* hndl) {
  volatile unsigned* gen = &pool->task_group_states[hndl->index].generation;
  while(*gen == hndl->generation 
     && pool->task_group_states[hndl->index].task_count) {}
  return status_ok;
}

status_e thread_pool_wait_for_all(thread_pool* pool){
  thread_task* next_task;
  while((next_task = __get_next_task(pool))){
    __execute_task(pool, next_task);
  }
  for(;;){
    size_t sum = 0;
    for(size_t i = 0; i < pool->task_state_capacity; ++i)
      sum += pool->task_group_states[i].task_count;
    if(!sum) return status_ok;
  }
  return status_failed;
}

//
//  INTERNAL METHODS
//

void *__thread_main(void* args) {
  __thread_information* thread_info = (__thread_information*)args;

  // Fill statistics if available
  struct timespec begin;
  if(thread_info->statistics) {
    clock_gettime(CLOCK_MONOTONIC, &thread_info->statistics->creation_time);
    begin = thread_info->statistics->creation_time;
  }
  else 
    clock_gettime(CLOCK_MONOTONIC, &begin);

  while(1){
    thread_task* next_task = __get_next_task(thread_info->pool);
    // the task has to be executed since it has been taken out of the queue
    if(next_task) {

      // Fill statistics if available
      if(thread_info->statistics)
      {
        // measure time outside to prevent incorrect times while in execution
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        thread_info->statistics->idle_time += __get_time_diff(&begin, &end);
        __execute_task(thread_info->pool, next_task);
        clock_gettime(CLOCK_MONOTONIC, &begin);
      }
      else 
        __execute_task(thread_info->pool, next_task);
      
    }

    // Check if this thread has to terminate, set the status and leave the loop
    int will_terminate = thread_status_will_terminate;
    if(atomic_compare_exchange_strong(&thread_info->status, &will_terminate, thread_status_killed)){
      break;
    }


//    if(!next_task)
//      nanosleep(&waiting_time_start, &waiting_time_end);
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

thread_task* __get_next_task(thread_pool *pool) {
  thread_task* next_task = priority_queue_pop(pool->waiting_tasks);
  return next_task;
}

status_e __create_thread(__thread_information* thread_info, pthread_t* pp){
  thread_info->status = thread_status_created;
  thread_info->time_spend_idle = 0.f;
  pthread_create(pp,NULL , &__thread_main, thread_info);

  return status_ok;
}


void __execute_task(thread_pool* pool, thread_task* task)
{
  (*task->routine)(task->args);
    --pool->task_group_states[task->group_id].task_count;
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