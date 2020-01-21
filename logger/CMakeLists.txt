cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME castislogger)
project(${PROJECT_NAME})
find_package(Boost)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(
  SYSTEM ${Boost_INCLUDE_DIRS}
)
set(CMAKE_CXX_STANDARD 17)

set(SRCS
castisaccesslogger.cpp
castislogger.cpp
)

add_library(${PROJECT_NAME} ${SRCS})
cotire(${PROJECT_NAME})
