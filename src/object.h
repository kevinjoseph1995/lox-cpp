//
// Created by kevin on 8/28/22.
//

#ifndef LOX_CPP_OBJECT_H
#define LOX_CPP_OBJECT_H

#include <cstdint>
#include <string>
#include <variant>

#include "chunk.h"
#include "error.h"
#include "value.h"

enum class ObjectType {
    STRING,
    FUNCTION,
    CLOSURE,
    NATIVE_FUNCTION,
    UPVALUE
};

struct Object {
public:
    [[nodiscard]] auto GetType() const -> ObjectType
    {
        return type;
    }

protected:
    Object() = delete;
    Object(ObjectType type)
        : type(type)
    {
    }
    friend class Heap;
    ObjectType type;
    Object* next = nullptr;
};

struct StringObject : public Object {
    StringObject()
        : Object(ObjectType::STRING)
    {
    }
    std::string data;
};

struct FunctionObject : public Object {
    FunctionObject()
        : Object(ObjectType::FUNCTION)
    {
    }
    std::string function_name;
    uint32_t arity {};
    Chunk chunk;
    uint16_t upvalue_count {};
};

using NativeFunction = std::add_pointer_t<RuntimeErrorOr<Value>(uint32_t num_arguments, Value*)>;
struct NativeFunctionObject : public Object {
    NativeFunctionObject()
        : Object(ObjectType::NATIVE_FUNCTION)
    {
    }
    NativeFunction native_function;
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
    std::variant<Value, uint16_t> m_data;
};

struct ClosureObject : Object {
    ClosureObject()
        : Object(ObjectType::CLOSURE)
    {
    }
    std::vector<UpvalueObject*> upvalues;
    FunctionObject const* function = nullptr;
};
#endif // LOX_CPP_OBJECT_H
