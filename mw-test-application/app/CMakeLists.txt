cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mw_test_app)
zephyr_compile_definitions(PROJECT_NAME=${PROJECT_NAME})

target_sources(app PRIVATE src/main.c)

# Include source directory
target_include_directories(app PRIVATE src)
target_include_directories(app PRIVATE src/common)

# Include files that are common for all modules
add_subdirectory(src/common)

# # Include modules
add_subdirectory(src/modules/button)
add_subdirectory(src/modules/led)
add_subdirectory(src/modules/uart)
