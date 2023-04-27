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


#ifndef LOX_CPP_VALUE_H
#define LOX_CPP_VALUE_H

#include <variant>

#include "error.h"

struct Object;

struct NilType { };

struct Value : public std::variant<NilType, double, bool, Object*> {
    template<typename T>
    Value(T&& value)
        : std::variant<NilType, double, bool, Object*>(static_cast<variant const>(std::forward<T>(value)))
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
