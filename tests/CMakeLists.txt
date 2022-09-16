add_executable(test_compiler test_compiler.cpp test_virtual_machine.cpp)
target_link_libraries(test_compiler gtest_main lox_compiler fmt)
add_test(NAME test_compiler COMMAND test_compiler)
target_compile_options(test_compiler PRIVATE
        -Wall -Wextra -Werror -fno-exceptions -Wno-restrict)