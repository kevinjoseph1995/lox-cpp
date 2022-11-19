//
// Created by kevin on 11/6/22.
//
#include "parser_state.h"
#include <cstdint>

auto ParserState::Initialize(const Source& source) -> void
{
    m_source = &source;
    m_scanner.Reset(source);
}

auto ParserState::Advance() -> void
{
    LOX_ASSERT(m_source != nullptr);
    previous_token = current_token;
    while (true) {
        auto token_or_error = m_scanner.GetNextToken();
        if (!token_or_error) {
            ReportError(previous_token->line_number, token_or_error.error().error_message);
        } else {
            current_token = token_or_error.value();
            break;
        }
    }
}

auto ParserState::Consume(TokenType type) -> bool
{
    LOX_ASSERT(m_source != nullptr);
    LOX_ASSERT(current_token.has_value());
    if (current_token->type == type) {
        Advance();
        return true;
    }
    return false;
}

auto ParserState::ReportError(uint64_t line_number, std::string_view error_string) -> void
{
    LOX_ASSERT(m_source != nullptr);
    if (m_panic) {
        return;
    }
    m_panic = true;
    if (m_source->IsFromFile()) {
        fmt::print(stderr, "Error:{} at {}:{}\n", error_string, m_source->GetFilename(), line_number);
    } else {
        fmt::print(stderr, "Error:{} on line:{}\n", error_string, line_number);
    }
    encountered_error = true;
}

bool ParserState::Match(TokenType type) const
{
    LOX_ASSERT(m_source != nullptr);
    LOX_ASSERT(current_token.has_value());
    if (current_token->type == type) {
        return true;
    }
    return false;
}

auto ParserState::ErrorAt(const Token& token, std::string_view message) -> void
{
    LOX_ASSERT(m_source != nullptr);
    LOX_ASSERT(token.start + token.length <= m_source->GetSource().length());

    auto error_string = fmt::format("[line {}] Error", token.line_number);

    if (token.type == TokenType::TOKEN_EOF) {
        error_string.append(" at end");
    } else {
        auto token_source = std::string_view(m_source->GetSource().data() + token.start, token.length);
        error_string.append(fmt::format(" at {}", token_source));
    }
    error_string.append(fmt::format(": {}\n", message));
    ReportError(token.line_number, error_string);
}
