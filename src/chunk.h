// MIT License

// Copyright (c) 2023 Kevin Joseph

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
    OP_CLOSE_UPVALUE,
    OP_CLASS,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_METHOD
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
