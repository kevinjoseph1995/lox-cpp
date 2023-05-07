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

#ifndef LOX_CPP_VIRTUAL_MACHINE_H
#define LOX_CPP_VIRTUAL_MACHINE_H

#include <cstdint>
#include <list>
#include <memory>
#include <stack>
#include <string_view>
#include <unordered_map>

#include "chunk.h"
#include "compiler.h"
#include "error.h"
#include "heap.h"
#include "object.h"
#include "source.h"

class VirtualMachine {
public:
    VirtualMachine(std::string* external_stream = nullptr);

    [[nodiscard]] auto Interpret(Source const& source_code) -> ErrorOr<VoidType>;

private:
    [[nodiscard]] auto currentChunk() -> Chunk const&;
    [[nodiscard]] auto isAtEnd() -> bool;
    [[nodiscard]] auto run() -> RuntimeErrorOr<VoidType>;
    [[nodiscard]] auto readByte() -> uint8_t;
    [[nodiscard]] auto readConstant() -> Value;
    [[nodiscard]] auto readIndex() -> uint16_t;
    [[nodiscard]] auto popStack() -> Value;
    [[nodiscard]] auto peekStack(uint32_t index_from_top) -> Value const&;
    [[nodiscard]] auto captureUpvalue(uint16_t index) -> UpvalueObject*;
    [[nodiscard]] auto binaryOperation(OpCode op) -> RuntimeErrorOr<VoidType>;
    [[nodiscard]] auto runtimeError(std::string error_message) -> RuntimeError;
    [[nodiscard]] auto call(Value& callable, uint16_t num_arguments) -> RuntimeErrorOr<VoidType>;
    auto closeUpvalues(uint16_t stack_index) -> void;
    [[maybe_unused]] auto dumpCallFrameStack() -> void;
    auto registerNativeFunctions() -> void;

private:
    struct CallFrame {
        CallFrame(ClosureObject* f, uint64_t ip, uint64_t s)
            : closure(f)
            , instruction_pointer(ip)
            , slot(s)
        {
        }
        ClosureObject* closure = nullptr;
        uint64_t instruction_pointer = 0;
        uint64_t slot = 0;
    };
    std::vector<CallFrame> m_frames;

    ParserState m_parser_state;
    std::unique_ptr<Compiler> m_compiler = nullptr;
    std::string* const m_external_stream = nullptr;

    std::vector<Value> m_value_stack;
    Table m_globals;
    std::list<UpvalueObject*> m_open_upvalues;
    // Make sure the heap is the last object that's destroyed as it's the owner of all lox Objects
    std::unique_ptr<Heap> m_heap { nullptr };
    // This is an unfortuante intertwining dependency that's being injected. TODO: Refactor this
    // Basically the heap is owned by the virtual machine but the heap can access the innards of the the virtual machine.
    friend class Heap;
};

#endif // LOX_CPP_VIRTUAL_MACHINE_H
