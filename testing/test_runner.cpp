#include <gtest/gtest.h>
#include "tests/threadpool/test_pool.hpp"
#include "tests/priorityqueue/test_queue.hpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}