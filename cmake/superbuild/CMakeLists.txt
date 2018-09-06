cmake_minimum_required( VERSION 3.3.0 )

project( tsm-Superbuild )
LIST(APPEND CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}")


if (NOT DEFINED INSTALL_DIR)
  message(FATAL_ERROR " You must pass a variable INSTALL_DIR for the libs, i.e. with -DINSTALL_DIR=/install/path (avoid '~')")
endif()

include(dependencies)

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
