add_library(lox_compiler STATIC compiler.cpp common.h common.cpp virtual_machine.cpp virtual_machine.h)
target_include_directories(lox_compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(lox_cpp main.cpp)
target_link_libraries(lox_cpp lox_compiler)
target_compile_options(lox_cpp PRIVATE -fno-exceptions)