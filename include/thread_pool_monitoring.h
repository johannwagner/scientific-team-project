#ifndef THREAD_POOL_MONITORING_H
#define THREAD_POOL_MONITORING_H

#include "thread_pool.h"

//
//	EXTERNAL METHODS
//

// Returns the average fraction of time the active threads have been working.
double thread_pool_get_time_working(thread_pool* pool);

#endif //THREAD_POOL_MONITORING_H
