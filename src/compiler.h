//
// Created by kevin on 8/12/22.
//

#ifndef LOX_CPP_COMPILER_H
#define LOX_CPP_COMPILER_H
#include <cstdint>
#include <optional>

#include "chunk.h"
#include "error.h"
#include "heap.h"
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
using ParseFunc = void (Compiler::*)(bool);
struct ParseRule {
    ParseFunc prefix;
    ParseFunc infix;
    Precedence precedence;
};
using ParseTable = std::array<ParseRule, static_cast<int>(TokenType::NUMBER_OF_TOKEN_TYPES)>;

class Compiler {
public:
    Compiler() = delete;
    Compiler(Heap& heap)
        : m_heap(heap)
    {
    }
    [[nodiscard]] ErrorOr<VoidType> CompileSource(Source const& source, Chunk& chunk);

private:
    // Compiler state
    Scanner m_scanner;
    Source const* m_source = nullptr;
    Chunk* m_current_chunk = nullptr;
    Heap& m_heap;

    struct ParserState {
        std::optional<Token> previous_token;
        std::optional<Token> current_token;
    } m_parser {};

    struct ErrorState {
        bool panic = false;
        bool encountered_error = false;
    } m_error_state;

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
        int32_t current_scope_depth = 0;
        std::vector<Local> locals;
    } m_locals_state;

private:
    friend consteval ParseTable GenerateParseTable();

    // Clear state
    void reset(Source const& source, Chunk& chunk);

    // Token processing
    void advance();
    bool consume(TokenType type);
    [[nodiscard]] bool match(TokenType type) const;

    // Error reporting
    void errorAt(Token const& token, std::string_view message);
    void reportError(int32_t line_number, std::string_view error_string);
    void synchronizeError();

    // Chunk manipulation functions
    void emitByte(uint8_t byte);
    void addConstant(Value constant);
    void emitIndex(uint16_t index);
    [[nodiscard]] uint64_t emitJump(OpCode op_code);
    [[nodiscard]] uint16_t identifierConstant(Token const& token);
    void patchJump(uint64_t offset);
    void emitLoop(uint64_t loop_start);

    // Statement parsing functions and associated helpers
    void declaration();
    void statement();
    void printStatement();
    void ifStatement();
    void whileStatement();
    void expressionStatement();
    void block();
    void beginScope();
    void endScope();
    void variableDeclaration();
    ErrorOr<uint16_t> parseVariable(std::string_view error_message);
    void declareVariable();
    void defineVariable(uint16_t constant_pool_index);
    std::optional<uint32_t> resolveVariable(std::string_view identifier_name);
    void markInitialized();

    // Expressions
    void parsePrecedence(Precedence level);
    // Non-terminals
    void expression();
    void binary(bool can_assign);
    void grouping(bool can_assign);
    void unary(bool can_assign);
    void and_(bool can_assign);
    void or_(bool can_assign);
    // Terminals
    void literal(bool can_assign);
    void number(bool can_assign);
    void variable(bool can_assign);
    void string(bool can_assign);
};

#endif // LOX_CPP_COMPILER_H
