//
// Created by kevin on 8/6/22.
//

#ifndef LOX_CPP_CHUNK_H
#define LOX_CPP_CHUNK_H

#include <cstdint>
#include <variant>
#include <vector>

#include "error.h"
#include "value.h"

enum OpCode : uint8_t {
    OP_RETURN,
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_GREATER_EQUAL,
    OP_NOT_EQUAL,
    OP_PRINT
};

static constexpr auto MAX_NUMBER_CONSTANTS = 256; // Currently we can only store as many constants that can be addressed by 8 bits

struct Chunk {
    std::vector<uint8_t> byte_code;
    std::vector<int32_t> lines;
    std::vector<Value> constant_pool;
    void Reset()
    {
        byte_code.clear();
        lines.clear();
        constant_pool.clear();
    }
};

void Disassemble_chunk(Chunk const& chunk);

uint64_t Disassemble_instruction(Chunk const& chunk, uint64_t offset);

#endif // LOX_CPP_CHUNK_H
