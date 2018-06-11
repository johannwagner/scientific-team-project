#include <gtest/gtest.h>
#include <atomic>
#include <cmath>

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60932
using atomic_int = std::atomic<int>;
#define CPP_TEST 1

extern "C" 
{    
    #include "../../../include/thread_pool.h"
}

// Test the creatrion of thread pools
TEST(ThreadPool, Create) {

    thread_pool* pool = thread_pool_create(2, 0);
    EXPECT_TRUE(pool);

    thread_pool_free(pool);

}

// Test the creatrion of group Ids of thread pools
TEST(ThreadPool, Name) {
    thread_pool* pool = thread_pool_create_named(2, "ThreadPool", 0);
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

TEST(ThreadPool, WAIT){

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    thread_pool* pool = thread_pool_create(2, 0);
    thread_task tasks[6];

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
}


TEST(ThreadPool, WAIT_FOR_ALL){

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    thread_pool* pool = thread_pool_create(2, 0);
    thread_task tasks[6];

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

TEST(ThreadPool, TASK_STATISTICS){

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    thread_pool* pool = thread_pool_create(2, 1);
    thread_task tasks[6];

    for(int i = 0; i < 6; i++){
        tasks[i].args = (void*)&test[i];
        tasks[i].routine = work;
        thread_pool_enqueue_task(&tasks[i], pool, NULL);
    }

    ASSERT_NE(pool->statistics->task_complete_count, pool->statistics->task_enqueued_count);
    thread_pool_wait_for_all(pool);

    // All entries in the array have to be 0 to ensure the pool waits for all tasks
    for(int i = 0; i < 6; i++){
        //std::cout << "task_complete_time " <<  tasks[i].statistics->complete_time.tv_nsec << std::endl;
        ASSERT_GT(tasks[i].statistics->enqueue_time.tv_nsec, 0);
        ASSERT_GT(tasks[i].statistics->execution_time.tv_nsec, tasks[i].statistics->enqueue_time.tv_nsec);
        ASSERT_GT(tasks[i].statistics->complete_time.tv_nsec, tasks[i].statistics->execution_time.tv_nsec);
        
    }

    ASSERT_EQ(pool->statistics->task_complete_count, 6);
    ASSERT_EQ(pool->statistics->task_complete_count, pool->statistics->task_enqueued_count);

    thread_pool_free(pool);
}

