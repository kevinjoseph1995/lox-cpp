//
// Created by kevin on 8/12/22.
//

#ifndef LOX_CPP_COMPILER_H
#define LOX_CPP_COMPILER_H
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

private:
    friend consteval ParseTable GenerateParseTable();

    // Clear state
    void reset(Source const& source, Chunk& chunk);

    // Token processing
    void advance();
    [[nodiscard]] bool consume(TokenType type);
    [[nodiscard]] bool match(TokenType type) const;

    // Error reporting
    void errorAt(Token const& token, std::string_view message);
    void reportError(int32_t line_number, std::string_view error_string);
    void synchronizeError();

    // Chunk manipulation functions
    void emitByte(uint8_t byte);
    void addConstant(Value constant);
    void emitConstantIndex(uint16_t index);
    [[nodiscard]] int32_t identifierConstant(Token const& token);

    // Parse functions
    void declaration();
    void statement();
    void variableDeclaration();
    void printStatement();
    void expressionStatement();
    void block();
    // Non-terminals
    void parsePrecedence(Precedence level);
    void expression();
    void binary(bool can_assign);
    void grouping(bool can_assign);
    void unary(bool can_assign);
    // Terminals
    void literal(bool can_assign);
    void number(bool can_assign);
    void variable(bool can_assign);
    void string(bool can_assign);
};

#endif // LOX_CPP_COMPILER_H
