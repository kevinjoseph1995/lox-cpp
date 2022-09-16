//
// Created by kevin on 8/12/22.
//

#include <functional>

#include "compiler.h"
#include "fmt/core.h"

consteval ParseTable GenerateParseTable()
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
    table[BANG]          = { .prefix = &Compiler::unary,    .infix = nullptr,           .precedence = PREC_NONE };
    table[EQUAL]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[BANG_EQUAL]    = { .prefix = nullptr,             .infix = &Compiler::binary, .precedence = PREC_EQUALITY };
    table[EQUAL_EQUAL]   = { .prefix = nullptr,             .infix = &Compiler::binary, .precedence = PREC_EQUALITY };
    table[GREATER]       = { .prefix = nullptr,             .infix = &Compiler::binary, .precedence = PREC_COMPARISON };
    table[GREATER_EQUAL] = { .prefix = nullptr,             .infix = &Compiler::binary, .precedence = PREC_COMPARISON };
    table[LESS]          = { .prefix = nullptr,             .infix = &Compiler::binary, .precedence = PREC_COMPARISON };
    table[LESS_EQUAL]    = { .prefix = nullptr,             .infix = &Compiler::binary, .precedence = PREC_COMPARISON };
    table[IDENTIFIER]    = { .prefix = &Compiler::variable, .infix = nullptr,           .precedence = PREC_NONE };
    table[STRING]        = { .prefix = &Compiler::string,   .infix = nullptr,           .precedence = PREC_NONE };
    table[NUMBER]        = { .prefix = &Compiler::number,   .infix = nullptr,           .precedence = PREC_NONE };
    table[AND]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[CLASS]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[ELSE]          = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[FALSE]         = { .prefix = &Compiler::literal,  .infix = nullptr,           .precedence = PREC_NONE };
    table[FOR]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[FUN]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[IF]            = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[NIL]           = { .prefix = &Compiler::literal,  .infix = nullptr,           .precedence = PREC_NONE };
    table[OR]            = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[PRINT]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[RETURN]        = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[SUPER]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[THIS]          = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[TRUE]          = { .prefix = &Compiler::literal,  .infix = nullptr,           .precedence = PREC_NONE };
    table[VAR]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[WHILE]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[TOKEN_EOF]     = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    // clang-format on
    return table;
}

static constexpr auto PARSE_TABLE = GenerateParseTable();

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

void Compiler::reportError(int32_t line_number, std::string_view error_string)
{
    if (m_error_state.panic) {
        return;
    }
    m_error_state.panic = true;
    if (m_source->IsFromFile()) {
        fmt::print(stderr, "Error:{} at {}:{}\n", error_string, m_source->GetFilename(), line_number);
    } else {
        fmt::print(stderr, "Error:{} on line:{}\n", error_string, line_number);
    }
    m_error_state.encountered_error = true;
}

void Compiler::errorAt(const Token& token, std::string_view message)
{
    LOX_ASSERT(token.start + token.length <= m_source->GetSource().length());

    auto error_string = fmt::format("[line {}] Error", token.line_number);

    if (token.type == TokenType::TOKEN_EOF) {
        error_string.append(" at end");
    } else {
        auto token_source = std::string_view(m_source->GetSource().data() + token.start, token.length);
        error_string.append(fmt::format(" at {}", token_source));
    }
    error_string.append(fmt::format(": {}\n", message));
    reportError(token.line_number, error_string);
}

void Compiler::reset(Source const& source, Chunk& chunk)
{
    m_source = &source;
    m_current_chunk = &chunk;
    m_scanner.Reset(source);
    m_parser = ParserState {};
}

ErrorOr<VoidType> Compiler::CompileSource(Source const& source, Chunk& chunk)
{
    this->reset(source, chunk);

    this->advance();

    while (!match(TokenType::TOKEN_EOF)) {
        declaration();
    }

    if (m_error_state.encountered_error) {
        return Error { .type = ErrorType::ParseError, .error_message = "Parse error" };
    }

    return VoidType {};
}

void Compiler::advance()
{
    m_parser.previous_token = m_parser.current_token;
    while (true) {
        auto token_or_error = m_scanner.GetNextToken();
        if (token_or_error.IsError()) {
            reportError(m_parser.previous_token->line_number, token_or_error.GetError().error_message);
        } else {
            m_parser.current_token = token_or_error.GetValue();
            break;
        }
    }
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
    emitByte(OP_CONSTANT);
    emitConstantIndex(static_cast<uint16_t>(m_current_chunk->constant_pool.size() - 1));
}

void Compiler::parsePrecedence(Precedence level)
{
    /*
     * Explanation for future me on how Pratt Parsing works
     * Example 1: Parsing the expression -1 == 1
     * The "-" token will be mapped to the "unary" member-function which will generate the op-code for the negation and also number-literal "1"
     * On returning from  (this->*prefixRuleFunction)(); the current token will be EQUAL_EQUAL.
     * Since EQUAL_EQUAL has a higher precedence than PREC_ASSIGNMENT which is the default precedence with which "expression" is called, we parse the infix expression
     *
     * Example 2: Parsing the expression 2 + -1 * 3
     *  We enter parsePrecedence with a precedence of PREC_ASSIGNMENT
     *  The first token is a number literal "number" is called. Since it's a terminal it's guaranteed that  prefixRuleFunction() will not call parsePrecedence internally.
     *
     *  At the end of (this->*prefixRuleFunction)(); in this level the current token will be PLUS.
     *  Since PREC_TERM > PREC_ASSIGNMENT we will try to parse the infix expression next.
     *      On calling advance previous=PLUS and current=MINUS
     *      We then call Compiler::binary. Inside binary, we need to parse the right-hand side of the binary operator. We call parsePrecedence with a precedence one higher
     *      than the current level("because the binary operators are left-associative"). In this special case the RHS parsing will end up consuming all the remaining tokens.
     *      It does not stop at (MINUS, NUMBER) because STAR has a precedence == (current_precedence_level + 1).
     */
    advance();
    auto prefixRuleFunction = GetRule(m_parser.previous_token->type)->prefix;
    if (prefixRuleFunction == nullptr) {
        reportError(m_parser.previous_token->line_number, "Expected expression");
        return;
    }
    bool can_assign = level <= PREC_ASSIGNMENT;
    (this->*prefixRuleFunction)(can_assign);

    while (level <= GetRule(m_parser.current_token->type)->precedence) {
        advance();
        auto infixRuleFunction = GetRule(m_parser.previous_token->type)->infix;
        (this->*infixRuleFunction)(can_assign);
    }
    if (can_assign and match(TokenType::EQUAL)) {
        advance(); // Move past the "="
        reportError(m_parser.previous_token->line_number, "Invalid assignment target");
    }
}

void Compiler::expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

void Compiler::grouping(bool)
{
    expression();
    if (!consume(TokenType::RIGHT_PAREN)) {
        errorAt(m_parser.current_token.value(), "Expected \")\" at the end of a group expression");
    }
}

void Compiler::number(bool)
{
    LOX_ASSERT(m_parser.previous_token.has_value());
    LOX_ASSERT(m_parser.previous_token->type == TokenType::NUMBER);
    LOX_ASSERT(m_parser.previous_token->start + m_parser.previous_token->length <= m_source->GetSource().length());

    char* endpoint = nullptr;
    double value = std::strtod(m_source->GetSource().data() + m_parser.previous_token->start, &endpoint);

    LOX_ASSERT(endpoint != m_source->GetSource().data() + m_parser.previous_token->start);
    LOX_ASSERT(endpoint - (m_source->GetSource().data() + m_parser.previous_token->start) == static_cast<int64_t>(m_parser.previous_token->length));

    addConstant(value);
}

void Compiler::literal(bool)
{
    LOX_ASSERT(m_parser.previous_token.has_value());
    switch (m_parser.previous_token.value().type) {
    case FALSE:
        emitByte(OP_FALSE);
        break;
    case NIL:
        emitByte(OP_NIL);
        break;
    case TRUE:
        emitByte(OP_TRUE);
        break;
    default:
        LOX_ASSERT(false, "Internal error");
    }
}

void Compiler::binary(bool)
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
    case BANG_EQUAL:
        emitByte(OP_NOT_EQUAL);
        break;
    case EQUAL_EQUAL:
        emitByte(OP_EQUAL);
        break;
    case LESS:
        emitByte(OP_LESS);
        break;
    case LESS_EQUAL:
        emitByte(OP_LESS_EQUAL);
        break;
    case GREATER:
        emitByte(OP_GREATER);
        break;
    case GREATER_EQUAL:
        emitByte(OP_GREATER_EQUAL);
        break;
    default:
        LOX_ASSERT(false); // Unreachable.
    }
}

void Compiler::unary(bool)
{
    LOX_ASSERT(m_parser.previous_token.has_value());

    auto const type = m_parser.previous_token->type;
    parsePrecedence(PREC_UNARY);
    if (type == TokenType::MINUS) {
        emitByte(OP_NEGATE);
    } else if (type == TokenType::BANG) {
        emitByte(OP_NOT);
    } else {
        LOX_ASSERT(false);
    }
}

void Compiler::string(bool)
{
    LOX_ASSERT(m_parser.previous_token.has_value());
    LOX_ASSERT(m_parser.previous_token->type == TokenType::STRING);
    auto string_object = m_heap.AllocateStringObject(m_source->GetSource().substr(m_parser.previous_token->start + 1, m_parser.previous_token->length - 2));
    this->addConstant(string_object);
}

void Compiler::declaration()
{
    if (match(TokenType::VAR)) {
        variableDeclaration();
    } else {
        statement();
    }
    if (m_error_state.panic) {
        synchronizeError();
    }
}

void Compiler::statement()
{
    if (match(TokenType::PRINT)) {
        printStatement();
    } else {
        expressionStatement();
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

bool Compiler::match(TokenType type) const
{
    LOX_ASSERT(m_parser.current_token.has_value());
    if (m_parser.current_token->type == type) {
        return true;
    }
    return false;
}

void Compiler::printStatement()
{
    LOX_ASSERT(consume(TokenType::PRINT));
    expression();
    if (!consume(TokenType::SEMICOLON)) {
        reportError(m_parser.previous_token->line_number, "Expected semi-colon at the end of print statement");
    } else {
        emitByte(OP_PRINT);
    }
}
void Compiler::expressionStatement()
{
    expression();
    if (!consume(TokenType::SEMICOLON)) {
        reportError(m_parser.previous_token->line_number, "Expected semi-colon at the end of expression-statement");
    } else {
        emitByte(OP_POP);
    }
}

void Compiler::synchronizeError()
{
    while (m_parser.current_token->type != TokenType::TOKEN_EOF) {
        if (m_parser.previous_token->type == TokenType::SEMICOLON) {
            // Indicates the end of the previous statement
            return;
        }
        switch (m_parser.current_token->type) {
        // Advance until we hit one of the control flow or declaration keywords.
        case ELSE:
        case FOR:
        case FUN:
        case IF:
        case PRINT:
        case RETURN:
        case VAR:
        case WHILE:
            return;
        default:
            advance();
        }
    }
}
void Compiler::variableDeclaration()
{
    LOX_ASSERT(consume(TokenType::VAR));
    int32_t identifier_index_in_constant_pool = -1;
    // Need to extract the variable name out from the token
    if (match(TokenType::IDENTIFIER)) {
        identifier_index_in_constant_pool = identifierConstant(m_parser.current_token.value());
        advance(); // Move past identifier token
    } else {
        reportError(m_parser.previous_token->line_number, "Expected identifier after \"var\" keyword");
        return;
    }

    // Parse the initial value, emit Nil if there is no initializer expression
    if (match(TokenType::EQUAL)) {
        auto _ = consume(TokenType::EQUAL);
        static_cast<void>(_);
        expression();
    } else {
        emitByte(OP_NIL);
    }
    if (!consume(SEMICOLON)) {
        reportError(m_parser.previous_token->line_number, "Expected semi-colon at the end of variable declaration");
    }

    LOX_ASSERT(identifier_index_in_constant_pool >= 0);
    LOX_ASSERT(identifier_index_in_constant_pool < MAX_NUMBER_CONSTANTS);
    emitByte(OP_DEFINE_GLOBAL);
    emitConstantIndex(static_cast<uint16_t>(identifier_index_in_constant_pool));
}

void Compiler::variable(bool can_assign)
{
    auto identifier_index_in_constant_pool = identifierConstant(m_parser.previous_token.value());
    if (can_assign and match(TokenType::EQUAL)) {
        // Look-ahead one token if we find an "=" then this is an assignment
        advance(); // Move past the "="
        expression(); // Emit the instructions for the expression that would be evaluated to the value that this identifier must be assigned with.
        emitByte(OP_SET_GLOBAL);
        emitConstantIndex(static_cast<uint16_t>(identifier_index_in_constant_pool));
    } else {
        emitByte(OP_GET_GLOBAL);
        emitConstantIndex(static_cast<uint16_t>(identifier_index_in_constant_pool));
    }
}

int32_t Compiler::identifierConstant(const Token& token)
{
    LOX_ASSERT(token.type == TokenType::IDENTIFIER);
    auto string_object_ptr = m_heap.AllocateStringObject(m_source->GetSource().substr(token.start, token.length));
    m_current_chunk->constant_pool.push_back(string_object_ptr);
    return m_current_chunk->constant_pool.size() - 1;
}

void Compiler::emitConstantIndex(uint16_t index)
{
    // Extract the 8 LSB's
    emitByte(static_cast<uint8_t>(0x00FFU & index));
    emitByte(static_cast<uint8_t>((0xFF00U & index) >> 8));
}

void Compiler::block()
{
}
