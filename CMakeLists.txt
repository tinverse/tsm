cmake_minimum_required(VERSION 3.5)
project(stm VERSION 0.1.0)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall" CACHE STRING "CXX Flags")
set_directory_properties(PROPERTIES EP_BASE "${PROJECT_SOURCE_DIR}/../external-build")
set(SM_INSTALL_DIR ${PROJECT_SOURCE_DIR}/ThirdParty/${CMAKE_SYSTEM_NAME} CACHE PATH "ThirdParty stuff")

include(ExternalProject)

ExternalProject_Add(gflags
    GIT_REPOSITORY https://github.com/gflags/gflags.git
    GIT_TAG master
    CMAKE_GENERATOR Ninja
    INSTALL_DIR ${SM_INSTALL_DIR}
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )

ExternalProject_Add(glog DEPENDS gflags
    GIT_REPOSITORY https://github.com/google/glog.git
    GIT_TAG master
    CMAKE_GENERATOR Ninja
    INSTALL_DIR ${SM_INSTALL_DIR}
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=RelWithDebInfo
        -DGFLAGS_NAMESPACE=google
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )

ExternalProject_Add(gtest DEPENDS glog
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG master
    CMAKE_GENERATOR Ninja
    INSTALL_DIR ${SM_INSTALL_DIR}
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )

set(GTEST_ROOT "${SM_INSTALL_DIR}" CACHE PATH "GTest Root Dir")
set(GLOG_ROOT_DIR "${SM_INSTALL_DIR}" CACHE PATH "Glog Root Dir")
set(GFLAGS_ROOT_DIR "${SM_INSTALL_DIR}" CACHE PATH "GFlags Root Dir")

find_package(GFlags)
find_package(GTest)
find_package(Glog)

message(INFO ${CMAKE_MODULE_PATH} " " ${GLOG_FOUND} " " ${GLOG_INCLUDE_DIRS} " " ${GLOG_LIBRARIES})
message(INFO ${SM_INSTALL_DIR})
message(INFO ${SM_INSTALL_DIR})

add_library(libstatemachine StateMachine.cpp)
target_include_directories(libstatemachine SYSTEM PRIVATE ${GLOG_INCLUDE_DIRS} ${GFLAGS_INCLUDE_DIRS})


add_executable(statemachine_test StateMachineTests.cpp)
add_dependencies(statemachine_test libstatemachine gtest)

target_include_directories(statemachine_test SYSTEM PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(statemachine_test PRIVATE libstatemachine ${GTEST_LIBRARIES} ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES} pthread)

#install(TARGETS statemachine DESTINATION bin)
#install(TARGETS statemachine_test DESTINATION bin)
