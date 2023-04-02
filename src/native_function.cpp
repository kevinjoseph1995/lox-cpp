//
// Created by kevin on 11/25/22.
//

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