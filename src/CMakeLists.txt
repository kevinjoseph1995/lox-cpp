add_library(lox_compiler STATIC chunk.cpp virtual_machine.cpp scanner.cpp compiler.cpp heap.cpp source.cpp)
target_include_directories(lox_compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lox_compiler fmt)
target_compile_options(lox_compiler PRIVATE -Wall -Wextra -Werror -fno-exceptions)

target_compile_options(lox_compiler PRIVATE $<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined>)
target_link_options(lox_compiler PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)


add_executable(lox_cpp main.cpp)
target_link_libraries(lox_cpp lox_compiler)
target_compile_options(lox_cpp PRIVATE -Wall -Wextra -Werror -fno-exceptions)

target_compile_options(lox_cpp PRIVATE $<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined>)
target_link_options(lox_cpp PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)