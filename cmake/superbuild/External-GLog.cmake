message( "External project - GTest" )
set (GTest_VERSION 1.8.0)
find_package(Git)
if(NOT GIT_FOUND)
  message(ERROR "Cannot find git. git is required for Superbuild")
endif()

option( USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)

set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "QNX")
  set (BUILD_SHARED_LIBS OFF)
endif()
# Build Static Libs for GTest as that is recommended
ExternalProject_Add(GTest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-${GTest_VERSION}
    SOURCE_DIR GTest
    BINARY_DIR GTest-build

    CMAKE_GENERATOR ${EP_CMAKE_GENERATOR}
    UPDATE_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_DEPENDENCIES_DIR}
        -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DBUILD_GTEST:BOOL=ON
        -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
        -DBUILD_GMOCK:BOOL=ON
)

set( GLOG_ROOT ${INSTALL_DEPENDENCIES_DIR} )
