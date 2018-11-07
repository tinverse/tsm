set (TEST_PROJECT "tsm_test")

add_executable(${TEST_PROJECT}
  main.cpp
  EventQueue.cpp
  GarageDoorSM.cpp
  CdPlayerHSM.cpp
  OrthogonalCdPlayerHSM.cpp
)

target_include_directories(tsm_test
  PUBLIC ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
  SYSTEM PRIVATE ${GLOG_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(${TEST_PROJECT}
  PRIVATE tsm ${GTEST_LIBRARIES} ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES} pthread)

include(GoogleTest)
gtest_discover_tests(${TEST_PROJECT} TEST_PREFIX "hsm:" TEST_LIST LIST_OF_TESTS)
gtest_add_tests(
  TARGET ${TEST_PROJECT}
  TEST_LIST ${LIST_OF_TESTS}
)

# test coverage
include(coverage)

install(TARGETS ${TEST_PROJECT} RUNTIME DESTINATION test)
