cmake_minimum_required(VERSION 3.10)
project(hello_tsm VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 14)

# set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_FLAGS "-g -gdwarf-version")

PROJECT(stm32-base)

enable_language(ASM)

find_package(CMSIS REQUIRED)
find_package(STM32HAL COMPONENTS gpio tim REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMSIS_INCLUDE_DIRS}
    ${STM32HAL_INCLUDE_DIR}
    freertos/FreeRTOS/Source/include
    freertos/FreeRTOS/Source/portable/GCC/ARM_CM3/
)

set(PROJECT_SOURCES
    freertos/FreeRTOS/Source/portable/MemMang/heap_1.c
    freertos/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c
    freertos/FreeRTOS/Source/tasks.c
    freertos/FreeRTOS/Source/queue.c
    freertos/FreeRTOS/Source/list.c
    freertos/FreeRTOS/Source/timers.c
    #src/newlib.c
    src/main.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES} ${CMSIS_SOURCES} ${STM32HAL_SOURCES})

stm32_set_target_properties(${CMAKE_PROJECT_NAME})
stm32_add_hex_bin_targets(${CMAKE_PROJECT_NAME})

# READ THIS:
# Set tsm_DIR to a folder that contains tsmConfig.cmake
# For e.g.
# git clone https://github.com/tinverse/tsm tsm
# cd tsm && mkdir build && cd build && cmake -DCMAKE_INSTALL_PREFIX=/tmp ..
# make install
# mkdir build-hello_tsm && cd build-hello_tsm
# cmake -Dtsm_DIR=/tmp/lib/cmake/tsm ../../examples/hello_tsm
# make
# This should build this example. You can use this as a guide to include tsm
# in your projects.

if (NOT tsm_DIR)
    message(WARNING "tsm_DIR not defined")
endif(NOT tsm_DIR)

find_package(tsm REQUIRED CONFIG)

add_executable(hello_tsm
    main.cpp
)




