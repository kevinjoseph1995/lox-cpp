//
// Created by kevin on 8/6/22.
//

#ifndef LOX_CPP_COMMON_H
#define LOX_CPP_COMMON_H

#include <cstdint>
#include <vector>

using Value = double;

enum OpCode : uint8_t
{
    OP_RETURN,
    OP_CONSTANT
};

struct Chunk
{
    std::vector<OpCode> byte_code;
    std::vector<int32_t> lines;
    std::vector<Value> constant_pool;
};

void disassemble_chunk(Chunk const &chunk);

#endif // LOX_CPP_COMMON_H
