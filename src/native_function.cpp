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


#include "native_function.h"
#include <chrono>

auto SystemTimeNow(uint32_t num_arguments, Value* values) -> RuntimeErrorOr<Value>
{
    if (num_arguments != 0) {
        return std::unexpected(Error { .error_message = fmt::format("Number of arguments to SystemTimeNow not 0") });
    }
    static_cast<void>(values);
    auto now = std::chrono::high_resolution_clock::now();
    auto time_since_epoch_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    return Value { static_cast<double>(time_since_epoch_us.count()) };
}

auto Echo(uint32_t num_arguments, Value* values) -> RuntimeErrorOr<Value>
{
    LOX_ASSERT(values != nullptr);
    if (num_arguments != 1) {
        return std::unexpected(Error { .error_message = fmt::format("Number of arguments to SystemTimeNow not 0") });
    }
    return values[0];
}