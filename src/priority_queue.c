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



void priority_queue_push(priority_queue_t* queue, void* data, size_t priority)
{
    pthread_mutex_lock(&queue->mutex);

    if(queue->num_elements >= queue->capacity) priority_queue_resize(queue, ((queue->num_elements+1) * 3) / 2);

    queue->data[queue->num_elements].element = data;
    queue->data[queue->num_elements].priority = priority;
    ++queue->num_elements;

    pthread_mutex_unlock(&queue->mutex);
}

void* priority_queue_pop(priority_queue_t* queue)
{
    pthread_mutex_lock(&queue->mutex);

    --queue->num_elements;

    void* ptr = queue->data[queue->num_elements].element;
 //   queue->data[0] = queue->data[queue->num_elements];

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