//
// Created by kevin on 8/12/22.
//

#include "compiler.h"
#include "fmt/core.h"

[[maybe_unused]] static void PrintTokens(std::vector<Token> const& tokens, const std::string* source)
{
    for (auto token : tokens) {
        fmt::print("{}\n", FormatToken(token, source));
    }
}

void Compiler::reportError(std::string_view error_string)
{
    if (m_panic) {
        return;
    }
    m_panic = true;
    fmt::print(stderr, "{}", error_string);
    m_encountered_error = true;
}

void Compiler::errorAt(const Token& token, std::string_view message)
{
    LOX_ASSERT(token.start + token.length <= m_source_code->length());

    auto error_string = fmt::format("[line {}] Error", token.line_number);

    if (token.type == TokenType::TOKEN_EOF) {
        error_string.append(" at end");
    } else {
        auto token_source = std::string_view(m_source_code->data() + token.start, token.length);
        error_string.append(fmt::format(" at {}", token_source));
    }
    error_string.append(fmt::format(": {}\n", message));
    reportError(error_string);
}

void Compiler::reset(std::string const* source, Chunk& chunk)
{
    m_source_code = source;
    chunk.Reset();
    m_current_chunk = &chunk;
    m_scanner.Reset(source);
    m_parser = ParserState {};
}

ErrorOr<VoidType> Compiler::CompileSource(std::string const* source, Chunk& chunk)
{
    LOX_ASSERT(source != nullptr);
    this->reset(source, chunk);

    this->advance();

    return VoidType {};
}

void Compiler::advance()
{
    m_parser.previous_token = m_parser.current_token;
    while (true) {
        auto token_or_error = m_scanner.GetNextToken();
        m_parser.current_token = token_or_error.GetValue();
        if (token_or_error.IsError()) {
            reportError(token_or_error.GetError().error_message);
        } else {
            break;
        }
    }
}

bool Compiler::consume(TokenType type)
{
    LOX_ASSERT(m_parser.current_token.has_value());
    if (m_parser.current_token->type == type) {
        advance();
        return true;
    }
    return false;
}

void Compiler::emitByte(uint8_t byte)
{
    LOX_ASSERT(m_current_chunk != nullptr);
    m_current_chunk->byte_code.push_back(byte);
}

void Compiler::addConstant(Value constant)
{
    LOX_ASSERT(m_current_chunk != nullptr);
    LOX_ASSERT(m_current_chunk->constant_pool.size() < MAX_NUMBER_CONSTANTS);
    m_current_chunk->constant_pool.push_back(constant);
}

void Compiler::number()
{
    // Pre-conditions
    LOX_ASSERT(m_parser.previous_token.has_value());
    LOX_ASSERT(m_parser.previous_token->type == TokenType::NUMBER);
    LOX_ASSERT(m_parser.previous_token->start + m_parser.previous_token->length <= m_source_code->length());
    char* endpoint = nullptr;
    double value = std::strtod(m_source_code->data() + m_parser.previous_token->start, &endpoint);
    // Post conditions
    LOX_ASSERT(endpoint != m_source_code->data() + m_parser.previous_token->start);
    LOX_ASSERT(endpoint - m_source_code->data() + m_parser.previous_token->start == m_parser.previous_token->length);
    emitByte(OP_CONSTANT);
    addConstant(value);
}

void Compiler::expression()
{
}
void Compiler::grouping()
{
    expression();
    if (!consume(TokenType::RIGHT_PAREN)) {
        errorAt(m_parser.current_token.value(), "Expected \")\" at the end of a group expression");
    }
}
