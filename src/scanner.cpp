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

#include "scanner.h"
#include "error.h"

auto GetTokenSpan(Token const& token) -> Span
{
    return { token.start, token.start + token.length };
}

auto Scanner::Reset(Source const& source) -> void
{
    m_source = &source;
    m_current_index = 0;
    m_start = 0;
    m_line = 1;
}

auto Scanner::GetNextToken() -> ScanErrorOr<Token>
{
    LOX_ASSERT(m_source != nullptr);

    this->consumeWhitespacesAndComments();

    if (isAtEnd()) {
        return this->makeToken(TokenType::TOKEN_EOF);
    }

    m_start = m_current_index;

    char ch = this->advance();

    if (std::isalpha(ch)) {
        return this->identifierOrKeyword();
    }

    if (std::isdigit(ch)) {
        return this->number();
    }

    switch (ch) {
    // Single-character tokens.
    case '(':
        return makeToken(TokenType::LEFT_PAREN);
    case ')':
        return makeToken(TokenType::RIGHT_PAREN);
    case '{':
        return makeToken(TokenType::LEFT_BRACE);
    case '}':
        return makeToken(TokenType::RIGHT_BRACE);
    case ',':
        return makeToken(TokenType::COMMA);
    case '.':
        return makeToken(TokenType::DOT);
    case '-':
        return makeToken(TokenType::MINUS);
    case '+':
        return makeToken(TokenType::PLUS);
    case ';':
        return makeToken(TokenType::SEMICOLON);
    case '/':
        return makeToken(TokenType::SLASH);
    case '*':
        return makeToken(TokenType::STAR);
    // One or two character tokens.
    case '!': {
        auto result = matchEqual();
        if (!result)
            return std::unexpected(result.error());
        return result.value() ? makeToken(TokenType::BANG_EQUAL)
                              : makeToken(TokenType::BANG);
    }
    case '=': {
        auto result = matchEqual();
        if (!result)
            return std::unexpected(result.error());
        return result.value() ? makeToken(TokenType::EQUAL_EQUAL)
                              : makeToken(TokenType::EQUAL);
    }
    case '>': {
        auto result = matchEqual();
        if (!result)
            return std::unexpected(result.error());
        return result.value() ? makeToken(TokenType::GREATER_EQUAL)
                              : makeToken(TokenType::GREATER);
    }
    case '<': {
        auto result = matchEqual();
        if (!result)
            return std::unexpected(result.error());
        return result.value() ? makeToken(TokenType::LESS_EQUAL)
                              : makeToken(TokenType::LESS);
    }
    case '"':
        return this->string();
    default:
        return std::unexpected(ScanError { { fmt::format("Unidentified character: \"{}\"(index:{})", m_source->GetSource().at(m_start), m_start) }, Span { m_start, m_start } });
    }
}

auto Scanner::advance() -> char
{
    LOX_ASSERT(m_source != nullptr);
    // Update m_current_index and return previous
    return m_source->GetSource().at(m_current_index++);
}

auto Scanner::makeToken(TokenType type) const -> Token
{
    LOX_ASSERT(m_source != nullptr);
    return Token {
        .type = type,
        .length = m_current_index - m_start,
        .line_number = m_line,
        .start = m_start,
    };
}

auto Scanner::peek() const -> char
{
    LOX_ASSERT(m_source != nullptr);
    LOX_ASSERT(!isAtEnd());
    return m_source->GetSource().at(m_current_index);
}

auto Scanner::consumeWhitespacesAndComments() -> void
{
    LOX_ASSERT(m_source != nullptr);
    while (!isAtEnd()) {
        char c = this->peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t': {
            advance();
            break;
        }
        case '\n': {
            ++m_line;
            advance();
            break;
        }
        case '/': {
            if (m_current_index + 1 < m_source->GetSource().length() && m_source->GetSource().at(m_current_index + 1) == '/') {
                // Skip a comment line
                while (!isAtEnd() && peek() != '\n') {
                    advance();
                }
                break;
            }
        }
        default:
            return;
        }
    }
}

auto Scanner::matchEqual() -> ScanErrorOr<bool>
{
    if (isAtEnd()) {
        return std::unexpected(ScanError { { "Expected tokens after \"=\"" }, Span { static_cast<uint64_t>(m_current_index - 1), static_cast<uint64_t>(m_current_index - 1) } });
    }
    if ('=' == m_source->GetSource().at(m_current_index)) {
        ++m_current_index;
        return true;
    } else {
        return false;
    }
}

auto Scanner::string() -> ScanErrorOr<Token>
{
    if (isAtEnd()) {
        return std::unexpected(ScanError {
            { "Unterminated string literal" }, Span { m_start, m_current_index } });
    }
    while (peek() != '"' && !isAtEnd()) {
        advance();
    }
    advance(); // Move past the closing quotes
    return makeToken(TokenType::STRING);
}

auto Scanner::isAtEnd() const -> bool
{
    return m_current_index == m_source->GetSource().length();
}

auto Scanner::number() -> ScanErrorOr<Token>
{
    while (!isAtEnd() && std::isdigit(peek())) {
        advance();
    }
    if (!isAtEnd() && peek() == '.') {
        advance();
        while (!isAtEnd() && std::isdigit(peek())) {
            advance();
        }
    }
    return makeToken(TokenType::NUMBER);
}

auto Scanner::identifierOrKeyword() -> ScanErrorOr<Token>
{
    while (!this->isAtEnd() && (std::isalnum(m_source->GetSource().at(m_current_index)) || m_source->GetSource().at(m_current_index) == '_')) {
        advance();
    }
    // The token lies between [m_start, m_current_index)

    auto getTokenType = [](char const* const start, uint64_t length) -> TokenType {
        LOX_ASSERT(length >= 1);
        // Pre-conditions are that (start + length - 1) is a valid index into the string
        // Check if keyword
        switch (start[0]) {
        case 'a':
            return length == 3 && strncmp(start + 1, "nd", 2) == 0 ? TokenType::AND : TokenType::IDENTIFIER;
        case 'c':
            return length == 5 && strncmp(start + 1, "lass", 4) == 0 ? TokenType::CLASS : TokenType::IDENTIFIER;
        case 'e':
            return length == 4 && strncmp(start + 1, "lse", 3) == 0 ? TokenType::ELSE : TokenType::IDENTIFIER;
        case 'i':
            return length == 2 && strncmp(start + 1, "f", 1) == 0 ? TokenType::IF : TokenType::IDENTIFIER;
        case 'n':
            return length == 3 && strncmp(start + 1, "il", 2) == 0 ? TokenType::NIL : TokenType::IDENTIFIER;
        case 'o':
            return length == 2 && strncmp(start + 1, "r", 1) == 0 ? TokenType::OR : TokenType::IDENTIFIER;
        case 'p':
            return length == 5 && strncmp(start + 1, "rint", 4) == 0 ? TokenType::PRINT : TokenType::IDENTIFIER;
        case 'r':
            return length == 6 && strncmp(start + 1, "eturn", 5) == 0 ? TokenType::RETURN : TokenType::IDENTIFIER;
        case 's':
            return length == 5 && strncmp(start + 1, "uper", 4) == 0 ? TokenType::SUPER : TokenType::IDENTIFIER;
        case 'v':
            return length == 3 && strncmp(start + 1, "ar", 2) == 0 ? TokenType::VAR : TokenType::IDENTIFIER;
        case 'w':
            return length == 5 && strncmp(start + 1, "hile", 4) == 0 ? TokenType::WHILE : TokenType::IDENTIFIER;
        case 'f': {
            if (length > 1) {
                switch (start[1]) {
                case 'a':
                    return length == 5 && strncmp(start + 2, "lsa", 2) == 0 ? TokenType::FALSE : TokenType::IDENTIFIER;
                case 'o':
                    return length == 3 && strncmp(start + 2, "r", 1) == 0 ? TokenType::FOR : TokenType::IDENTIFIER;
                case 'u':
                    return length == 3 && strncmp(start + 2, "n", 1) == 0 ? TokenType::FUN : TokenType::IDENTIFIER;
                default:
                    return TokenType::IDENTIFIER;
                }
            }
            return TokenType::IDENTIFIER;
        }
        case 't': {
            if (length > 1) {
                switch (start[1]) {
                case 'h':
                    return length == 4 && strncmp(start + 2, "is", 2) == 0 ? TokenType::THIS : TokenType::IDENTIFIER;
                case 'r':
                    return length == 4 && strncmp(start + 2, "ue", 1) == 0 ? TokenType::TRUE : TokenType::IDENTIFIER;
                default:
                    return TokenType::IDENTIFIER;
                }
            }
            return TokenType::IDENTIFIER;
        }
        default:
            return TokenType::IDENTIFIER;
        }
    };
    LOX_ASSERT(m_current_index <= m_source->GetSource().length());
    return makeToken(getTokenType(m_source->GetSource().data() + m_start, m_current_index - m_start));
}

[[maybe_unused]] auto GetTokenTypeString(TokenType type) -> char const*
{
    switch (type) {
    case TokenType::LEFT_PAREN:
        return "LEFT_PAREN";
    case TokenType::RIGHT_PAREN:
        return "RIGHT_PAREN";
    case TokenType::LEFT_BRACE:
        return "LEFT_BRACE";
    case TokenType::RIGHT_BRACE:
        return "RIGHT_BRACE";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::DOT:
        return "DOT";
    case TokenType::MINUS:
        return "MINUS";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::SEMICOLON:
        return "SEMICOLON";
    case TokenType::SLASH:
        return "SLASH";
    case TokenType::STAR:
        return "STAR";
    case TokenType::BANG:
        return "BANG";
    case TokenType::BANG_EQUAL:
        return "BANG_EQUAL";
    case TokenType::EQUAL:
        return "EQUAL";
    case TokenType::EQUAL_EQUAL:
        return "EQUAL_EQUAL";
    case TokenType::GREATER:
        return "GREATER";
    case TokenType::GREATER_EQUAL:
        return "GREATER_EQUAL";
    case TokenType::LESS:
        return "LESS";
    case TokenType::LESS_EQUAL:
        return "LESS_EQUAL";
    case TokenType::STRING:
        return "STRING";
    case TokenType::NUMBER:
        return "NUMBER";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::AND:
        return "AND";
    case TokenType::CLASS:
        return "CLASS";
    case TokenType::ELSE:
        return "ELSE";
    case TokenType::FALSE:
        return "FALSE";
    case TokenType::FOR:
        return "FOR";
    case TokenType::FUN:
        return "FUN";
    case TokenType::IF:
        return "IF";
    case TokenType::NIL:
        return "NIL";
    case TokenType::OR:
        return "OR";
    case TokenType::PRINT:
        return "PRINT";
    case TokenType::RETURN:
        return "RETURN";
    case TokenType::SUPER:
        return "SUPER";
    case TokenType::THIS:
        return "THIS";
    case TokenType::TRUE:
        return "TRUE";
    case TokenType::VAR:
        return "VAR";
    case TokenType::WHILE:
        return "WHILE";
    case TokenType::TOKEN_EOF:
        return "TOKEN_EOF";
    case TokenType::NUMBER_OF_TOKEN_TYPES:
        LOX_ASSERT(false);
    }
    LOX_ASSERT(false);
}

auto FormatToken(Token const& token, std::string const* source_code) -> std::string
{
    if (token.type == TokenType::IDENTIFIER) {
        auto name = std::string_view(source_code->data() + token.start, token.length);
        return fmt::format("IDENTIFIER[{}] LineNumber:{}  StartIndex:{} Length:{}",
            name, token.line_number, token.start, token.length);
    } else if (token.type == TokenType::STRING) {
        auto string_literal = std::string_view(source_code->data() + token.start, token.length);
        return fmt::format("STRING[{}] LineNumber:{}  StartIndex:{} Length:{}",
            string_literal, token.line_number, token.start, token.length);
    } else if (token.type == TokenType::NUMBER) {
        auto number_literal = std::string_view(source_code->data() + token.start, token.length);
        return fmt::format("NUMBER[{}] LineNumber:{}  StartIndex:{} Length:{}",
            number_literal, token.line_number, token.start, token.length);
    } else {
        return fmt::format("{} LineNumber:{}  StartIndex:{} Length:{}",
            GetTokenTypeString(token.type), token.line_number, token.start, token.length);
    }
}
