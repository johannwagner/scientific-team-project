#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "status.h"

struct thread_pool {
	pthread_t* pool;
	size_t size;
};

struct __enqueued_task {
	thread_task* thread;
	size_t priority;
	pthread_t id;
	size_t is_activated;
};

thread_pool* thread_pool_create(size_t num_threads) {
	struct thread_pool creation_pool;
	creation_pool->size = num_threads;
	//here for some thread creation
	
}	

void __thread_main(void* args, pthread_attr_t* attr, void *(*routine), size_t* is_activated) {
	while(true){
		if(*is_activated) {
			routine(args, attr);
		}
		else {
			//noop for now should change this in the future to
			//more efficient sleep or something
		}
	}
}

#endif //THREAD_POOL_H

