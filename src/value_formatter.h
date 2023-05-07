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

#ifndef LOX_CPP_VALUE_FORMATTER_H
#define LOX_CPP_VALUE_FORMATTER_H

#include <fmt/core.h>

#include "object.h"
#include "value.h"

template<>
struct fmt::formatter<Value> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(Value const& value, FormatContext& ctx)
    {
        switch (value.index()) {
        case 0:
            return fmt::format_to(ctx.out(), "Nil");
        case 1: {
            double const* double_pointer = std::get_if<double>(&value);
            LOX_ASSERT(double_pointer != nullptr);
            return fmt::format_to(ctx.out(), "{}", *double_pointer);
        }
        case 2: {
            bool const* bool_pointer = std::get_if<bool>(&value);
            LOX_ASSERT(bool_pointer != nullptr);
            return fmt::format_to(ctx.out(), "{}", *bool_pointer);
        }
        case 3: {
            auto const object_ptr = value.AsObjectPtr();
            switch (object_ptr->GetType()) {
            case ObjectType::FUNCTION: {
                auto function_object = *static_cast<FunctionObject const*>(object_ptr);
                return fmt::format_to(ctx.out(), "function<{}, arity={}>", function_object.function_name, function_object.arity);
            }
            case ObjectType::STRING: {
                auto string_object = *static_cast<StringObject const*>(object_ptr);
                return fmt::format_to(ctx.out(), "{}", string_object.data);
            }
            case ObjectType::CLOSURE: {
                auto closure_object = *static_cast<ClosureObject const*>(object_ptr);
                return fmt::format_to(ctx.out(), "closure<{}, arity={}>", closure_object.function->function_name, closure_object.function->arity);
            }
            case ObjectType::NATIVE_FUNCTION: {
                return fmt::format_to(ctx.out(), "native_function");
            }
            case ObjectType::UPVALUE: {
                return fmt::format_to(ctx.out(), "upvalue_object");
            }
            case ObjectType::CLASS: {
                auto class_object = *static_cast<ClassObject const*>(object_ptr);
                return fmt::format_to(ctx.out(), "class_object[{}]", class_object.class_name);
            }
            case ObjectType::INSTANCE: {
                // TODO: Figure out how to print the fields as well
                auto const& instance = *static_cast<InstanceObject const*>(object_ptr);
                return fmt::format_to(ctx.out(), "instance[class[{}]]", instance.class_->class_name);
            }
            case ObjectType::BOUND_METHOD: {
                auto const& bound_method = *static_cast<BoundMethodObject const*>(object_ptr);
                return fmt::format_to(ctx.out(), "instance[class[{}]]", bound_method.receiver->class_->class_name);
                break;
            }
            }
            [[fallthrough]];
        }
        default: {
            LOX_ASSERT(false);
        }
        }
    };
};

#endif
