//
// Created by kevin on 8/12/22.
//

#ifndef LOX_CPP_COMPILER_H
#define LOX_CPP_COMPILER_H

#include "chunk.h"

#include <array>
#include <optional>

#include "error.h"
#include "heap.h"
#include "scanner.h"

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
using ParseFunc = void (Compiler::*)();
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
    /**
     *
     * @param source[in] Input source code
     * @param chunk[out] Output bytecode chunk
     * @return Compilation status, returns VoidType on successful compilation
     */
    [[nodiscard]] ErrorOr<VoidType> CompileSource(std::string const& source, Chunk& chunk);

private:
    // Compiler state
    Scanner m_scanner;
    std::string const* m_source_code = nullptr;
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

    // Reset state
    void reset(std::string const& source, Chunk& chunk);

    // Token processing
    void advance();
    [[nodiscard]] bool consume(TokenType type);
    [[nodiscard]] bool match(TokenType type);

    // Error reporting
    void errorAt(Token const& token, std::string_view message);
    void reportError(std::string_view error_string);
    void synchronizeError();

    // Chunk manipulation functions
    void emitByte(uint8_t byte);
    void addConstant(Value constant);
    [[nodiscard]] int32_t identifierConstant(Token const& token);

    // Parse functions
    void parsePrecedence(Precedence level);
    void declaration();
    void statement();
    void variableDeclaration();
    void printStatement();
    void expressionStatement();
    void expression();
    void literal();
    void variable();
    void number();
    void binary();
    void unary();
    void grouping();
    void string();
};

#endif // LOX_CPP_COMPILER_H
