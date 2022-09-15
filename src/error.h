//
// Created by kevin on 8/13/22.
//

#ifndef LOX_CPP_ERROR_H
#define LOX_CPP_ERROR_H

#include "fmt/core.h"
#include <stdio.h>
#include <string>
#include <variant>

void PrintAssertionMessage(const char* file, int line, const char* function_name, const char* message = nullptr);

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
struct ErrorOr : public std::variant<Error, T> {
    ErrorOr(Error&& error)
        : std::variant<Error, T>(std::forward<Error>(error))
    {
    }
    ErrorOr(T&& value)
        : std::variant<Error, T>(std::forward<T>(value))
    {
    }

    ~ErrorOr() = default;

    [[nodiscard]] bool IsError() const { return std::holds_alternative<Error>(*this); }
    [[nodiscard]] bool IsValue() const { return std::holds_alternative<T>(*this); }

    [[nodiscard]] Error const& GetError() const
    {
        auto ptr = std::get_if<Error>(this);
        LOX_ASSERT(ptr != nullptr, "Invalid ErrorOr access");
        return *ptr;
    }

    [[nodiscard]] T const& GetValue() const
    {
        auto ptr = std::get_if<T>(this);
        LOX_ASSERT(ptr != nullptr, "Invalid ErrorOr access");
        return *ptr;
    }
};

#endif // LOX_CPP_ERROR_H
