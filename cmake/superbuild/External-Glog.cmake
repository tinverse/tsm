message( "External project - Glog" )
set (Glog_VERSION 0.3.5)
option( USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)

set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

ExternalProject_Add(Glog
    DEPENDS Gflags
    GIT_REPOSITORY ${git_protocol}://github.com/google/glog.git
    GIT_TAG v${Glog_VERSION}
    CMAKE_GENERATOR ${CMAKE_GENERATOR}
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DWITH_GFLAGS=ON
        -DGFLAGS_NAMESPACE=google
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    )

set( GLOG_ROOT_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "Glog Root dir")
