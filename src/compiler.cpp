//
// Created by kevin on 8/12/22.
//

#include "compiler.h"
#include "fmt/core.h"
#include <functional>

consteval ParseTable Compiler::GenerateParseTable()
{
    ParseTable table;
    // clang-format off
    table[LEFT_PAREN]    = { .prefix = &Compiler::grouping, .infix = nullptr,           .precedence = PREC_NONE };
    table[RIGHT_PAREN]   = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[LEFT_BRACE]    = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[RIGHT_BRACE]   = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[COMMA]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[DOT]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[MINUS]         = { .prefix = &Compiler::unary,    .infix = &Compiler::binary, .precedence = PREC_TERM };
    table[PLUS]          = { .prefix = nullptr,             .infix = &Compiler::binary, .precedence = PREC_TERM };
    table[SEMICOLON]     = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[SLASH]         = { .prefix = nullptr,             .infix = &Compiler::binary, .precedence = PREC_FACTOR };
    table[STAR]          = { .prefix = nullptr,             .infix = &Compiler::binary, .precedence = PREC_FACTOR };
    table[BANG]          = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[BANG_EQUAL]    = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[EQUAL]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[EQUAL_EQUAL]   = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[GREATER]       = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[GREATER_EQUAL] = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[LESS]          = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[LESS_EQUAL]    = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[IDENTIFIER]    = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[STRING]        = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[NUMBER]        = { .prefix = &Compiler::number,   .infix = nullptr,           .precedence = PREC_NONE };
    table[AND]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[CLASS]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[ELSE]          = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[FALSE]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[FOR]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[FUN]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[IF]            = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[NIL]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[OR]            = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[PRINT]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[RETURN]        = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[SUPER]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[THIS]          = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[TRUE]          = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[VAR]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[WHILE]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[TOKEN_EOF]     = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    // clang-format on
    return table;
}

static constexpr auto PARSE_TABLE = Compiler::GenerateParseTable();

[[maybe_unused]] static void PrintTokens(std::vector<Token> const& tokens, const std::string* source)
{
    for (auto token : tokens) {
        fmt::print("{}\n", FormatToken(token, source));
    }
}

static constexpr ParseRule const* GetRule(TokenType type)
{
    return &PARSE_TABLE[type];
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
    this->expression();

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

void Compiler::parsePrecedence(Precedence level)
{
    advance();
    auto prefixRuleFunction = GetRule(m_parser.previous_token->type)->prefix;
    if (prefixRuleFunction == nullptr) {
        reportError("Expected expression");
        return;
    }
    (this->*prefixRuleFunction)();

    while (level <= GetRule(m_parser.current_token->type)->precedence) {
        advance();
        auto infixRuleFunction = GetRule(m_parser.previous_token->type)->infix;
        (this->*infixRuleFunction)();
    }
}

void Compiler::expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

void Compiler::grouping()
{
    expression();
    if (!consume(TokenType::RIGHT_PAREN)) {
        errorAt(m_parser.current_token.value(), "Expected \")\" at the end of a group expression");
    }
}

void Compiler::number()
{
    LOX_ASSERT(m_parser.previous_token.has_value());
    LOX_ASSERT(m_parser.previous_token->type == TokenType::NUMBER);
    LOX_ASSERT(m_parser.previous_token->start + m_parser.previous_token->length <= m_source_code->length());

    char* endpoint = nullptr;
    double value = std::strtod(m_source_code->data() + m_parser.previous_token->start, &endpoint);

    LOX_ASSERT(endpoint != m_source_code->data() + m_parser.previous_token->start);
    LOX_ASSERT(endpoint - m_source_code->data() + m_parser.previous_token->start == m_parser.previous_token->length);

    emitByte(OP_CONSTANT);
    addConstant(value);
}

void Compiler::binary()
{
    LOX_ASSERT(m_parser.previous_token.has_value());

    auto type = m_parser.previous_token->type;
    parsePrecedence(static_cast<Precedence>(GetRule(m_parser.previous_token->type)->precedence + 1));
    switch (type) {
    case PLUS:
        emitByte(OP_ADD);
        break;
    case MINUS:
        emitByte(OP_SUBTRACT);
        break;
    case STAR:
        emitByte(OP_MULTIPLY);
        break;
    case SLASH:
        emitByte(OP_DIVIDE);
        break;
    default:
        LOX_ASSERT(false); // Unreachable.
    }
}

void Compiler::unary()
{
    LOX_ASSERT(m_parser.previous_token.has_value());

    auto const type = m_parser.previous_token->type;
    parsePrecedence(PREC_UNARY);
    if (type == TokenType::MINUS) {
        emitByte(OP_NEGATE);
    } else {
        LOX_ASSERT(false);
    }
}
