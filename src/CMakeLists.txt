add_library(lox_compiler STATIC common.cpp virtual_machine.cpp scanner.cpp compiler.cpp)
target_include_directories(lox_compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lox_compiler fmt)


add_executable(lox_cpp main.cpp)
target_link_libraries(lox_cpp lox_compiler)
target_compile_options(lox_cpp PRIVATE -fno-exceptions)