//
// Created by kevin on 8/28/22.
//

#ifndef LOX_CPP_OBJECT_H
#define LOX_CPP_OBJECT_H

#include "chunk.h"
#include "error.h"
#include "value.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

enum class ObjectType {
    STRING,
    FUNCTION,
    CLOSURE,
    NATIVE_FUNCTION,
    UPVALUE
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
#endif // LOX_CPP_OBJECT_H
