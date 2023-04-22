//
// Created by kevin on 9/15/22.
//

#include "error.h"
#include <exception>

void PrintAssertionMessage(char const* file, int line, char const* function_name, char const* message)
{
    if (message == nullptr) {
        fmt::print(stderr, "Assertion failed at {}:{} in {}", file, line, function_name);
    } else {
        fmt::print(stderr, "Assertion failed at {}:{} in FUNC:\"{}\" with MESSAGE:\"{}\"", file, line, function_name, message);
    }
}
