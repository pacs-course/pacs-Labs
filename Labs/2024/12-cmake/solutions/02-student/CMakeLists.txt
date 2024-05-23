cmake_minimum_required(VERSION 3.5)

project(Student)

# Enable optimizations.
set(CMAKE_BUILD_TYPE Release)

# Generate executable.
set(SCRS "test.cpp;src/Student.cpp")
add_executable(test_student ${SCRS})

# Include directory with header files.
target_include_directories(test_student PUBLIC src)
