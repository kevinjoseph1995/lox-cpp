//
// Created by kevin on 8/29/22.
//

#include "heap.h"
#include "error.h"
#include "object.h"
#include <stdexcept>
#include <string_view>

auto Heap::Reset() -> void
{
    auto current = m_head;
    while (current != nullptr) {
        auto next = current->next;
        switch (current->type) {
        case ObjectType::STRING: {
            delete static_cast<StringObject*>(current);
            break;
        }
        case ObjectType::FUNCTION: {
            delete static_cast<FunctionObject*>(current);
            break;
        }
        case ObjectType::CLOSURE: {
            delete static_cast<ClosureObject*>(current);
            break;
        }
        case ObjectType::NATIVE_FUNCTION: {
            delete static_cast<NativeFunctionObject*>(current);
            break;
        }
        }
        current = next;
    }
    m_head = nullptr;
}

auto Heap::Allocate(ObjectType type) -> Object*
{
    Object* new_object = nullptr;
    switch (type) {
    case ObjectType::STRING: {
        new_object = new StringObject;
        break;
    }
    case ObjectType::FUNCTION: {
        new_object = new FunctionObject;
        break;
    }
    case ObjectType::CLOSURE:
        new_object = new ClosureObject;
        break;
    case ObjectType::NATIVE_FUNCTION:
        new_object = new NativeFunctionObject;
        break;
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

auto Heap::insertAtHead(Object* new_node) -> void
{
    LOX_ASSERT(new_node != nullptr);
    if (m_head == nullptr) {
        m_head = new_node;
    } else {
        new_node->next = m_head;
        m_head = new_node;
    }
}

auto Heap::AllocateStringObject(std::string_view string_data) -> StringObject*
{
    auto* object_ptr = Allocate(ObjectType::STRING);
    LOX_ASSERT(object_ptr->type == ObjectType::STRING);
    auto string_object_ptr = static_cast<StringObject*>(object_ptr);
    string_object_ptr->data = string_data;
    return string_object_ptr;
}

auto Heap::AllocateFunctionObject(std::string_view function_name, uint32_t arity) -> FunctionObject*
{
    auto* object_ptr = Allocate(ObjectType::FUNCTION);
    LOX_ASSERT(object_ptr->type == ObjectType::FUNCTION);
    auto function_object_ptr = static_cast<FunctionObject*>(object_ptr);
    function_object_ptr->function_name = function_name;
    function_object_ptr->arity = arity;
    return function_object_ptr;
}

auto Heap::AllocateClosureObject(FunctionObject const* function) -> ClosureObject*
{
    auto* object_ptr = Allocate(ObjectType::CLOSURE);
    LOX_ASSERT(object_ptr->type == ObjectType::CLOSURE);
    auto closure_object_ptr = static_cast<ClosureObject*>(object_ptr);
    closure_object_ptr->function = function;
    return closure_object_ptr;
}

auto Heap::AllocateNativeFunctionObject(NativeFunction function) -> NativeFunctionObject*
{
    auto* object_ptr = Allocate(ObjectType::NATIVE_FUNCTION);
    LOX_ASSERT(object_ptr->type == ObjectType::NATIVE_FUNCTION);
    auto native_function_object_ptr = static_cast<NativeFunctionObject*>(object_ptr);
    native_function_object_ptr->native_function = function;
    return native_function_object_ptr;
}
