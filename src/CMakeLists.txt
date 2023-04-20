# C++ standard
set(CMAKE_CXX_STANDARD 23)


if(NOT DEBUG_GC_LOGGING)
        set(DEBUG_GC_LOGGING "ON" CACHE STRING "Enable Garbage collection logging" FORCE)
        # Set the possible values of build type for cmake-gui
        set_property(CACHE DEBUG_GC_LOGGING PROPERTY STRINGS "ON" "OFF")
endif()

if(NOT STRESS_TEST_GC)
        set(STRESS_TEST_GC "ON" CACHE STRING "Stress test garbage collector" FORCE)
        # Set the possible values of build type for cmake-gui
        set_property(CACHE STRESS_TEST_GC PROPERTY STRINGS "ON" "OFF")
endif()

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

target_compile_definitions(lox_compiler PRIVATE
        $<$<STREQUAL:${DEBUG_GC_LOGGING},ON>:DEBUG_GC_LOGGING=1>
        $<$<STREQUAL:${STRESS_TEST_GC},ON>:STRESS_TEST_GC=1>
)
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
