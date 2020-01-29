cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME awsome_example)
project(${PROJECT_NAME})
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.56 COMPONENTS log log_setup regex REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)

option(LOGGER_BUILD_FMT "Enable fmt" ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(cotire)

include_directories(
  SYSTEM ${Boost_INCLUDE_DIRS}
  submodules/fmt/include
  ${CMAKE_SOURCE_DIR}
)

add_subdirectory(logger)
add_subdirectory(examples EXCLUDE_FROM_ALL)
add_subdirectory(unittest EXCLUDE_FROM_ALL)

if (LOGGER_BUILD_FMT)
  add_subdirectory(submodules/fmt)
endif()
