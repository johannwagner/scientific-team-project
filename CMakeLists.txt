cmake_minimum_required(VERSION 3.0.2)
project(gecko_thread_pool)

set(CMAKE_C_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS "-O3 -Wall -Wextra -Werror")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("include")
include_directories("testing")

find_package (Threads)

# find all source and header files except main
file(GLOB_RECURSE SOURCE_FILES "include/*.h" "src/*.c")
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c)

add_executable(gecko_thread_pool 
        ${SOURCE_FILES}
        src/main.c)

target_link_libraries (gecko_thread_pool ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (gecko_thread_pool m)

if (UNIX) 
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
        include_directories("/usr/local/include")

        add_executable(gecko_thread_pool_tests
                testing/test_runner.cpp
                ${SOURCE_FILES}
                ${TEST_FILES}
        )

        add_custom_target(tests
                COMMAND bin/gecko_tests
        )

        target_link_libraries (gecko_thread_pool_tests ${CMAKE_THREAD_LIBS_INIT})
        target_link_libraries (gecko_thread_pool_tests m)
        target_link_libraries (gecko_thread_pool_tests /usr/local/lib/libgtest.a)
endif (UNIX)
