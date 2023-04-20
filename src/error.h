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

template <typename... T>
static inline void GCDebugLog(fmt::format_string<T...> fmt [[maybe_unused]], T&&... args [[maybe_unused]])
{
#ifdef DEBUG_GC_LOGGING
    fmt::print("[GC][Debug]: ");
    fmt::print(fmt, std::forward<T>(args)...);
    fmt::print("\n");
    fflush(stdout);
#endif
}
struct Error {
    std::string error_message;
};

struct Span {
    uint64_t start;
    uint64_t end;
};

struct ScanError : Error {
    Span span;
};

struct ParseError : Error {
    Span span;
};

static_assert(std::is_destructible_v<Error>);

struct VoidType { };

template <typename T>
using ScanErrorOr = std::expected<T, ScanError>;

template <typename T>
using ParseErrorOr = std::expected<T, ParseError>;

using CompilationError = Error;
template <typename T>
using CompilationErrorOr = std::expected<T, CompilationError>;

using RuntimeError = Error;
template <typename T>
using RuntimeErrorOr = std::expected<T, RuntimeError>;

template <typename T>
using ErrorOr = std::expected<T, Error>;

#endif // LOX_CPP_ERROR_H
