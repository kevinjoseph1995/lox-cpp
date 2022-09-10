//
// Created by kevin on 8/28/22.
//

#ifndef LOX_CPP_OBJECT_H
#define LOX_CPP_OBJECT_H

#include <string>

enum class ObjectType {
    STRING,
};

class Object {
public:
    [[nodiscard]] ObjectType GetType() const
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

#endif // LOX_CPP_OBJECT_H
