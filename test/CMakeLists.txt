find_package(Catch2 REQUIRED CONFIG)

set (TEST_PROJECT "tsm_test")

add_executable(${TEST_PROJECT}
  main.cpp
  TestMachines.cpp
  GarageDoorSM.cpp
  CdPlayerHSM.cpp
  EventQueue.cpp
  OrthogonalCdPlayerHSM.cpp
)

target_include_directories(tsm_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TEST_PROJECT}
  PRIVATE Catch2::Catch2 pthread)

include(CTest)
include(Catch)
catch_discover_tests(${TEST_PROJECT})

# test coverage
include(coverage)

install(TARGETS ${TEST_PROJECT} RUNTIME DESTINATION test)
