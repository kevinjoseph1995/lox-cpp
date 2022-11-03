cmake_minimum_required(VERSION 3.22)
project(lox_cpp)

# Compile commands json generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++ standard
set(CMAKE_CXX_STANDARD 23)

# CCACHE Setup
if (CCACHE_FOUND)
    message(CCACHE_FOUND)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif (CCACHE_FOUND)
find_program(CCACHE_FOUND ccache)

# Use mold as linker if found
find_program(MOLD_FOUND "mold")
IF (MOLD_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -B/usr/local/libexec/mold")
ENDIF ()

add_subdirectory(src)
add_subdirectory(third_party)
add_subdirectory(tests)
