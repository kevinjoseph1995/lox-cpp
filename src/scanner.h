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

#ifndef LOX_CPP_SCANNER_H
#define LOX_CPP_SCANNER_H

#include "error.h"
#include "source.h"

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

auto GetTokenSpan(Token const& token) -> Span;

[[maybe_unused]] [[maybe_unused]] char const* GetTokenTypeString(TokenType type);

std::string FormatToken(Token const& token, std::string const* source_code);

class Scanner {
public:
    auto Reset(Source const& source_code) -> void;
    [[nodiscard]] auto GetNextToken() -> ScanErrorOr<Token>;

private:
    [[nodiscard]] auto number() -> ScanErrorOr<Token>;
    [[nodiscard]] auto string() -> ScanErrorOr<Token>;
    [[nodiscard]] auto identifierOrKeyword() -> ScanErrorOr<Token>;
    [[nodiscard]] auto peek() const -> char;
    [[nodiscard]] auto matchEqual() -> ScanErrorOr<bool>;
    [[nodiscard]] auto makeToken(TokenType) const -> Token;
    [[nodiscard]] auto isAtEnd() const -> bool;

    auto consumeWhitespacesAndComments() -> void;
    auto advance() -> char;

private:
    Source const* m_source = nullptr;
    uint64_t m_current_index = 0; // Current scanner index
    uint64_t m_start = 0;         // Start of the current token under consideration
    uint64_t m_line = 0;          // Current line number
};

#endif // LOX_CPP_SCANNER_H
