cmake_minimum_required(VERSION 3.10)
project(hello_tsm VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
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
find_package (Threads)
target_link_libraries(hello_tsm
  PRIVATE tsm::tsm Threads::Threads)



