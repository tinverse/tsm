message( "External project - Gflags" )
set (Gflags_VERSION 2.2.1)
find_package(Git)
if(NOT GIT_FOUND)
  message(ERROR "Cannot find git. git is required for Superbuild")
endif()

option( USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)

set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "https")
endif()

ExternalProject_Add(Gflags
    GIT_REPOSITORY ${git_protocol}://github.com/gflags/gflags.git
    GIT_TAG v${Gflags_VERSION}
    CMAKE_GENERATOR ${CMAKE_GENERATOR}
    UPDATE_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    )

set( GFLAGS_ROOT_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "")
