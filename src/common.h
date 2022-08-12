//
// Created by kevin on 8/6/22.
//

#ifndef LOX_CPP_COMMON_H
#define LOX_CPP_COMMON_H

#include <cstdint>
#include <vector>

#define unlikely(x) __builtin_expect(!!(x), 0)
// TODO: Kevin Don't use GCC intrinsic here
#define LOX_ASSERT(expr, ...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (unlikely(!(expr)))                                                                                         \
        {                                                                                                              \
            printf("FILE: %s, LINE %d, FUNC: %s, ASSERT_FAILED:  " #expr "\n", __FILE__, __LINE__, __func__);          \
            __builtin_trap();                                                                                          \
        }                                                                                                              \
    } while (0)

using Value = double;

enum OpCode : uint8_t
{
    OP_RETURN,
    OP_CONSTANT
};

struct Chunk
{
    std::vector<uint8_t> byte_code;
    std::vector<int32_t> lines;
    std::vector<Value> constant_pool;
};

void Disassemble_chunk(Chunk const &chunk);

uint64_t Disassemble_instruction(Chunk const &chunk, uint64_t offset);

#endif // LOX_CPP_COMMON_H
