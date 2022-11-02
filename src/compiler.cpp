//
// Created by kevin on 8/12/22.
//

#include <cstdint>
#include <functional>
#include <optional>

#include "chunk.h"
#include "compiler.h"
#include "error.h"
#include "fmt/core.h"
#include "scanner.h"

consteval auto GenerateParseTable() -> ParseTable
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
    table[AND]           = { .prefix = nullptr,             .infix = &Compiler::and_,   .precedence = PREC_AND };
    table[CLASS]         = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[ELSE]          = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[FALSE]         = { .prefix = &Compiler::literal,  .infix = nullptr,           .precedence = PREC_NONE };
    table[FOR]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[FUN]           = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[IF]            = { .prefix = nullptr,             .infix = nullptr,           .precedence = PREC_NONE };
    table[NIL]           = { .prefix = &Compiler::literal,  .infix = nullptr,           .precedence = PREC_NONE };
    table[OR]            = { .prefix = nullptr,             .infix = &Compiler::or_,    .precedence = PREC_OR };
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

[[maybe_unused]] static auto PrintTokens(std::vector<Token> const& tokens, const std::string* source) -> void
{
    for (auto token : tokens) {
        fmt::print("{}\n", FormatToken(token, source));
    }
}

static constexpr auto GetRule(TokenType type) -> ParseRule const*
{
    return &PARSE_TABLE[type];
}

Compiler::Compiler(Heap& heap)
    : m_heap(heap)
{
}

auto Compiler::reportError(int32_t line_number, std::string_view error_string) -> void
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

auto Compiler::errorAt(const Token& token, std::string_view message) -> void
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

auto Compiler::reset(Source const& source) -> void
{
    m_source = &source;
    m_function = m_heap.AllocateFunctionObject("",0);
    m_context = Context::SCRIPT;
    m_scanner.Reset(source);
    m_parser.Reset();
    m_locals_state.Reset();
    m_locals_state.current_scope_depth = 0;
    m_locals_state.locals.emplace_back("",0);
    m_error_state.panic = false;
    m_error_state.encountered_error = false;
}

auto Compiler::CompileSource(const Source& source) -> ErrorOr<FunctionObject*>
{
    this->reset(source);

    this->advance();

    while (!match(TokenType::TOKEN_EOF)) {
        declaration();
    }

    if (m_error_state.encountered_error) {
        return Error { .type = ErrorType::ParseError, .error_message = "Parse error" };
    }

    emitByte(OP_RETURN);
    return m_function;
}

auto Compiler::advance() -> void
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

auto Compiler::emitByte(uint8_t byte) -> void
{
    LOX_ASSERT(currentChunk() != nullptr);
    currentChunk()->byte_code.push_back(byte);
}

auto Compiler::addConstant(Value constant) -> void
{
    LOX_ASSERT(currentChunk() != nullptr);
    LOX_ASSERT(currentChunk()->constant_pool.size() < MAX_NUMBER_CONSTANTS, "Exceeded the maximum number of supported constants");

    currentChunk()->constant_pool.push_back(constant);
    emitByte(OP_CONSTANT);
    emitIndex(static_cast<uint16_t>(currentChunk()->constant_pool.size() - 1));
}

auto Compiler::parsePrecedence(Precedence level) -> void
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

auto Compiler::expression() -> void
{
    parsePrecedence(PREC_ASSIGNMENT);
}

auto Compiler::grouping(bool) -> void
{
    expression();
    if (!consume(TokenType::RIGHT_PAREN)) {
        errorAt(m_parser.current_token.value(), "Expected \")\" at the end of a group expression");
    }
}

auto Compiler::number(bool) -> void
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

auto Compiler::literal(bool) -> void
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

auto Compiler::binary(bool) -> void
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

auto Compiler::unary(bool) -> void
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

auto Compiler::string(bool) -> void
{
    LOX_ASSERT(m_parser.previous_token.has_value());
    LOX_ASSERT(m_parser.previous_token->type == TokenType::STRING);
    auto string_object = m_heap.AllocateStringObject(m_source->GetSource().substr(m_parser.previous_token->start + 1, m_parser.previous_token->length - 2));
    this->addConstant(string_object);
}

auto Compiler::declaration() -> void
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

auto Compiler::statement() -> void
{
    if (match(TokenType::PRINT)) {
        printStatement();
    } else if (match(TokenType::LEFT_BRACE)) {
        block();
    } else if (match(TokenType::IF)) {
        ifStatement();
    } else if (match(TokenType::WHILE)) {
        whileStatement();
    } else if (match(TokenType::FOR)) {
        forStatement();
    } else {
        expressionStatement();
    }
}

auto Compiler::consume(TokenType type) -> bool
{
    LOX_ASSERT(m_parser.current_token.has_value());
    if (m_parser.current_token->type == type) {
        advance();
        return true;
    }
    return false;
}

auto Compiler::match(TokenType type) const -> bool
{
    LOX_ASSERT(m_parser.current_token.has_value());
    if (m_parser.current_token->type == type) {
        return true;
    }
    return false;
}

auto Compiler::printStatement() -> void
{
    auto _ = consume(TokenType::PRINT);
    static_cast<void>(_);
    expression();
    if (!consume(TokenType::SEMICOLON)) {
        reportError(m_parser.previous_token->line_number, "Expected semi-colon at the end of print statement");
    } else {
        emitByte(OP_PRINT);
    }
}
auto Compiler::expressionStatement() -> void
{
    expression();
    if (!consume(TokenType::SEMICOLON)) {
        reportError(m_parser.previous_token->line_number, "Expected semi-colon at the end of expression-statement");
    } else {
        emitByte(OP_POP);
    }
}

auto Compiler::synchronizeError() -> void
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
auto Compiler::variableDeclaration() -> void
{
    consume(TokenType::VAR);
    auto identifier_index_in_constant_pool = parseVariable("Expected identifier after \"var\" keyword");
    if (identifier_index_in_constant_pool.IsError()) {
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

    defineVariable(identifier_index_in_constant_pool.GetValue());
}

auto Compiler::variable(bool can_assign) -> void
{
    std::string_view new_local_identifier_name { m_source->GetSource().begin() + m_parser.previous_token.value().start,
        m_source->GetSource().begin() + m_parser.previous_token.value().start + m_parser.previous_token.value().length };

    OpCode set_op;
    OpCode get_op;
    uint16_t index;
    auto variable_resolution_result = resolveVariable(new_local_identifier_name);

    if (variable_resolution_result.has_value()) {
        // Was able to successfully resolve variable which means that it's a local variable
        index = variable_resolution_result.value();
        set_op = OP_SET_LOCAL;
        get_op = OP_GET_LOCAL;

    } else {
        // Global variable
        index = identifierConstant(m_parser.previous_token.value());
        set_op = OP_SET_GLOBAL;
        get_op = OP_GET_GLOBAL;
    }

    if (can_assign and match(TokenType::EQUAL)) {
        // Look-ahead one token if we find an "=" then this is an assignment
        advance(); // Move past the "="
        expression(); // Emit the instructions for the expression that would be evaluated to the value that this identifier must be assigned with.
        emitByte(set_op);
        emitIndex(index);
    } else {
        emitByte(get_op);
        emitIndex(index);
    }
}

auto Compiler::identifierConstant(const Token& token) -> uint16_t
{
    LOX_ASSERT(token.type == TokenType::IDENTIFIER);
    auto string_object_ptr = m_heap.AllocateStringObject(m_source->GetSource().substr(token.start, token.length));
    currentChunk()->constant_pool.push_back(string_object_ptr);
    LOX_ASSERT(currentChunk()->constant_pool.size() <= MAX_NUMBER_CONSTANTS);
    return currentChunk()->constant_pool.size() - 1;
}

auto Compiler::emitIndex(uint16_t index) -> void
{
    // Extract the 8 LSB's
    emitByte(static_cast<uint8_t>(0x00FFU & index));
    emitByte(static_cast<uint8_t>((0xFF00U & index) >> 8U));
}

auto Compiler::block() -> void
{
    beginScope();
    consume(TokenType::LEFT_BRACE);
    while (m_parser.current_token->type != TokenType::TOKEN_EOF && m_parser.current_token->type != RIGHT_BRACE) {
        declaration();
    }
    if (!consume(TokenType::RIGHT_BRACE)) {
        return;
    }
    endScope();
}

auto Compiler::parseVariable(std::string_view error_message) -> ErrorOr<uint16_t>
{
    // Need to extract the variable name out from the token
    if (!consume(TokenType::IDENTIFIER)) {
        reportError(m_parser.previous_token->line_number, error_message);
        return Error { .type = ErrorType::ParseError, .error_message = "" };
    }
    declareVariable();
    if (m_locals_state.current_scope_depth > 0) {
        // Local variable
        return 0;
    }
    auto index = identifierConstant(m_parser.previous_token.value());
    LOX_ASSERT(index < MAX_NUMBER_CONSTANTS);
    return index;
}

auto Compiler::defineVariable(uint16_t constant_pool_index) -> void
{
    if (m_locals_state.current_scope_depth > 0) {
        // Local variable
        markInitialized();
        return;
    }
    emitByte(OP_DEFINE_GLOBAL);
    emitIndex(static_cast<uint16_t>(constant_pool_index));
}

auto Compiler::declareVariable() -> void
{
    if (m_locals_state.current_scope_depth == 0) {
        // Global variable
        return;
    }

    std::string_view new_local_identifier_name { m_source->GetSource().begin() + m_parser.previous_token.value().start,
        m_source->GetSource().begin() + m_parser.previous_token.value().start + m_parser.previous_token.value().length };

    /*
     * The following check ensures that the following is not permitted:
     * {
     *      var a = 10; // Scope depth = 1
     *      var a = 10; // Also scope depth = 1
     * }
     * However the following is valid:
     * {
     *     {
     *          {
     *              var a = 20; // Scope depth =3
     *          }
     *     }
     *     var a = 10; // When parsing this scope-depth is 1
     * }
     * ___________________________________________________
     * {
     *      var a = 1;
     *      {
     *          var a = 1; // Valid shadowing
     *      {
     * }
     *
     */
    for (auto it = m_locals_state.locals.rbegin(); it != m_locals_state.locals.rend(); ++it) {
        auto const& local = *it;
        if (local.local_scope_depth != -1 && local.local_scope_depth < m_locals_state.current_scope_depth) {
            break;
        }
        if (new_local_identifier_name == local.identifier_name) {
            reportError(m_parser.previous_token->line_number, "Already a variable with this name in this scope.");
            return;
        }
    }
    LOX_ASSERT(m_locals_state.locals.size() < MAX_NUMBER_LOCAL_VARIABLES, fmt::format("Exceeded maximum number of local variables:{}", MAX_NUMBER_LOCAL_VARIABLES).c_str());

    m_locals_state.locals.emplace_back(new_local_identifier_name, -1); // The -1 here indicates that the local is still uninitialized
}
auto Compiler::beginScope() -> void
{
    ++m_locals_state.current_scope_depth;
}

auto Compiler::endScope() -> void
{
    LOX_ASSERT(m_locals_state.current_scope_depth >= 1);
    --m_locals_state.current_scope_depth;

    int32_t i = m_locals_state.locals.size() - 1;
    for (; i >= 0; --i) {
        if (m_locals_state.locals[i].local_scope_depth > m_locals_state.current_scope_depth) {
            emitByte(OP_POP);
            continue;
        } else {
            break;
        }
    }
    LOX_ASSERT(i >= -1);
    m_locals_state.locals.resize(i + 1);
}

auto Compiler::resolveVariable(std::string_view identifier_name) -> std::optional<uint32_t>
{
    auto it = std::find_if(m_locals_state.locals.rbegin(), m_locals_state.locals.rend(), [&](LocalsState::Local const& local) {
        return local.identifier_name == identifier_name;
    });
    if (it == m_locals_state.locals.rend()) {
        // We don't actually report an error as the identifier could be referring to a global variable
        return {};
    }
    if (it->local_scope_depth == -1) {
        reportError(m_parser.previous_token->line_number, "Can't read local variable in its own initializer.");
    }
    auto index = std::distance(it, m_locals_state.locals.rend()) - 2; // TODO:  The -2 here is because the 0-index local is not observable outside the compiler. Clean this up later
    LOX_ASSERT(index >= 0);
    return index;
}

auto Compiler::markInitialized() -> void
{
    LOX_ASSERT(!m_locals_state.locals.empty());
    (m_locals_state.locals.end() - 1)->local_scope_depth = m_locals_state.current_scope_depth;
}

auto Compiler::forStatement() -> void
{
    beginScope(); // Add a scope to restrict variables declared in the initializer clause to within the for-body
    auto _ = consume(TokenType::FOR);
    LOX_ASSERT(_);
    if (!consume(TokenType::LEFT_PAREN)) {
        reportError(m_parser.previous_token->line_number, "Expected \"(\" after the for keyword");
    }
    ////////////////////////////// Initializer //////////////////////////////
    if (match(TokenType::SEMICOLON)) {
        consume(TokenType::SEMICOLON);
    } else if (consume(TokenType::VAR)) {
        variableDeclaration();
    } else if (!match(TokenType::SEMICOLON)) {
        expressionStatement();
    }
    /////////////////////////////////////////////////////////////////////////
    //////////////////////////// Condition clause ///////////////////////////
    auto loop_start = currentChunk()->byte_code.size();

    std::optional<uint64_t> exit_jump;
    if (!match(TokenType::SEMICOLON)) {
        expression();
        exit_jump = emitJump(OpCode::OP_JUMP_IF_FALSE);
        emitByte(OP_POP);
    }
    if (!consume(TokenType::SEMICOLON)) {
        reportError(m_parser.previous_token->line_number, "Expected \";\" after optional conditional-clause");
    }
    /////////////////////////////////////////////////////////////////////////
    //////////////////////////// Increment clause ///////////////////////////
    if (!match(TokenType::RIGHT_PAREN)) {
        auto for_body_jump = emitJump(OP_JUMP);
        auto increment_start = currentChunk()->byte_code.size();
        expression();
        emitByte(OP_POP);
        emitLoop(loop_start); // Loop back to the start of the condition
        loop_start = increment_start; // Adjust the loop start to jump to the evaluation of the increment expression after the end of the for body
        patchJump(for_body_jump);
    }
    if (!consume(TokenType::RIGHT_PAREN)) {
        reportError(m_parser.previous_token->line_number, "Expected \")\" after optional increment-clause");
    }
    /////////////////////////////////////////////////////////////////////////
    statement();
    emitLoop(loop_start);

    if (exit_jump.has_value()) {
        patchJump(exit_jump.value());
        emitByte(OP_POP);
    }

    endScope();
}

auto Compiler::whileStatement() -> void
{
    auto loop_start = currentChunk()->byte_code.size();
    auto _ = consume(TokenType::WHILE);
    LOX_ASSERT(_);
    if (!consume(TokenType::LEFT_PAREN)) {
        reportError(m_parser.previous_token->line_number, "Expected \"(\" after the while keyword");
    }
    expression();
    if (!consume(TokenType::RIGHT_PAREN)) {
        reportError(m_parser.previous_token->line_number, "Expected \")\" after the while-condition");
    }
    auto break_destination = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    emitLoop(loop_start);
    patchJump(break_destination);
    emitByte(OP_POP);
}

auto Compiler::emitLoop(uint64_t loop_start) -> void
{
    emitByte(OP_LOOP);
    auto offset = currentChunk()->byte_code.size() - loop_start + 2;
    LOX_ASSERT(offset < MAX_JUMP_OFFSET, "Loop body too large");
    emitIndex(offset);
}

auto Compiler::ifStatement() -> void
{
    consume(TokenType::IF);
    if (!consume(TokenType::LEFT_PAREN)) {
        reportError(m_parser.previous_token->line_number, "Expected \"(\" after the if statement");
    }
    expression();
    if (!consume(TokenType::RIGHT_PAREN)) {
        reportError(m_parser.previous_token->line_number, "Expected \")\" after the if-condition");
    }

    auto jump_destination = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    auto else_destination = emitJump(OP_JUMP);
    patchJump(jump_destination);
    emitByte(OP_POP);

    if (match(TokenType::ELSE)) {
        consume(TokenType::ELSE);
        statement();
    }
    patchJump(else_destination);
}

auto Compiler::emitJump(OpCode op_code) -> uint64_t
{
    LOX_ASSERT(op_code == OP_JUMP_IF_FALSE || op_code == OP_JUMP);
    emitByte(op_code);
    emitIndex(0xFFFF);
    return currentChunk()->byte_code.size() - 2;
}

auto Compiler::patchJump(uint64_t offset) -> void
{
    LOX_ASSERT(offset + 2 <= currentChunk()->byte_code.size());
    auto jump = currentChunk()->byte_code.size() - offset - 2;
    if (jump > MAX_JUMP_OFFSET) {
        reportError(m_parser.previous_token->line_number, fmt::format("Jump offset:{} is larger than supported limit: {}", jump, MAX_JUMP_OFFSET));
        return;
    }
    currentChunk()->byte_code[offset] = static_cast<uint8_t>(0x00FFU & jump);
    currentChunk()->byte_code[offset + 1] = static_cast<uint8_t>((0xFF00U & jump) >> 8U);
}

auto Compiler::and_(bool can_assign) -> void
{
    static_cast<void>(can_assign);
    LOX_ASSERT(m_parser.previous_token.has_value());
    LOX_ASSERT(m_parser.previous_token->type == TokenType::AND);
    // We've already parsed the LHS expression
    auto jump_destination = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // To pop the LHS value of the stack as we know it's "true"ish
    parsePrecedence(Precedence::PREC_AND); // Consume all the tokens according to the current precedence level emitting op-codes for the RHS expression
    patchJump(jump_destination);
}

auto Compiler::or_(bool can_assign) -> void
{
    static_cast<void>(can_assign);
    LOX_ASSERT(m_parser.previous_token.has_value());
    LOX_ASSERT(m_parser.previous_token->type == TokenType::OR);
    // We've already parsed the LHS expression
    auto false_destination = emitJump(OP_JUMP_IF_FALSE);
    auto true_destination = emitJump(OP_JUMP);
    patchJump(false_destination);
    emitByte(OP_POP); // To pop the LHS value of the stack as we know it's "false"ish
    parsePrecedence(Precedence::PREC_OR); // Consume all the tokens according to the current precedence level emitting op-codes for the RHS expression
    patchJump(true_destination);
}
auto Compiler::currentChunk() -> Chunk*
{
    return &m_function->chunk;
}
