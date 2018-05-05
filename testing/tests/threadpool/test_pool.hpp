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