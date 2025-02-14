cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(uart_thread_test)

# add location_module (the unit under test)
target_sources(app PRIVATE ../../../app/src/modules/uart/uart_thread.c)
target_sources(app PRIVATE ../../../app/src/common/zbus_channels.c)

target_include_directories(app PRIVATE ../../../app/src/common)

# add test file
target_sources(app PRIVATE src/test_uart_thread.c)
