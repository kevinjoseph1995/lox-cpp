//
// Created by kevin on 8/12/22.
//

#ifndef LOX_CPP_COMPILER_H
#define LOX_CPP_COMPILER_H

#include "common.h"
#include "error.h"
#include "scanner.h"
#include <array>
#include <optional>

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
    /**
     *
     * @param source[in] Input source code
     * @param chunk[out] Output bytecode chunk
     * @return Compilation status, returns VoidType on successful compilation
     */
    ErrorOr<VoidType> CompileSource(std::string const* source, Chunk& chunk);

private:
    // Compiler state
    Scanner m_scanner;
    std::string const* m_source_code = nullptr;
    Chunk* m_current_chunk = nullptr;
    struct ParserState {
        std::optional<Token> previous_token;
        std::optional<Token> current_token;
    } m_parser {};

    //  Error state
    bool m_panic = false;
    bool m_encountered_error = false;

private:
    friend consteval ParseTable GenerateParseTable();

    // Reset state
    void reset(std::string const* source, Chunk& chunk);

    // Token processing
    void advance();
    bool consume(TokenType type);

    // Error reporting
    void errorAt(Token const& token, std::string_view message);
    void reportError(std::string_view error_string);

    // Chunk manipulation functions
    void emitByte(uint8_t byte);
    void addConstant(Value constant);

    // Parse functions
    void parsePrecedence(Precedence level);
    void expression();
    void number();
    void binary();
    void unary();
    void grouping();
};

#endif // LOX_CPP_COMPILER_H
