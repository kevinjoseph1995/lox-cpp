// MIT License

// Copyright (c) 2023 Kevin Joseph

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#ifndef LOX_CPP_ERROR_H
#define LOX_CPP_ERROR_H

#include <fmt/core.h>

#include <expected>
#include <stdio.h>
#include <string>
#include <variant>

auto PrintAssertionMessage(char const* file, int line, char const* function_name, char const* message = nullptr) -> void;
__attribute__((always_inline)) auto PrintBackTrace() -> void;

#define LOX_ASSERT(expr, ...)                                                               \
    do {                                                                                    \
        if ((!(expr))) [[unlikely]] {                                                       \
            PrintAssertionMessage(__FILE__, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__); \
            PrintBackTrace();                                                               \
            __builtin_trap();                                                               \
        }                                                                                   \
    } while (0)

template<typename... T>
static inline void GCDebugLog(fmt::format_string<T...> fmt [[maybe_unused]], T&&... args [[maybe_unused]])
{
#ifdef DEBUG_GC_LOGGING
    fmt::print("[GC][Debug]");
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

template<typename T>
using ScanErrorOr = std::expected<T, ScanError>;

template<typename T>
using ParseErrorOr = std::expected<T, ParseError>;

using CompilationError = Error;
template<typename T>
using CompilationErrorOr = std::expected<T, CompilationError>;

using RuntimeError = Error;
template<typename T>
using RuntimeErrorOr = std::expected<T, RuntimeError>;

template<typename T>
using ErrorOr = std::expected<T, Error>;

#endif // LOX_CPP_ERROR_H
