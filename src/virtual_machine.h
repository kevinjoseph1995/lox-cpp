//
// Created by kevin on 8/6/22.
//

#ifndef LOX_CPP_VIRTUAL_MACHINE_H
#define LOX_CPP_VIRTUAL_MACHINE_H

#include "chunk.h"
#include "compiler.h"

#include <cstdint>
#include <stack>

static constexpr auto MAX_STACK_SIZE = 256;

class VirtualMachine {
public:
    /**
     * Reset virtual machine state
     */
    [[maybe_unused]] void Reset();

    /**
     *
     * @return
     */
    [[nodiscard]] ErrorOr<VoidType> Interpret(std::string const& source_code);

private:
    [[nodiscard]] ErrorOr<VoidType> run();
    [[nodiscard]] uint8_t readByte();
    [[nodiscard]] Value readConstant();
    [[nodiscard]] Value popStack();
    [[nodiscard]] ErrorOr<VoidType> binaryOperation(OpCode op);

private:
    Chunk m_current_chunk {};
    uint64_t m_instruction_pointer = 0;

    std::vector<Value> m_value_stack;

    Compiler m_compiler;
};

#endif // LOX_CPP_VIRTUAL_MACHINE_H
