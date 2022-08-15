//
// Created by kevin on 8/6/22.
//

#ifndef LOX_CPP_COMMON_H
#define LOX_CPP_COMMON_H

#include <cstdint>
#include <vector>

enum OpCode : uint8_t {
    OP_RETURN,
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE
};

using Value = double;

static constexpr auto MAX_NUMBER_CONSTANTS = 256;

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

#endif // LOX_CPP_COMMON_H
