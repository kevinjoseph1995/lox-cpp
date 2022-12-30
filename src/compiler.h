//
// Created by kevin on 8/12/22.
//

#ifndef LOX_CPP_COMPILER_H
#define LOX_CPP_COMPILER_H
#include <array>
#include <cstdint>
#include <optional>

#include "chunk.h"
#include "error.h"
#include "heap.h"
#include "parser_state.h"
#include "scanner.h"
#include "source.h"

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

struct Upvalue {
    uint16_t index {};
    bool is_local = false;
};

class Compiler {
public:
    Compiler() = delete;
    Compiler(Heap& heap, ParserState& parser_state, Compiler* parent_compiler = nullptr);
    [[nodiscard]] auto CompileSource(Source const& source) -> CompilationErrorOr<FunctionObject*>;

private:
    // Compiler state

    Source const* m_source = nullptr;
    FunctionObject* m_function = nullptr;
    Compiler* m_parent_compiler = nullptr;

    Heap& m_heap;
    ParserState& m_parser_state;

    struct LocalsState {
        struct Local {
            std::string_view identifier_name; // Underlying string is owned by the source
            int32_t local_scope_depth = 0; // Set to -1 after declaring a local and gets set to the actual scope when defining the variable
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
    auto function() -> void;
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
    [[nodiscard]] auto addUpvalue(uint16_t index, bool is_local) -> uint16_t;
    auto markInitialized() -> void;
    auto argumentList() -> uint16_t;

    // Expressions
    auto parsePrecedence(Precedence level) -> void;
    // Non-terminals
    auto expression() -> void;
    auto call(bool can_assign) -> void;
    auto binary(bool can_assign) -> void;
    auto grouping(bool can_assign) -> void;
    auto unary(bool can_assign) -> void;
    auto and_(bool can_assign) -> void;
    auto or_(bool can_assign) -> void;
    // Terminals
    auto literal(bool can_assign) -> void;
    auto number(bool can_assign) -> void;
    auto variable(bool can_assign) -> void;
    auto string(bool can_assign) -> void;
};

#endif // LOX_CPP_COMPILER_H
