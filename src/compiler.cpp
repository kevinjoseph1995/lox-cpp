//
// Created by kevin on 8/12/22.
//

#include "compiler.h"

bool Compiler::CompileSource(std::string const *source)
{
    this->reset(source);

    if (!this->extract_tokens())
    {
        return false;
    }
    // TODO Parser
    return false;
}

void Compiler::reset(const std::string *source)
{
    m_scanner.Reset(source);
    m_tokens.clear(); // Clear old tokens
}

bool Compiler::extract_tokens()
{
    while (true)
    {
        auto token = m_scanner.GetNextToken();
        if (token.type == TokenType::TOKEN_ERROR)
        {
            return false;
        }
        m_tokens.push_back(token);
        if (token.type == TokenType::TOKEN_EOF)
        {
            break;
        }
    }
    return true;
}
