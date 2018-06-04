#include <check.h>
#include <stdio.h>
#include <math.h>
#include "../../../include/thread_pool.h"

// Test the creation of thread pools
START_TEST(thread_pool_create_) 
{
    thread_pool* pool = thread_pool_create(2);
    ck_assert(pool);

    thread_pool_free(pool);
    //ck_assert(!pool);
}
END_TEST

// Test the naming of thread pools
START_TEST(thread_pool_name) 
{
    thread_pool* pool = thread_pool_create_named(2, "ThreadPool");
    ck_assert(pool);
    ck_assert_str_eq(pool->name, "ThreadPool");

    thread_pool_free(pool);
    
}
END_TEST

void work(void* args)
{
	int* res = args;

    double exp = *res;

	double v = 0.0;
	for(double i = 0; i < 100024; ++i)
	{
		v += pow(i, 1 / exp);
	}

	for(; *res > 0; (*res)--)
    {
        
    }

    printf("%s\n", "Work finished");

}

void work_large(void* args)
{
	double* res = args;
	double exp = *res;

	double v = 0.0;
	for(double i = 0; i < 100024; ++i)
	{
		v += pow(i, 1 / exp);
	}
	*res = v;
}

START_TEST(thread_pool_tasks){

    thread_pool* pool = thread_pool_create(2);
    task_handle hndl;

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    
    thread_task* test_task = (thread_task*)calloc(6, sizeof(thread_task));

    for(int i = 0; i < 6; i++){
        
        test_task[i].args = (void*)&test[i];
        test_task[i].routine = work;
    }

    thread_pool_enqueue_tasks(test_task, pool, 6, &hndl);
    printf("%s", "Waiting");
    thread_pool_wait_for_task(pool, &hndl);
    printf("%s", "Waiting finished");

    for(int i = 0; i < 6; i++){
        ck_assert_int_eq(test[i], 0);
    }
    thread_pool_free(pool);
    
}
END_TEST

START_TEST(thread_pool_tasks_wait){

    thread_pool* pool = thread_pool_create(2);

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    
    thread_task tasks[6];

    for(int i = 0; i < 6; i++){
        
        tasks[i].args = (void*)&test[i];
        tasks[i].routine = work;
    }

    printf("%s\n", "Waiting");
    thread_pool_enqueue_tasks_wait(tasks, pool, 6);
    printf("%s\n", "Waiting finished");

    for(int i = 0; i < 6; i++){
        ck_assert_int_eq(test[i], 0);
    }
    thread_pool_free(pool);
    
}
END_TEST

/*
/


*/
