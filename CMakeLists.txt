cmake_minimum_required(VERSION 3.5)

project(tsm VERSION 0.1.0)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall" CACHE STRING "CXX Flags")
set_directory_properties(PROPERTIES EP_BASE "${PROJECT_SOURCE_DIR}/../external-build")
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/ThirdParty/${CMAKE_SYSTEM_NAME} CACHE PATH "ThirdParty stuff")

set(CMAKE_FIND_ROOT_PATH ${CMAKE_INSTALL_PREFIX})


set(GTEST_ROOT "${CMAKE_INSTALL_PREFIX}" CACHE PATH "GTest Root Dir")
set(GLOG_ROOT_DIR "${CMAKE_INSTALL_PREFIX}" CACHE PATH "Glog Root Dir")
set(GFLAGS_ROOT_DIR "${CMAKE_INSTALL_PREFIX}" CACHE PATH "GFlags Root Dir")

find_package(Gflags REQUIRED)
find_package(GTest REQUIRED)
find_package(Glog REQUIRED)

message(INFO ${CMAKE_MODULE_PATH} " " ${GLOG_FOUND} " " ${GLOG_INCLUDE_DIRS} " " ${GLOG_LIBRARIES})
message(INFO ${CMAKE_INSTALL_PREFIX})

add_library(tsm tsm.cpp)
target_include_directories(tsm SYSTEM PRIVATE ${GLOG_INCLUDE_DIRS} ${GFLAGS_INCLUDE_DIRS})


add_executable(tsm_test tsm_tests.cpp)

target_include_directories(tsm_test SYSTEM PRIVATE ${GLOG_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})
target_link_libraries(tsm_test PRIVATE tsm ${GTEST_LIBRARIES} ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES} pthread)

#install(TARGETS tsm DESTINATION bin)
#install(TARGETS tsm_test DESTINATION bin)
