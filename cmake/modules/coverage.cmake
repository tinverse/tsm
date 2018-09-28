if (BUILD_COVERAGE)
  find_program(GCOV_PATH gcov)
  find_program(GENHTML_PATH genhtml)
  find_program(LCOV_PATH lcov)

  if(NOT LCOV_PATH)
      message(FATAL_ERROR "lcov not found.")
  endif(NOT LCOV_PATH)

  if(NOT GCOV_PATH)
      message(FATAL_ERROR "gcov not found.")
  endif(NOT GCOV_PATH)

  if(NOT GENHTML_PATH)
      message(FATAL_ERROR "GENHTML not found.")
  endif(NOT GENHTML_PATH)

  include(CodeCoverage)
  set(CMAKE_BUILD_TYPE "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")

  add_custom_target(coverage
    COMMAND ${LCOV_PATH} -z -d .
    COMMAND ${LCOV_PATH} --no-external -b '${PROJECT_SOURCE_DIR}' -c -i -d . -o ${TEST_PROJECT}_base.info
    COMMAND ${TEST_PROJECT} 
    COMMAND ${LCOV_PATH} --no-external -b '${PROJECT_SOURCE_DIR}' -c -d . -o ${TEST_PROJECT}_test.info
    COMMAND ${LCOV_PATH} -a ${TEST_PROJECT}_base.info -a ${TEST_PROJECT}_test.info -o ${TEST_PROJECT}_total.info
    COMMAND ${LCOV_PATH} --remove ${TEST_PROJECT}_total.info '${PROJECT_BINARY_DIR}/*' -o coverage.info 
    COMMAND ${GENHTML_PATH} -o ${TEST_PROJECT}_test_coverage coverage.info 
    COMMAND rm ${PROJECT_BINARY_DIR}/${TEST_PROJECT}_*.info
    DEPENDS ${TEST_PROJECT}
    BYPRODUCTS ${TEST_PROJECT}_base.info ${TEST_PROJECT}_test.info ${TEST_PROJECT}_total.info coverage.info
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )
endif(BUILD_COVERAGE)
