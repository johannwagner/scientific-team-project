#include "thread_pool_monitoring.h"
#include <stdio.h>

//
// EXTERNAL METHODS
// 

thread_pool_stats thread_pool_get_stats(thread_pool* pool) {

  // In case no tasks care completed, no averages can be calculated
  if(pool->statistics->task_complete_count) {
    pool->statistics->avg_complete_time = pool->statistics->complete_time / pool->statistics->task_complete_count;
    pool->statistics->avg_wait_time = pool->statistics->wait_time / pool->statistics->task_complete_count;
  }
  return *pool->statistics;
}

thread_stats thread_pool_get_thread_stats(thread_pool* pool, size_t id) {
  thread_stats* thread_stats = pool->thread_infos[id]->statistics;
  
  // busy_time = running_time - idle_time
  struct timespec current;
  clock_gettime(CLOCK_MONOTONIC, &current);
  thread_stats->busy_time = __get_time_diff(&thread_stats->creation_time, &current) - thread_stats->idle_time;
  return *thread_stats;
}

//
// HELP METHODS
//

double thread_pool_get_time_working(thread_pool* pool){
  struct timespec end;
  
  clock_gettime(CLOCK_MONOTONIC, &end);
  double avg = 0.f;
  for(size_t i = 0; i < pool->size; ++i){
    struct timespec begin = pool->thread_infos[i]->statistics->creation_time;
    double t = __get_time_diff(&begin, &end);
    avg += t / (t + pool->thread_infos[i]->statistics->idle_time);
  }
  return avg / pool->size;
}