#Automatically set build dependencies to on/off

#check to see if dependencies are built
set(GTEST_ROOT ${INSTALL_DIR})

if (BUILD_TESTS)
  find_package(GTest)
endif (BUILD_TESTS)

set(BUILD_DEPENDENCIES TRUE)
if (GTEST_FOUND)
    set(BUILD_DEPENDENCIES FALSE)
endif(GTEST_FOUND)

if (BUILD_DEPENDENCIES)
    set_directory_properties(PROPERTIES EP_BASE ${INSTALL_DIR})
    set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "ThirdParty stuff")

    # Make sure that CMAKE_INSTALL_PREFIX is set appropriately
    set(CMAKE_FIND_ROOT_PATH ${CMAKE_INSTALL_PREFIX})

    #No shared libs for gtest
    #set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build with Shared Libs")

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
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../CMake)
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release")
    endif(NOT CMAKE_BUILD_TYPE)

    set(Patches_DIR ${CMAKE_CURRENT_SOURCE_DIR}/patches)

    set(tsm_DEPENDENCIES)

    #---------------------------- GTest --------------------------------------------
    if(BUILD_TESTS)
        configure_file(cmake/superbuild/External-GTest.txt.in ${INSTALL_DIR}/ThirdParty/GTest/CMakeLists.txt)

        execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
            WORKING_DIRECTORY "${INSTALL_DIR}/ThirdParty/GTest" )
        execute_process(COMMAND "${CMAKE_COMMAND}" --build .
            WORKING_DIRECTORY "${INSTALL_DIR}/ThirdParty/GTest" )

        set( tsm_DEPENDENCIES ${tsm_DEPENDENCIES} GTest )
      endif(BUILD_TESTS)

endif (BUILD_DEPENDENCIES)

