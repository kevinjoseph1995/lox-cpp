# C++ standard
set(CMAKE_CXX_STANDARD 23)

option(LOX_DEBUG_GC_LOGGING "Enable Garbage collection logging" ON)
option(LOX_STRESS_TEST_GC "Stress test garbage collector" ON)
option(LOX_DEBUG_TRACE_EXECUTION "Log op's being executed in the VM" OFF)
option(LOX_ENABLE_BACKTRACE "Enable backtrace" OFF)

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
target_link_libraries(lox_compiler PUBLIC fmt $<$<STREQUAL:${LOX_ENABLE_BACKTRACE},ON>:Backward::Backward>)
target_compile_definitions(lox_compiler PRIVATE
        $<$<STREQUAL:${LOX_DEBUG_GC_LOGGING},ON>:DEBUG_GC_LOGGING=1>
        $<$<STREQUAL:${LOX_STRESS_TEST_GC},ON>:STRESS_TEST_GC=1>
        $<$<STREQUAL:${LOX_DEBUG_TRACE_EXECUTION},ON>:DEBUG_TRACE_EXECUTION=1>
        $<$<STREQUAL:${LOX_ENABLE_BACKTRACE},ON>:ENABLE_BACKTRACE=1>
)
target_compile_options(lox_compiler PUBLIC
        -Wall -Wextra -Werror -fno-exceptions -Wconversion -march=native  $<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-Wno-dangling-reference>
        $<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined;-fsanitize=signed-integer-overflow;-fsanitize=null;-fsanitize=float-cast-overflow;-fsanitize=alignment>)
target_link_options(lox_compiler PUBLIC
        $<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined>)


add_executable(lox_cpp main.cpp)
target_link_libraries(lox_cpp lox_compiler)

target_compile_options(lox_cpp PUBLIC
        -Wall -Wextra -Werror -fno-exceptions -Wconversion -march=native $<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-Wno-dangling-reference>
        $<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined;-fsanitize=signed-integer-overflow;-fsanitize=null;-fsanitize=float-cast-overflow;-fsanitize=alignment>)
target_link_options(lox_cpp PUBLIC
        $<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined>)
