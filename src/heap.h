//
// Created by kevin on 8/29/22.
//

#ifndef LOX_CPP_HEAP_H
#define LOX_CPP_HEAP_H

#include "object.h"

class VirtualMachine;

class Heap {
public:
    Heap(VirtualMachine& vm)
        : m_vm(vm)
    {
    }
    ~Heap();
    [[nodiscard]] auto AllocateStringObject(std::string_view) -> StringObject*;
    [[nodiscard]] auto AllocateFunctionObject(std::string_view function_name, uint32_t arity) -> FunctionObject*;
    [[nodiscard]] auto AllocateClosureObject(FunctionObject const* function) -> ClosureObject*;
    [[nodiscard]] auto AllocateNativeFunctionObject(NativeFunction) -> NativeFunctionObject*;
    [[nodiscard]] auto AllocateNativeUpvalueObject() -> UpvalueObject*;

protected:
    auto reset() -> void;
    [[nodiscard]] auto allocateObject(ObjectType) -> Object*;
    auto freeObject(Object* object) -> void;
    auto insertAtHead(Object* new_node) -> void;
    // GC related member functions
    auto collectGarbage() -> void;
    auto markRoots() -> void;

protected:
    Object* m_head = nullptr;
    VirtualMachine& m_vm;
};

#endif // LOX_CPP_HEAP_H
