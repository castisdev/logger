cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME awsome_example)
project(${PROJECT_NAME})
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.56 COMPONENTS log log_setup thread filesystem system regex date_time chrono REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_executable(example examples/example.cpp)
if(WIN32)
  target_link_libraries(example ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g -Os)
  set_target_properties(example PROPERTIES COMPILE_FLAGS "-std=c++11")
  target_link_libraries(example ${Boost_LIBRARIES} pthread rt)
endif(WIN32)
  target_link_libraries(
    unittest
    ${Boost_LIBRARIES}
    pthread rt gtest_main gtest gcov)
endif(WIN32)
