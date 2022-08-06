add_library(lox_compiler STATIC compiler.cpp)
target_include_directories(lox_compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})