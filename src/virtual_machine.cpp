//
// Created by kevin on 8/6/22.
//

#include "virtual_machine.h"
#include "error.h"
#include <fmt/core.h>

#define DEBUG_TRACE_EXECUTION

static bool IsFalsy(Value const& value)
{
    return value.IsNil() || (value.IsBool() && !(*std::get_if<bool>(&value)));
}

ErrorOr<VoidType> VirtualMachine::Interpret(std::string const* source_code)
{
    auto compilation_status = m_compiler.CompileSource(source_code, m_current_chunk);
    if (compilation_status.IsError()) {
        return compilation_status;
    }
    return this->run();
}

void VirtualMachine::Reset()
{
    m_current_chunk.Reset();
    m_instruction_pointer = 0;
    m_value_stack.clear();
}

ErrorOr<VoidType> VirtualMachine::run()
{
    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
        Disassemble_instruction(m_current_chunk, m_instruction_pointer);
#endif
        auto const instruction = static_cast<OpCode>(readByte());
        switch (instruction) {
        case OP_RETURN:
            fmt::print("{}", *(m_value_stack.end() - 1)); // TODO Remove me
            return VoidType {};
        case OP_CONSTANT: {
            m_value_stack.push_back(readConstant());
            break;
        }
        case OP_NEGATE: {
            Value value = popStack();
            double* double_value_ptr = std::get_if<double>(&value);
            if (double_value_ptr == nullptr) {
                return Error { .type = ErrorType::RuntimeError,
                    .error_message = fmt::format("Cannot negate non-number type, line number:{}", m_current_chunk.lines[m_instruction_pointer]) };
            }
            m_value_stack.emplace_back(-(*double_value_ptr));
            break;
        }
        case OP_ADD: {
            auto result = binaryOperation(OP_ADD);
            if (result.IsError()) {
                return result;
            }
            break;
        }
        case OP_SUBTRACT: {
            auto result = binaryOperation(OP_SUBTRACT);
            if (result.IsError()) {
                return result;
            }
            break;
        }
        case OP_MULTIPLY: {
            auto result = binaryOperation(OP_MULTIPLY);
            if (result.IsError()) {
                return result;
            }
            break;
        }
        case OP_DIVIDE: {
            auto result = binaryOperation(OP_DIVIDE);
            if (result.IsError()) {
                return result;
            }
            break;
        }
        case OP_NIL:
            m_value_stack.emplace_back(NilType {});
            break;
        case OP_TRUE:
            m_value_stack.emplace_back(true);
            break;
        case OP_FALSE:
            m_value_stack.emplace_back(false);
            break;
        case OP_NOT:
            m_value_stack.emplace_back(IsFalsy(popStack()));
            break;
        }
    }
}

uint8_t VirtualMachine::readByte()
{
    LOX_ASSERT(m_instruction_pointer < m_current_chunk.byte_code.size());
    return m_current_chunk.byte_code.at(m_instruction_pointer++);
}

Value VirtualMachine::readConstant()
{
    auto constant_pool_index = static_cast<uint64_t>(readByte());
    return m_current_chunk.constant_pool.at(constant_pool_index);
}

Value VirtualMachine::popStack()
{
    LOX_ASSERT(!m_value_stack.empty());
    auto value = m_value_stack.at(m_value_stack.size() - 1);
    m_value_stack.pop_back();
    return value;
}

ErrorOr<VoidType> VirtualMachine::binaryOperation(OpCode op)
{
    Value rhs = popStack();
    Value lhs = popStack();

#define BINARY_OP_WRAPPER(op)                                                                                                                                                                             \
    do {                                                                                                                                                                                                  \
        double const* lhs_double_ptr = std::get_if<double>(&lhs);                                                                                                                                         \
        if (lhs_double_ptr == nullptr) {                                                                                                                                                                  \
            return Error { .type = ErrorType::RuntimeError, .error_message = fmt::format("LHS of \"op\" operator is not of number type, line number:{}", m_current_chunk.lines[m_instruction_pointer]) }; \
        }                                                                                                                                                                                                 \
        double const* rhs_double_ptr = std::get_if<double>(&rhs);                                                                                                                                         \
        if (rhs_double_ptr == nullptr) {                                                                                                                                                                  \
            return Error { .type = ErrorType::RuntimeError, .error_message = fmt::format("RHS of \"op\" operator is not of number type, line number:{}", m_current_chunk.lines[m_instruction_pointer]) }; \
        }                                                                                                                                                                                                 \
        m_value_stack.emplace_back((*lhs_double_ptr)op(*rhs_double_ptr));                                                                                                                                 \
    } while (0)

    switch (op) {
    case OP_ADD: {
        BINARY_OP_WRAPPER(+);
        break;
    }
    case OP_SUBTRACT: {
        BINARY_OP_WRAPPER(-);
        break;
    }
    case OP_MULTIPLY: {
        BINARY_OP_WRAPPER(*);
        break;
    }
    case OP_DIVIDE: {
        BINARY_OP_WRAPPER(/);
        break;
    }
    default:
        LOX_ASSERT(false, "Unreachable code, internal error");
    }
#undef BINARY_OP_WRAPPER
    return VoidType {};
}
