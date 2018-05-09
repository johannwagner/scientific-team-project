# gecko_pool: Thread Pool in geckoDB/Bolster - Concept
*Disclaimer: This script is by no means final and can still undergo major changes*

![alt text](https://i.imgur.com/RCb9g1m.jpg
 "Hahhahah ... witzig")

## Idea
The basic idea is to avoid creating and closing threads in each function in bolster that has to split up work into multiple threads. 

A thread pool struct is created with a runtime fixed amount of threads and a queue for the tasks that are going to be dispatched to the threads. We start with the round robin approach for dispatching tasks to threads. 

We trade tasks as not preemtable, consequently a thread that works on a tasks will finish this task before getting a new one.  We assume that tasks are created outside the thread pool and the data parts, passed as arguments, are independend.

We provide the functionality that the calling function can easily wait for the enqueued tasks to be finished. This will be implemented with grouping. Each task has internally a group id. The enqueueing of tasks waits until all tasks with the same group id are finished.

Tasks are grouped outside of the thread pools logic via create_group_id and enqueueTask or inside the thread pools logic via the enqueueTasks method. See  Usage of the thread pool for more information

## Thread and tasks states

## Enqueueing and dispatching
In case of enqueueing the thread pool receives a struct consisting of thread information and creates an enqueued_task object and commits these to the task queue. Internally a task id is assigned so that the completion of all task can be noted and a quasi join is applicable to the thread pool.

---

The task queue includes all pending tasks and dispatches them to an idling thread in the thread pool. This is achieved by setting an active flag or disrupting sleep from outside and assigning the function pointer as well as arguments and attributes to the thread.

---

![](https://i.imgur.com/QBx6Y5H.png)


---

Basic thread main
```
int thread_main(void* args, void* routine) {
    while(true){
        if(is_active){
            routine(some_arguments)
        } else {
            sleep
        }
    }
}
```

## Usage of thread pool
``` 
gecko_pool* gecko_pool = gecko_pool_create(16);
```

## Structs
We divide our structure into 3 main structs containing relavent information for each scope.

*This may also include an Id which can be given by the user, currently only assigned by the threadpool*
First the __thread_task__ structure:
```
struct thread_task {
    void *arg
    const pthread_attr_t *attr
    void *(*function)
}
```
This structure is intended to be used in src like bolster.h instead of the current pthreads to hold all relevant informatinon the user is required to deliver.

Second the internal __\_\_enqueued_task structure__:
```
struct __enqueued_task {
    thread_task* thread
    **whatever** id
}
```
This struct is only intended to be used internally of the ThreadPool and contains additionally to the Thread information an id which is assigned to all task based on their membership to a task.

Finally the internal __\_\_gecko_pool structure__:
*This struct is not final and still under consideration which information should be contained*
```
struct __gecko_pool {
    pthread_t* pool
    size_t size
}
```
This structure is used only internally in the thread pool to save relevant information about the initilization.

## External methods

### ```gecko_pool* gecko_pool_create(int num_threads)```
Params:
- num_threads - Number of threads that the pool should contain

Creates an instance of the thread pool. This will create an array with num_threads threads which are running on idle mode at the beginning. 

---

### ```status_e gecko_pool_enqueue_tasks(thread_task* tasks)```
Params:
- tasks - Pointer to array of tasks which should be dispatched

Creates internally a group id and assigns this id to all tasks in the array. Then enqueues the passed tasks to the thread pool queue. This methods returns when all passed tasks are finished.

---

### ```status_e gecko_pool_enqueue_task(thread_task* task)```
Params:
- task - Pointer to one task which should be dispatched

Enqueues the passed task to the thread pool queue. If the task does not have a group id, this method returns if the task is finished. If the task already has a group id, this method returns immediatly and the result must be awaited with wait_for_id.

---

### ```size_t gecko_pool_create_group_id()```
Params:

Creates and returns an (incremented or random) id to group tasks.

---

### ```status_e gecko_pool_wait_for_id(size_t id)```
Params:
- id - Group id of tasks

Waits until all tasks with the passed group id are finished and then returns a status. If no task in the queue with the passed id is found, the related status is retured.

## Internal methods

### ```**whatever** __thread_main(size_t thread_id)```
Params:
- thread_id - The id for this thread, could be the index in the pool array
- Not clear what should be passed to this method...

The main logic assigned to each thread. The idea is to idle until the scheduler wakes the current thread up and dispatches a task to it. If the task is finished, the finished callback is called and the thread idles until the next task is dispatched to it.

First thoughts on code:
```
**whatever** __thread_main(**whatever** thread_id) {
    while(true) {
        
        // Get the task from the dispatcher
        __enqueued_task* task = __get_enqueued_task(thread_id);
        
        // real execution of the task
        __execute_task(task); 
        
        // callback for the pool to know that the task is finished
        __task_finished(task); 
        
        // Idle around until the dispatcher gives another task
        // If the idle can be interrupted from outside
        // Otherwise wrap it with if(is_active)
        __thread_idle()
    }
}
```

---

### ```**whatever** __execute_task(__enqueued_task* task)```
Params:
- task - the task that has to be executed

Executes the passed task, only god knows how...

---

### ```**whatever** __task_finished(__enqueued_task* task)```
Params:
- task - the task that is finished

Say to the thread pool: Hey this task is finished, do whatever you want with this information...

---

### ```void __thread_idle()```
Params:

Idle until the dispatcher gives this thread a new task.