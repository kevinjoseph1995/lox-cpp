//
// Created by kevin on 8/12/22.
//

#include "compiler.h"
#include "fmt/core.h"

ErrorOr<Chunk> Compiler::CompileSource(std::string const* source)
{
    LOX_ASSERT(source != nullptr);
    this->reset(source);
    if (!this->extract_tokens()) {
        return Error { .type = ErrorType::ScanError, .error_message = "Error while scanning" };
    }

    return Chunk {};
}

void Compiler::reset(const std::string* source)
{
    m_scanner.Reset(source);
    m_tokens.clear(); // Clear old tokens
}

bool Compiler::extract_tokens()
{
    LOX_ASSERT(m_tokens.empty());
    for (;;) {
        auto token_or_result = m_scanner.GetNextToken();
        if (token_or_result.IsError()) {
            auto& error = token_or_result.GetError();
            fmt::print(stderr, "[ERROR][SCANNER] {}\n", error.error_message);
            fflush(stderr);
            return false;
        }
        auto& last_inserted = m_tokens.emplace_back(token_or_result.GetValue());
        if (last_inserted.type == TokenType::TOKEN_EOF) {
            break;
        }
    }
    return true;
}

[[maybe_unused]] static void printCurrentTokens(std::vector<Token> const& tokens, const std::string* source)
{
    for (auto token : tokens) {
        fmt::print("{}\n", FormatToken(token, source));
    }
}
