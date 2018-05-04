#include <gtest/gtest.h>

extern "C" 
{
	#include "../../../include/priority_queue.h"
}

TEST(PriorityQueue, IsEmpty) {

    priority_queue_t queue;
	priority_queue_init(&queue);

	for(size_t i = 0; i < 10; ++i)
		priority_queue_push(&queue, (void*)i, i);

	EXPECT_EQ(!priority_queue_is_empty(&queue), 1);
}
