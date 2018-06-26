#include <gtest/gtest.h>
#include <atomic>
#include <cmath>

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60932
using atomic_int = std::atomic<int>;
#define CPP_TEST 1

extern "C" 
{    
    #include "thread_pool.h"
    #include "../../../include/thread_pool_monitoring.h"
}

// Test the creatrion of thread pools
TEST(ThreadPool, Create) {

    thread_pool_t* pool = thread_pool_create(2, 0);
    EXPECT_TRUE(pool);

    thread_pool_free(pool);

}

// Test the creatrion of group Ids of thread pools
TEST(ThreadPool, Name) {
    thread_pool_t* pool = thread_pool_create_named(2, "ThreadPool", 0);
    EXPECT_TRUE(pool);
    ASSERT_STREQ(pool->name, "ThreadPool");

    thread_pool_free(pool);
}

void basicTask (void* args) {
    int* number = (int*) args;
    std::cout << "number has value: " <<  *number << std::endl;
}

void work(void* args)
{
	int* res = (int*)args;

    double exp = *res;

	double v = 0.0;
	for(double i = 0; i < 100024; ++i)
	{
		v += pow(i, 1 / exp);
	}

	for(; *res > 0; (*res)--)
    {
        
    }

    //std::cout << "Work finished" << std::endl;
}

TEST(ThreadPool, Resize) {

    thread_pool_t* pool = thread_pool_create(4, 0);
    EXPECT_TRUE(pool);

    status_e status = thread_pool_resize(pool, 6);

    EXPECT_EQ(status, status_ok);
    EXPECT_EQ(pool->size, 6);
    //EXPECT_EQ(sizeof(pool->pool), 6 * sizeof(pthread_t));

    thread_pool_free(pool);

}

TEST(ThreadPool, WAIT){

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    thread_pool_t* pool = thread_pool_create(2, 0);
    thread_task_t tasks[6];

    for(int i = 0; i < 6; i++){
        tasks[i].args = (void*)&test[i];
        tasks[i].routine = work;
    }

    thread_pool_enqueue_tasks_wait(tasks, pool, 6);

    // All entries in the array have to be 0 to ensure the pool waits for all tasks
    for(int i = 0; i < 6; i++){
        ASSERT_EQ(test[i], 0);
    }

    thread_pool_free(pool);
    int i = 5;
}


TEST(ThreadPool, WAIT_FOR_ALL){

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    thread_pool_t* pool = thread_pool_create(2, 0);
    thread_task_t tasks[6];

    for(int i = 0; i < 6; i++){
        tasks[i].args = (void*)&test[i];
        tasks[i].routine = work;
        thread_pool_enqueue_task(&tasks[i], pool, NULL);
    }

    thread_pool_wait_for_all(pool);

    // All entries in the array have to be 0 to ensure the pool waits for all tasks
    for(int i = 0; i < 6; i++){
        ASSERT_EQ(test[i], 0);
    }

    thread_pool_free(pool);
}

void LIVE_RESIZE_work(void* args)
{
	int* res = static_cast<int*>(args);
	int sum = 0;
	for(int i = 0; i <= *res; ++i)
		sum += i * i / 8;

	*res = sum;
}

TEST(ThreadPool, LIVE_RESIZE){
	thread_pool_t* pool = thread_pool_create(2, 0);
	task_handle_t hndl;
	const int numThreads = 1 << 11;
	thread_task_t tasks[numThreads];
	int results[numThreads];
	for(int i = numThreads-1; i >= 0; --i){
		results[i] = i;
		tasks[i].args = &results[i];
		tasks[i].routine = LIVE_RESIZE_work;
		tasks[i].priority = 0;
		if(i == 0) tasks[i].priority = 1;
		thread_pool_enqueue_task(&tasks[i], pool, &hndl);
		if( i == numThreads * 1 / 3) {
			ASSERT_EQ(pool->size, 3);
			thread_pool_resize(pool, 4);
			ASSERT_EQ(pool->size, 4);
		}
		if(i == numThreads * 2 / 3) {
			ASSERT_EQ(pool->size, 4);
			thread_pool_resize(pool, 3);
			ASSERT_EQ(pool->size, 3);
		}
		if(i == numThreads * 3 / 4) {
			ASSERT_EQ(pool->size, 2);
			thread_pool_resize(pool,4);
			ASSERT_EQ(pool->size, 4);
		}
	}
	ASSERT_EQ(pool->size, 4);

	thread_pool_wait_for_all(pool);
	thread_pool_free(pool);

	// verify results
	int sum = 0;
	for(int i = 0; i < numThreads; ++i)
	{
		sum += i*i / 8;
		ASSERT_EQ(results[i], sum);
	}
}

TEST(ThreadPool, TASK_STATISTICS){

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    thread_pool_t* pool = thread_pool_create(2, 1);
    thread_task_t tasks[6];

    for(int i = 0; i < 6; i++){
        tasks[i].args = (void*)&test[i];
        tasks[i].routine = work;
        thread_pool_enqueue_task(&tasks[i], pool, NULL);
    }

    thread_pool_wait_for_all(pool);

    // All entries in the array have to be 0 to ensure the pool waits for all tasks
    for(int i = 0; i < 6; i++){
        //std::cout << "task_complete_time " <<  tasks[i].statistics->complete_time.tv_nsec << std::endl;
        //ASSERT_GT(tasks[i].statistics.enqueue_time.tv_nsec, 0);
        //ASSERT_GT(__get_time_diff(&tasks[i].statistics.enqueue_time, &tasks[i].statistics.execution_time), 0);
        //ASSERT_GT(__get_time_diff(&tasks[i].statistics.execution_time, &tasks[i].statistics.complete_time), 0);
    }

    thread_pool_free(pool);
}

TEST(ThreadPool, POOL_STATISTICS){

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    thread_pool_t* pool = thread_pool_create(2, 1);
    thread_task_t tasks[6];
    thread_pool_stats pool_stats;

    for(int i = 0; i < 6; i++){
        tasks[i].args = (void*)&test[i];
        tasks[i].routine = work;
        thread_pool_enqueue_task(&tasks[i], pool, NULL);
    }

    pool_stats = thread_pool_get_stats(pool);
    ASSERT_EQ(pool_stats.task_enqueued_count, 6);
    ASSERT_NE(pool_stats.task_complete_count, 6);

    thread_pool_wait_for_all(pool);

    pool_stats = thread_pool_get_stats(pool);
    ASSERT_EQ(pool_stats.task_complete_count, pool_stats.task_enqueued_count);
    std::cout << "Avg wait time (milis) " << pool_stats.avg_wait_time / 1000000.0 <<  " Avg complete time (milis) " << pool_stats.avg_complete_time / 1000000.0 << std::endl;

    thread_pool_free(pool);
}

TEST(ThreadPool, THREAD_STATISTICS){

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    thread_pool_t* pool = thread_pool_create(2, 1);
    thread_task_t tasks[6];
    thread_stats thread_stats;

    for(int i = 0; i < 6; i++){
        tasks[i].args = (void*)&test[i];
        tasks[i].routine = work;
    }

    thread_pool_enqueue_tasks_wait(tasks, pool, 6);

    thread_stats = thread_pool_get_thread_stats(pool, 0);
    //ASSERT_GT(thread_stats.task_count, 0);
    std::cout << "busy time for first thread (milis) " << thread_stats.busy_time / 1000000.0 << std::endl;
    std::cout << "idle time for first thread (milis) " << thread_stats.idle_time / 1000000.0 << std::endl;

    thread_pool_free(pool);
}
