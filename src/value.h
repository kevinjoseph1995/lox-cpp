//
// Created by kevin on 8/27/22.
//

#ifndef LOX_CPP_VALUE_H
#define LOX_CPP_VALUE_H

#include <variant>

#include "error.h"

struct Object;

struct NilType { };

struct Value : public std::variant<NilType, double, bool, Object*> {
    template <typename T>
    Value(T&& value)
        : std::variant<NilType, double, bool, Object*>(static_cast<const variant>(std::forward<T>(value)))
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
#endif // LOX_CPP_VALUE_H
