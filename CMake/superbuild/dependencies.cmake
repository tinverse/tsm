#Automatically set build dependencies to on/off
option(BUILD_DEPENDENCIES "" ON)
if (BUILD_TESTS)
    find_package(Catch2 3 QUIET)
endif (BUILD_TESTS)

if (CATCH2_FOUND)
  set(BUILD_DEPENDENCIES FALSE)
endif(CATCH2_FOUND)

if (BUILD_DEPENDENCIES)
  set_directory_properties(PROPERTIES EP_BASE ${INSTALL_DIR})
  set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "ThirdParty stuff")

  # Make sure that CMAKE_INSTALL_PREFIX is set appropriately
  set(CMAKE_FIND_ROOT_PATH ${CMAKE_INSTALL_PREFIX})

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

  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
  endif(NOT CMAKE_BUILD_TYPE)

  set(Patches_DIR ${CMAKE_CURRENT_SOURCE_DIR}/patches)

  set(tsm_DEPENDENCIES "")

  #---------------------------- Catch2 --------------------------------------------
  if(BUILD_TESTS)
    configure_file(CMake/superbuild/External-Catch2.txt.in ${INSTALL_DIR}/ThirdParty/Catch2/CMakeLists.txt)

    execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" -Bbuild -H.
        WORKING_DIRECTORY "${INSTALL_DIR}/ThirdParty/Catch2" )
    execute_process(COMMAND "${CMAKE_COMMAND}" --build build/
        WORKING_DIRECTORY "${INSTALL_DIR}/ThirdParty/Catch2" )

    set( tsm_DEPENDENCIES ${tsm_DEPENDENCIES} Catch2 )
  endif(BUILD_TESTS)

endif (BUILD_DEPENDENCIES)

