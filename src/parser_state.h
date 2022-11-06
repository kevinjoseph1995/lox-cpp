//
// Created by kevin on 11/6/22.
//

#ifndef LOX_CPP_PARSER_STATE_H
#define LOX_CPP_PARSER_STATE_H

#include "scanner.h"
#include <optional>

class ParserState {
public:
    auto Initialize(Source const& source) -> void;
    auto Advance() -> void;
    auto Consume(TokenType type) -> bool;
    auto ErrorAt(Token const& token, std::string_view message) -> void;
    [[nodiscard]] auto Match(TokenType type) const -> bool;
    [[nodiscard]] auto CurrentToken() const -> std::optional<Token> const&
    {
        return current_token;
    }
    [[nodiscard]] auto PreviousToken() const -> std::optional<Token> const&
    {
        return previous_token;
    }
    auto ReportError(int32_t line_number, std::string_view error_string) -> void;
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
