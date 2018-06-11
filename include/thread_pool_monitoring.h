#ifndef THREAD_POOL_MONITORING_H
#define THREAD_POOL_MONITORING_H

#include "thread_pool.h"

//
//	EXTERNAL METHODS
//

// Returns the average fraction of time the active threads have been working.
double thread_pool_get_time_working(thread_pool* pool);

// Fill all stats of the passed thread pool instance
thread_pool_stats thread_pool_get_stats(thread_pool* pool);

// Fill all stats of the thread matching the given id in the thread pool
thread_stats thread_pool_get_thread_stats(thread_pool* pool, size_t id);



#endif //THREAD_POOL_MONITORING_H
