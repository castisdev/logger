cmake_minimum_required (VERSION 3.16.0)
set(PROJECT_NAME castislogger)
project(${PROJECT_NAME})
find_package(Boost)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRCS
castisaccesslogger.cpp
castislogger.cpp
)

add_library(${PROJECT_NAME} ${SRCS})
