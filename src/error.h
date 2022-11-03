//
// Created by kevin on 8/13/22.
//

#ifndef LOX_CPP_ERROR_H
#define LOX_CPP_ERROR_H

#include "fmt/core.h"
#include <expected>
#include <stdio.h>
#include <string>
#include <variant>

auto PrintAssertionMessage(const char* file, int line, const char* function_name, const char* message = nullptr) -> void;

// TODO: Print stack-trace here, research available solutions
// Look into: https://github.com/bombela/backward-cpp/blob/master/backward.hpp
#define unlikely(x) __builtin_expect(!!(x), 0)
#define LOX_ASSERT(expr, ...)                                                               \
    do {                                                                                    \
        if (unlikely(!(expr))) {                                                            \
            PrintAssertionMessage(__FILE__, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__); \
            __builtin_trap();                                                               \
        }                                                                                   \
    } while (0)

enum class ErrorType {
    ScanError,
    ParseError,
    RuntimeError
};

struct Error {
    ErrorType type;
    std::string error_message;
};

struct VoidType { };

template <typename T>
using ErrorOr = std::expected<T, Error>;

#endif // LOX_CPP_ERROR_H
