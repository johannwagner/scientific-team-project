#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "priority_queue.h"
#include "thread_pool.h"
#include "thread_pool_monitoring.h"

#define WORK_SIZE 128
#define NUM_TASKS 2048

void work_large(void* args)
{
	double* res = args;
	double exp = *res;

	double v = 0.0;
	for(double i = 0; i < WORK_SIZE; ++i)
	{
		v += pow(i, 1 / exp);
	}
	*res = v;
}

void work(void* args)
{
	int* res = args;
	int sum = 0;
	for(int i = 0; i <= *res; ++i)
		sum += i * i / 8;

	*res = sum;
//	printf("%d %d\n", n, sum);
}

void resize_test()
{
	thread_pool_t* pool = thread_pool_create(2, 0);
	task_handle_t hndl;
	const int numThreads = 1 << 11;
	thread_task_t    tasks[numThreads];
	int results[numThreads];
	for(int i = numThreads-1; i >= 0; --i){
		results[i] = i;
		tasks[i].args = &results[i];
		tasks[i].routine = work;
		tasks[i].priority = 0;
		if(i == 0) tasks[i].priority = 1;
		thread_pool_enqueue_task(&tasks[i], pool, &hndl);
		if( i == numThreads * 1 / 3) thread_pool_resize(pool, 4);
		if(i == numThreads * 2 / 3) thread_pool_resize(pool, 3);
		if(i == numThreads * 3 / 4) thread_pool_resize(pool, 2);
	}

//	thread_pool_wait_for_task(pool, &hndl);
	thread_pool_wait_for_all(pool);
	thread_pool_free(pool);

	// verify results
	int sum = 0;
	for(int i = 0; i < numThreads; ++i)
	{
		sum += i*i / 8;
		if(results[i] != sum) printf("error at %d: %d != %d\n", i, sum, results[i]);
	}
	printf("interleaving test completed.\n");

}

void performance_test(int numThreads, int numTasks)
{
	clock_t begin = clock();
	double exp = 2.7;
	for(int i = 0; i < 2048; ++i)
		work_large(&exp);
	clock_t end = clock();
	float time = (double)(end - begin) / CLOCKS_PER_SEC;
	time /= 2048;
	printf("time for one task: time: %f | %f\n", time, exp);

	thread_pool_t* pool = thread_pool_create(numThreads, 1);

	double results[numTasks];
	thread_task_t tasks[numTasks];

	for(int i = 0; i < numTasks; ++i)
	{
		results[i] = (double)(i+1);
		tasks[i].args = &results[i];
		tasks[i].routine = work_large;
		tasks[i].priority = 0;
		thread_pool_enqueue_task(&tasks[i], pool, NULL);
	}
	sleep(1);

	float a = thread_pool_get_time_working(pool);
	printf("fraction of time spend working: %f\n",a);

	thread_pool_free(pool);
}

float wait_performance_test(int numThreads, int numTasks, FILE *ps, FILE *pu)
{
	thread_pool_t* pool = thread_pool_create(numThreads, 1);

	double results[numTasks];
	thread_task_t tasks[numTasks];

	for(int i = 0; i < numTasks; ++i)
	{
		results[i] = (double)(i+1);
		tasks[i].args = &results[i];
		tasks[i].routine = work_large;
		tasks[i].priority = i;
		task_handle_t hndl;
		thread_pool_enqueue_task(&tasks[i], pool, &hndl);
		if(i % 11 == 0) thread_pool_wait_for_task(pool, &hndl);
	}
	thread_pool_wait_for_all(pool);

	float a = thread_pool_get_time_working(pool);
//	printf("fraction of time spend working: %f\n",a);
    thread_pool_stats stats = thread_pool_get_stats(pool);
    fprintf(pu, "%i %f \n",numThreads, a);
    fprintf(ps, "%i %lli %lli \n", numThreads, stats.avg_complete_time, stats.avg_wait_time);
	thread_pool_free(pool);
	return a;
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
	performance_test(2, 4000);
	resize_test();
	FILE *ps;
	FILE *pu;

	ps = fopen("Statistics/pool_avg.csv","w+");
    fprintf(ps, "Threads BusyTime IdleTime \n");
    pu = fopen("Statistics/pool_util.csv","w+");
    fprintf(pu, "Threads Util \n");
//	float sum = 0.f;
	for(int i = 2; i < 33; ++i)
	{
		wait_performance_test(i, 6000,ps, pu);
	}
//	sum /= 1000;
//	printf("average: %f\n", sum);
//	getchar();
	return 0;
}