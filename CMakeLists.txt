cmake_minimum_required(VERSION 3.5)

project(tsm VERSION 0.1.0)
LIST(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/superbuild")
if (NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif(NOT CMAKE_CXX_STANDARD)

if (NOT INSTALL_DIR)
    set(INSTALL_DIR ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_NAME} CACHE PATH "Install Dir")
endif(NOT INSTALL_DIR)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT
        OR CMAKE_INSTALL_PREFIX STREQUAL "/usr/local"
        OR CMAKE_INSTALL_PREFIX MATCHES "^C:/Program Files")

    set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR}
      CACHE PATH "CMake Install Prefix"
      FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT
        OR CMAKE_INSTALL_PREFIX STREQUAL "/usr/local"
        OR CMAKE_INSTALL_PREFIX MATCHES "^C:/Program Files")


if (BUILD_DEPENDENCIES)
    include(dependencies)
    set_directory_properties(PROPERTIES EP_BASE ${INSTALL_DIR})
    set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "ThirdParty stuff")
else (BUILD_DEPENDENCIES)

    if(MSVC)
      # Force to always compile with W4
      if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
      else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
      endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-long-long -pedantic")
    endif()

    set(CMAKE_FIND_ROOT_PATH ${INSTALL_DIR} CACHE PATH "")
    find_package(Gflags REQUIRED)
    find_package(GTest REQUIRED)
    find_package(Glog REQUIRED)

    message(STATUS "Module path:" ${CMAKE_MODULE_PATH})
    message(STATUS "GLog:" ${GLOG_FOUND} " " ${GLOG_INCLUDE_DIRS} " " ${GLOG_LIBRARIES})
    message(STATUS "GTest:" ${GTEST_FOUND} " " ${GTEST_INCLUDE_DIRS} " " ${GTEST_LIBRARIES})
    message(STATUS ${CMAKE_INSTALL_PREFIX})

    add_library(tsm
        UniqueId.cpp
    )

    target_include_directories(tsm SYSTEM PRIVATE ${GLOG_INCLUDE_DIRS} ${GFLAGS_INCLUDE_DIRS})
    target_link_libraries(tsm PRIVATE ${GTEST_LIBRARIES} ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES})

    add_executable(tsm_test
      test/main.cpp
      test/EventQueue.cpp
      test/GarageDoorSM.cpp
      test/CdPlayerHSM.cpp
      test/OrthogonalCdPlayerHSM.cpp
      )

    target_include_directories(tsm_test
      PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/test
      SYSTEM PRIVATE ${GLOG_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
      )

    target_link_libraries(tsm_test
      PRIVATE tsm ${GTEST_LIBRARIES} ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES} pthread)

    include(GoogleTest)
    gtest_discover_tests(tsm_test TEST_PREFIX test)
endif(BUILD_DEPENDENCIES)
