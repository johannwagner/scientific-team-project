# gecko_pool
gecko_pool is a thread pool designed to be easy to use while still remaining
more efficient then creating new threads for new problems.

## TODOs:
- Per-Pool Priority for threads
- Thread naming inside pool (e.g., "worker-", then 1st thread is "worker-1", 2nd is "worker2") etc.
- Error checking (e.g., null check with require_nonnull macro)
- code "Documentation" from HackMd to doxygen code docu in sources

## Configure gtest framework

### Ubuntu
```bash
sudo apt-get install libgtest-dev
```

### ArchLinux
```bash
pacman -S gtest
```
### Set Location of the library
```bash
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make

sudo cp *.a /usr/local/lib/
sudo cp ../gtest /usr/local/include/gtest -r
```

### Compile the Project
```bash
cmake /path/to/project/.
cd /path/to/project
make
```

### Build charts
```bash
cd /path/to/project/Statistics
make charts
```

*Note csv do currently not comply with csv standard*

# gecko_pool: Documentation

## Getting started
The basic idea is to avoid creating and closing threads in each function in
bolster that has to split up work into multiple threads. 

A thread pool struct is created with an amount of threads and a queue for the
tasks that are going to be dispatched to the threads. We start with the *first
come first serve* approach to schedule tasks to threads.

We trade tasks as not preemtable, consequently a thread that works on a tasks
will finish this task before getting a new one.  We assume that tasks are
created outside the thread pool and the data parts, passed as arguments, are
independend.

We provide the functionality that the calling function can easily wait for the
enqueued tasks to be finished. This will be implemented with grouping. Tasks can
be grouped with a handle. The enqueueing of tasks waits until all tasks with the
same handle are finished. See Usage of thread pool for further information.

Additionally, statistics can be collected via the boolean flag
*enable_monitoring*. 

## Public Structs

### Thread Task

To use the thread pool, tasks have to be enqueued to an instance. Each task is
represented with the following struct:
```c
        typedef struct thread_task_t {
  void* args;            // Arguments passed to the routine
        task_routine routine;  // Executed routine
  size_t group_id;       // For grouping tasks
  size_t priority;       // 0 as highest priority
  task_stats statistics; // Captured statistics if enabled

        } thread_task_t;
```

*args* and *routine* are mandatory attributes, the rest is optional.

### Task Handle

Tasks can be enqueued with a task handle to wait for all tasks having the same
handle at enqueueing time. Note: Enqueueing tasks with the same handle one after
another will change the handle every enqueue call and consequently only the last
task can be awaited. The awaiting is implemented using a slotmap and a
generation counter.

```c
typedef struct task_handle_t{
  size_t index;         // Index of the slotmap
  unsigned generation;  // Generation of the slot

} task_handle_t;
```

### Thread Pool
The whole thread pool struct contains many information about the pool itself and
about the threads.

```c
typedef struct thread_pool_t {
  char* name;                      // Passed name to set_name or create_named
  pthread_t* pool;                 // Array of used threads
  priority_queue_t* waiting_tasks; // Priority Queue containing the waiting
tasks
  size_t task_state_capacity;      // Number of tasks that can be tracked
  size_t size;                     // Actual number of threads in use
  size_t capacity;                 // Number of threads that can be used via
resizing
  struct __thread_information_t** thread_infos; // Information for each thread
  thread_pool_stats_t* statistics;   // Collected statistics
  int enable_monitoring;           // Monitoring enabled true/false

} thread_pool_t;
```

## Usage of thread pool

### Initialization

```c
// The first parameter sets the number of threads, 
// the second is a boolean flag to enable/disable statistics
thread_pool_t* thread_pool = thread_pool_create(16, 0);
thread_pool_t* thread_pool_n = thread_pool_create_named(16, "my_thread_pool",
0);

thread_pool_free(thread_pool);
thread_pool_free(thread_pool_n);
```

### Enqueueing tasks
```c
// Be sure to have some data and a routine to execute
void work(void* args) {
  int* res = (int*)args;
  for(; *res > 0; (*res)--){}

}
int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };

// Create an array of tasks...
thread_pool_t* pool = thread_pool_create(2, 1);
thread_task_t tasks[6];

for(int i = 0; i < 6; i++){
    // ...and assign data and routine to each task
    tasks[i].args = (void*)&test[i];
    tasks[i].routine = work;
    thread_pool_enqueue_task(&tasks[i], pool, NULL);

}
```

### Waiting for a set of tasks

```c
thread_pool_t* pool = thread_pool_create(2, 0);
thread_task_t tasks[6];
for(int i = 0; i < 6; i++){
    tasks[i].args = (void*)&test[i];
    tasks[i].routine = work;


}
// Enqueues all tasks and waits until they are finished
// The calling thread executes one of these tasks before waiting for the others
thread_pool_enqueue_tasks_wait(tasks, pool, 6);
```

### Waiting for all tasks

```c
thread_pool_t* pool = thread_pool_create(2, 0);
thread_task_t tasks[6];

for(int i = 0; i < 6; i++){
    tasks[i].args = (void*)&test[i];
    tasks[i].routine = work;
    thread_pool_enqueue_task(&tasks[i], pool, NULL);

}
// Waits for all tasks in the pool
// The calling thread executes one of these tasks before waiting for the others
thread_pool_wait_for_all(pool);
```

## Statistics

### Thread Pool Statistics
```c
typedef struct thread_pool_stats_t {
  struct timespec creation_time; 
  unsigned int task_enqueued_count;
  unsigned int task_complete_count; 
  long long complete_time; 
  long long wait_time;
  long long avg_complete_time;
  long long avg_wait_time;

} thread_pool_stats_t;
```
Usage:
```c 
// Pass enable_monitoring = true to the create function
thread_pool_t* pool = thread_pool_create(2, 1);
thread_task_t tasks[6];
thread_pool_stats_t pool_stats;

for(int i = 0; i < 6; i++){
    tasks[i].args = (void*)&test[i];
    tasks[i].routine = work;
    thread_pool_enqueue_task(&tasks[i], pool, NULL);

}
// Explicitly call get_stats to make sure all stats are filled
pool_stats = thread_pool_get_stats(pool);

//...
```
### Thread statistics
```c
typedef struct thread_stats_t {
  struct timespec creation_time;
  long long idle_time;
  long long busy_time;
  size_t task_count; // Number of tasks processed by this thread

} thread_stats_t;
```
Usage:
```c
thread_pool_t* pool = thread_pool_create(2, 1);
thread_task_t tasks[6];
thread_stats_t thread_stats;

for(int i = 0; i < 6; i++){
    tasks[i].args = (void*)&test[i];
    tasks[i].routine = work;

}

thread_pool_enqueue_tasks_wait(tasks, pool, 6);

// Pass index of thread in array as second parameter
thread_stats = thread_pool_get_thread_stats(pool, 0);
```
### Task Statistics
```c
typedef struct task_stats_t {
  struct timespec enqueue_time;
  struct timespec execution_time;
  struct timespec complete_time;

} task_stats_t;
```
Usage: Just enable monitoring and work with the filled statistics. We avoid
using heap memory for task statistics because they have to be freed manually
after for each task.

## Thread Pool Methods

```c
thread_pool_t* thread_pool_create(size_t num_threads, int enable_monitoring)
```
Creates a new thread pool instance along with the passed number of threads.

- *num_threads*: Number of threads used for this thread pool instance
- *enable_monitoring*: Boolean flag to enable/disable performance monitoring
--- 

```c
thread_pool_t* thread_pool_create_named(size_t num_threads, const char* name,
int enable_monitoring)
```
Creates a new thread pool instance along with the passed number of threads and a
specific name.

- *num_threads*: Number of threads used for this thread pool instance
- *name*: Name of the thread pool instance
- *enable_monitoring*: Boolean flag to enable/disable performance monitoring
--- 

```c
void thread_pool_free(thread_pool_t* pool)
```

Releases all resources hold by the threadpool. Currently working threads may
finish but tasks left in the queue will be discarded.
- *pool*: Thread pool isntance
---
```c
void thread_pool_set_name(thread_pool_t* pool, const char* name)
```
Sets the name to the passed thread pool instance.
- *pool*: Thread pool instance
- *name*: Name for the instance
---
```c
const char* thread_pool_get_name(thread_pool_t* pool)
```
Returns the name of the passed thread pool instance
- *pool*: Thread pool instance
--- 
```c
status_e thread_pool_resize(thread_pool_t* pool, size_t num_threads)
```
Sets the number of active threads to num_threads. Currently working threads are
terminated after their task is completed.
- *pool*: Thread pool instance
- *num_threads*: New number of threads. Must be smaller than 2 times initial
  thread pool size.

Returns: status_ok if success, status_failed if num_threads is not valid

---
```c
status_e thread_pool_enqueue_tasks(thread_task_t* task, thread_pool_t* pool, 
size_t num_tasks, task_handle_t* hndl)
```
Add multiple tasks to be executed. Their progress is tracked by a single handle.
This handle can be awaited using *thread_pool_wait_for_task*. Handle can be
nullptr.
- *task*: Array of tasks to be executed
- *pool*: Thread pool instance
- *num_tasks*: Number of elements in task
- *hndl*: Optional handle to wait for tasks

Returns: status_ok if success, status_failed if failed

--- 
```c
status_e thread_pool_enqueue_task(thread_task_t* task, thread_pool_t* pool, 
task_handle_t* hndl)
```
Same as *thread_pool_enqueue_tasks* with a single task. 
- *task*: Pointer to a single task to be executed
- *pool*: Thread pool instance
- *hndl*: Optional handle to wait for tasks

Returns: status_ok if success, status_failed if failed

---
```c
status_e thread_pool_enqueue_tasks_wait(thread_task_t* task, thread_pool_t*
pool, size_t num_tasks)
```
Add multiple tasks to be executed. Waits until all passed tasks are finished.
The main thread also participates in task execution. Creates internal handle to
wait for the tasks.
- *task*: Array of tasks to be executed
- *pool*: Thread pool instance
- *num_tasks*: Number of elements in task

Returns: status_ok if success, status_failed if failed

---
```c
status_e thread_pool_wait_for_task(thread_pool_t* pool, task_handle_t* hndl)
```
Waits until the tasks referenced by hndl are completed.
- *pool*: Thread pool instance
- *hndl*: Handle to wait for tasks

Returns: status_ok if success, status_failed if failed

--- 
```c
status_e thread_pool_wait_for_all(thread_pool_t* pool)
```
Waits until all tasks currently in the queue are executed. The main thread also
participates in task execution.
- *pool*: Thread pool instance

Returns: status_ok if success, status_failed if failed

## Thread Pool Statistic Methods
```c
double thread_pool_get_time_working(thread_pool_t* pool)
```
Returns the average fraction of time the active threads have been working.
- *pool*: Thread pool instance

--- 
```c
thread_pool_stats_t thread_pool_get_stats(thread_pool_t* pool)
```
Fill all stats of the passed thread pool instance.
- *pool*: Thread pool instance

---
```c
thread_stats_t thread_pool_get_thread_stats(thread_pool_t* pool, size_t id)
```
Fill all stats of the thread matching the given id in the thread pool.
- *pool*: Thread pool instance
- *id*: Index of the thread in the pool array
---

