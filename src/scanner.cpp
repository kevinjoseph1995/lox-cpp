//
// Created by kevin on 8/11/22.
//

#include "scanner.h"
#include "common.h"

void Scanner::Reset(const std::string *source_code)
{
    LOX_ASSERT(source_code != nullptr);
    m_source_code_ptr = source_code;
    m_current_index = 0;
    m_start = 0;
    m_line = 0;
}

Token Scanner::GetNextToken()
{
    LOX_ASSERT(m_source_code_ptr != nullptr);
    if (isAtEnd())
    {
        return this->makeToken(TokenType::TOKEN_EOF);
    }

    this->consumeWhitespaces();

    m_start = m_current_index;

    char ch = this->advance();

    if (std::isdigit(ch))
    {
        return this->number();
    }

    switch (ch)
    {
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
    case '!':
        return matchEqual() ? makeToken(TokenType::BANG_EQUAL) : makeToken(TokenType::BANG);
    case '=':
        return matchEqual() ? makeToken(TokenType::EQUAL_EQUAL) : makeToken(TokenType::EQUAL);
    case '>':
        return matchEqual() ? makeToken(TokenType::GREATER_EQUAL) : makeToken(TokenType::GREATER);
    case '<':
        return matchEqual() ? makeToken(TokenType::LESS_EQUAL) : makeToken(TokenType::LESS);
    case '"':
        return string();
    default:
        // TODO
        exit(1);
    }
}

char Scanner::advance()
{
    LOX_ASSERT(m_source_code_ptr != nullptr);
    // Update m_current_index and return previous
    return m_source_code_ptr->at(m_current_index++);
}

Token Scanner::makeToken(TokenType type) const
{
    LOX_ASSERT(m_source_code_ptr != nullptr);
    return Token{
        .type = type,
        .length = m_current_index - m_start,
        .line_number = m_line,
        .start = m_start,
        .error_message = std::string(),
    };
}

Token Scanner::errorToken(const char *error_message) const
{
    return Token{
        .type = TokenType::TOKEN_ERROR,
        .length = 0,
        .line_number = m_line,
        .start = m_start,
        .error_message = error_message,
    };
}

char Scanner::peek() const
{
    LOX_ASSERT(m_source_code_ptr != nullptr);
    LOX_ASSERT(!isAtEnd());
    return m_source_code_ptr->at(m_current_index);
}

void Scanner::consumeWhitespaces()
{
    LOX_ASSERT(m_source_code_ptr != nullptr);
    LOX_ASSERT(!isAtEnd());
    while (!isAtEnd())
    {
        char c = this->peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            ++m_line;
            advance();
            break;
        case '/':
            if (m_current_index + 1 < m_source_code_ptr->length() && m_source_code_ptr->at(m_current_index + 1) == '/')
            {
                // Skip a comment line
                while (!isAtEnd() && peek() != '\n')
                {
                    advance();
                }
            }
            break;
        default:
            return;
        }
    }
}

bool Scanner::matchEqual()
{
    LOX_ASSERT(!isAtEnd());
    if ('=' == m_source_code_ptr->at(m_current_index))
    {
        ++m_current_index;
        return true;
    }
    else
    {
        return false;
    }
}

Token Scanner::string()
{
    LOX_ASSERT(!isAtEnd());
    while (peek() != '"' && !isAtEnd())
    {
        advance();
    }
    if (isAtEnd())
    {
        return this->errorToken("Unterminated string literal");
    }
    advance(); // Move past the closing quotes
    return makeToken(TokenType::STRING);
}

bool Scanner::isAtEnd() const
{
    return m_current_index == m_source_code_ptr->length();
}

Token Scanner::number()
{
    while (!isAtEnd() && std::isdigit(peek()))
    {
        advance();
    }
    if (!isAtEnd() && peek() == '.')
    {
        advance();
        while (!isAtEnd() && std::isdigit(peek()))
        {
            advance();
        }
    }
    return makeToken(TokenType::NUMBER);
}
