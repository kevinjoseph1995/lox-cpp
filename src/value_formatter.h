// Created by kevin on 10-08-2022

#ifndef LOX_CPP_VALUE_FORMATTER_H
#define LOX_CPP_VALUE_FORMATTER_H

#include <exception>
#include <fmt/core.h>

#include "object.h"
#include "value.h"

template <>
struct fmt::formatter<Value> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
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
                break;
            }
            case ObjectType::UPVALUE: {
                return fmt::format_to(ctx.out(), "upvalue_object");
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
