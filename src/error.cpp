//
// Created by kevin on 9/15/22.
//

#include "error.h"
#include <exception>

void PrintAssertionMessage(const char* file, int line, const char* function_name, const char* message)
{
    if (message == nullptr) {
        fmt::print(stderr, "Assertion failed at {}:{} in {}", file, line, function_name);
    } else {
        fmt::print(stderr, "Assertion failed at {}:{} in FUNC:\"{}\" with MESSAGE:\"{}\"", file, line, function_name, message);
    }
}
