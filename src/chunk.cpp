//
// Created by kevin on 8/6/22.
//
#include "chunk.h"

#include "error.h"
#include "value_formatter.h"

#include <fmt/core.h>

auto Disassemble_chunk(Chunk const& chunk) -> void
{
    uint64_t offset = 0;
    auto const number_of_instructions = chunk.byte_code.size();
    while (offset < number_of_instructions) {
        offset = Disassemble_instruction(chunk, offset);
    }
}

auto Disassemble_instruction(Chunk const& chunk, uint64_t offset) -> uint64_t
{
    auto getIndex = [](uint8_t lsb, uint8_t hsb) -> uint16_t {
        return static_cast<uint16_t>(hsb << 8) + lsb;
    };

    auto const opcode = static_cast<OpCode>(chunk.byte_code[offset]);
    switch (opcode) {
    case OP_RETURN:
        fmt::print("{:#08x} OP_RETURN\n", offset);
        return ++offset;
    case OP_CONSTANT: {
        fmt::print("{:#08x} OP_CONSTANT {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    }
    case OP_NEGATE: {
        fmt::print("{:#08x} OP_NEGATE\n", offset);
        return ++offset;
    }
    case OP_ADD: {
        fmt::print("{:#08x} OP_ADD\n", offset);
        return ++offset;
    }
    case OP_SUBTRACT: {
        fmt::print("{:#08x} OP_SUBTRACT\n", offset);
        return ++offset;
    }
    case OP_MULTIPLY: {
        fmt::print("{:#08x} OP_MULTIPLY\n", offset);
        return ++offset;
    }
    case OP_DIVIDE: {
        fmt::print("{:#08x} OP_DIVIDE\n", offset);
        return ++offset;
    }
    case OP_NIL: {
        fmt::print("{:#08x} OP_NIL\n", offset);
        return ++offset;
    }
    case OP_TRUE: {
        fmt::print("{:#08x} OP_TRUE\n", offset);
        return ++offset;
    }
    case OP_FALSE: {
        fmt::print("{:#08x} OP_FALSE\n", offset);
        return ++offset;
    }
    case OP_NOT: {
        fmt::print("{:#08x} OP_NOT\n", offset);
        return ++offset;
    }
    case OP_EQUAL: {
        fmt::print("{:#08x} OP_EQUAL\n", offset);
        return ++offset;
    }
    case OP_GREATER: {
        fmt::print("{:#08x} OP_GREATER\n", offset);
        return ++offset;
    }
    case OP_LESS: {
        fmt::print("{:#08x} OP_LESS\n", offset);
        return ++offset;
    }
    case OP_LESS_EQUAL: {
        fmt::print("{:#08x} OP_LESS_EQUAL\n", offset);
        return ++offset;
    }
    case OP_GREATER_EQUAL: {
        fmt::print("{:#08x} OP_GREATER_EQUAL\n", offset);
        return ++offset;
    }
    case OP_NOT_EQUAL: {
        fmt::print("{:#08x} OP_NOT_EQUAL\n", offset);
        return ++offset;
    }
    case OP_PRINT: {
        fmt::print("{:#08x} OP_PRINT\n", offset);
        return ++offset;
    }
    case OP_POP: {
        fmt::print("{:#08x} OP_POP\n", offset);
        return ++offset;
    }
    case OP_DEFINE_GLOBAL: {
        fmt::print("{:#08x} OP_DEFINE_GLOBAL {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    }
    case OP_GET_GLOBAL: {
        fmt::print("{:#08x} OP_GET_GLOBAL {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    }
    case OP_SET_GLOBAL: {
        fmt::print("{:#08x} OP_SET_GLOBAL {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    }
    case OP_GET_LOCAL: {
        fmt::print("{:#08x} OP_GET_LOCAL {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    }
    case OP_SET_LOCAL: {
        fmt::print("{:#08x} OP_SET_LOCAL {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    }
    case OP_JUMP_IF_FALSE: {
        fmt::print("{:#08x} OP_JUMP_IF_FALSE {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    }
    case OP_JUMP: {
        fmt::print("{:#08x} OP_JUMP {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    }
    case OP_LOOP: {
        fmt::print("{:#08x} OP_LOOP {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    }
    case OP_CALL: {
        fmt::print("{:#08x} OP_CALL num_args:{}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    }
    case OP_CLOSURE: {
        auto const function_index_in_constant_pool = getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]);
        fmt::print("{:#08x} OP_CLOSURE constant_index: {}\n", offset, function_index_in_constant_pool);
        offset += 3;
        auto const& value = chunk.constant_pool.at(function_index_in_constant_pool);
        auto object_ptr = value.AsObjectPtr();
        LOX_ASSERT(object_ptr != nullptr);
        LOX_ASSERT(object_ptr->GetType() == ObjectType::FUNCTION);
        auto function_ptr = static_cast<FunctionObject const*>(object_ptr);
        for (auto i = 0; i < function_ptr->upvalue_count; ++i) {
            auto const is_local = static_cast<bool>(chunk.byte_code[offset]);
            auto const upvalue_index = getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]);
            fmt::print("{:#08x}  |   Upvalue[is_local={}, index={}] \n", offset, is_local, upvalue_index);
            offset += 3;
        }
        return offset;
    }
    case OP_GET_UPVALUE:
        fmt::print("{:#08x} OP_GET_UPVALUE {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    case OP_SET_UPVALUE:
        fmt::print("{:#08x} OP_SET_UPVALUE {}\n", offset, getIndex(chunk.byte_code[offset + 1], chunk.byte_code[offset + 2]));
        offset += 3;
        return offset;
    case OP_CLOSE_UPVALUE: {
        fmt::print("{:#08x} OP_CLOSE_UPVALUE\n", offset);
        return ++offset;
    }
    }
    LOX_ASSERT(false);
}

auto DumpConstants(Chunk const& chunk) -> void
{
    for (auto const& constant : chunk.constant_pool) {
        fmt::print("{}\n", constant);
    }
}

auto Chunk::Clear() -> void
{
    byte_code.clear();
    lines.clear();
    constant_pool.clear();
}
