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

#include <cstdio>
#include <fmt/core.h>
#include <iterator>
#include <memory>
#include <ranges>

#include "error.h"
#include "heap.h"
#include "native_function.h"
#include "object.h"
#include "value.h"
#include "value_formatter.h"
#include "virtual_machine.h"

static auto IsFalsy(Value const& value) -> bool
{
    return value.IsNil() || (value.IsBool() && !value.AsBool());
}

auto VirtualMachine::registerNativeFunctions() -> void
{
    this->m_globals["SystemTimeNow"] = m_heap->AllocateNativeFunctionObject(SystemTimeNow);
    this->m_globals["Echo"] = m_heap->AllocateNativeFunctionObject(Echo);
}

auto VirtualMachine::Interpret(Source const& source) -> ErrorOr<VoidType>
{
    auto compiled_function_result = m_compiler->CompileSource(source);
    if (!compiled_function_result) {
        return std::unexpected(compiled_function_result.error());
    }
    auto new_closure = m_heap->AllocateClosureObject(compiled_function_result.value());
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
                return std::unexpected(runtimeError(fmt::format("Cannot negate non-number type, line number:{}", currentChunk().lines[m_frames.rbegin()->instruction_pointer])));
            }
            m_value_stack.emplace_back(-value.AsDouble());
            break;
        }
        case OP_ADD: {
            auto result = binaryOperation(OP_ADD);
            if (!result) {
                return std::unexpected(result.error());
            }
            break;
        }
        case OP_SUBTRACT: {
            auto result = binaryOperation(OP_SUBTRACT);
            if (!result) {
                return std::unexpected(result.error());
            }
            break;
        }
        case OP_MULTIPLY: {
            auto result = binaryOperation(OP_MULTIPLY);
            if (!result) {
                return std::unexpected(result.error());
            }
            break;
        }
        case OP_DIVIDE: {
            auto result = binaryOperation(OP_DIVIDE);
            if (!result) {
                return std::unexpected(result.error());
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
                return std::unexpected(result.error());
            }
            break;
        }
        case OP_LESS: {
            auto result = binaryOperation(OP_LESS);
            if (!result) {
                return std::unexpected(result.error());
            }
            break;
        }
        case OP_LESS_EQUAL: {
            auto result = binaryOperation(OP_LESS_EQUAL);
            if (!result) {
                return std::unexpected(result.error());
            }
            break;
        }
        case OP_GREATER_EQUAL: {
            auto result = binaryOperation(OP_GREATER_EQUAL);
            if (!result) {
                return std::unexpected(result.error());
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
                return std::unexpected(runtimeError(fmt::format("Undefined variable:{}", identifier_string_object->data)));
            }
            m_value_stack.push_back(m_globals.at(identifier_string_object->data));
            break;
        }
        case OP_SET_GLOBAL: {
            auto identifier_name_value = currentChunk().constant_pool.at(readIndex());
            LOX_ASSERT(identifier_name_value.IsObject() && identifier_name_value.AsObjectPtr()->GetType() == ObjectType::STRING);
            auto identifier_string_object = static_cast<StringObject*>(identifier_name_value.AsObjectPtr());
            if (m_globals.count(identifier_string_object->data) == 0) {
                return std::unexpected(runtimeError(fmt::format("Undefined variable:{}", identifier_string_object->data)));
            }
            m_globals[identifier_string_object->data] = peekStack(0); // Over-write existing value
            break;
        }
        case OP_GET_LOCAL: {
            auto const slotIndex = m_frames.rbegin()->slot + readIndex() - 1;
            m_value_stack.push_back(m_value_stack.at(slotIndex));
            break;
        }
        case OP_SET_LOCAL: {
            auto const slotIndex = m_frames.rbegin()->slot + readIndex() - 1;
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
            auto callable_object = peekStack(num_arguments);
            auto function_dispatch_status = call(callable_object, num_arguments);
            if (!function_dispatch_status) {
                return std::unexpected(runtimeError(function_dispatch_status.error().error_message));
            }
            break;
        }
        case OP_CLOSURE: {
            auto value = readConstant();
            LOX_ASSERT(value.IsObject());
            auto object_ptr = value.AsObjectPtr();
            LOX_ASSERT(object_ptr->GetType() == ObjectType::FUNCTION);
            auto function_ptr = static_cast<FunctionObject*>(object_ptr);
            std::vector<UpvalueObject*> upvalues;
            for (auto i = 0; i < function_ptr->upvalue_count; ++i) {
                auto const is_local = static_cast<bool>(readByte());
                auto const index = readIndex();
                if (is_local) {
                    upvalues.push_back(captureUpvalue(static_cast<uint16_t>(m_frames.back().slot) + index - 1));
                } else {
                    upvalues.push_back(m_frames.back().closure->upvalues.at(index));
                }
            }
            auto closure = m_heap->AllocateClosureObject(function_ptr);
            closure->upvalues = std::move(upvalues);
            m_value_stack.push_back(closure);
            break;
        }
        case OP_GET_UPVALUE: {
            auto upvalue_index = readIndex();
            auto* const upvalue = m_frames.back().closure->upvalues.at(upvalue_index);
            if (upvalue->IsClosed()) {
                m_value_stack.push_back(upvalue->GetClosedValue());
            } else {
                m_value_stack.push_back(m_value_stack.at(upvalue->GetStackIndex()));
            }
            break;
        }
        case OP_SET_UPVALUE: {
            auto upvalue_index = readIndex();
            auto* const upvalue = m_frames.back().closure->upvalues.at(upvalue_index);
            if (upvalue->IsClosed()) {
                upvalue->SetClosedValue(peekStack(0));
            } else {
                m_value_stack.at(upvalue->GetStackIndex()) = peekStack(0);
            }
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
        case OP_CLASS: {
            auto value = readConstant();
            LOX_ASSERT(value.IsObject());
            auto string_object_ptr = static_cast<StringObject*>(value.AsObjectPtr());
            m_value_stack.push_back(m_heap->AllocateClassObject(string_object_ptr->data));
            break;
        }
        case OP_GET_PROPERTY: {
            auto instance = popStack();
            if (not(instance.IsObject() && instance.AsObject().GetType() == ObjectType::INSTANCE)) {
                return std::unexpected(RuntimeError { .error_message = "Can only get property for instance types" });
            }
            auto instance_object_ptr = static_cast<InstanceObject*>(instance.AsObjectPtr());
            auto property = readConstant();
            LOX_ASSERT(property.IsObject() && property.AsObject().GetType() == ObjectType::STRING);
            auto const& property_name = static_cast<StringObject&>(property.AsObject()).data;
            if (instance_object_ptr->fields.contains(property_name)) {
                m_value_stack.push_back(instance_object_ptr->fields.at(property_name));
                break;
            }
            // The field was not found in the instance property table
            // Check if this is a class method
            if (not instance_object_ptr->class_->methods.contains(property_name)) {
                return std::unexpected(RuntimeError { .error_message = fmt::format("{} not found", property_name) });
            }
            m_value_stack.push_back(m_heap->AllocateBoundMethodObject(instance_object_ptr, instance_object_ptr->class_->methods.at(property_name)));
            break;
        }
        case OP_SET_PROPERTY: {
            auto const rhs = popStack();
            auto instance = popStack();
            if (not(instance.IsObject() && instance.AsObject().GetType() == ObjectType::INSTANCE)) {
                return std::unexpected(RuntimeError { .error_message = "Can only set property for instance types" });
            }
            auto instance_object_ptr = static_cast<InstanceObject*>(instance.AsObjectPtr());
            auto property = readConstant();
            LOX_ASSERT(property.IsObject() && property.AsObject().GetType() == ObjectType::STRING);
            instance_object_ptr->fields[static_cast<StringObject&>(property.AsObject()).data] = rhs; // Will either add/update the propery to the instance
            m_value_stack.push_back(rhs);
            break;
        }
        case OP_METHOD: {
            auto object = peekStack(0);
            LOX_ASSERT(object.IsObject() && object.AsObject().GetType() == ObjectType::CLOSURE);
            auto closure_object_ptr = static_cast<ClosureObject*>(object.AsObjectPtr());
            object = peekStack(1);
            LOX_ASSERT(object.IsObject() && object.AsObject().GetType() == ObjectType::CLASS);
            auto class_object_ptr = static_cast<ClassObject*>(object.AsObjectPtr());
            object = readConstant();
            LOX_ASSERT(object.IsObject() && object.AsObject().GetType() == ObjectType::STRING);
            auto method_name = static_cast<StringObject*>(object.AsObjectPtr()); // Will add the  to the instance
            class_object_ptr->methods[method_name->data] = closure_object_ptr;
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
            return std::unexpected(runtimeError(fmt::format("RHS of \"{}\" is not a number type. Is {}", getOperatorString(_operator), rhs)));
        }

        Value lhs = popStack();
        if (!lhs.IsDouble()) {
            dumpCallFrameStack();
            return std::unexpected(runtimeError(fmt::format("LHS of \"{}\" is not a number type. Is {}", getOperatorString(_operator), lhs)));
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
            return std::unexpected(runtimeError(fmt::format("LHS of \"+\" is not a string type.")));
        }
        auto& lhs_object = lhs.AsObject();
        if (lhs_object.GetType() != ObjectType::STRING) {
            return std::unexpected(runtimeError(fmt::format("LHS of \"+\" is not a string type.")));
        }
        auto lhs_string_object = static_cast<StringObject*>(&lhs_object);
        lhs_string_object->data += static_cast<StringObject const*>(&rhs_object)->data;
        m_value_stack.emplace_back(static_cast<Object*>(lhs_string_object));
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
    m_heap = std::make_unique<Heap>(*this);
    m_compiler = std::make_unique<Compiler>(*m_heap, m_parser_state);
    m_heap->SetCompilerContext(m_compiler.get());
}
auto VirtualMachine::isAtEnd() -> bool
{
    return m_frames.rbegin()->instruction_pointer == currentChunk().byte_code.size();
}

auto VirtualMachine::call(Value& callable, uint16_t num_arguments) -> RuntimeErrorOr<VoidType>
{
    if (!callable.IsObject()) {
        return std::unexpected(RuntimeError { .error_message = "Not a callable_object" });
    }
    auto const object_ptr = callable.AsObjectPtr();
    switch (object_ptr->GetType()) {
    case ObjectType::CLOSURE: {
        auto closure_object = static_cast<ClosureObject*>(object_ptr);
        auto function_object_ptr = closure_object->function;
        if (function_object_ptr->arity != num_arguments) {
            return std::unexpected(RuntimeError { .error_message = "Number of arguments provided does not match the number of function parameters" });
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
        // This is ugly and the perfect candidate for std::expected::and_then
        // P2505R1 Monadic Functions for std::expected
        // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2505r1.html
        // TODO: Move to the and_then when it becomes available in the standard library
        return return_value.has_value()
               ? /* Error-less branch*/
               [this, num_arguments, value = return_value.value()]() -> RuntimeErrorOr<VoidType> {
                   auto num_to_pop = static_cast<int32_t>(num_arguments + 1); // Reset the call stack | . | . | ... | Native Function Object | Arg1 | Arg 2 | ... | ArgN |
                   while (num_to_pop > 0) {
                       auto _ = popStack();
                       static_cast<void>(_);
                       --num_to_pop;
                   }
                   m_value_stack.push_back(value);
                   return VoidType {};
               }()
               : /* Error branch*/
               std::unexpected(return_value.error()) /*Error*/;
    }
    case ObjectType::CLASS: {
        auto class_ptr = static_cast<ClassObject*>(object_ptr);
        m_value_stack.push_back(m_heap->AllocateInstanceObject(class_ptr));
        return VoidType {};
    }
    case ObjectType::BOUND_METHOD: {
        auto bound_object_ptr = static_cast<BoundMethodObject*>(object_ptr);
        if (bound_object_ptr->method->function->arity != num_arguments) {
            return std::unexpected(RuntimeError { .error_message = "Number of arguments provided does not match the number of function parameters" });
        }

        // At this point the state of the stack is as follows:
        // | | | | ... | <ClosureObject> | param_1 | param_2 | ... | param_n |

        // Set up the new call frame
        m_value_stack.at(m_value_stack.size() - num_arguments - 1) = bound_object_ptr->receiver;
        // At this point the state of the stack is as follows:
        // | | | | ... | InstanceObject | param_1 | param_2 | ... | param_n |
        m_frames.emplace_back(bound_object_ptr->method, 0, m_value_stack.size() - num_arguments);
        return VoidType {};
    }
    default:
        return std::unexpected(RuntimeError { .error_message = "Not a callable_object" });
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
        if (open_upvalue->GetStackIndex() <= slot_index) {
            break;
        }
        prev_it = it;
    }
    if (it != m_open_upvalues.end() && (*it)->GetStackIndex() == slot_index) {
        return *it;
    }
    LOX_ASSERT(slot_index < m_value_stack.size());
    auto upvalue_object = m_heap->AllocateNativeUpvalueObject();
    upvalue_object->SetStackIndex(slot_index);

    m_open_upvalues.insert(prev_it, upvalue_object);

    return upvalue_object;
}
auto VirtualMachine::closeUpvalues(uint16_t stack_index) -> void
{
    auto it = m_open_upvalues.begin();
    while (it != m_open_upvalues.end() && (*it)->GetStackIndex() >= stack_index) {
        (*it)->Close(m_value_stack.at((*it)->GetStackIndex()));
        auto next = std::next(it);
        m_open_upvalues.erase(it);
        it = next;
    }
}
