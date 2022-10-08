//
// Created by kevin on 8/6/22.
//

#ifndef LOX_CPP_VIRTUAL_MACHINE_H
#define LOX_CPP_VIRTUAL_MACHINE_H
#include <cstdint>
#include <memory>
#include <stack>
#include <unordered_map>

#include "chunk.h"
#include "compiler.h"
#include "error.h"
#include "heap.h"
#include "source.h"

using GlobalTable = std::unordered_map<std::string, Value>;

class VirtualMachine {
public:
    VirtualMachine(std::string* external_stream = nullptr);

    [[nodiscard]] ErrorOr<VoidType> Interpret(Source const& source_code);

private:
    [[nodiscard]] auto isAtEnd() -> bool;
    [[nodiscard]] auto run() -> ErrorOr<VoidType>;
    [[nodiscard]] auto readByte() -> uint8_t;
    [[nodiscard]] auto readConstant() -> Value;
    [[nodiscard]] auto readIndex() -> uint16_t;
    [[nodiscard]] auto popStack() -> Value;
    [[nodiscard]] auto peekStack(uint32_t index_from_top) -> Value const&;
    [[nodiscard]] auto binaryOperation(OpCode op) -> ErrorOr<VoidType>;
    [[nodiscard]] auto runtimeError(std::string error_message) -> Error;

private:
    Chunk m_current_chunk {};
    uint64_t m_instruction_pointer = 0;
    std::unique_ptr<Compiler> m_compiler = nullptr;
    std::string* const m_external_stream = nullptr;

    Heap m_heap {};
    std::vector<Value> m_value_stack;
    GlobalTable m_globals;
};

#endif // LOX_CPP_VIRTUAL_MACHINE_H
