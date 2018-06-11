#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include "tests/thread_pool/test_thread_pool.h"

Suite * thread_pool_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Thread Pool");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, thread_pool_create_);
    tcase_add_test(tc_core, thread_pool_name);
    //tcase_add_test(tc_core, thread_pool_tasks);
    tcase_add_test(tc_core, thread_pool_tasks_wait);
    

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
