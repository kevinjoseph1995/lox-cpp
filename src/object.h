//
// Created by kevin on 8/28/22.
//

#ifndef LOX_CPP_OBJECT_H
#define LOX_CPP_OBJECT_H

#include "chunk.h"
#include <string>

enum class ObjectType {
    STRING,
    FUNCTION
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
    FunctionObject() {
        type = ObjectType::FUNCTION;
    }
    std::string function_name;
    uint32_t arity;
    Chunk chunk;
};

#endif // LOX_CPP_OBJECT_H
