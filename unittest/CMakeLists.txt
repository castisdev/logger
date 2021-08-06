file(GLOB SRCS_TEST "*.cpp" "*.h")
add_executable(logger_unittest ${SRCS_TEST})
if(WIN32)
  target_link_libraries(logger_unittest castislogger fmt::fmt ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g)
  set_target_properties(logger_unittest PROPERTIES COMPILE_FLAGS "--coverage")
  target_link_libraries(
    logger_unittest
    castislogger fmt::fmt ${Boost_LIBRARIES}
    pthread rt gtest_main gtest gcov)
endif(WIN32)
