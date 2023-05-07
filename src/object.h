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

#ifndef LOX_CPP_OBJECT_H
#define LOX_CPP_OBJECT_H

#include "chunk.h"
#include "error.h"
#include "native_function.h"
#include "value.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

using Table = std::unordered_map<std::string, Value>;

enum class ObjectType {
    STRING,
    FUNCTION,
    CLOSURE,
    NATIVE_FUNCTION,
    UPVALUE,
    CLASS,
    INSTANCE,
    BOUND_METHOD
};

template<>
struct fmt::formatter<ObjectType> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
    template<typename FormatContext>
    auto format(ObjectType const& value, FormatContext& ctx)
    {
        switch (value) {
        case ObjectType::STRING:
            return fmt::format_to(ctx.out(), "ObjectType::STRING");
        case ObjectType::FUNCTION:
            return fmt::format_to(ctx.out(), "ObjectType::FUNCTION");
        case ObjectType::CLOSURE:
            return fmt::format_to(ctx.out(), "ObjectType::CLOSURE");
        case ObjectType::NATIVE_FUNCTION:
            return fmt::format_to(ctx.out(), "ObjectType::NATIVE_FUNCTION");
        case ObjectType::UPVALUE:
            return fmt::format_to(ctx.out(), "ObjectType::UPVALUE");
        case ObjectType::CLASS:
            return fmt::format_to(ctx.out(), "ObjectType::CLASS");
        case ObjectType::INSTANCE:
            return fmt::format_to(ctx.out(), "ObjectType::INSTANCE");
        case ObjectType::BOUND_METHOD:
            return fmt::format_to(ctx.out(), "ObjectType::BOUND_METHOD");
        }
    }
};

struct Object {
    [[nodiscard]] auto GetType() const -> ObjectType
    {
        return type;
    }

    inline void MarkObjectAsReachable() const
    {
        GCDebugLog("Marking object(type={}) at:{} as reachable", type, static_cast<void const*>(this));
        marked = true;
    }

    Object() = delete;

    Object(ObjectType type)
        : type(type)
    {
    }

private:
    friend class Heap;
    ObjectType type {};
    Object* next = nullptr;
    mutable bool marked = false;
};

struct StringObject : public Object {
    StringObject()
        : Object(ObjectType::STRING)
    {
    }
    StringObject(std::string_view d)
        : Object(ObjectType::STRING)
        , data(d)
    {
    }
    std::string data;
};

struct FunctionObject : public Object {
    FunctionObject()
        : Object(ObjectType::FUNCTION)
    {
    }
    FunctionObject(std::string_view fn_name, uint32_t a)
        : Object(ObjectType::FUNCTION)
        , function_name(fn_name)
        , arity(a)
    {
    }

    std::string function_name {};
    uint32_t arity {};
    Chunk chunk {};
    uint16_t upvalue_count {};
};

using NativeFunction = std::add_pointer_t<RuntimeErrorOr<Value>(uint32_t num_arguments, Value*)>;
struct NativeFunctionObject : public Object {
    NativeFunctionObject()
        : Object(ObjectType::NATIVE_FUNCTION)
    {
    }
    NativeFunction native_function { nullptr };
};

struct UpvalueObject : public Object {
    UpvalueObject()
        : Object(ObjectType::UPVALUE)
    {
    }
    void Close(Value const& value)
    {
        m_data = value;
    }
    bool IsClosed() const
    {
        return std::holds_alternative<Value>(m_data);
    }
    Value GetClosedValue() const
    {
        LOX_ASSERT(IsClosed());
        return *std::get_if<Value>(&m_data);
    }
    void SetClosedValue(Value const& value)
    {
        LOX_ASSERT(IsClosed());
        m_data = value;
    }
    uint16_t GetStackIndex() const
    {
        LOX_ASSERT(!IsClosed());
        return *std::get_if<uint16_t>(&m_data);
    }
    void SetStackIndex(uint16_t stack_index)
    {
        m_data = stack_index;
    }

private:
    std::variant<Value, uint16_t> m_data {};
};

struct ClosureObject : public Object {
    ClosureObject()
        : Object(ObjectType::CLOSURE)
    {
    }
    std::vector<UpvalueObject*> upvalues {};
    FunctionObject* function = nullptr;
};

struct ClassObject : public Object {
    ClassObject()
        : Object(ObjectType::CLASS)
    {
    }
    ClassObject(std::string_view cls_name)
        : Object(ObjectType::CLASS)
        , class_name(cls_name)
    {
    }
    std::unordered_map<std::string, ClosureObject*> methods;
    std::string class_name;
};

struct InstanceObject : public Object {
    InstanceObject()
        : Object(ObjectType::INSTANCE)
    {
    }
    InstanceObject(ClassObject* cls)
        : Object(ObjectType::INSTANCE)
        , class_(cls)
    {
    }
    ClassObject* class_ = nullptr;
    Table fields {};
};

struct BoundMethodObject : public Object {
    BoundMethodObject()
        : Object(ObjectType::BOUND_METHOD)
    {
    }
    InstanceObject* receiver {};
    ClosureObject* method {};
};
#endif // LOX_CPP_OBJECT_H
