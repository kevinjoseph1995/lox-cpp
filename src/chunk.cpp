//
// Created by kevin on 8/6/22.
//
#include "chunk.h"

#include "fmt/core.h"

#include "error.h"

void Disassemble_chunk(Chunk const& chunk)
{
    uint64_t offset = 0;
    auto const number_of_instructions = chunk.byte_code.size();
    while (offset < number_of_instructions) {
        offset = Disassemble_instruction(chunk, offset);
    }
}

uint64_t Disassemble_instruction(Chunk const& chunk, uint64_t offset)
{
    auto const opcode = static_cast<OpCode>(chunk.byte_code[offset]);
    switch (opcode) {
    case OP_RETURN:
        fmt::print("{:#08x} OP_RETURN\n", offset);
        return ++offset;
    case OP_CONSTANT:
        fmt::print("{:#08x} OP_CONSTANT {}\n", offset, chunk.byte_code[offset + 1]);
        offset += 2;
        return offset;
    case OP_NEGATE:
        fmt::print("{:#08x} OP_NEGATE\n", offset);
        return ++offset;
    case OP_ADD:
        fmt::print("{:#08x} OP_ADD\n", offset);
        return ++offset;
    case OP_SUBTRACT:
        fmt::print("{:#08x} OP_SUBTRACT\n", offset);
        return ++offset;
    case OP_MULTIPLY:
        fmt::print("{:#08x} OP_MULTIPLY\n", offset);
        return ++offset;
    case OP_DIVIDE:
        fmt::print("{:#08x} OP_DIVIDE\n", offset);
        return ++offset;
    case OP_NIL:
        fmt::print("{:#08x} OP_NIL\n", offset);
        return ++offset;
    case OP_TRUE:
        fmt::print("{:#08x} OP_TRUE\n", offset);
        return ++offset;
    case OP_FALSE:
        fmt::print("{:#08x} OP_FALSE\n", offset);
        return ++offset;
    case OP_NOT:
        fmt::print("{:#08x} OP_NOT\n", offset);
        return ++offset;
    case OP_EQUAL:
        fmt::print("{:#08x} OP_EQUAL\n", offset);
        return ++offset;
    case OP_GREATER:
        fmt::print("{:#08x} OP_GREATER\n", offset);
        return ++offset;
    case OP_LESS:
        fmt::print("{:#08x} OP_LESS\n", offset);
        return ++offset;
    case OP_LESS_EQUAL:
        fmt::print("{:#08x} OP_LESS_EQUAL\n", offset);
        return ++offset;
    case OP_GREATER_EQUAL:
        fmt::print("{:#08x} OP_GREATER_EQUAL\n", offset);
        return ++offset;
    case OP_NOT_EQUAL:
        fmt::print("{:#08x} OP_NOT_EQUAL\n", offset);
        return ++offset;
    case OP_PRINT:
        fmt::print("{:#08x} OP_PRINT\n", offset);
        return ++offset;
    case OP_POP:
        fmt::print("{:#08x} OP_POP\n", offset);
        return ++offset;
    case OP_DEFINE_GLOBAL:
        fmt::print("{:#08x} OP_DEFINE_GLOBAL {}\n", offset, chunk.byte_code[offset + 1]);
        offset += 2;
        return offset;
    case OP_GET_GLOBAL:
        fmt::print("{:#08x} OP_GET_GLOBAL {}\n", offset, chunk.byte_code[offset + 1]);
        offset += 2;
        return offset;
    case OP_SET_GLOBAL:
        fmt::print("{:#08x} OP_SET_GLOBAL {}\n", offset, chunk.byte_code[offset + 1]);
        offset += 2;
        return offset;
    }
    LOX_ASSERT(false);
}

void Chunk::Clear()
{
    byte_code.clear();
    lines.clear();
    constant_pool.clear();
}