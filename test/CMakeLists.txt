find_package(Catch2 REQUIRED CONFIG)



option(TSM_USE_VALGRIND "Use valgrind by default" ON)
find_program(VALGRIND_PATH valgrind)
if(NOT VALGRIND_PATH)
    option(TSM_USE_VALGRIND "Valgrind not found" OFF)
endif(NOT VALGRIND_PATH)

set (TEST_PROJECT tsm_test)

add_executable(${TEST_PROJECT}
  main.cpp
  TestMachines.cpp
  GarageDoorSM.cpp
  CdPlayerHsm.cpp
  EventQueue.cpp
  OrthogonalCdPlayerHsm.cpp
)

target_link_libraries(${TEST_PROJECT}
  PRIVATE Catch2::Catch2 Threads::Threads tsm::tsm)

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

install(TARGETS ${TEST_PROJECT} RUNTIME DESTINATION test)
