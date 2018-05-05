#include <gtest/gtest.h>

extern "C" 
{
	#include "../../../include/thread_pool.h"
}

// Test the creatrion of thread pools
TEST(ThreadPool, Create) {

    thread_pool* pool = thread_pool_create(4);
    EXPECT_TRUE(pool);
}

// Test the creatrion of group Ids of thread pools
TEST(ThreadPool, CreateGroupId) {

    thread_pool* pool = thread_pool_create(16);
    size_t id = gecko_pool_create_group_id();
    EXPECT_GT(id, 0);
}