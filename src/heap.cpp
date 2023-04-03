//
// Created by kevin on 8/29/22.
//

#include "heap.h"
#include "error.h"
#include "fmt/core.h"
#include "object.h"
#include "virtual_machine.h"

#include <cstddef>
#include <cstdio>
#include <stdexcept>
#include <string_view>
#include <utility>

#define STRESS_TEST_GC 1
#define DEBUG_GC_LOGGING 1

template <typename... T>
static inline void GCDebugLog(fmt::format_string<T...> fmt, T&&... args)
{
#ifdef DEBUG_GC_LOGGING
    fmt::print("[GC][Debug]: ");
    fmt::print(fmt, std::forward<T>(args)...);
    fmt::print("\n");
    fflush(stdout);
#endif
}

Heap::~Heap()
{
    reset();
}

auto Heap::reset() -> void
{
    auto current = m_head;
    while (current != nullptr) {
        auto next = current->next;
        freeObject(current);
        current = next;
    }
    m_head = nullptr;
}

auto Heap::AllocateStringObject(std::string_view string_data) -> StringObject*
{
    auto* object_ptr = allocateObject(ObjectType::STRING);
    LOX_ASSERT(object_ptr->type == ObjectType::STRING);
    auto string_object_ptr = static_cast<StringObject*>(object_ptr);
    string_object_ptr->data = string_data;
    return string_object_ptr;
}

auto Heap::AllocateFunctionObject(std::string_view function_name, uint32_t arity) -> FunctionObject*
{
    auto* object_ptr = allocateObject(ObjectType::FUNCTION);
    LOX_ASSERT(object_ptr->type == ObjectType::FUNCTION);
    auto function_object_ptr = static_cast<FunctionObject*>(object_ptr);
    function_object_ptr->function_name = function_name;
    function_object_ptr->arity = arity;
    return function_object_ptr;
}

auto Heap::AllocateClosureObject(FunctionObject const* function) -> ClosureObject*
{
    auto* object_ptr = allocateObject(ObjectType::CLOSURE);
    LOX_ASSERT(object_ptr->type == ObjectType::CLOSURE);
    auto closure_object_ptr = static_cast<ClosureObject*>(object_ptr);
    closure_object_ptr->function = function;
    return closure_object_ptr;
}

auto Heap::AllocateNativeFunctionObject(NativeFunction function) -> NativeFunctionObject*
{
    auto* object_ptr = allocateObject(ObjectType::NATIVE_FUNCTION);
    LOX_ASSERT(object_ptr->type == ObjectType::NATIVE_FUNCTION);
    auto native_function_object_ptr = static_cast<NativeFunctionObject*>(object_ptr);
    native_function_object_ptr->native_function = function;
    return native_function_object_ptr;
}
auto Heap::AllocateNativeUpvalueObject() -> UpvalueObject*
{
    auto* object_ptr = allocateObject(ObjectType::UPVALUE);
    LOX_ASSERT(object_ptr->type == ObjectType::UPVALUE);
    auto upvalue_obj_ptr = static_cast<UpvalueObject*>(object_ptr);
    return upvalue_obj_ptr;
}

auto Heap::allocateObject(ObjectType type) -> Object*
{
#ifdef STRESS_TEST_GC
    collectGarbage();
#endif

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
    case ObjectType::UPVALUE:
        new_object = new UpvalueObject;
        break;
    }
    LOX_ASSERT(new_object != nullptr);
    LOX_ASSERT(new_object->GetType() == type);
    insertAtHead(new_object);
    return new_object;
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

auto Heap::collectGarbage() -> void
{
    GCDebugLog("Begining garbage collection");
}

auto Heap::freeObject(Object* object) -> void
{
    switch (object->type) {
    case ObjectType::STRING: {
        GCDebugLog("Freeing object of type STRING");
        delete static_cast<StringObject*>(object);
        break;
    }
    case ObjectType::FUNCTION: {
        GCDebugLog("Freeing object of type FUNCTION");
        delete static_cast<FunctionObject*>(object);
        break;
    }
    case ObjectType::CLOSURE: {
        GCDebugLog("Freeing object of type CLOSURE");
        delete static_cast<ClosureObject*>(object);
        break;
    }
    case ObjectType::NATIVE_FUNCTION: {
        GCDebugLog("Freeing object of type NATIVE_FUNCTION");
        delete static_cast<NativeFunctionObject*>(object);
        break;
    }
    case ObjectType::UPVALUE:
        GCDebugLog("Freeing object of type UPVALUE");
        delete static_cast<UpvalueObject*>(object);
        break;
    }
}

auto Heap::markRoots() -> void
{

    // Mark all the values on the stack as reachable
    for (auto& value : m_vm.m_value_stack) {
        if (value.IsObject()) {
            value.AsObject().markObjectAsReachable();
        }
    }
}
