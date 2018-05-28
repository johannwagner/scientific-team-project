#include <gtest/gtest.h>

extern "C" 
{
	#include "../../../include/priority_queue.h"
}

// Test if push and isEmpty works correkt
TEST(PriorityQueue, IsEmpty) {

    priority_queue_t queue;
	priority_queue_init(&queue);

	EXPECT_EQ(priority_queue_is_empty(&queue), 1);

	for(size_t i = 0; i < 10; ++i)
		priority_queue_push(&queue, (void*)i, i);

	EXPECT_EQ(!priority_queue_is_empty(&queue), 1);
}

// Test if push with Prio works correktly
TEST(PriorityQueue, PushPrio) {

    priority_queue_t queue;
	priority_queue_init(&queue);

	for(size_t i = 1; i < 5; ++i)
		priority_queue_push(&queue, (void*)i, i);
	
	int test = 99;
	priority_queue_push(&queue,(void*)&test, 0);

	EXPECT_EQ(*(int*)priority_queue_pop(&queue), test);
}