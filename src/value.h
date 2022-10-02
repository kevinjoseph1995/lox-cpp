//
// Created by kevin on 8/27/22.
//

#ifndef LOX_CPP_VALUE_H
#define LOX_CPP_VALUE_H

#include <variant>

#include "error.h"
#include "fmt/format.h"
#include "object.h"

struct NilType { };

struct Value : public std::variant<NilType, double, bool, Object*> {
    template <typename T>
    Value(T&& value)
        : std::variant<NilType, double, bool, Object*>(std::forward<T>(value))
    {
    }
    Value()
        : std::variant<NilType, double, bool, Object*>(NilType {})
    {
    }
    [[nodiscard]] auto IsNil() const -> bool;
    [[nodiscard]] auto IsBool() const -> bool;
    [[nodiscard]] auto IsDouble() const -> bool;
    [[nodiscard]] auto IsObject() const -> bool;

    [[nodiscard]] auto AsBool() -> bool&;
    [[nodiscard]] auto AsDouble() -> double&;
    [[nodiscard]] auto AsDouble() const -> double const&;
    [[nodiscard]] auto AsObject() const -> Object const&;
    [[nodiscard]] auto AsObject() -> Object&;
    [[nodiscard]] auto AsObjectPtr() -> Object*;
    [[nodiscard]] auto AsObjectPtr() const -> Object const*;
    [[nodiscard]] auto AsBool() const -> bool const&;

    [[nodiscard]] auto operator==(Value const& other) const -> bool;
    [[nodiscard]] auto operator!=(Value const& other) const -> bool;
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
        case 3: {
            Object const& object = value.AsObject();
            LOX_ASSERT(object.GetType() == ObjectType::STRING, "Unsupported type, cannot format");
            auto string_object = *static_cast<StringObject const*>(&object); // Fix me
            return fmt::format_to(ctx.out(), "{}", string_object.data);
        }
        default:
            LOX_ASSERT(false);
        }
    };
};

#endif // LOX_CPP_VALUE_H
