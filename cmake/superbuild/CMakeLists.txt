cmake_minimum_required( VERSION 3.3.0 )

project( StateMachine-Superbuild )


if (NOT DEFINED INSTALL_DIR)
  message(FATAL_ERROR " You must pass a variable INSTALL_DIR for the libs, i.e. with -DINSTALL_DIR=/install/path (avoid '~')")
endif()

set( INSTALL_DEPENDENCIES_DIR "${INSTALL_DIR}" CACHE STRING "Dep install" )

set( EP_CMAKE_GENERATOR "Ninja" CACHE STRING "CMAKE Generator" )
set( BUILD_SHARED_LIBS ON CACHE BOOL "Build with Shared Libs")

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set ( ep_common_args
        "-DCMAKE_EXECUTABLE_FORMAT:STRING=ELF" )
endif()


# set CMAKE_MODULE_PATH for cmake macro/function and modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../CMake)

if( APPLE )
    include( CMakeOSXVariables )
endif()

if( CMAKE_HOST_WIN32 )

    string( LENGTH "${CMAKE_CURRENT_SOURCE_DIR}" n )
    if( n GREATER 50 )
        message( FATAL_ERROR
            "source code directory path length is too long (${n} > 50)."
            "Please move the StateMachine source code directory to a directory with a shorter path."
            )
    endif()

    string( LENGTH "${CMAKE_CURRENT_BINARY_DIR}" n )
    if( n GREATER 50 )
        message( FATAL_ERROR
            "StateMachine build directory path length is too long (${n} > 50)."
            "Please move the StateMachine build directory to a directory with a shorter path."
            )
    endif()

endif()

option( BUILD_SHARED_LIBS "Build shared libraries" ON )

include( ExternalProject )

if (NOT CMAKE_BUILD_TYPE)
    set( BUILD_TYPE Release)
else (  )
    set( BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif(  )

if( NOT INSTALL_DEPENDENCIES_DIR )
    set( INSTALL_DEPENDENCIES_DIR ${CMAKE_BINARY_DIR}/INSTALL CACHE STRING "Install directory for dependencies")
endif()
set( CMAKE_INSTALL_PREFIX "${INSTALL_DEPENDENCIES_DIR}" CACHE STRING "Dep install" )

set( Patches_DIR ${CMAKE_CURRENT_SOURCE_DIR}/patches )
set( StateMachine_DEPENDENCIES )

option( USE_SYSTEM_GTest "Use system libraries for GTest" OFF )
if( ${USE_SYSTEM_GTest} MATCHES "OFF" )
    include( External-GTest.cmake )
    set( StateMachine_DEPENDENCIES ${StateMachine_DEPENDENCIES} GTest )
else()
    find_package(GTest REQUIRED)
endif()

# The dev dir is two levels above the superbuild cmake dir: dev/CMake/superbuild
get_filename_component(DEV_CMAKE_SUBDIR ${StateMachine-Superbuild_SOURCE_DIR} PATH)
get_filename_component(StateMachine_SOURCE_DIR ${DEV_CMAKE_SUBDIR} PATH)

ExternalProject_Add( StateMachine
    DEPENDS ${StateMachine_DEPENDENCIES}
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    SOURCE_DIR   ${StateMachine_SOURCE_DIR}
    BINARY_DIR StateMachine-build
    CMAKE_GENERATOR ${EP_CMAKE_GENERATOR}
    CMAKE_ARGS
    ${ep_common_args}
    -DIPP_ROOT:PATH=${IPP_ROOT}
    -DQWT_ROOT:PATH=${QWT_ROOT}
    -DBOOST_ROOT:PATH=${BOOST_ROOT}
    -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}
    -DINSTALL_DEPENDENCIES_DIR:PATH=${INSTALL_DEPENDENCIES_DIR}
    -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
    -DVYLIBDIR:STRING=${INSTALL_DEPENDENCIES_DIR}
    -DQt5_DIR:PATH=${Qt5_DIR}
    INSTALL_COMMAND ""
    )
