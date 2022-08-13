//
// Created by kevin on 8/12/22.
//

#ifndef LOX_CPP_COMPILER_H
#define LOX_CPP_COMPILER_H

#include "common.h"
#include "error.h"
#include "scanner.h"

class Compiler {
public:
    ErrorOr<Chunk> CompileSource(std::string const* source);

private:
    void reset(std::string const* source);
    bool extract_tokens();

private:
    Scanner m_scanner;
    std::vector<Token> m_tokens;
};

#endif // LOX_CPP_COMPILER_H
