//
// Created by kevin on 11/25/22.
//

#include "value.h"
#ifndef LOX_CPP_NATIVE_FUNCTION_H
#    define LOX_CPP_NATIVE_FUNCTION_H

#endif // LOX_CPP_NATIVE_FUNCTION_H

[[nodiscard]] auto SystemTimeNow(uint32_t num_arguments, Value* values) -> RuntimeErrorOr<Value>;
[[nodiscard]] auto Echo(uint32_t num_arguments, Value* values) -> RuntimeErrorOr<Value>;