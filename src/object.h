//
// Created by kevin on 8/28/22.
//

#ifndef LOX_CPP_OBJECT_H
#define LOX_CPP_OBJECT_H

#include <cstdint>
#include <string>

enum class ObjectType {
    STRING,
};

struct Object {
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

#endif // LOX_CPP_OBJECT_H
