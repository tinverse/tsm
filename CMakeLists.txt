cmake_minimum_required(VERSION 3.7)

if(NOT DEFINED PROJECT_NAME)
  set(NOT_SUBPROJECT ON)
endif()


project(tsm VERSION 0.1.0)
LIST(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/superbuild")

include (CMakePackageConfigHelpers)
include (GNUInstallDirs)

#Compiler MUST be at least CXX 11 compliant
set(CMAKE_CXX_STANDARD 14)

if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
endif(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)

if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
endif(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
endif(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)

if (NOT INSTALL_DIR)
    set(INSTALL_DIR ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_NAME} CACHE PATH "Install Dir" FORCE)
endif(NOT INSTALL_DIR)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "Overriding install dir" FORCE)
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

find_package(Threads)

add_library(tsm INTERFACE)

# depend on some obvious c++11 features so the dependency is transitively added dependents
target_compile_features(tsm
  INTERFACE
    cxx_alignas
    cxx_alignof
    cxx_attributes
    cxx_auto_type
    cxx_constexpr
    cxx_defaulted_functions
    cxx_deleted_functions
    cxx_final
    cxx_lambdas
    cxx_noexcept
    cxx_override
    cxx_range_for
    cxx_rvalue_references
    cxx_static_assert
    cxx_strong_enums
    cxx_trailing_return_types
    cxx_unicode_literals
    cxx_user_literals
    cxx_variadic_macros
)

target_include_directories(tsm
  INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# provide a namespaced alias for clients to 'link' against if tsm is included as a sub-project
add_library(tsm::tsm ALIAS tsm)

# Only perform the installation steps when tsm is not being used as
# a subproject via `add_subdirectory`, or the destinations will break,
# see https://github.com/tsmorg/tsm/issues/1373
if (NOT_SUBPROJECT)
    set(TSM_CMAKE_CONFIG_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tsm")
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/CMake/tsmConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/tsmConfig.cmake
        INSTALL_DESTINATION
          ${TSM_CMAKE_CONFIG_DESTINATION}
    )


    # create and install an export set for tsm target as tsm::tsm
    install(
      TARGETS
        tsm
      EXPORT
        tsmTargets
      DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    )


    install(
      EXPORT
        tsmTargets
      NAMESPACE
        tsm::
      DESTINATION
        ${TSM_CMAKE_CONFIG_DESTINATION}
    )

    # By default, FooConfigVersion is tied to architecture that it was
    # generated on. Because tsm is header-only, it is arch-independent
    # and thus tsmConfigVersion should not be tied to the architecture
    # it was generated on.
    #
    # CMake does not provide a direct customization point for this in
    # `write_basic_package_version_file`, but it can be accomplished
    # indirectly by temporarily undefining `CMAKE_SIZEOF_VOID_P`.
    set(TSM_CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P})
    unset(CMAKE_SIZEOF_VOID_P)
    write_basic_package_version_file(
      "${CMAKE_CURRENT_BINARY_DIR}/tsmConfigVersion.cmake"
      COMPATIBILITY
        SameMajorVersion
    )
    set(CMAKE_SIZEOF_VOID_P ${TSM_CMAKE_SIZEOF_VOID_P})

    install(
      DIRECTORY
        "include/"
      DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}"
    )

    install(
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/tsmConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/tsmConfigVersion.cmake"
      DESTINATION
        ${TSM_CMAKE_CONFIG_DESTINATION}
    )

    # Install documentation
    if(TSM_INSTALL_DOCS)
      install(
        DIRECTORY
          docs/
        DESTINATION
          "${CMAKE_INSTALL_DOCDIR}"
      )
    endif(TSM_INSTALL_DOCS)

    ## Provide some pkg-config integration
    set(PKGCONFIG_INSTALL_DIR
        ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
        CACHE PATH "Path where tsm.pc is installed"
    )
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/CMake/tsm.pc.in
      ${CMAKE_CURRENT_BINARY_DIR}/tsm.pc
      @ONLY
    )
    install(
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/tsm.pc"
      DESTINATION
        ${PKGCONFIG_INSTALL_DIR}
    )

endif(NOT_SUBPROJECT)
# Build tests by default.
option(BUILD_TESTS "Enable Testing" ON)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(dependencies)

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
message(STATUS "Module path:" ${CMAKE_MODULE_PATH})

if (BUILD_TESTS)
  add_subdirectory(test)
endif(BUILD_TESTS)

message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

# generate documentation
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)

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

  if (NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "/var/empty/local")
    add_custom_command(TARGET tsm_doc POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_BINARY_DIR}/docs/html
                   ${CMAKE_INSTALL_PREFIX}/share/doc
         COMMENT "Installing docs to ${CMAKE_INSTALL_PREFIX}/share/doc")

  endif (NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "/var/empty/local")
endif(BUILD_DOCUMENTATION)

if (NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "/var/empty/local")
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_PREFIX} 
        FILES_MATCHING PATTERN "*.h")
endif (NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "/var/empty/local")
