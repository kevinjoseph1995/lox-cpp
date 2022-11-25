//
// Created by kevin on 8/6/22.
//

#ifndef LOX_CPP_VIRTUAL_MACHINE_H
#define LOX_CPP_VIRTUAL_MACHINE_H
#include <cstdint>
#include <memory>
#include <stack>
#include <unordered_map>

#include "chunk.h"
#include "compiler.h"
#include "error.h"
#include "heap.h"
#include "object.h"
#include "source.h"

using GlobalTable = std::unordered_map<std::string, Value>;

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
    [[nodiscard]] auto binaryOperation(OpCode op) -> RuntimeErrorOr<VoidType>;
    [[nodiscard]] auto runtimeError(std::string error_message) -> RuntimeError;
    [[nodiscard]] auto call(Value const& callable, uint16_t num_arguments) -> RuntimeErrorOr<VoidType>;
    [[maybe_unused]] auto dumpCallFrameStack() -> void;
    auto registerNativeFunctions() -> void;

private:
    struct CallFrame {
        CallFrame(ClosureObject const* f, uint64_t ip, uint64_t s)
            : closure(f)
            , instruction_pointer(ip)
            , slot(s)
        {
        }
        ClosureObject const* closure = nullptr;
        uint64_t instruction_pointer = 0;
        uint64_t slot = 0;
    };
    std::vector<CallFrame> m_frames;

    ParserState m_parser_state;
    std::unique_ptr<Compiler> m_compiler = nullptr;
    std::string* const m_external_stream = nullptr;

    Heap m_heap {};
    std::vector<Value> m_value_stack;
    GlobalTable m_globals;
};

#endif // LOX_CPP_VIRTUAL_MACHINE_H
