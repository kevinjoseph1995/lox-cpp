//
// Created by kevin on 8/29/22.
//

#ifndef LOX_CPP_HEAP_H
#define LOX_CPP_HEAP_H

#include "object.h"

class Heap {
public:
    ~Heap();
    auto Reset() -> void;
    [[nodiscard]] auto Allocate(ObjectType) -> Object*;
    [[nodiscard]] auto AllocateStringObject(std::string_view) -> StringObject*;
    [[nodiscard]] auto AllocateFunctionObject(std::string_view function_name, uint32_t arity) -> FunctionObject*;
    [[nodiscard]] auto AllocateClosureObject(FunctionObject const* function) -> ClosureObject*;
    [[nodiscard]] auto AllocateNativeFunctionObject(NativeFunction) -> NativeFunctionObject*;
    [[nodiscard]] auto AllocateNativeUpvalueObject() -> UpvalueObject*;

private:
    auto insertAtHead(Object* new_node) -> void;

private:
    Object* m_head = nullptr;
};

#endif // LOX_CPP_HEAP_H
