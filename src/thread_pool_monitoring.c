#include "thread_pool_monitoring.h"

//
// EXTERNAL METHODS
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