#include <check.h>
#include <stdio.h>
#include "../../../include/thread_pool.h"

// Test the creatrion of thread pools
START_TEST(thread_pool_create_) 
{
    thread_pool* pool = thread_pool_create(2);
    ck_assert(pool);

    thread_pool_free(pool);
    //ck_assert(!pool);
}
END_TEST

void work(void* args)
{
	int* res = args;
    printf("Work %p\n", res);
	for(; *res > 0; (*res)--)
    {
        
    }
}

START_TEST(thread_pool_tasks){

    thread_pool* pool = thread_pool_create(2);
    task_handle hndl;

    int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000 };
    
    for(int i = 0; i < 6; i++){
        
        thread_task* test_task = (thread_task*)malloc(sizeof(thread_task));
        test_task->args = (void*)&test[i];
        test_task->routine = work;

        //std::cout << "enqueueing " << i << std::endl;
        gecko_pool_enqueue_task(test_task, pool, &hndl);
        thread_pool_wait_for_task(pool, &hndl);
        //std::cout << "enqueued " << i << std::endl;
    }

    for(int i = 0; i < 6; i++){
        //printf("%d\n", test[i]);
        ck_assert_int_eq(test[0], 0);
    }
    thread_pool_free(pool);
    
}
END_TEST

Suite * thread_pool_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Thread Pool");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, thread_pool_create_);
    tcase_add_test(tc_core, thread_pool_tasks);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = thread_pool_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
/


*/
