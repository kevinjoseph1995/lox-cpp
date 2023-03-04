//
// Created by kevin on 8/6/22.
//

#include <cstdio>
#include <fmt/core.h>
#include <ranges>

#include "error.h"
#include "native_function.h"
#include "object.h"
#include "value_formatter.h"
#include "virtual_machine.h"

// #define DEBUG_TRACE_EXECUTION

static auto IsFalsy(Value const& value) -> bool
{
    return value.IsNil() || (value.IsBool() && !value.AsBool());
}

auto VirtualMachine::registerNativeFunctions() -> void
{
    this->m_globals["SystemTimeNow"] = m_heap.AllocateNativeFunctionObject(SystemTimeNow);
    this->m_globals["Echo"] = m_heap.AllocateNativeFunctionObject(Echo);
}

auto VirtualMachine::Interpret(Source const& source) -> ErrorOr<VoidType>
{
    auto compiled_function_result = m_compiler->CompileSource(source);
    if (!compiled_function_result) {
        return tl::unexpected(compiled_function_result.error());
    }
    auto new_closure = m_heap.AllocateClosureObject(compiled_function_result.value());
    m_frames.emplace_back(new_closure, 0, 0);
    registerNativeFunctions();
    return this->run();
}
auto VirtualMachine::run() -> RuntimeErrorOr<VoidType>
{
    while (true) {
        if (isAtEnd()) {
            LOX_ASSERT(m_value_stack.empty()); // Remove me once we add statements that produce side - effects
            return VoidType {};
        }
#ifdef DEBUG_TRACE_EXECUTION
        Disassemble_instruction(currentChunk(), m_frames.rbegin()->instruction_pointer);
#endif

        auto const instruction = static_cast<OpCode>(readByte());
        switch (instruction) {
        case OP_RETURN: {
            if (m_frames.size() == 1) {
                return VoidType {};
            }
            auto return_value = popStack();

            // Discarding the call frame
            // TODO: Close upvalues
            auto num_to_pop = static_cast<int32_t>(m_frames.back().closure->function->arity + 1);
            while (num_to_pop > 0) {
                auto _ = popStack();
                static_cast<void>(_);
                --num_to_pop;
            }

            m_frames.pop_back(); // Reset the call frame
            m_value_stack.push_back(return_value);
            break;
        }
        case OP_CONSTANT: {
            m_value_stack.push_back(readConstant());
            break;
        }
        case OP_NEGATE: {
            Value value = popStack();
            if (!value.IsDouble()) {
                return tl::unexpected(runtimeError(fmt::format("Cannot negate non-number type, line number:{}", currentChunk().lines[m_frames.rbegin()->instruction_pointer])));
            }
            m_value_stack.emplace_back(-value.AsDouble());
            break;
        }
        case OP_ADD: {
            auto result = binaryOperation(OP_ADD);
            if (!result) {
                return tl::unexpected(result.error());
            }
            break;
        }
        case OP_SUBTRACT: {
            auto result = binaryOperation(OP_SUBTRACT);
            if (!result) {
                return tl::unexpected(result.error());
            }
            break;
        }
        case OP_MULTIPLY: {
            auto result = binaryOperation(OP_MULTIPLY);
            if (!result) {
                return tl::unexpected(result.error());
            }
            break;
        }
        case OP_DIVIDE: {
            auto result = binaryOperation(OP_DIVIDE);
            if (!result) {
                return tl::unexpected(result.error());
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
            if (!result) {
                return tl::unexpected(result.error());
            }
            break;
        }
        case OP_LESS: {
            auto result = binaryOperation(OP_LESS);
            if (!result) {
                return tl::unexpected(result.error());
            }
            break;
        }
        case OP_LESS_EQUAL: {
            auto result = binaryOperation(OP_LESS_EQUAL);
            if (!result) {
                return tl::unexpected(result.error());
            }
            break;
        }
        case OP_GREATER_EQUAL: {
            auto result = binaryOperation(OP_GREATER_EQUAL);
            if (!result) {
                return tl::unexpected(result.error());
            }
            break;
        }
        case OP_PRINT: {
            LOX_ASSERT(!m_value_stack.empty());
            auto value = popStack();
            if (m_external_stream) {
                fmt::format_to(std::back_inserter(*m_external_stream), "{}\n", value);
            } else {
                fmt::print("{}\n", value);
                fflush(stdout); // Force flush
            }
            break;
        }
        case OP_POP: {
            auto _ = popStack();
            static_cast<void>(_);
            break;
        }
        case OP_DEFINE_GLOBAL: {
            // Need to get the variable name from the constant pool
            auto identifier_name_value = currentChunk().constant_pool.at(readIndex());
            LOX_ASSERT(identifier_name_value.IsObject() && identifier_name_value.AsObjectPtr()->GetType() == ObjectType::STRING);
            auto string_object = static_cast<StringObject*>(identifier_name_value.AsObjectPtr());
            m_globals[string_object->data] = popStack();
            break;
        }
        case OP_GET_GLOBAL: {
            auto identifier_name_value = currentChunk().constant_pool.at(readIndex());
            LOX_ASSERT(identifier_name_value.IsObject() && identifier_name_value.AsObjectPtr()->GetType() == ObjectType::STRING);
            auto identifier_string_object = static_cast<StringObject*>(identifier_name_value.AsObjectPtr());
            if (m_globals.count(identifier_string_object->data) == 0) {
                return tl::unexpected(runtimeError(fmt::format("Undefined variable:{}", identifier_string_object->data)));
            }
            m_value_stack.push_back(m_globals.at(identifier_string_object->data));
            break;
        }
        case OP_SET_GLOBAL: {
            auto identifier_name_value = currentChunk().constant_pool.at(readIndex());
            LOX_ASSERT(identifier_name_value.IsObject() && identifier_name_value.AsObjectPtr()->GetType() == ObjectType::STRING);
            auto identifier_string_object = static_cast<StringObject*>(identifier_name_value.AsObjectPtr());
            if (m_globals.count(identifier_string_object->data) == 0) {
                return tl::unexpected(runtimeError(fmt::format("Undefined variable:{}", identifier_string_object->data)));
            }
            m_globals[identifier_string_object->data] = peekStack(0); // Over-write existing value
            break;
        }
        case OP_GET_LOCAL: {
            auto const slotIndex = m_frames.rbegin()->slot + readIndex();
            m_value_stack.push_back(m_value_stack.at(slotIndex));
            break;
        }
        case OP_SET_LOCAL: {
            auto const slotIndex = m_frames.rbegin()->slot + readIndex();
            m_value_stack.at(slotIndex) = peekStack(0);
            break;
        }
        case OP_JUMP_IF_FALSE: {
            auto condition_value = peekStack(0); // Not popping it off yet
            auto offset = readIndex();
            if (IsFalsy(condition_value)) {
                m_frames.rbegin()->instruction_pointer += offset;
            }
            break;
        }
        case OP_JUMP: {
            m_frames.rbegin()->instruction_pointer += readIndex();
            break;
        }
        case OP_LOOP: {
            m_frames.rbegin()->instruction_pointer -= readIndex();
            break;
        }
        case OP_CALL: {
            auto const num_arguments = readIndex();
            auto const callable_object = peekStack(num_arguments);
            auto function_dispatch_status = call(callable_object, num_arguments);
            if (!function_dispatch_status) {
                return tl::unexpected(runtimeError(function_dispatch_status.error().error_message));
            }
            break;
        }
        case OP_CLOSURE: {
            auto value = readConstant();
            LOX_ASSERT(value.IsObject());
            auto object_ptr = value.AsObjectPtr();
            LOX_ASSERT(object_ptr->GetType() == ObjectType::FUNCTION);
            auto function_ptr = static_cast<FunctionObject*>(object_ptr);
            auto closure = m_heap.AllocateClosureObject(function_ptr);
            for (auto i = 0; i < function_ptr->upvalue_count; ++i) {
                auto const is_local = static_cast<bool>(readByte());
                auto const index = readIndex();
                if (is_local) {
                    closure->upvalues.push_back(captureUpvalue(static_cast<uint16_t>(m_frames.back().slot) + index));
                } else {
                    closure->upvalues.push_back(closure->upvalues.at(index));
                }
            }
            m_value_stack.push_back(closure);
            break;
        }
        case OP_GET_UPVALUE: {
            auto upvalue_index = readIndex();
            auto& upvalue = m_frames.back().closure->upvalues.at(upvalue_index);
            m_value_stack.push_back(upvalue->GetClosedValue());
            break;
        }
        case OP_SET_UPVALUE: {
            auto upvalue_index = readIndex();
            auto* const upvalue = m_frames.back().closure->upvalues.at(upvalue_index);
            upvalue->SetClosedValue(peekStack(0));
            break;
        }
        case OP_CLOSE_UPVALUE: {
            auto index = m_value_stack.size() - 1;
            LOX_ASSERT(index <= MAX_INDEX_SIZE);
            closeUpvalues(static_cast<uint16_t>(index));
            auto _ = popStack();
            static_cast<void>(_);
            break;
        }
        }
    }
}

auto VirtualMachine::readByte() -> uint8_t
{
    LOX_ASSERT(m_frames.rbegin()->instruction_pointer < currentChunk().byte_code.size());
    return currentChunk().byte_code.at(m_frames.rbegin()->instruction_pointer++);
}

auto VirtualMachine::readConstant() -> Value
{
    return currentChunk().constant_pool.at(readIndex());
}

auto VirtualMachine::popStack() -> Value
{
    LOX_ASSERT(!m_value_stack.empty());
    auto value = m_value_stack.at(m_value_stack.size() - 1);
    m_value_stack.pop_back();
    return value;
}

auto VirtualMachine::peekStack(uint32_t index_from_top) -> Value const&
{
    LOX_ASSERT(m_value_stack.size() > index_from_top);
    return m_value_stack.at(m_value_stack.size() - 1 - index_from_top);
}

auto VirtualMachine::binaryOperation(OpCode op) -> ErrorOr<VoidType>
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
            return tl::unexpected(runtimeError(fmt::format("RHS of \"{}\" is not a number type. Is {}", getOperatorString(_operator), rhs)));
        }

        Value lhs = popStack();
        if (!lhs.IsDouble()) {
            dumpCallFrameStack();
            return tl::unexpected(runtimeError(fmt::format("LHS of \"{}\" is not a number type. Is {}", getOperatorString(_operator), lhs)));
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
            return tl::unexpected(runtimeError(fmt::format("LHS of \"+\" is not a string type.")));
        }
        auto const& lhs_object = lhs.AsObject();
        if (lhs_object.GetType() != ObjectType::STRING) {
            return tl::unexpected(runtimeError(fmt::format("LHS of \"+\" is not a string type.")));
        }
        auto new_string_object = m_heap.AllocateStringObject("");
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

VirtualMachine::VirtualMachine(std::string* external_stream)
    : m_external_stream(external_stream)
{
    m_compiler = std::make_unique<Compiler>(m_heap, m_parser_state);
}
auto VirtualMachine::isAtEnd() -> bool
{
    return m_frames.rbegin()->instruction_pointer == currentChunk().byte_code.size();
}

auto VirtualMachine::call(Value const& callable, uint16_t num_arguments) -> RuntimeErrorOr<VoidType>
{
    if (!callable.IsObject()) {
        return tl::unexpected(RuntimeError { .error_message = "Not a callable_object" });
    }
    auto const object_ptr = callable.AsObjectPtr();
    switch (object_ptr->GetType()) {
    case ObjectType::CLOSURE: {
        auto closure_object = static_cast<ClosureObject const*>(object_ptr);
        auto function_object_ptr = closure_object->function;
        if (function_object_ptr->arity != num_arguments) {
            return tl::unexpected(RuntimeError { .error_message = "Number of arguments provided does not match the number of function parameters" });
        }
        // At this point the state of the stack is as follows:
        // | | | | ... | <CALLABLE_OBJECT> | param_1 | param_2 | ... | param_n |

        // Set up the new call frame
        m_frames.emplace_back(closure_object, 0, m_value_stack.size() - num_arguments);
        return VoidType {};
    }
    case ObjectType::NATIVE_FUNCTION: {
        auto native_function_object_ptr = static_cast<NativeFunctionObject const*>(object_ptr);
        RuntimeErrorOr<Value> return_value;
        if (num_arguments == 0) {
            return_value = native_function_object_ptr->native_function(num_arguments, nullptr);
        } else {
            auto stack_top_ptr = m_value_stack.data() + m_value_stack.size() - 1;
            auto first_arg_ptr = stack_top_ptr - (num_arguments - 1);
            return_value = native_function_object_ptr->native_function(num_arguments, first_arg_ptr);
        }
        return return_value.and_then([this](Value& value) {
            m_value_stack.push_back(value);
            return RuntimeErrorOr<VoidType> { VoidType {} };
        });
    }
    default:
        return tl::unexpected(RuntimeError { .error_message = "Not a callable_object" });
    }
}

auto VirtualMachine::runtimeError(std::string error_message) -> RuntimeError
{
    m_frames.rbegin()->instruction_pointer = currentChunk().byte_code.size();
    return RuntimeError { std::move(error_message) };
}

auto VirtualMachine::readIndex() -> uint16_t
{
    auto lsb = readByte();
    auto hsb = static_cast<uint16_t>(readByte() << 8);
    return static_cast<uint16_t>(hsb + lsb);
}

auto VirtualMachine::currentChunk() -> Chunk const&
{
    return m_frames.rbegin()->closure->function->chunk;
}

auto VirtualMachine::dumpCallFrameStack() -> void
{
    fmt::print(stderr, "Slot start: {}\n", m_frames.back().slot);
    for (int32_t index = static_cast<int32_t>(m_value_stack.size() - 1); index >= 0; --index) {
        fmt::print(stderr, "Index:{} | Value: {}\n", index, m_value_stack.at(static_cast<size_t>(index)));
    }
}

auto VirtualMachine::captureUpvalue(uint16_t slot_index) -> UpvalueObject*
{
    // Check if the upvalue is in our list
    auto it = m_open_upvalues.begin();
    auto prev_it = it;
    for (; it != m_open_upvalues.end(); std::advance(it, 1)) {
        auto open_upvalue = *it;
        if (open_upvalue->GetStackIndex() < slot_index) {
            break;
        }
        prev_it = it;
    }
    if (it != m_open_upvalues.end() && (*it)->GetStackIndex() == slot_index) {
        return *it;
    }
    LOX_ASSERT(slot_index < m_value_stack.size());
    auto upvalue_object = m_heap.AllocateNativeUpvalueObject();
    upvalue_object->SetStackIndex(slot_index);

    m_open_upvalues.insert(prev_it, upvalue_object);

    return upvalue_object;
}
auto VirtualMachine::closeUpvalues(uint16_t stack_index) -> void
{
    auto it = m_open_upvalues.begin();
    while (it != m_open_upvalues.end() && (*it)->GetStackIndex() >= stack_index) {
        (*it)->Close(m_value_stack.at(stack_index));
        ++it;
    }
}
