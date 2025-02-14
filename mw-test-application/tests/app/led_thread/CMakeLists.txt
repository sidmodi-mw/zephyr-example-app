cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(led_thread_test)

# add location_module (the unit under test)
target_sources(app
    PRIVATE 
    ../../../app/src/modules/led/led_thread.c
    ../../../app/src/common/zbus_channels.c
)

zephyr_include_directories(${ZEPHYR_BASE}/include/zephyr/)
zephyr_include_directories(${ZEPHYR_BASE}/subsys/testsuite/include)
target_include_directories(app PRIVATE ../../../app/src/common)

target_link_options(app PRIVATE --whole-archive)
# Options that cannot be passed through Kconfig fragments
target_compile_definitions(app PRIVATE
	-DCONFIG_APP_LED_LOG_LEVEL=4
	-DCONFIG_APP_LED_THREAD_STACK_SIZE=1024
	-DCONFIG_APP_LED_MSGQ_SIZE=5
	-DCONFIG_APP_LED_WATCHDOG_TIMEOUT_SECONDS=10
	-DCONFIG_APP_LED_MSGQ_TIMEOUT_SECONDS=5
)

# add test file
target_sources(app PRIVATE src/test_led_thread.c)
