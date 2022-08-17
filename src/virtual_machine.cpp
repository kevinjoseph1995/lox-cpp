//
// Created by kevin on 8/6/22.
//

#include "virtual_machine.h"
#include "error.h"
#include <fmt/core.h>

#define DEBUG_TRACE_EXECUTION

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
            fmt::print("{}", *(m_value_stack.end() - 1));
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
    double* value = std::get_if<double>(&m_value_stack.at(m_value_stack.size() - 1));
    m_value_stack.pop_back();
    return *value;
}

ErrorOr<VoidType> VirtualMachine::binaryOperation(OpCode op)
{
    char operator_ch;
    double (*binary_operation_function)(double const&, double const&);
    switch (op) {
    case OP_ADD:
        operator_ch = '+';
        // Cool trick from Timur's cpp north video
        // https://www.youtube.com/watch?v=iWKewYYKPHk
        binary_operation_function = +[](double const& lhs, double const& rhs) -> double { return lhs + rhs; };
        break;
    case OP_SUBTRACT:
        binary_operation_function = +[](double const& lhs, double const& rhs) -> double { return lhs - rhs; };
        operator_ch = '-';
        break;
    case OP_MULTIPLY:
        binary_operation_function = +[](double const& lhs, double const& rhs) -> double { return lhs * rhs; };
        operator_ch = '*';
        break;
    case OP_DIVIDE:
        binary_operation_function = +[](double const& lhs, double const& rhs) -> double { return lhs / rhs; };
        operator_ch = '/';
        break;
    default:
        LOX_ASSERT(false, "Unreachable code, internal error");
    }

    Value lhs = popStack();
    Value rhs = popStack();

    double const* lhs_double_ptr = std::get_if<double>(&lhs);
    if (lhs_double_ptr == nullptr) {
        return Error { .type = ErrorType::RuntimeError, .error_message = fmt::format("LHS of \"{}\" operator is not of number type, line number:{}", operator_ch, m_current_chunk.lines[m_instruction_pointer]) };
    }
    double const* rhs_double_ptr = std::get_if<double>(&rhs);
    if (rhs_double_ptr == nullptr) {
        return Error { .type = ErrorType::RuntimeError, .error_message = fmt::format("RHS of \"{}\" operator is not of number type, line number:{}", operator_ch, m_current_chunk.lines[m_instruction_pointer]) };
    }
    m_value_stack.emplace_back(binary_operation_function(*lhs_double_ptr, *rhs_double_ptr));
    return VoidType {};
}
