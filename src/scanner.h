//
// Created by kevin on 8/11/22.
//

#ifndef LOX_CPP_SCANNER_H
#define LOX_CPP_SCANNER_H

#include <cassert>
#include <string>

#include "error.h"
enum TokenType {
    // Single-character tokens.
    LEFT_PAREN = 0,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,
    // One or two character tokens.
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    // Literals.
    STRING,
    NUMBER,
    IDENTIFIER,
    // Keywords.
    AND,
    CLASS,
    ELSE,
    FALSE,
    FOR,
    FUN,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    TOKEN_EOF,
    NUMBER_OF_TOKEN_TYPES
};

struct Token {
    TokenType type {};
    uint64_t length {};
    uint64_t line_number {};
    uint64_t start {};
};

[[maybe_unused]] [[maybe_unused]] char const* GetTokenTypeString(TokenType type);
std::string FormatToken(Token const& token, std::string const* source_code);

class Scanner {
public:
    /**
     * Resets scanner state with new source code to scan
     * @param source_code
     */
    void Reset(std::string const* source_code);

    /**
     *
     * @return Next token(can be in invalid state)
     */
    ErrorOr<Token> GetNextToken();

private:
    [[nodiscard]] ErrorOr<Token> number();

    [[nodiscard]] ErrorOr<Token> string();

    [[nodiscard]] ErrorOr<Token> identifierOrKeyword();

    [[nodiscard]] char peek() const;

    char advance();

    [[nodiscard]] bool matchEqual();

    [[nodiscard]] Token makeToken(TokenType) const;

    void consumeWhitespacesAndComments();

    [[nodiscard]] bool isAtEnd() const;

private:
    std::string const* m_source_code_ptr = nullptr;
    uint64_t m_current_index = 0; // Current scanner index
    uint64_t m_start = 0; // Start of the current token under consideration
    uint64_t m_line = 0; // Current line number
};

#endif // LOX_CPP_SCANNER_H
