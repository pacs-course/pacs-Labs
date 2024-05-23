cmake_minimum_required(VERSION 3.5)

project(Student)

# Enable optimizations.
set(CMAKE_BUILD_TYPE Release)

# Generate executable.
add_executable(student "test.cpp;src/Student.cpp")

# Include directory with header files.
target_include_directories(student PRIVATE src)
