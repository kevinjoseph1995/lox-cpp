//
// Created by kevin on 8/6/22.
//

#ifndef LOX_CPP_VIRTUAL_MACHINE_H
#define LOX_CPP_VIRTUAL_MACHINE_H
#include <cstdint>
#include <stack>
#include <unordered_map>

#include "chunk.h"
#include "compiler.h"
#include "heap.h"
#include "source.h"

using GlobalTable = std::unordered_map<std::string, Value>;

class VirtualMachine {
public:
    VirtualMachine();

    [[nodiscard]] ErrorOr<VoidType> Interpret(Source const& source_code);
    void SetExternalOutputStringStream(std::string* external_stream);

private:
    [[nodiscard]] bool isAtEnd();
    [[nodiscard]] ErrorOr<VoidType> run();
    [[nodiscard]] uint8_t readByte();
    [[nodiscard]] Value readConstant();
    [[nodiscard]] uint16_t readConstantPoolIndex();
    [[nodiscard]] Value popStack();
    Value const& peekStack(uint32_t index_from_top);
    [[nodiscard]] ErrorOr<VoidType> binaryOperation(OpCode op);
    Error runtimeError(std::string error_message);

private:
    Chunk m_current_chunk {};
    uint64_t m_instruction_pointer = 0;
    std::unique_ptr<Compiler> m_compiler = nullptr;
    std::string* m_external_stream = nullptr;

    Heap m_heap {};
    std::vector<Value> m_value_stack;
    GlobalTable m_globals;
};

#endif // LOX_CPP_VIRTUAL_MACHINE_H
