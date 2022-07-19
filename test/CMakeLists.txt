find_package(Catch2 REQUIRED CONFIG)



option(TSM_USE_VALGRIND "Use valgrind by default" ON)
find_program(VALGRIND_PATH valgrind)
if(NOT VALGRIND_PATH)
    option(TSM_USE_VALGRIND "Valgrind not found" OFF)
endif(NOT VALGRIND_PATH)

set (TEST_PROJECT tsm_test)

add_executable(${TEST_PROJECT}
  main.cpp
  CdPlayerHsm.cpp
  EventQueue.cpp
  GarageDoorSM.cpp
  OrthogonalCdPlayerHsm.cpp
  Switch.cpp
  TestMachines.cpp
  TrafficLightHsm.cpp
)

if(MSVC)
    target_compile_options(${TEST_PROJECT} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${TEST_PROJECT} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

target_link_libraries(${TEST_PROJECT}
  PRIVATE Catch2::Catch2 Threads::Threads tsm::tsm)
# add_custom_command(TARGET ${TEST_PROJECT} POST_BUILD
#     COMMAND "strip" -g -S  --strip-debug --verbose
#     "$<TARGET_FILE:${TEST_PROJECT}>"
#             COMMENT "strip debug symbols done on final binary.")

include(CTest)
include(Catch)

# configure unit tests via CTest
if (TSM_USE_VALGRIND)
    add_test(NAME ValgrindRunTests COMMAND valgrind --leak-check=full --error-exitcode=1 $<TARGET_FILE:${TEST_PROJECT}>)
    set_tests_properties(ValgrindRunTests PROPERTIES PASS_REGULAR_EXPRESSION "no leaks are possible")
    add_test(NAME ValgrindListTests COMMAND valgrind --leak-check=full --error-exitcode=1 $<TARGET_FILE:${TEST_PROJECT}> --list-tests --verbosity high)
    set_tests_properties(ValgrindListTests PROPERTIES PASS_REGULAR_EXPRESSION "no leaks are possible")
    add_test(NAME ValgrindListTags COMMAND valgrind --leak-check=full --error-exitcode=1 $<TARGET_FILE:${TEST_PROJECT}> --list-tags)
    set_tests_properties(ValgrindListTags PROPERTIES PASS_REGULAR_EXPRESSION "no leaks are possible")
endif()

catch_discover_tests(${TEST_PROJECT})

# test coverage
include(coverage)
set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)
install(TARGETS ${TEST_PROJECT} RUNTIME DESTINATION test)
