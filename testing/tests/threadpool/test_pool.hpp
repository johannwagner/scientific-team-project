#include <gtest/gtest.h>

extern "C"
{
	#include "../../../include/thread_pool.h"
}

// Test the creatrion of thread pools
TEST(ThreadPool, Create) {

    thread_pool* pool = thread_pool_create(2);
    EXPECT_TRUE(pool);

    thread_pool_free(pool);

}

// Test the creatrion of group Ids of thread pools
TEST(ThreadPool, CreateGroupId) {

    thread_pool* pool = thread_pool_create(2);
    size_t id = gecko_pool_create_group_id();
    size_t id2 = gecko_pool_create_group_id();

    EXPECT_NE(id, id2);

    thread_pool_free(pool);
}

void basicTask (void* args) {
    int* number = (int*) args;
    std::cout << "number has value: " <<  *number << std::endl;
}

TEST(ThreadPool, BasicTasks){

    thread_pool* pool = thread_pool_create(2);
    size_t id = gecko_pool_create_group_id();
    size_t id2 = gecko_pool_create_group_id();

    thread_task* test_task = (thread_task*)malloc(sizeof(thread_task));

    int test = 5;

    test_task->args = (void*)&test;
    test_task->routine = &basicTask;

    for(int i = 0; i < 6; i++){
        std::cout << "enqueueing " << i << std::endl;
        gecko_pool_enqueue_task(test_task, pool, NULL);
        std::cout << "enqueued " << i << std::endl;
    }

    thread_pool_free(pool);
}
