//
// Created by kevin on 8/6/22.
//

#include "virtual_machine.h"
#include "error.h"
#include <fmt/core.h>

#define DEBUG_TRACE_EXECUTION

ErrorOr<VoidType> VirtualMachine::Interpret(std::string const* source_code)
{
    m_compiler.CompileSource(source_code);
    return VoidType {};
}

[[maybe_unused]] ErrorOr<VoidType> VirtualMachine::run()
{
    LOX_ASSERT(m_current_chunk != nullptr);
    LOX_ASSERT(m_instruction_pointer == 0);

    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
        Disassemble_instruction(*m_current_chunk, m_instruction_pointer);
#endif
        auto const instruction = this->read_byte();
        switch (instruction) {
        case OP_RETURN: {
            // TODO
        }
        case OP_CONSTANT: {
            auto constant = this->read_constant();
            fmt::print("{}\n", constant);
            break;
        }
        default: {
            // TODO
        }
        }
    }
    return VoidType {};
}

uint8_t VirtualMachine::read_byte()
{
    auto current_byte = m_current_chunk->byte_code[m_instruction_pointer];
    ++m_instruction_pointer;
    return current_byte;
}

Value VirtualMachine::read_constant()
{
    auto const constant_index = this->read_byte();
    return m_current_chunk->constant_pool[constant_index];
}

void VirtualMachine::Reset()
{
    m_current_chunk = nullptr;
    m_instruction_pointer = 0;
    m_value_stack.clear();
}
