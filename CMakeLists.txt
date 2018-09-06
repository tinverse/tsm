cmake_minimum_required(VERSION 3.5)

project(tsm VERSION 0.1.0)
LIST(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/superbuild")

set(CMAKE_CXX_STANDARD 14)

if (NOT INSTALL_DIR)
    set(INSTALL_DIR ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_NAME} CACHE PATH "Install Dir")
endif(NOT INSTALL_DIR)

if (BUILD_DEPENDENCIES)
    include(dependencies)
    set_directory_properties(PROPERTIES EP_BASE ${INSTALL_DIR})
    set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "ThirdParty stuff")
else (BUILD_DEPENDENCIES)
    set(CMAKE_FIND_ROOT_PATH ${INSTALL_DIR} CACHE PATH "")
    find_package(Gflags REQUIRED)
    find_package(GTest REQUIRED)
    find_package(Glog REQUIRED)

    message(STATUS "Module path:" ${CMAKE_MODULE_PATH})
    message(STATUS "GLog:" ${GLOG_FOUND} " " ${GLOG_INCLUDE_DIRS} " " ${GLOG_LIBRARIES})
    message(STATUS "GTest:" ${GTEST_FOUND} " " ${GTEST_INCLUDE_DIRS} " " ${GTEST_LIBRARIES})
    message(STATUS ${CMAKE_INSTALL_PREFIX})

    add_library(tsm
        tsm.cpp
        UniqueId.cpp
    )

    target_include_directories(tsm SYSTEM PRIVATE ${GLOG_INCLUDE_DIRS} ${GFLAGS_INCLUDE_DIRS})
    target_link_libraries(tsm PRIVATE ${GTEST_LIBRARIES} ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES})


    add_executable(tsm_test tsm_tests.cpp)

    target_include_directories(tsm_test SYSTEM PRIVATE ${GLOG_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})
    target_link_libraries(tsm_test PRIVATE tsm ${GTEST_LIBRARIES} ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES} pthread)
endif(BUILD_DEPENDENCIES)
