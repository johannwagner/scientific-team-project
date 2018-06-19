cmake_minimum_required(VERSION 3.0.2)
project(gecko_thread_pool)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_C_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS "-Wall -Wextra")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CHECK_INSTALL_DIR "/usr/share/doc/check/examples")

include_directories("include")
include_directories("testing")

find_package (Threads)

# find all source and header files except main
file(GLOB_RECURSE SOURCE_FILES "include/*.h" "src/*.c")
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c)

#file(GLOB_RECURSE TEST_FILES "testing/*.c")
file(GLOB_RECURSE TEST_FILES "testing/*.hpp")

add_executable(gecko_thread_pool 
        ${SOURCE_FILES}
        src/main.c)

target_link_libraries (gecko_thread_pool ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (gecko_thread_pool m)

if (UNIX) 
        #find_package(Check REQUIRED)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -o0 -pthread")
        include_directories("/usr/local/include")

        #include_directories(${CHECK_INCLUDE_DIRS})
        #link_directories(${CHECK_LIBRARY_DIRS})

        add_executable(gecko_thread_pool_tests
                ${SOURCE_FILES}
                ${TEST_FILES}
                testing/test_runner.cpp
        )

        add_custom_target(tests
                COMMAND bin/gecko_tests
        )

        target_link_libraries (gecko_thread_pool_tests ${CMAKE_THREAD_LIBS_INIT})
        target_link_libraries (gecko_thread_pool_tests m)
        #target_link_libraries(gecko_thread_pool_tests ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
        target_link_libraries (gecko_thread_pool_tests /usr/local/lib/libgtest.a)

        #target_link_libraries (gecko_thread_pool_tests /usr/lib/x86_64-linux-gnu/libcheck.a)
endif (UNIX)
