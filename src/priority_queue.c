#include "priority_queue.h"
#include <stdlib.h>
#include <string.h>

void priority_queue_init(priority_queue_t* queue)
{
    queue->num_elements = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    queue->capacity = 0;
    queue->data = NULL;
}

void priority_queue_resize(priority_queue_t* queue, size_t size)
{
    __element_info_t* new_data = malloc(sizeof(__element_info_t) * size);
    
    memcpy(new_data, queue->data, sizeof(__element_info_t) * queue->num_elements);
    free(queue->data);

    queue->data = new_data;
}


inline static void swap(__element_info_t* el1, __element_info_t* el2)
{
    __element_info_t tmp = *el1;
    *el1 = *el2;
    *el2 = tmp; 
}

static void down_heap(__element_info_t* heap, size_t size)
{
    size_t cur = 0;
    size_t id = 1;
    for(size_t next = 3;; next = (next << 1)+1)
    {
        size_t min = 0xffffffff;
        size_t minId = 0;
        while(id != next && id < size)
        {
            if(heap[id].priority < min)
            {
                min = heap[id].priority;
                minId = id;
            }
            ++id;
        }
        if(heap[cur].priority <= heap[minId].priority) break;
        swap(&heap[cur], &heap[minId]);
        cur = minId;
        if(id == size) break;
    }
}

static void up_heap(__element_info_t* heap, size_t size)
{
    if(size == 1) return;
    size_t log = 31 -__builtin_clz(size);
    
    size_t begin = 1 << (log - 1);
    size_t cur = size-1;

    for(;;)
    {
        size_t end = (begin << 1) + 1;
        if(end > size) end = size;
        size_t max = heap[begin].priority;
        size_t id = begin;
        for(size_t i = begin+1; i < end; ++i)
        {
            if(heap[i].priority >= max)
            {
                max = heap[i].priority;
                id = i;
            }
        }
        if(heap[id].priority <= heap[cur].priority) break;
        else
        {
            swap(&heap[cur], &heap[id]);
            cur = id;
        }
        if(!begin) break;
        begin >>= 1;
    }
}

void priority_queue_push(priority_queue_t* queue, void* data, size_t priority)
{
    pthread_mutex_lock(&queue->mutex);

    if(queue->num_elements >= queue->capacity) priority_queue_resize(queue, ((queue->num_elements+1) * 3) / 2);

    queue->data[queue->num_elements].element = data;
    queue->data[queue->num_elements].priority = priority;
    ++queue->num_elements;
    up_heap(queue->data, queue->num_elements);

    pthread_mutex_unlock(&queue->mutex);
}

void* priority_queue_pop(priority_queue_t* queue)
{
    pthread_mutex_lock(&queue->mutex);

    --queue->num_elements;

    void* ptr = queue->data[0].element;
 //   queue->data[0] = queue->data[queue->num_elements];
    queue->data[0] = queue->data[queue->num_elements];
    down_heap(queue->data, queue->num_elements+1);

    pthread_mutex_unlock(&queue->mutex);
    return ptr;
}

int priority_queue_is_empty(priority_queue_t* queue)
{
    pthread_mutex_lock(&queue->mutex);

    int result = !queue->num_elements;

    pthread_mutex_unlock(&queue->mutex);
    return result;
}