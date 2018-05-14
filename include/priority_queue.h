#ifndef GECKO_PRIORITY_QUEUE_H
#define GECKO_PRIORITY_QUEUE_H

#include <pthread.h>

typedef struct __element_info_t
{
    size_t priority;
    void* element;
} __element_info_t; 

typedef struct priority_queue_t
{
    __element_info_t* data;
    size_t num_elements;
    size_t capacity;
    pthread_mutex_t mutex;
} priority_queue_t;

void priority_queue_init(priority_queue_t* queue);
void priority_queue_free(priority_queue_t* queue);
void priority_queue_push(priority_queue_t* queue, void* data, size_t priority);
void* priority_queue_pop(priority_queue_t* queue);
int priority_queue_is_empty(priority_queue_t* queue);


#endif //GECKO_PRIORITY_QUEUE_H