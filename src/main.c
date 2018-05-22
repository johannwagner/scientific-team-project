#include <stdio.h>
#include <stdlib.h>
#include "priority_queue.h"
#include "thread_pool.h"

void work(void* args)
{
	int* res = args;
	int sum = 0;
	for(int i = 0; i <= *res; ++i)
		sum += i * i / 8;

	*res = sum;
//	printf("%d %d\n", n, sum);
}

int main()
{
/*	priority_queue_t queue;
	priority_queue_init(&queue);

	for(size_t i = 0; i < 256; ++i)
	{
		priority_queue_push(&queue, (void*)(255-i), 255-i);	
	}

	while(!priority_queue_is_empty(&queue))
	{
		size_t i = (size_t)priority_queue_pop(&queue);
		printf("%I64d\n",i);
	}*/

	thread_pool* pool = thread_pool_create(2);
	task_handle hndl;
	const int numThreads = 1 << 11;
	thread_task tasks[numThreads];
	int results[numThreads];
	for(int i = numThreads-1; i >= 0; --i){
		results[i] = i;
		tasks[i].args = &results[i];
		tasks[i].routine = work;
		tasks[i].priority = 0;
		if(i == 0) tasks[i].priority = 1;
		gecko_pool_enqueue_task(&tasks[i], pool, &hndl);
		if( i == numThreads * 1 / 3) thread_pool_resize(pool, 4);
		if(i == numThreads * 2 / 3) thread_pool_resize(pool, 3);
		if(i == numThreads * 3 / 4) thread_pool_resize(pool,2);
	}
	float a = thread_pool_get_time_working(pool);
	thread_pool_wait_for_task(pool, &hndl);
	float b = thread_pool_get_time_working(pool);
	thread_pool_free(pool);
	printf("fraction of time working: %f, %f\n", a, b);

	// verify results
	int sum = 0;
	for(int i = 0; i < numThreads; ++i)
	{
		sum += i*i / 8;
		if(results[i] != sum) printf("error at %d: %d != %d\n", i, sum, results[i]);
	}
	printf("tests completed.");

	getchar();
	return 0;
}