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

#ifndef LOX_CPP_COMPILER_H
#define LOX_CPP_COMPILER_H

#include "chunk.h"
#include "error.h"
#include "heap.h"
#include "object.h"
#include "parser_state.h"
#include "scanner.h"
#include "source.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

// clang-format off
enum  Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
};
// clang-format on

class Compiler;
using ParseFunc = auto (Compiler::*)(bool) -> void;
struct ParseRule {
    ParseFunc prefix;
    ParseFunc infix;
    Precedence precedence;
};
using ParseTable = std::array<ParseRule, static_cast<int>(TokenType::NUMBER_OF_TOKEN_TYPES)>;

class Compiler {
public:
    enum class FunctionCompilerType {
        TOP_LEVEL_SCRIPT,
        FUNCTION,
        METHOD,
        INITIALIZER
    };

    Compiler() = delete;
    Compiler(Heap& heap,
        ParserState& parser_state,
        Compiler* parent_compiler = nullptr,
        FunctionCompilerType function_type = FunctionCompilerType::TOP_LEVEL_SCRIPT);
    [[nodiscard]] auto CompileSource(Source const& source) -> CompilationErrorOr<FunctionObject*>;
    [[maybe_unused]] auto DumpCompiledChunk() const -> void;

private:
    friend class Heap;

    // Compiler state
    Source const* m_source = nullptr;
    FunctionObject* m_function = nullptr;
    Compiler* m_parent_compiler = nullptr;

    bool m_within_class = false;
    Heap& m_heap;
    ParserState& m_parser_state;
    FunctionCompilerType m_function_type = FunctionCompilerType::TOP_LEVEL_SCRIPT;

    struct LocalsState {
        struct Local {
            std::string_view identifier_name; // Underlying string is owned by the source
            int32_t local_scope_depth = 0;    // Set to -1 after declaring a local and gets set to the actual scope when defining the variable
            bool is_captured = false;
            Local() = default;
            Local(std::string_view identifier_name, int32_t local_scope_depth)
                : identifier_name(identifier_name)
                , local_scope_depth(local_scope_depth)
            {
            }
        };
        auto Reset() -> void
        {
            current_scope_depth = 0;
            locals.clear();
        }
        int32_t current_scope_depth = 0;
        std::vector<Local> locals;
    } m_locals_state;

    struct Upvalue {
        enum Type : uint8_t {
            NotLocal = 0,
            Local, // An upvalue is local if the associated variable associated
                   // is found in the directly enclosing function/closure.
        };
        Type type {};
        uint16_t index {}; // The relative offset to the found variable.
        // At runtime this offset will tell us how many slots
        // to skip over on the stack to get to the variable of interest.
    };
    std::vector<Upvalue> m_upvalues {};

private:
    friend consteval auto GenerateParseTable() -> ParseTable;

    auto synchronizeError() -> void;
    auto endCompiler() -> FunctionObject*;

    // Chunk manipulation functions
    auto emitByte(uint8_t byte) -> void;
    auto addConstant(Value constant) -> void;
    auto emitIndex(uint16_t index) -> void;
    [[nodiscard]] auto emitJump(OpCode op_code) -> uint64_t;
    [[nodiscard]] auto identifierConstant(Token const& token) -> uint16_t;
    auto patchJump(uint64_t offset) -> void;
    auto emitLoop(uint64_t loop_start) -> void;
    auto currentChunk() -> Chunk*;

    // Statement parsing functions and associated helpers
    auto declaration() -> void;
    auto statement() -> void;
    auto printStatement() -> void;
    auto functionDeclaration() -> void;
    auto returnStatement() -> void;
    auto classDeclaration() -> void;
    auto function(FunctionCompilerType function_type) -> void;
    auto method() -> void;
    auto setFunctionName() -> void;
    auto ifStatement() -> void;
    auto whileStatement() -> void;
    auto forStatement() -> void;
    auto expressionStatement() -> void;
    auto block() -> void;
    auto beginScope() -> void;
    auto endScope() -> void;
    auto variableDeclaration() -> void;
    [[nodiscard]] auto parseVariable(std::string_view error_message) -> ParseErrorOr<uint16_t>;
    auto declareVariable() -> void;
    auto defineVariable(uint16_t constant_pool_index) -> void;
    [[nodiscard]] auto resolveVariable(std::string_view identifier_name) -> std::optional<uint16_t>;
    [[nodiscard]] auto resolveUpvalue(std::string_view identifier_name) -> std::optional<uint16_t>;
    [[nodiscard]] auto addUpvalue(uint16_t index, Upvalue::Type type) -> uint16_t;
    auto markInitialized() -> void;
    auto argumentList() -> uint16_t;

    // Expressions
    auto parsePrecedence(Precedence level) -> void;
    // Non-terminals
    auto expression() -> void;
    auto call(bool can_assign) -> void;
    auto dot(bool can_assign) -> void;
    auto binary(bool can_assign) -> void;
    auto grouping(bool can_assign) -> void;
    auto unary(bool can_assign) -> void;
    auto and_(bool can_assign) -> void;
    auto or_(bool can_assign) -> void;
    // Terminals
    auto literal(bool can_assign) -> void;
    auto number(bool can_assign) -> void;
    auto variable(bool can_assign) -> void;
    auto this_(bool can_assign) -> void;
    auto namedVariable(std::string_view identifier_name, bool can_assign) -> void;
    auto string(bool can_assign) -> void;
};

#endif // LOX_CPP_COMPILER_H
