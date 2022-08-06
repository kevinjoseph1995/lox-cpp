cmake_minimum_required(VERSION 3.23)
project(lox_cpp)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(src)
add_executable(lox_cpp main.cpp)
target_link_libraries(lox_cpp lox_compiler)
