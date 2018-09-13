# Make sure that CMAKE_INSTALL_PREFIX is set appropriately
set(CMAKE_FIND_ROOT_PATH ${CMAKE_INSTALL_PREFIX})

set( BUILD_SHARED_LIBS ON CACHE BOOL "Build with Shared Libs")

#RPATH 
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_MACOSX_RPATH ON CACHE BOOL "")
endif()

set(CMAKE_SKIP_BUILD_RPATH FALSE CACHE BOOL "")
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE CACHE BOOL "")

set(CMAKE_INSTALL_RPATH
    "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/lib64"
    CACHE STRING "")
set(INSTALL_RPATH_USE_LINK_PATH TRUE CACHE BOOL "")

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib"
          isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH
      "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/lib64")
endif("${isSystemDir}" STREQUAL "-1")


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
    include( External-Gflags )
    set( tsm_DEPENDENCIES ${tsm_DEPENDENCIES} Gflags )
else()
    find_package(Gflags REQUIRED)
endif()

#---------------------------- Glog ---------------------------------------------
option( USE_SYSTEM_Glog "Use system libraries for Glog" OFF )
if( ${USE_SYSTEM_Glog} MATCHES "OFF" )
    include( External-Glog )
    set( tsm_DEPENDENCIES ${tsm_DEPENDENCIES} Glog )
else()
    find_package(Glog REQUIRED)
endif()

#---------------------------- GTest --------------------------------------------
option( USE_SYSTEM_GTest "Use system libraries for GTest" OFF )
if( ${USE_SYSTEM_GTest} MATCHES "OFF" )
    include( External-GTest )
    set( tsm_DEPENDENCIES ${tsm_DEPENDENCIES} GTest )
else()
    find_package(GTest REQUIRED)
endif()


