# C++ standard
set(CMAKE_CXX_STANDARD 23)

add_library(lox_compiler STATIC
        chunk.cpp
        virtual_machine.cpp
        scanner.cpp
        compiler.cpp
        heap.cpp
        source.cpp
        value.cpp
        error.cpp
        parser_state.cpp
        native_function.cpp)
target_include_directories(lox_compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lox_compiler fmt)
target_compile_options(lox_compiler PUBLIC
        -Wall -Wextra -Werror -fno-exceptions -Wconversion -march=native
        $<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined;-fsanitize=signed-integer-overflow;-fsanitize=null;-fsanitize=float-cast-overflow;-fsanitize=alignment>)
target_link_options(lox_compiler PUBLIC
        $<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined>)


add_executable(lox_cpp main.cpp)
target_link_libraries(lox_cpp lox_compiler)

target_compile_options(lox_cpp PUBLIC
        -Wall -Wextra -Werror -fno-exceptions -Wconversion -march=native
        $<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined;-fsanitize=signed-integer-overflow;-fsanitize=null;-fsanitize=float-cast-overflow;-fsanitize=alignment>)
target_link_options(lox_cpp PUBLIC
        $<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined>)
