# Findtsm.cmake
#
# Finds the tsm library
#
# This will define the following variables
#
#    tsm_FOUND
#    tsm_INCLUDE_DIRS
#
# and the following imported targets
#
#     tsm::tsm
#

find_package(PkgConfig)
pkg_check_modules(PC_tsm QUIET tsm)

find_path(tsm_INCLUDE_DIR
    NAMES tsm.h
    PATHS ${PC_tsm_INCLUDE_DIRS}
    PATH_SUFFIXES tsm
)

set(tsm_VERSION ${PC_tsm_VERSION})

mark_as_advanced(tsm_FOUND tsm_INCLUDE_DIR tsm_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(tsm
    REQUIRED_VARS tsm_INCLUDE_DIR
    VERSION_VAR tsm_VERSION
)

if(tsm_FOUND)
    set(tsm_INCLUDE_DIRS ${tsm_INCLUDE_DIR})
endif()

if(tsm_FOUND AND NOT TARGET tsm::tsm)
    add_library(tsm::tsm INTERFACE IMPORTED)
    set_target_properties(tsm::tsm PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${tsm_INCLUDE_DIR}"
    )
endif()

