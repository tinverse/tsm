cmake_minimum_required(VERSION 3.5)
project(hello_tsm VERSION 0.1.0)

# This is key. Set tsm_DIR to a folder that contains tsmConfig.cmake
set (tsm_DIR "../../build/Linux/lib/cmake/tsm" CACHE PATH "")

find_package(tsm REQUIRED CONFIG)

add_executable(hello_tsm
    main.cpp
)
find_package (Threads)
target_link_libraries(hello_tsm
  PRIVATE tsm::tsm Threads::Threads)



