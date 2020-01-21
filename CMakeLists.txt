cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME awsome_example)
project(${PROJECT_NAME})
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.56 COMPONENTS log log_setup thread filesystem system regex date_time chrono REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(cotire)

include_directories(
  SYSTEM ${Boost_INCLUDE_DIRS}
  submodules/fmt/include
  ${CMAKE_SOURCE_DIR}
)

add_executable(example examples/example.cpp)
if(WIN32)
  target_link_libraries(example castislogger fmt::fmt ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g -Os)
  set_target_properties(example PROPERTIES COMPILE_FLAGS "-std=c++11")
  target_link_libraries(example castislogger fmt::fmt ${Boost_LIBRARIES} pthread rt)
endif(WIN32)

add_executable(example_async examples/example_async.cpp)
if(WIN32)
  target_link_libraries(example_async castislogger fmt::fmt ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g -Os)
  set_target_properties(example_async PROPERTIES COMPILE_FLAGS "-std=c++11")
  target_link_libraries(example_async castislogger fmt::fmt ${Boost_LIBRARIES} pthread rt)
endif(WIN32)

add_executable(example_filter examples/example_filter.cpp)
if(WIN32)
  target_link_libraries(example_filter castislogger fmt::fmt ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g -Os)
  set_target_properties(example_filter PROPERTIES COMPILE_FLAGS "-std=c++11")
  target_link_libraries(example_filter castislogger fmt::fmt ${Boost_LIBRARIES} pthread rt)
endif(WIN32)

add_executable(example_ch examples/example_ch.cpp)
if(WIN32)
  target_link_libraries(example_ch castislogger fmt::fmt ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g -Os)
  set_target_properties(example_ch PROPERTIES COMPILE_FLAGS "-std=c++11")
  target_link_libraries(example_ch castislogger fmt::fmt ${Boost_LIBRARIES} pthread rt)
endif(WIN32)

add_executable(example_access examples/example_access.cpp)
if(WIN32)
  target_link_libraries(example_access castislogger fmt::fmt ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g -Os)
  set_target_properties(example_access PROPERTIES COMPILE_FLAGS "-std=c++11")
  target_link_libraries(example_access castislogger fmt::fmt ${Boost_LIBRARIES} pthread rt)
endif(WIN32)

file(GLOB SRCS_TEST "unittest/*.cpp" "unittest/*.h")
add_executable(unittest ${SRCS_TEST})
if(WIN32)
  target_link_libraries(unittest castislogger fmt::fmt ${Boost_LIBRARIES})
else(WIN32)
  add_definitions(-Wall -g)
  set_target_properties(unittest PROPERTIES COMPILE_FLAGS "--coverage -std=c++11")
  target_link_libraries(
    unittest
    castislogger fmt::fmt ${Boost_LIBRARIES}
    pthread rt gtest_main gtest gcov)
endif(WIN32)

add_subdirectory(logger)
add_subdirectory(submodules/fmt EXCLUDE_FROM_ALL)
