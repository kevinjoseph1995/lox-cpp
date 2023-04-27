//
// Created by kevin on 8/29/22.
//

#ifndef LOX_CPP_HEAP_H
#define LOX_CPP_HEAP_H

#include "error.h"
#include "object.h"

#include <cstdint>
#include <vector>

class VirtualMachine;
class Compiler;

class Heap {
public:
    Heap(VirtualMachine& vm);
    ~Heap();
    [[nodiscard]] auto AllocateStringObject(std::string_view) -> StringObject*;
    [[nodiscard]] auto AllocateFunctionObject(std::string_view function_name, uint32_t arity) -> FunctionObject*;
    [[nodiscard]] auto AllocateClosureObject(FunctionObject* function) -> ClosureObject*;
    [[nodiscard]] auto AllocateNativeFunctionObject(NativeFunction) -> NativeFunctionObject*;
    [[nodiscard]] auto AllocateNativeUpvalueObject() -> UpvalueObject*;
    [[nodiscard]] auto AllocateClassObject(std::string_view class_name) -> ClassObject*;
    auto SetCompilerContext(Compiler* current_compiler) -> void;

protected:
    auto reset() -> void;
    [[nodiscard]] auto allocateObject(ObjectType) -> Object*;
    auto freeObject(Object* object) -> void;
    auto insertAtHead(Object* new_node) -> Object*;
    // GC related member functions
    auto collectGarbage() -> void;
    auto markRoots() -> void;
    auto markRoot(Object* object_ptr) -> void;
    auto markRoot(Value value) -> void;
    auto traceObjects() -> void;
    auto blackenObject(Object* object) -> void;
    auto sweep() -> void;

protected:
    Compiler* m_current_compiler = nullptr; // Set the current function that's being compiled
    uint64_t m_number_of_heap_objects_allocated = 0;
    uint64_t m_bytes_allocated = 0;
    uint64_t m_next_collection_threhold = 1024U; // 1KB
    Object* m_head = nullptr;
    VirtualMachine& m_vm;
    std::vector<Object*> m_greyed_objects {}; // Refer 26.4.1 : The tricolor abstraction from https://craftinginterpreters.com/garbage-collection.html#tracing-object-references
};

class HeapContextManager {
public:
    HeapContextManager(Heap& heap, Compiler* current, Compiler* new_compiler);
    ~HeapContextManager();

private:
    Heap& m_heap;
    Compiler* m_current = nullptr;
};

#endif // LOX_CPP_HEAP_H
