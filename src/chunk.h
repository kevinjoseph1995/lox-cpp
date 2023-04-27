//
// Created by kevin on 8/6/22.
//

#ifndef LOX_CPP_CHUNK_H
#define LOX_CPP_CHUNK_H

#include "error.h"
#include "value.h"

#include <cstdint>
#include <limits>
#include <variant>
#include <vector>

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
    OP_PRINT,
    OP_POP,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,
    OP_CALL,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE
};

static constexpr auto MAX_INDEX_SIZE = std::numeric_limits<uint16_t>::max();
static constexpr auto MAX_NUMBER_CONSTANTS = MAX_INDEX_SIZE; // Currently we can only store as many constants that can be addressed by 16 bits
static constexpr auto MAX_NUMBER_LOCAL_VARIABLES = MAX_INDEX_SIZE;
static constexpr auto MAX_JUMP_OFFSET = MAX_INDEX_SIZE;
static constexpr auto MAX_NUMBER_OF_FUNCTION_PARAMETERS = MAX_INDEX_SIZE;

struct Chunk {
    std::vector<uint8_t> byte_code;
    std::vector<int32_t> lines;
    std::vector<Value> constant_pool;
    void Clear();
};

[[maybe_unused]] auto Disassemble_chunk(Chunk const& chunk) -> void;
[[maybe_unused]] auto DumpConstants(Chunk const& chunk) -> void;
[[maybe_unused]] auto Disassemble_instruction(Chunk const& chunk, uint64_t offset) -> uint64_t;

#endif // LOX_CPP_CHUNK_H
