//
// Created by kevin on 8/6/22.
//

#ifndef LOX_CPP_CHUNK_H
#define LOX_CPP_CHUNK_H

#include <cstdint>
#include <variant>
#include <vector>

#include <fmt/format.h>

#include "error.h"

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
    OP_NOT_EQUAL
};

struct NilType { };

struct Value : public std::variant<NilType, double, bool> {
    template <typename T>
    Value(T&& value)
        : std::variant<NilType, double, bool>(std::forward<T>(value))
    {
    }
    [[nodiscard]] bool IsNil() const
    {
        return std::holds_alternative<NilType>(*this);
    }
    [[nodiscard]] bool IsDouble() const
    {
        return std::holds_alternative<double>(*this);
    }
    [[nodiscard]] bool IsBool() const
    {
        return std::holds_alternative<bool>(*this);
    }
    bool operator==(Value const& other) const
    {
        if (this->index() != other.index()) {
            return false;
        }
        if (this->IsBool()) {
            return std::get_if<bool>(this) == std::get_if<bool>(&other);
        } else if (this->IsDouble()) {
            return std::get_if<double>(this) == std::get_if<double>(&other);
        } else {
            return true;
        }
    }
    bool operator!=(Value const& other) const
    {
        return !(*this == other);
    }
};

template <>
struct fmt::formatter<Value> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(Value const& value, FormatContext& ctx)
    {
        switch (value.index()) {
        case 0:
            return fmt::format_to(ctx.out(), "Nil");
        case 1: {
            double const* double_pointer = std::get_if<double>(&value);
            LOX_ASSERT(double_pointer != nullptr);
            return fmt::format_to(ctx.out(), "{}", *double_pointer);
        }
        case 2: {
            bool const* bool_pointer = std::get_if<bool>(&value);
            LOX_ASSERT(bool_pointer != nullptr);
            return fmt::format_to(ctx.out(), "{}", *bool_pointer);
        }
        default:
            LOX_ASSERT(false);
        }
    };
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
