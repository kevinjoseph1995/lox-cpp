//
// Created by kevin on 8/27/22.
//

#ifndef LOX_CPP_VALUE_H
#define LOX_CPP_VALUE_H

#include <variant>

#include "error.h"
#include "fmt/format.h"

struct NilType { };

struct Value : public std::variant<NilType, double, bool> {
    template <typename T>
    Value(T&& value)
        : std::variant<NilType, double, bool>(std::forward<T>(value))
    {
    }
    [[nodiscard]] bool IsNil() const
    {
        return std::holds_alternative<NilType>(*this);
    }
    [[nodiscard]] bool IsDouble() const
    {
        return std::holds_alternative<double>(*this);
    }
    [[nodiscard]] bool IsBool() const
    {
        return std::holds_alternative<bool>(*this);
    }
    bool operator==(Value const& other) const
    {
        if (this->index() != other.index()) {
            return false;
        }
        if (this->IsBool()) {
            return std::get_if<bool>(this) == std::get_if<bool>(&other);
        } else if (this->IsDouble()) {
            return std::get_if<double>(this) == std::get_if<double>(&other);
        } else {
            return true;
        }
    }
    bool operator!=(Value const& other) const
    {
        return !(*this == other);
    }
};

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
        default:
            LOX_ASSERT(false);
        }
    };
};

#endif // LOX_CPP_VALUE_H
