//
// Created by kevin on 8/6/22.
//

#include "virtual_machine.h"
#include "error.h"
#include <fmt/core.h>

#ifndef NDEBUG
#define DEBUG_TRACE_EXECUTION
#endif

static bool IsFalsy(Value const& value)
{
    return value.IsNil() || (value.IsBool() && !value.AsBool());
}

ErrorOr<VoidType> VirtualMachine::Interpret(Source const& source)
{
    auto compilation_status = m_compiler->CompileSource(source, m_current_chunk);
    if (compilation_status.IsError()) {
        return compilation_status;
    }
    return this->run();
}

ErrorOr<VoidType> VirtualMachine::run()
{
    while (true) {
        if (isAtEnd()) {
            LOX_ASSERT(m_value_stack.empty()); // Remove me once we add statements that produce side - effects
            return VoidType {};
        }
#ifdef DEBUG_TRACE_EXECUTION
        Disassemble_instruction(m_current_chunk, m_instruction_pointer);
#endif

        auto const instruction = static_cast<OpCode>(readByte());
        switch (instruction) {
        case OP_RETURN: {
            LOX_ASSERT(false);
            break;
        }
        case OP_CONSTANT: {
            m_value_stack.push_back(readConstant());
            break;
        }
        case OP_NEGATE: {
            Value value = popStack();
            if (!value.IsDouble()) {
                return runtimeError(fmt::format("Cannot negate non-number type, line number:{}", m_current_chunk.lines[m_instruction_pointer]));
            }
            m_value_stack.emplace_back(-value.AsDouble());
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
        case OP_EQUAL: {
            Value rhs = popStack();
            Value lhs = popStack();
            m_value_stack.emplace_back(rhs == lhs);
            break;
        }
        case OP_NOT_EQUAL: {
            Value rhs = popStack();
            Value lhs = popStack();
            m_value_stack.emplace_back(rhs != lhs);
            break;
        }
        case OP_GREATER: {
            auto result = binaryOperation(OP_GREATER);
            if (result.IsError()) {
                return result;
            }
            break;
        }
        case OP_LESS: {
            auto result = binaryOperation(OP_LESS);
            if (result.IsError()) {
                return result;
            }
            break;
        }
        case OP_LESS_EQUAL: {
            auto result = binaryOperation(OP_LESS_EQUAL);
            if (result.IsError()) {
                return result;
            }
            break;
        }
        case OP_GREATER_EQUAL: {
            auto result = binaryOperation(OP_GREATER_EQUAL);
            if (result.IsError()) {
                return result;
            }
            break;
        }
        case OP_PRINT: {
            LOX_ASSERT(!m_value_stack.empty());
            auto value = popStack();
            fmt::print("{}\n", value);
            fflush(stdout); // Force flush
            break;
        }
        case OP_POP: {
            auto _ = popStack();
            static_cast<void>(_);
            break;
        }
        case OP_DEFINE_GLOBAL: {
            // Need to get the variable name from the constant pool
            auto constant_pool_index = static_cast<uint64_t>(readByte());
            auto identifier_name_value = m_current_chunk.constant_pool.at(constant_pool_index);
            LOX_ASSERT(identifier_name_value.IsObject() && identifier_name_value.AsObjectPtr()->GetType() == ObjectType::STRING);
            auto string_object = static_cast<StringObject*>(identifier_name_value.AsObjectPtr());
            m_globals[string_object->data] = popStack();
            break;
        }
        case OP_GET_GLOBAL: {
            auto constant_pool_index = static_cast<uint64_t>(readByte());
            auto identifier_name_value = m_current_chunk.constant_pool.at(constant_pool_index);
            LOX_ASSERT(identifier_name_value.IsObject() && identifier_name_value.AsObjectPtr()->GetType() == ObjectType::STRING);
            auto identifier_string_object = static_cast<StringObject*>(identifier_name_value.AsObjectPtr());
            if (m_globals.count(identifier_string_object->data) == 0) {
                return runtimeError(fmt::format("Undefined variable:{}", identifier_string_object->data));
            }
            m_value_stack.push_back(m_globals.at(identifier_string_object->data));
            break;
        }
        case OP_SET_GLOBAL: {
            auto constant_pool_index = static_cast<uint64_t>(readByte());
            auto identifier_name_value = m_current_chunk.constant_pool.at(constant_pool_index);
            LOX_ASSERT(identifier_name_value.IsObject() && identifier_name_value.AsObjectPtr()->GetType() == ObjectType::STRING);
            auto identifier_string_object = static_cast<StringObject*>(identifier_name_value.AsObjectPtr());
            if (m_globals.count(identifier_string_object->data) == 0) {
                return runtimeError(fmt::format("Undefined variable:{}", identifier_string_object->data));
            }
            m_globals[identifier_string_object->data] = peekStack(0); // Over-write existing value
            break;
        }
        default: {
            LOX_ASSERT(false, "Unhandled op-code");
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
    auto value = m_value_stack.at(m_value_stack.size() - 1);
    m_value_stack.pop_back();
    return value;
}

Value const& VirtualMachine::peekStack(uint32_t index_from_top)
{
    LOX_ASSERT(m_value_stack.size() > index_from_top);
    return m_value_stack.at(m_value_stack.size() - 1 - index_from_top);
}

ErrorOr<VoidType> VirtualMachine::binaryOperation(OpCode op)
{
    auto getOperatorString = [](auto _op) {
        using decayed_type = typename std::decay<decltype(_op)>::type;
        if constexpr (std::is_same_v<decayed_type, std::plus<double>>) {
            return "+";
        } else if constexpr (std::is_same_v<decayed_type, std::minus<double>>) {
            return "-";
        } else if constexpr (std::is_same_v<decayed_type, std::multiplies<double>>) {
            return "*";
        } else if constexpr (std::is_same_v<decayed_type, std::divides<double>>) {
            return "/";
        } else if constexpr (std::is_same_v<decayed_type, std::less<double>>) {
            return "<";
        } else if constexpr (std::is_same_v<decayed_type, std::less_equal<double>>) {
            return "<=";
        } else if constexpr (std::is_same_v<decayed_type, std::greater<double>>) {
            return ">";
        } else if constexpr (std::is_same_v<decayed_type, std::greater_equal<double>>) {
            return ">=";
        } else {
            return "UNKNOWN";
        };
    };

    auto binaryOpWrapper = [&](auto _operator) -> ErrorOr<VoidType> {
        Value rhs = popStack();
        if (!rhs.IsDouble()) {
            return runtimeError(fmt::format("RHS of \"{}\" is not a number type.", getOperatorString(_operator)));
        }

        Value lhs = popStack();
        if (!lhs.IsDouble()) {
            return runtimeError(fmt::format("LHS of \"{}\" is not a number type.", getOperatorString(_operator)));
        }

        m_value_stack.emplace_back(_operator(lhs.AsDouble(), rhs.AsDouble()));
        return VoidType {};
    };

    auto stringConcatenation = [&]() -> ErrorOr<VoidType> {
        Value rhs = popStack();

        auto const& rhs_object = rhs.AsObject();
        LOX_ASSERT(rhs_object.GetType() == ObjectType::STRING);

        Value lhs = popStack();
        if (!lhs.IsObject()) {
            return runtimeError(fmt::format("LHS of \"+\" is not a string type."));
        }
        auto const& lhs_object = lhs.AsObject();
        if (lhs_object.GetType() != ObjectType::STRING) {
            return runtimeError(fmt::format("LHS of \"+\" is not a string type."));
        }
        auto new_string_object = m_heap.AllocateStringObject();
        LOX_ASSERT(new_string_object->GetType() == ObjectType::STRING);
        new_string_object->data.append(static_cast<StringObject const*>(&lhs_object)->data);
        new_string_object->data.append(static_cast<StringObject const*>(&rhs_object)->data);
        m_value_stack.emplace_back(static_cast<Object*>(new_string_object));
        return VoidType {};
    };

    switch (op) {
    case OP_ADD: {
        auto const& rhs = peekStack(0);
        if (rhs.IsObject() && (rhs.AsObject().GetType() == ObjectType::STRING)) {
            return stringConcatenation();
        } else {
            return binaryOpWrapper(std::plus<double> {});
        }
    }
    case OP_SUBTRACT:
        return binaryOpWrapper(std::minus<double> {});

    case OP_MULTIPLY:
        return binaryOpWrapper(std::multiplies<double> {});

    case OP_DIVIDE:
        return binaryOpWrapper(std::divides<double> {});

    case OP_GREATER_EQUAL:
        return binaryOpWrapper(std::greater_equal<double> {});

    case OP_GREATER:
        return binaryOpWrapper(std::greater<double> {});

    case OP_LESS_EQUAL:
        return binaryOpWrapper(std::less_equal<double> {});

    case OP_LESS:
        return binaryOpWrapper(std::less<double> {});

    default:
        LOX_ASSERT(false, "Not a binary operation");
    }
}

VirtualMachine::VirtualMachine()
{
    m_compiler = std::make_unique<Compiler>(m_heap);
}
bool VirtualMachine::isAtEnd()
{
    return m_instruction_pointer == m_current_chunk.byte_code.size();
}

Error VirtualMachine::runtimeError(std::string error_message)
{
    m_instruction_pointer = m_current_chunk.byte_code.size();
    return Error { .type = ErrorType::RuntimeError,
        .error_message = std::move(error_message) };
}
