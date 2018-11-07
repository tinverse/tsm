cmake_minimum_required(VERSION 3.7)

project(tsm VERSION 0.1.0)
LIST(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/superbuild")

#Compiler MUST be at least CXX 14 compliant
set(CMAKE_CXX_STANDARD 14)

if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
endif()

if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
endif()

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
endif()

if (NOT INSTALL_DIR)
    set(INSTALL_DIR ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_NAME} CACHE PATH "Install Dir")
endif(NOT INSTALL_DIR)

#Automatically set build dependencies to on/off
if (NOT DEFINED BUILD_DEPENDENCIES)
  #check to see if dependencies are built
  set(GTEST_ROOT ${INSTALL_DIR})
  find_package(GTest)
  if (GTEST_FOUND)
    set(BUILD_DEPENDENCIES OFF)
  else(GTEST_FOUND)
    set(BUILD_DEPENDENCIES ON)
  endif(GTEST_FOUND)
endif(NOT DEFINED BUILD_DEPENDENCIES)

if (BUILD_DEPENDENCIES)
    include(dependencies)
    set_directory_properties(PROPERTIES EP_BASE ${INSTALL_DIR})
    set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "ThirdParty stuff")
else (BUILD_DEPENDENCIES)
    if(MSVC)
      # Force to always compile with W4
      if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
      else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
      endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-long-long -pedantic -Wno-shadow -fpermissive")
    endif()

    set(CMAKE_FIND_ROOT_PATH ${INSTALL_DIR} CACHE PATH "")
    find_package(Gflags REQUIRED)
    find_package(GTest REQUIRED)
    find_package(Glog REQUIRED)

    message(STATUS "Module path:" ${CMAKE_MODULE_PATH})
    message(STATUS "GLog:" ${GLOG_FOUND} " " ${GLOG_INCLUDE_DIRS} " " ${GLOG_LIBRARIES})
    message(STATUS "GTest:" ${GTEST_FOUND} " " ${GTEST_INCLUDE_DIRS} " " ${GTEST_LIBRARIES})
    message(STATUS ${CMAKE_INSTALL_PREFIX})

    add_library(tsm
      Event.cpp
      UniqueId.cpp
    )

    target_include_directories(tsm SYSTEM PRIVATE ${GLOG_INCLUDE_DIRS} ${GFLAGS_INCLUDE_DIRS})
    target_link_libraries(tsm PRIVATE ${GTEST_LIBRARIES} ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES})

    install(TARGETS tsm ARCHIVE DESTINATION lib)

    add_subdirectory(test)

    # generate documentation
    find_package(Doxygen)
    option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

    if(${DOXYGEN_FOUND})
      set(DOC_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/doc CACHE PATH "")
    endif(${DOXYGEN_FOUND})

    if(BUILD_DOCUMENTATION)
      if(NOT DOXYGEN_FOUND)
          message(FATAL_ERROR "Doxygen is needed to build the documentation.")
      endif()

      set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
      set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile)

      configure_file(${doxyfile_in} ${doxyfile} @ONLY)

      add_custom_target(tsm_doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "Generating API documentation with Doxygen"
          VERBATIM)

   add_custom_command(TARGET tsm_doc POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                  ${CMAKE_CURRENT_BINARY_DIR}/docs/html
                  ${CMAKE_INSTALL_PREFIX}/share/doc
        COMMENT "Installing docs to ${CMAKE_INSTALL_PREFIX}/share/doc")
  endif()
endif(BUILD_DEPENDENCIES)
