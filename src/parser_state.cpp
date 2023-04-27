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

#include "parser_state.h"

#include <cstdint>

auto ParserState::Initialize(Source const& source) -> void
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
            ReportError(previous_token->line_number, GetTokenSpan(*previous_token), token_or_error.error().error_message);
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

auto ParserState::ReportError(uint64_t const line_number, Span const& span, std::string_view const error_string) -> void
{
    LOX_ASSERT(m_source != nullptr);
    if (m_panic) {
        return;
    }
    m_panic = true;
    encountered_error = true;

    auto line_start = [this, &span]() {
        auto line_start = m_source->GetSource().rfind('\n', span.start);
        if (line_start == std::string::npos) {
            line_start = 0;
        }
        line_start += 1;
        return line_start;
    }();

    auto line_end = [this, &span]() {
        auto line_end = m_source->GetSource().find('\n', span.start);
        if (line_end == std::string::npos) {
            line_end = m_source->GetSource().length();
        }
        return line_end;
    }();
    auto error_line_prefix = fmt::format("{} |", line_number);
    auto line = std::string_view(m_source->GetSource().data() + line_start, line_end - line_start);
    auto error_message_prefix = std::string(error_line_prefix.length() + 1, ' ');
    fmt::print(stderr, "{}{}\n{}[{}]\n", error_line_prefix, line, error_message_prefix, error_string);
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
