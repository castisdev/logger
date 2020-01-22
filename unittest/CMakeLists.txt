file(GLOB SRCS_TEST "*.cpp" "*.h")
add_executable(unittest ${SRCS_TEST})
if(WIN32)
  target_link_libraries(unittest castislogger fmt::fmt ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g)
  set_target_properties(unittest PROPERTIES COMPILE_FLAGS "--coverage")
  target_link_libraries(
    unittest
    castislogger fmt::fmt ${Boost_LIBRARIES}
    pthread rt gtest_main gtest gcov)
endif(WIN32)
