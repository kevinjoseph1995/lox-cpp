//
// Created by kevin on 11/25/22.
//
#ifndef LOX_CPP_NATIVE_FUNCTION_H
#define LOX_CPP_NATIVE_FUNCTION_H

#include "value.h"

[[nodiscard]] auto SystemTimeNow(uint32_t num_arguments, Value* values) -> RuntimeErrorOr<Value>;
[[nodiscard]] auto Echo(uint32_t num_arguments, Value* values) -> RuntimeErrorOr<Value>;
#endif // LOX_CPP_NATIVE_FUNCTION_H