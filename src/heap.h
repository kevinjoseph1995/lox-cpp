//
// Created by kevin on 8/29/22.
//

#ifndef LOX_CPP_HEAP_H
#define LOX_CPP_HEAP_H

#include "object.h"

class Heap {
public:
    ~Heap();
    void Reset();
    [[nodiscard]] Object* Allocate(ObjectType);
    [[nodiscard]] StringObject* AllocateStringObject();

private:
    [[nodiscard]] Object* allocate(ObjectType);
    void insertAtHead(Object* new_node);

private:
    Object* m_head = nullptr;
};

#endif // LOX_CPP_HEAP_H
