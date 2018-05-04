cmake_minimum_required(VERSION 3.0.2)
project(gecko_thread_pool)

set(CMAKE_C_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS "-O3 -Wall -Wextra -Werror")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("include")

find_package (Threads)

set(SOURCE_FILES
        src/*.c)

add_executable(gecko_thread_pool ${SOURCE_FILES})

target_link_libraries (gecko_thread_pool ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (gecko_thread_pool m)
