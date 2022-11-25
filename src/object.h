//
// Created by kevin on 8/28/22.
//

#ifndef LOX_CPP_OBJECT_H
#define LOX_CPP_OBJECT_H

#include "chunk.h"
#include <string>

enum class ObjectType {
    STRING,
    FUNCTION,
    CLOSURE,
    NATIVE_FUNCTION
};

class Object {
public:
    [[nodiscard]] auto GetType() const -> ObjectType
    {
        return type;
    }

protected:
    Object() = default;
    friend class Heap;
    ObjectType type;
    Object* next = nullptr;
};

struct StringObject : public Object {
    StringObject()
    {
        type = ObjectType::STRING;
    }
    std::string data;
};

struct FunctionObject : public Object {
    FunctionObject()
    {
        type = ObjectType::FUNCTION;
    }
    std::string function_name;
    uint32_t arity;
    Chunk chunk;
};

using NativeFunction = std::add_pointer_t<RuntimeErrorOr<Value>(uint32_t num_arguments, Value*)>;
struct NativeFunctionObject : public Object {
    NativeFunctionObject()
    {
        type = ObjectType::NATIVE_FUNCTION;
    }
    NativeFunction native_function;
};

struct ClosureObject : Object {
    ClosureObject()
    {
        type = ObjectType::CLOSURE;
    }
    FunctionObject const* function = nullptr;
};
#endif // LOX_CPP_OBJECT_H
