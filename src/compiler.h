//
// Created by kevin on 8/12/22.
//

#ifndef LOX_CPP_COMPILER_H
#define LOX_CPP_COMPILER_H

#include "common.h"
#include "error.h"
#include "scanner.h"
#include <optional>

class Compiler {
public:
    /**
     *
     * @param source[in] Input source code
     * @param chunk[out] Output bytecode chunk
     * @return Compilation status, returns VoidType on successful compilation
     */
    ErrorOr<VoidType> CompileSource(std::string const* source, Chunk& chunk);

private:
    void reset(std::string const* source, Chunk& chunk);
    void errorAt(Token const& token, std::string_view message);
    void reportError(std::string_view error_string);
    void advance();
    bool consume(TokenType type);

    void emitByte(uint8_t byte);
    void addConstant(Value constant);

    void expression();
    void grouping();
    void number();

private:
    Scanner m_scanner;
    std::string const* m_source_code = nullptr;
    Chunk* m_current_chunk = nullptr;
    struct ParserState {
        std::optional<Token> previous_token;
        std::optional<Token> current_token;
    } m_parser {};

    bool m_panic = false;
    bool m_encountered_error = false;
};

#endif // LOX_CPP_COMPILER_H
