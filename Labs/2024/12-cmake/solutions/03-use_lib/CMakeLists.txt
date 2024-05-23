cmake_minimum_required(VERSION 3.5)

project(UseLib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(CMAKE_BUILD_TYPE)
  message("Manually set build type is: ${CMAKE_BUILD_TYPE}")
else()
  message("Setting build type to Release by default")
  set(CMAKE_BUILD_TYPE Release)
endif()

# Generate executable.
find_package (Eigen3 REQUIRED HINTS $ENV{mkEigenPrefix})

add_executable(main use_lib.cpp)
include_directories(${EIGEN3_INCLUDE_DIRS})

set_target_properties(main PROPERTIES CXX_STANDARD 17)