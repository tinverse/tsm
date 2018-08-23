cmake_minimum_required( VERSION 3.3.0 )

project( tsm-Superbuild )


if (NOT DEFINED INSTALL_DIR)
  message(FATAL_ERROR " You must pass a variable INSTALL_DIR for the libs, i.e. with -DINSTALL_DIR=/install/path (avoid '~')")
endif()
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR}
      CACHE PATH "CMake Install Prefix"
      FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_FIND_ROOT_PATH ${CMAKE_INSTALL_PREFIX})

set( BUILD_SHARED_LIBS ON CACHE BOOL "Build with Shared Libs")

# set CMAKE_MODULE_PATH for cmake macro/function and modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../CMake)

if( CMAKE_HOST_WIN32 )
    string( LENGTH "${CMAKE_CURRENT_SOURCE_DIR}" n )
    if( n GREATER 50 )
        message( FATAL_ERROR
            "source code directory path length is too long (${n} > 50)."
            "Please move the tsm source code directory to a directory with a shorter path."
            )
    endif()
    string( LENGTH "${CMAKE_CURRENT_BINARY_DIR}" n )
    if( n GREATER 50 )
        message( FATAL_ERROR
            "tsm build directory path length is too long (${n} > 50)."
            "Please move the tsm build directory to a directory with a shorter path."
            )
    endif()
endif()

option( BUILD_SHARED_LIBS "Build shared libraries" ON )

include( ExternalProject )

if (NOT CMAKE_BUILD_TYPE)
    set( CMAKE_BUILD_TYPE Release)
endif(  )

set( Patches_DIR ${CMAKE_CURRENT_SOURCE_DIR}/patches )


set( tsm_DEPENDENCIES)

#---------------------------- Gflags -------------------------------------------
option( USE_SYSTEM_Gflags "Use system libraries for Gflags" OFF )
if( ${USE_SYSTEM_Gflags} MATCHES "OFF" )
    include( External-Gflags.cmake )
    set( tsm_DEPENDENCIES ${tsm_DEPENDENCIES} Gflags )
else()
    find_package(Gflags REQUIRED)
endif()

#---------------------------- Glog ---------------------------------------------
option( USE_SYSTEM_Glog "Use system libraries for Glog" OFF )
if( ${USE_SYSTEM_Glog} MATCHES "OFF" )
    include( External-Glog.cmake )
    set( tsm_DEPENDENCIES ${tsm_DEPENDENCIES} Glog )
else()
    find_package(Glog REQUIRED)
endif()

#---------------------------- GTest --------------------------------------------
option( USE_SYSTEM_GTest "Use system libraries for GTest" OFF )
if( ${USE_SYSTEM_GTest} MATCHES "OFF" )
    include( External-GTest.cmake )
    set( tsm_DEPENDENCIES ${tsm_DEPENDENCIES} GTest )
else()
    find_package(GTest REQUIRED)
endif()

#---------------------------- tsm ---------------------------------------------
get_filename_component(tsm_CMAKE_SUBDIR ${tsm-Superbuild_SOURCE_DIR} PATH)
get_filename_component(tsm_SOURCE_DIR ${tsm_CMAKE_SUBDIR} PATH)

ExternalProject_Add( tsm
    DEPENDS ${tsm_DEPENDENCIES}
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    SOURCE_DIR   ${tsm_SOURCE_DIR}
    BINARY_DIR tsm-build
    CMAKE_GENERATOR ${CMAKE_GENERATOR}
    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_FIND_ROOT_PATH:PATH=${CMAKE_INSTALL_PREFIX}
    INSTALL_COMMAND ""
    )
