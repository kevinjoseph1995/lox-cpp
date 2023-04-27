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

#ifndef LOX_CPP_PARSER_STATE_H
#define LOX_CPP_PARSER_STATE_H

#include "scanner.h"

#include <optional>

class ParserState {
public:
    auto Initialize(Source const& source) -> void;
    auto Advance() -> void;
    auto Consume(TokenType type) -> bool;
    [[nodiscard]] auto Match(TokenType type) const -> bool;
    [[nodiscard]] auto CurrentToken() const -> std::optional<Token> const&
    {
        return current_token;
    }
    [[nodiscard]] auto PreviousToken() const -> std::optional<Token> const&
    {
        return previous_token;
    }
    auto ReportError(uint64_t const line_number, Span const& span, std::string_view const error_string) -> void;
    auto ResetPanicState() -> void
    {
        m_panic = false;
    }
    auto InPanicState() const -> bool
    {
        return m_panic;
    }
    auto EncounteredError() const -> bool
    {
        return encountered_error;
    }

private:
    Scanner m_scanner;
    Source const* m_source = nullptr;

    std::optional<Token> previous_token;
    std::optional<Token> current_token;
    auto Reset() -> void
    {
        previous_token.reset();
        current_token.reset();
    }

    bool m_panic = false;
    bool encountered_error = false;
};
#endif // LOX_CPP_PARSER_STATE_H
