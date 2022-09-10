//
// Created by kevin on 8/29/22.
//

#include "heap.h"
#include "error.h"

void Heap::Reset()
{
    auto current = m_head;
    while (current != nullptr) {
        auto next = current->next;
        switch (current->type) {
        case ObjectType::STRING:
            delete static_cast<StringObject*>(current);
            break;
        }
        current = next;
    }
}

Object* Heap::Allocate(ObjectType type)
{
    return allocate(type);
}

Object* Heap::allocate(ObjectType type)
{
    Object* new_object = nullptr;
    switch (type) {
    case ObjectType::STRING: {
        new_object = new StringObject;
        break;
    }
    }
    LOX_ASSERT(new_object != nullptr);
    LOX_ASSERT(new_object->GetType() == type);
    insertAtHead(new_object);
    return new_object;
}

Heap::~Heap()
{
    Reset();
}

void Heap::insertAtHead(Object* new_node)
{
    LOX_ASSERT(new_node != nullptr);
    if (m_head == nullptr) {
        m_head = new_node;
    } else {
        new_node->next = m_head;
        m_head = new_node;
    }
}

StringObject* Heap::AllocateStringObject()
{
    auto* object_ptr = allocate(ObjectType::STRING);
    LOX_ASSERT(object_ptr->type == ObjectType::STRING);
    return static_cast<StringObject*>(object_ptr);
}
