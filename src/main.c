#include <stdio.h>
#include "priority_queue.h"

int main()
{
	priority_queue_t queue;
	priority_queue_init(&queue);

	for(size_t i = 0; i < 10; ++i)
		priority_queue_push(&queue, (void*)i, i);

	while(!priority_queue_is_empty(&queue))
	{
		size_t i = (size_t)priority_queue_pop(&queue);
		printf("%I64d\n",i);
	}

	getchar();
}