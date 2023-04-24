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

static constexpr auto HEAP_GROW_FACTOR = 2;

Heap::Heap(VirtualMachine& vm)
    : m_vm(vm)
{
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
    m_bytes_allocated += sizeof(StringObject);
    return string_object_ptr;
}

auto Heap::AllocateFunctionObject(std::string_view function_name, uint32_t arity) -> FunctionObject*
{
    auto* object_ptr = allocateObject(ObjectType::FUNCTION);
    LOX_ASSERT(object_ptr->type == ObjectType::FUNCTION);
    auto function_object_ptr = static_cast<FunctionObject*>(object_ptr);
    function_object_ptr->function_name = function_name;
    function_object_ptr->arity = arity;
    m_bytes_allocated += sizeof(FunctionObject);
    return function_object_ptr;
}

auto Heap::AllocateClosureObject(FunctionObject* function) -> ClosureObject*
{
    auto* object_ptr = allocateObject(ObjectType::CLOSURE);
    LOX_ASSERT(object_ptr->type == ObjectType::CLOSURE);
    auto closure_object_ptr = static_cast<ClosureObject*>(object_ptr);
    closure_object_ptr->function = function;
    m_bytes_allocated += sizeof(ClosureObject);
    return closure_object_ptr;
}

auto Heap::AllocateNativeFunctionObject(NativeFunction function) -> NativeFunctionObject*
{
    auto* object_ptr = allocateObject(ObjectType::NATIVE_FUNCTION);
    LOX_ASSERT(object_ptr->type == ObjectType::NATIVE_FUNCTION);
    auto native_function_object_ptr = static_cast<NativeFunctionObject*>(object_ptr);
    native_function_object_ptr->native_function = function;
    m_bytes_allocated += sizeof(NativeFunctionObject);
    return native_function_object_ptr;
}
auto Heap::AllocateNativeUpvalueObject() -> UpvalueObject*
{
    auto* object_ptr = allocateObject(ObjectType::UPVALUE);
    LOX_ASSERT(object_ptr->type == ObjectType::UPVALUE);
    auto upvalue_obj_ptr = static_cast<UpvalueObject*>(object_ptr);
    m_bytes_allocated += sizeof(UpvalueObject);
    return upvalue_obj_ptr;
}

auto Heap::allocateObject(ObjectType type) -> Object*
{
#ifdef STRESS_TEST_GC
    collectGarbage();
#else
    if (m_bytes_allocated > m_next_collection_threhold) {
        collectGarbage();
    }
#endif
    ++m_number_of_heap_objects_allocated;
    return insertAtHead([type]() -> Object* {
        switch (type) {
        case ObjectType::STRING:
            GCDebugLog("Heap::allocateObject ObjectType::STRING");
            return new StringObject;
        case ObjectType::FUNCTION:
            GCDebugLog("Heap::allocateObject ObjectType::FUNCTION");
            return new FunctionObject;
        case ObjectType::CLOSURE:
            GCDebugLog("Heap::allocateObject ObjectType::CLOSURE");
            return new ClosureObject;
        case ObjectType::NATIVE_FUNCTION:
            GCDebugLog("Heap::allocateObject ObjectType::NATIVE_FUNCTION");
            return new NativeFunctionObject;
        case ObjectType::UPVALUE:
            GCDebugLog("Heap::allocateObject ObjectType::UPVALUE");
            return new UpvalueObject;
        }
    }());
}

auto Heap::insertAtHead(Object* new_node) -> Object*
{
    LOX_ASSERT(new_node != nullptr);
    if (m_head == nullptr) {
        m_head = new_node;
    } else {
        new_node->next = m_head;
        m_head = new_node;
    }
    return new_node;
}

auto Heap::collectGarbage() -> void
{
    GCDebugLog("[START]collectGarbage | Total number of allocated objects:{}", m_number_of_heap_objects_allocated);
    markRoots();
    traceObjects();
    sweep();
    m_next_collection_threhold = HEAP_GROW_FACTOR * m_next_collection_threhold;
    GCDebugLog("[END]collectGarbage");
}

auto Heap::sweep() -> void
{
    GCDebugLog("[START] sweep");
    auto currentObject = m_head;
    auto previousObject = static_cast<Object*>(nullptr);
    while (currentObject != nullptr) {
        // Iterate over our linked list of objects
        if (currentObject->marked) {
            // Has been marked reachable, carry on
            currentObject->marked = false;
            previousObject = currentObject;
            currentObject = currentObject->next;
        } else {
            // Not marked as reachable, we must free this object
            auto* unreachable = currentObject;
            currentObject = currentObject->next;
            if (previousObject != nullptr) {
                previousObject->next = unreachable->next;
            } else {
                m_head = unreachable->next;
            }
            freeObject(unreachable);
        }
    }

    GCDebugLog("[END] sweep");
}

auto Heap::freeObject(Object* object) -> void
{
    LOX_ASSERT(m_number_of_heap_objects_allocated > 0, "Precondition failed");
    --m_number_of_heap_objects_allocated;
    switch (object->type) {
    case ObjectType::STRING: {
        GCDebugLog("Freeing object of type STRING");
        delete static_cast<StringObject*>(object);
        m_bytes_allocated -= sizeof(StringObject);
        break;
    }
    case ObjectType::FUNCTION: {
        GCDebugLog("Freeing object of type FUNCTION");
        delete static_cast<FunctionObject*>(object);
        m_bytes_allocated -= sizeof(FunctionObject);
        break;
    }
    case ObjectType::CLOSURE: {
        GCDebugLog("Freeing object of type CLOSURE");
        delete static_cast<ClosureObject*>(object);
        m_bytes_allocated -= sizeof(ClosureObject);
        break;
    }
    case ObjectType::NATIVE_FUNCTION: {
        GCDebugLog("Freeing object of type NATIVE_FUNCTION");
        delete static_cast<NativeFunctionObject*>(object);
        m_bytes_allocated -= sizeof(NativeFunctionObject);
        break;
    }
    case ObjectType::UPVALUE:
        GCDebugLog("Freeing object of type UPVALUE");
        delete static_cast<UpvalueObject*>(object);
        m_bytes_allocated -= sizeof(UpvalueObject);
        break;
    }
}

auto Heap::markRoot(Object* object_ptr) -> void
{
    LOX_ASSERT(object_ptr != nullptr, "Failed Precondition");
    object_ptr->MarkObjectAsReachable();
    m_greyed_objects.push_back(object_ptr);
}

auto Heap::markRoot(Value value) -> void
{
    if (value.IsObject() && !value.AsObject().marked) {
        markRoot(value.AsObjectPtr());
    }
}

auto Heap::markRoots() -> void
{
    GCDebugLog("[START]markRoots");
    // Mark all the values on the stack as reachable
    for (auto& value : m_vm.m_value_stack) {
        markRoot(value);
    }

    // Mark all globals as reachable
    for (auto& [_, global_value] : m_vm.m_globals) {
        markRoot(global_value);
    }

    // Mark the call frame closures
    for (auto& call_frame : m_vm.m_frames) {
        markRoot(call_frame.closure);
    }

    // Mark open upvalues
    for (auto& upvalue : m_vm.m_open_upvalues) {
        markRoot(upvalue);
    }

    // Mark all the roots for objects that originate during the compilation phase
    auto current_compiler = m_current_compiler;
    while (current_compiler != nullptr) {
        markRoot(current_compiler->m_function);
        current_compiler = current_compiler->m_parent_compiler;
    }
    GCDebugLog("[END]markRoots");
}

auto Heap::traceObjects() -> void
{
    GCDebugLog("[START]traceObjects");
    while (not m_greyed_objects.empty()) {
        auto* object = m_greyed_objects.back();
        m_greyed_objects.pop_back();
        blackenObject(object);
    }
    GCDebugLog("[END]traceObjects");
}

auto Heap::blackenObject(Object* object) -> void
{
    GCDebugLog("[START]blackenObject");
    LOX_ASSERT(object != nullptr);
    switch (object->type) {
    case ObjectType::STRING:
    case ObjectType::NATIVE_FUNCTION:
        break; // No outgoing references nothing to do
    case ObjectType::UPVALUE: {
        auto upvalue = static_cast<UpvalueObject*>(object);
        if (upvalue->IsClosed()) {
            markRoot(upvalue->GetClosedValue());
        }
        break;
    }
    case ObjectType::FUNCTION: {
        auto function = static_cast<FunctionObject*>(object);
        for (auto& constant : function->chunk.constant_pool) {
            markRoot(constant);
        }
        break;
    }
    case ObjectType::CLOSURE: {
        auto closure = static_cast<ClosureObject*>(object);
        markRoot(closure->function);
        for (auto* upvalue : closure->upvalues) {
            markRoot(upvalue);
        }
        break;
    }
    }
    GCDebugLog("[END]blackenObject");
}

auto Heap::SetCompilerContext(Compiler* compiler) -> void
{
    m_current_compiler = compiler;
}

HeapContextManager::HeapContextManager(Heap& heap, Compiler* current, Compiler* new_compiler)
    : m_heap(heap)
    , m_current(current)
{
    LOX_ASSERT(current != nullptr);
    LOX_ASSERT(new_compiler != nullptr);
    m_heap.SetCompilerContext(new_compiler);
}

HeapContextManager::~HeapContextManager()
{
    // Restore the original compiler
    m_heap.SetCompilerContext(m_current);
}