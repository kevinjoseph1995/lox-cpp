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
    }
    LOX_ASSERT(false);
}