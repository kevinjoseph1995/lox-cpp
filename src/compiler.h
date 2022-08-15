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

class Compiler {
public:
    Compiler();
    /**
     *
     * @param source[in] Input source code
     * @param chunk[out] Output bytecode chunk
     * @return Compilation status, returns VoidType on successful compilation
     */
    ErrorOr<VoidType> CompileSource(std::string const* source, Chunk& chunk);

private:
    using ParseFunc = void (Compiler::*)();
    struct ParseRule {
        ParseFunc prefix;
        ParseFunc infix;
        Precedence precedence;
    };
    using ParseTable = std::array<ParseRule, static_cast<int>(TokenType::NUMBER_OF_TOKEN_TYPES)>;
    Scanner m_scanner;
    std::string const* m_source_code = nullptr;
    Chunk* m_current_chunk = nullptr;
    struct ParserState {
        std::optional<Token> previous_token;
        std::optional<Token> current_token;
    } m_parser {};

    bool m_panic = false;
    bool m_encountered_error = false;
    static ParseTable m_parse_table;

private:
    void reset(std::string const* source, Chunk& chunk);
    void errorAt(Token const& token, std::string_view message);
    void reportError(std::string_view error_string);
    void advance();
    bool consume(TokenType type);

    void emitByte(uint8_t byte);
    void addConstant(Value constant);

    void parsePrecedence(Precedence level);
    void expression();
    void number();
    void binary();
    void unary();
    void grouping();

    static ParseRule const* getRule(TokenType type);
};

#endif // LOX_CPP_COMPILER_H
