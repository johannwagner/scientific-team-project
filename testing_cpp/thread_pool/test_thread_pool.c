#include <check.h>
#include "../../include/thread_pool.h"

// Test the creatrion of thread pools
START_TEST(thread_pool_create_pool) 
{
    thread_pool* pool = thread_pool_create(2);
    ck_assert(pool);

    thread_pool_free(pool);
    //ck_assert(!pool);
}
END_TEST

/ Test the creatrion of group Ids of thread pools
TEST(ThreadPool, CreateGroupId) {

    thread_pool* pool = thread_pool_create(2);
    size_t id = gecko_pool_create_group_id();
    size_t id2 = gecko_pool_create_group_id();

    EXPECT_NE(id, id2);

    thread_pool_free(pool);
}

void basicTask (void* args) {
    int* number = (int*) args;
    std::cout << "number has value: " <<  *number << std::endl;
}

TEST(ThreadPool, BasicTasks){

    thread_pool* pool = thread_pool_create(2);
    size_t id = gecko_pool_create_group_id();
    size_t id2 = gecko_pool_create_group_id();

    thread_task* test_task = (thread_task*)malloc(sizeof(thread_task));
    
    int test = 5;
    
    test_task->args = (void*)&test;
    test_task->routine = &basicTask;

    for(int i = 0; i < 6; i++){
        std::cout << "enqueueing " << i << std::endl;
        //gecko_pool_enqueue_task(test_task, pool);
        std::cout << "enqueued " << i << std::endl;
    }

    thread_pool_free(pool);
}

Suite * money_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Thread Pool");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, thread_pool_create_pool);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = money_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
/


*/
