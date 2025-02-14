cmake_minimum_required(VERSION 3.20.0)

# Suppress warning about maybe-uninitialized from the uart_emul.c file from Zephyr
add_compile_options(-Wno-maybe-uninitialized)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(uart_thread_test)

# add location_module (the unit under test)
target_sources(app
    PRIVATE 
    ../../../app/src/modules/uart/uart_thread.c
    ../../../app/src/common/zbus_channels.c
)

zephyr_include_directories(${ZEPHYR_BASE}/subsys/testsuite/include)
zephyr_include_directories(${ZEPHYR_BASE}/include/zephyr/)
target_include_directories(app PRIVATE ../../../app/src/common)

target_link_options(app PRIVATE --whole-archive)
# Options that cannot be passed through Kconfig fragments
target_compile_definitions(app PRIVATE
	-DCONFIG_APP_UART_LOG_LEVEL=4
	-DCONFIG_APP_UART_THREAD_STACK_SIZE=1024
	-DCONFIG_APP_UART_WATCHDOG_TIMEOUT_SECONDS=10
	-DCONFIG_APP_UART_PIPE_READ_TIMEOUT_SECONDS=5
)

# add test file
target_sources(app PRIVATE src/test_uart_thread.c)
