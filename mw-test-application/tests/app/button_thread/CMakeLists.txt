cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(button_thread_test)

# generate runner for the test
# test_runner_generate(src/button_thread_test.c)

# add location_module (the unit under test)
target_sources(app PRIVATE ../../../app/src/threads/button_thread.c)

target_include_directories(app PRIVATE ../../../app/src/include)
target_include_directories(app PRIVATE ../../../app/src/threads)

# add test file
target_sources(app PRIVATE src/test_button_thread.c)
