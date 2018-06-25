#include "thread_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "benchmark_switches.h"

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

  pool->thread_tasks = calloc(num_threads, sizeof(thread_task*));
  pool->thread_infos = calloc(sizeof(__thread_information*) * pool->capacity, 1);
  pool->task_state_capacity = MAX_NUM_TASKS;
  pool->task_group_states = calloc(pool->task_state_capacity, sizeof(__task_state));
  pool->enable_monitoring = enable_monitoring;

  pthread_t* threads = malloc(sizeof(pthread_t) * pool->capacity);
  pool->pool = threads;

  if(enable_monitoring) {
    pool->statistics = calloc(1, sizeof(thread_pool_stats));
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
      thread_info->statistics = calloc(1, sizeof(thread_stats));
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
      if(pool->enable_monitoring) 
        free(pool->thread_infos[i]->statistics);
      
      free(pool->thread_infos[i]);
    }
    for(size_t i = pool->size; i < pool->capacity; ++i) {
       if(pool->enable_monitoring) 
        free(pool->thread_infos[i]->statistics);
        
      free(pool->thread_infos[i]);
    }
      

    priority_queue_free(pool->waiting_tasks);
    free(pool->pool);
    free(pool->waiting_tasks);
    free(pool->thread_tasks);
    free(pool->thread_infos);
    free(pool->task_group_states);
    if(pool->name) free(pool->name);
    if(pool->enable_monitoring) free(pool->statistics);
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
    if(num_threads > pool->capacity){
      return status_failed;
    }

    for(size_t i = pool->size; i < num_threads; ++i){
      //try to revive thread
      int will_terminate = thread_status_will_terminate;
      if(atomic_compare_exchange_strong(&pool->thread_infos[i]->status, &will_terminate, thread_status_idle));
      else{ 
        // create a new
        if(pool->enable_monitoring && !pool->thread_infos[i]->statistics) {
          pool->thread_infos[i]->statistics = calloc(1, sizeof(thread_stats));
        }
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

  pool->size = num_threads;

	return status_ok;
}

status_e thread_pool_enqueue_tasks(thread_task* tasks, thread_pool* pool, size_t num_tasks, task_handle* hndl) {
  
  // find unused slot
  size_t ind = 0;
#ifdef UNIFORM_SLOT_DISTRIBUTION
  for(; pool->task_group_states[ind].task_count;ind = (ind + 8) % MAX_NUM_TASKS);
#else
  for(; ind < pool->task_state_capacity && pool->task_group_states[ind].task_count; ++ind);
  if(ind == pool->task_state_capacity) return status_failed;
#endif
  // increment generation first to always be identifiable as finished
  ++pool->task_group_states[ind].generation;
  pool->task_group_states[ind].task_count = num_tasks;
  
  for(size_t i= 0; i < num_tasks; i++) {
    
    if(pool->enable_monitoring){
      //tasks[i].statistics = calloc(1, sizeof(task_stats));
      clock_gettime(CLOCK_MONOTONIC, &tasks[i].statistics.enqueue_time);
      pool->statistics->task_enqueued_count++;
    }
    
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
  
  if(pool->enable_monitoring){
    pool->statistics->task_enqueued_count++;
    //main_task->statistics = calloc(1, sizeof(task_stats));
    
    // No waiting if the calling thread executes the task
    clock_gettime(CLOCK_MONOTONIC, &main_task->statistics.enqueue_time);
    main_task->statistics.execution_time = main_task->statistics.enqueue_time;
    
    (*main_task->routine)(main_task->args);
    clock_gettime(CLOCK_MONOTONIC, &main_task->statistics.complete_time);
    pool->statistics->task_complete_count++;
  }
  else
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

    if(pool->enable_monitoring){
      clock_gettime(CLOCK_MONOTONIC, &next_task->statistics.execution_time);
      __execute_task(pool, next_task);
      clock_gettime(CLOCK_MONOTONIC, &next_task->statistics.complete_time);
      pool->statistics->task_complete_count++;

      // Just add the time, calculate the average at evaluation time
      pool->statistics->wait_time += __get_time_diff(&next_task->statistics.enqueue_time, &next_task->statistics.execution_time);
      pool->statistics->complete_time += __get_time_diff(&next_task->statistics.execution_time, &next_task->statistics.complete_time);
    }
    else 
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
  if(thread_info->pool->enable_monitoring) {
    clock_gettime(CLOCK_MONOTONIC, &thread_info->statistics->creation_time);
    begin = thread_info->statistics->creation_time;
  }

  while(1){
    thread_task* next_task = __get_next_task(thread_info->pool);
    // the task has to be executed since it has been taken out of the queue
    if(next_task) {

      // Fill statistics if available
      if(thread_info->pool->enable_monitoring)
      {
        // measure time outside to prevent incorrect times while in execution
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        next_task->statistics.execution_time = end;
        thread_info->statistics->idle_time += __get_time_diff(&begin, &end);
        
        __execute_task(thread_info->pool, next_task);
        clock_gettime(CLOCK_MONOTONIC, &begin);
        next_task->statistics.complete_time = begin;
        thread_info->statistics->task_count++;
        thread_info->pool->statistics->task_complete_count++;
        
        // Just add the time, calculate the average at evaluation time
        thread_info->pool->statistics->wait_time += __get_time_diff(&next_task->statistics.enqueue_time, &next_task->statistics.execution_time);
        thread_info->pool->statistics->complete_time += __get_time_diff(&next_task->statistics.execution_time, &next_task->statistics.complete_time);
        
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

thread_task* __get_next_task(thread_pool *pool) {
  thread_task* next_task = priority_queue_pop(pool->waiting_tasks);
  return next_task;
}

status_e __create_thread(__thread_information* thread_info, pthread_t* pp){
  thread_info->status = thread_status_created;
  pthread_create(pp,NULL , &__thread_main, thread_info);

  return status_ok;
}

void __execute_task(thread_pool* pool, thread_task* task)
{
  (*task->routine)(task->args);
    --pool->task_group_states[task->group_id].task_count;
}

