cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME example)
project(${PROJECT_NAME})
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.56 COMPONENTS log log_setup thread filesystem system regex date_time chrono REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_executable(example example.cpp)
if(WIN32)
  target_link_libraries(example ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g -Os)
  set_target_properties(example PROPERTIES COMPILE_FLAGS "-std=c++11")
  target_link_libraries(example ${Boost_LIBRARIES} pthread rt)
endif(WIN32)

add_executable(example_async example_async.cpp)
if(WIN32)
  target_link_libraries(example_async ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g -Os)
  set_target_properties(example_async PROPERTIES COMPILE_FLAGS "-std=c++11")
  target_link_libraries(example_async ${Boost_LIBRARIES} pthread rt)
endif(WIN32)

file(GLOB SRCS_TEST "unittest/*.cpp" "unittest/*.h")
add_executable(unittest ${SRCS_TEST})
set_target_properties(
  unittest 
  PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage -std=gnu++11"
)
if(WIN32)
  target_link_libraries(unittest ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g -Os)
  set_target_properties(example_async PROPERTIES COMPILE_FLAGS "-std=c++11")
  target_link_libraries(
    unittest
    ${Boost_LIBRARIES}
    pthread rt gtest_main gtest gcov)
endif(WIN32)
