//
// Created by kevin on 8/12/22.
//

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <ranges>

#include "chunk.h"
#include "compiler.h"
#include "error.h"
#include "fmt/core.h"
#include "heap.h"
#include "scanner.h"
#include "value.h"

consteval auto GenerateParseTable() -> ParseTable
{
    ParseTable table;
    // clang-format off
    table[LEFT_PAREN]    = { .prefix = &Compiler::grouping, .infix = &Compiler::call,   .precedence = PREC_CALL };
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

[[maybe_unused]] static auto PrintTokens(std::vector<Token> const& tokens, std::string const* source) -> void
{
    for (auto token : tokens) {
        fmt::print("{}\n", FormatToken(token, source));
    }
}

static constexpr auto GetRule(TokenType type) -> ParseRule const*
{
    return &PARSE_TABLE[type];
}

Compiler::Compiler(Heap& heap, ParserState& parser_state, Compiler* parent_compiler)
    : m_parent_compiler(parent_compiler)
    , m_heap(heap)
    , m_parser_state(parser_state)

{
    if (parent_compiler != nullptr) {
        // Not top level script an is function compiler
        m_function = m_heap.AllocateFunctionObject("_", 0);
    } else {
        m_function = m_heap.AllocateFunctionObject("TOP_LEVEL_SCRIPT", 0);
    }
    m_locals_state.locals.emplace_back("", 0);
}

auto Compiler::CompileSource(Source const& source) -> CompilationErrorOr<FunctionObject*>
{
    m_parser_state.Initialize(source);
    m_source = &source;

    m_parser_state.Advance();

    while (!m_parser_state.Match(TokenType::TOKEN_EOF)) {
        declaration();
    }

    if (m_parser_state.EncounteredError()) {
        return std::unexpected(CompilationError { { "Compilation failed" } });
    }
    return endCompiler();
}

auto Compiler::endCompiler() -> FunctionObject*
{
    emitByte(OP_NIL);
    emitByte(OP_RETURN); // This return wouldn't be executed in the case we already emitted a return.
    // However this return handles the case where functions don't have explicit return types and also the top-level script
    LOX_ASSERT(m_upvalues.size() <= MAX_INDEX_SIZE);
    m_function->upvalue_count = static_cast<uint16_t>(m_upvalues.size());
    return m_function;
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
    m_parser_state.Advance();
    auto prefixRuleFunction = GetRule(m_parser_state.PreviousToken()->type)->prefix;
    if (prefixRuleFunction == nullptr) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected expression");
        return;
    }
    bool can_assign = level <= PREC_ASSIGNMENT;
    (this->*prefixRuleFunction)(can_assign);

    while (level <= GetRule(m_parser_state.CurrentToken()->type)->precedence) {
        m_parser_state.Advance();
        auto infixRuleFunction = GetRule(m_parser_state.PreviousToken()->type)->infix;
        (this->*infixRuleFunction)(can_assign);
    }
    if (can_assign and m_parser_state.Match(TokenType::EQUAL)) {
        m_parser_state.Advance(); // Move past the "="
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Invalid assignment target");
    }
}

auto Compiler::expression() -> void
{
    parsePrecedence(PREC_ASSIGNMENT);
}

auto Compiler::grouping(bool) -> void
{
    expression();
    if (!m_parser_state.Consume(TokenType::RIGHT_PAREN)) {
        m_parser_state.ReportError(m_parser_state.CurrentToken()->line_number, GetTokenSpan(*m_parser_state.CurrentToken()), "Expected \")\" at the end of a group expression");
    }
}

auto Compiler::number(bool) -> void
{
    LOX_ASSERT(m_parser_state.PreviousToken().has_value());
    LOX_ASSERT(m_parser_state.PreviousToken()->type == TokenType::NUMBER);
    LOX_ASSERT(m_parser_state.PreviousToken()->start + m_parser_state.PreviousToken()->length <= m_source->GetSource().length());

    char* endpoint = nullptr;
    double value = std::strtod(m_source->GetSource().data() + m_parser_state.PreviousToken()->start, &endpoint);

    LOX_ASSERT(endpoint != m_source->GetSource().data() + m_parser_state.PreviousToken()->start);
    LOX_ASSERT(endpoint - (m_source->GetSource().data() + m_parser_state.PreviousToken()->start) == static_cast<int64_t>(m_parser_state.PreviousToken()->length));

    addConstant(value);
}

auto Compiler::literal(bool) -> void
{
    LOX_ASSERT(m_parser_state.PreviousToken().has_value());
    switch (m_parser_state.PreviousToken().value().type) {
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
    LOX_ASSERT(m_parser_state.PreviousToken().has_value());

    auto type = m_parser_state.PreviousToken()->type;
    parsePrecedence(static_cast<Precedence>(GetRule(m_parser_state.PreviousToken()->type)->precedence + 1));
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
    LOX_ASSERT(m_parser_state.PreviousToken().has_value());

    auto const type = m_parser_state.PreviousToken()->type;
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
    LOX_ASSERT(m_parser_state.PreviousToken().has_value());
    LOX_ASSERT(m_parser_state.PreviousToken()->type == TokenType::STRING);
    auto string_object = m_heap.AllocateStringObject(m_source->GetSource().substr(m_parser_state.PreviousToken()->start + 1, m_parser_state.PreviousToken()->length - 2));
    this->addConstant(string_object);
}

auto Compiler::declaration() -> void
{
    if (m_parser_state.Match(TokenType::VAR)) {
        variableDeclaration();
    } else if (m_parser_state.Match(TokenType::FUN)) {
        functionDeclaration();
    } else if (m_parser_state.Match(TokenType::RETURN)) {
        returnStatement();
    } else {
        statement();
    }
    if (m_parser_state.InPanicState()) {
        synchronizeError();
    }
}
auto Compiler::returnStatement() -> void
{
    LOX_ASSERT(m_parser_state.Match(TokenType::RETURN));
    m_parser_state.Consume(TokenType::RETURN);
    if (m_parent_compiler == nullptr) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Cannot return from top-level script");
        return;
    }
    if (m_parser_state.Match(TokenType::SEMICOLON)) {
        emitByte(OP_NIL);
    } else {
        expression();
    }
    if (!m_parser_state.Consume(TokenType::SEMICOLON)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected semi-colon at the end of return statement");
        return;
    }
    emitByte(OP_RETURN);
}

auto Compiler::statement() -> void
{
    if (m_parser_state.Match(TokenType::PRINT)) {
        printStatement();
    } else if (m_parser_state.Match(TokenType::LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else if (m_parser_state.Match(TokenType::IF)) {
        ifStatement();
    } else if (m_parser_state.Match(TokenType::WHILE)) {
        whileStatement();
    } else if (m_parser_state.Match(TokenType::FOR)) {
        forStatement();
    } else {
        expressionStatement();
    }
}
auto Compiler::functionDeclaration() -> void
{
    LOX_ASSERT(m_parser_state.Match(TokenType::FUN));
    m_parser_state.Consume(TokenType::FUN);
    auto constant_index_result = parseVariable("Expected function identifier");
    markInitialized(); // Marking as initialized as functions can refer to them even as we are compiling the body of the function.
    function();
    defineVariable(constant_index_result.value());
}

auto Compiler::setFunctionName() -> void
{
    LOX_ASSERT(m_parser_state.PreviousToken().has_value());
    LOX_ASSERT(m_parser_state.PreviousToken().value().type == TokenType::IDENTIFIER);
    LOX_ASSERT(m_parser_state.PreviousToken().value().start + +m_parser_state.PreviousToken().value().length <= m_source->GetSource().length());
    auto const start = m_parser_state.PreviousToken().value().start;
    auto const length = m_parser_state.PreviousToken().value().length;
    auto function_name = std::string_view(m_source->GetSource().data() + start, m_source->GetSource().data() + start + length);
    m_function->function_name = function_name;
}

auto Compiler::function() -> void
{
    Compiler function_compiler(m_heap, m_parser_state, this);
    ///////////////////////////////////////////////// Compile the function body ////////////////////////////////////////////////////////////////////////////////////////////
    function_compiler.m_source = this->m_source;
    function_compiler.setFunctionName();
    HeapContextManager heap_context_manager(m_heap, this, &function_compiler);

    function_compiler.beginScope();
    auto success = function_compiler.m_parser_state.Consume(TokenType::LEFT_PAREN);
    if (!success) {
        function_compiler.m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected open parenthesis after function identifier");
    }

    if (!function_compiler.m_parser_state.Match(TokenType::RIGHT_PAREN)) {
        do {
            ++(function_compiler.m_function->arity);
            if (function_compiler.m_function->arity > MAX_NUMBER_OF_FUNCTION_PARAMETERS) {
                function_compiler.m_parser_state.ReportError(function_compiler.m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()),
                    fmt::format("Exceeded more than {} function parameters", MAX_NUMBER_OF_FUNCTION_PARAMETERS));
            }
            auto constant_index = function_compiler.parseVariable("Expected function parameter identifier");
            if (constant_index) {
                function_compiler.defineVariable(constant_index.value());
            }
        } while (function_compiler.m_parser_state.Consume(TokenType::COMMA));
    }

    success = function_compiler.m_parser_state.Consume(TokenType::RIGHT_PAREN);
    if (!success) {
        function_compiler.m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected closing parenthesis after function identifier");
    }

    function_compiler.block();

    auto compiled_function = function_compiler.endCompiler();
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LOX_ASSERT(currentChunk() != nullptr);
    LOX_ASSERT(currentChunk()->constant_pool.size() < MAX_NUMBER_CONSTANTS, "Exceeded the maximum number of supported constants");
    currentChunk()->constant_pool.push_back(Value { compiled_function });

    // Emit OP_CLOSURE and it's operands
    /* |OP_CLOSURE|  Function_Obj_Cont_index_LSB  |  Function_Obj_Cont_index_USB  |  i=0,Upvalue_is_local  |  i=0,Upvalue_index  | ... |  i=n-1,Upvalue_is_local  |  i=n-1,Upvalue_index  |*/
    emitByte(OP_CLOSURE);
    emitIndex(static_cast<uint16_t>(currentChunk()->constant_pool.size() - 1));
    for (auto const& upvalue : function_compiler.m_upvalues) {
        emitByte(static_cast<uint8_t>(upvalue.type));
        emitIndex(upvalue.index);
    }
}

auto Compiler::printStatement() -> void
{
    auto _ = m_parser_state.Consume(TokenType::PRINT);
    static_cast<void>(_);
    expression();
    if (!m_parser_state.Consume(TokenType::SEMICOLON)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected semi-colon at the end of print statement");
    } else {
        emitByte(OP_PRINT);
    }
}
auto Compiler::expressionStatement() -> void
{
    expression();
    if (!m_parser_state.Consume(TokenType::SEMICOLON)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected semi-colon at the end of expression-statement");
    } else {
        emitByte(OP_POP);
    }
}

auto Compiler::synchronizeError() -> void
{
    m_parser_state.ResetPanicState();
    while (m_parser_state.CurrentToken()->type != TokenType::TOKEN_EOF) {
        if (m_parser_state.PreviousToken()->type == TokenType::SEMICOLON) {
            // Indicates the end of the previous statement
            return;
        }
        switch (m_parser_state.CurrentToken()->type) {
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
            m_parser_state.Advance();
        }
    }
}

auto Compiler::variableDeclaration() -> void
{
    m_parser_state.Consume(TokenType::VAR);
    auto identifier_index_in_constant_pool = parseVariable("Expected identifier after \"var\" keyword");
    if (!identifier_index_in_constant_pool) {
        return;
    }

    // Parse the initial value, emit Nil if there is no initializer expression
    if (m_parser_state.Match(TokenType::EQUAL)) {
        auto _ = m_parser_state.Consume(TokenType::EQUAL);
        static_cast<void>(_);
        expression();
    } else {
        emitByte(OP_NIL);
    }
    if (!m_parser_state.Consume(SEMICOLON)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected semi-colon at the end of variable declaration");
    }

    defineVariable(identifier_index_in_constant_pool.value());
}

auto Compiler::variable(bool can_assign) -> void
{
    auto start_offset = static_cast<std::string::difference_type>(m_parser_state.PreviousToken().value().start);
    auto end_offset = start_offset + static_cast<std::string::difference_type>(m_parser_state.PreviousToken()->length);
    std::string_view new_local_identifier_name { m_source->GetSource().begin() + start_offset, m_source->GetSource().begin() + end_offset };

    OpCode set_op;
    OpCode get_op;
    uint16_t index;
    auto variable_resolution_result = resolveVariable(new_local_identifier_name);

    if (variable_resolution_result.has_value()) {
        // Was able to successfully resolve variable which means that it's a local variable
        index = variable_resolution_result.value();
        set_op = OP_SET_LOCAL;
        get_op = OP_GET_LOCAL;

    } else if (variable_resolution_result = resolveUpvalue(new_local_identifier_name); variable_resolution_result.has_value()) {
        index = variable_resolution_result.value();
        set_op = OP_SET_UPVALUE;
        get_op = OP_GET_UPVALUE;
    } else {
        // Global variable
        index = identifierConstant(m_parser_state.PreviousToken().value());
        set_op = OP_SET_GLOBAL;
        get_op = OP_GET_GLOBAL;
    }

    if (can_assign and m_parser_state.Match(TokenType::EQUAL)) {
        // Look-ahead one token if we find an "=" then this is an assignment
        m_parser_state.Advance(); // Move past the "="
        expression();             // Emit the instructions for the expression that would be evaluated to the value that this identifier must be assigned with.
        emitByte(set_op);
        emitIndex(index);
    } else {
        emitByte(get_op);
        emitIndex(index);
    }
}

auto Compiler::identifierConstant(Token const& token) -> uint16_t
{
    LOX_ASSERT(token.type == TokenType::IDENTIFIER);
    auto string_object_ptr = m_heap.AllocateStringObject(m_source->GetSource().substr(token.start, token.length));
    currentChunk()->constant_pool.push_back(string_object_ptr);
    LOX_ASSERT(currentChunk()->constant_pool.size() <= MAX_NUMBER_CONSTANTS);
    return static_cast<uint16_t>(currentChunk()->constant_pool.size() - 1);
}

auto Compiler::emitIndex(uint16_t index) -> void
{
    // Extract the 8 LSB's
    emitByte(static_cast<uint8_t>(0x00FFU & index));
    emitByte(static_cast<uint8_t>((0xFF00U & index) >> 8U));
}

auto Compiler::block() -> void
{
    if (!m_parser_state.Consume(TokenType::LEFT_BRACE)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected opening brace at the start of block statement");
    }
    while (m_parser_state.CurrentToken()->type != TokenType::TOKEN_EOF && m_parser_state.CurrentToken()->type != RIGHT_BRACE) {
        declaration();
    }
    if (!m_parser_state.Consume(TokenType::RIGHT_BRACE)) {
        return;
    }
}

auto Compiler::parseVariable(std::string_view error_message) -> ParseErrorOr<uint16_t>
{
    // Need to extract the variable name out from the token
    if (!m_parser_state.Consume(TokenType::IDENTIFIER)) {
        return std::unexpected(ParseError { { std::string(error_message) }, GetTokenSpan(m_parser_state.PreviousToken().value()) });
    }
    declareVariable();
    if (m_locals_state.current_scope_depth > 0) {
        // Local variable
        return 0;
    }
    auto index = identifierConstant(m_parser_state.PreviousToken().value());
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

    std::string_view new_local_identifier_name { m_source->GetSource().begin() + static_cast<std::string::difference_type>(m_parser_state.PreviousToken().value().start),
        m_source->GetSource().begin() + static_cast<std::string::difference_type>(m_parser_state.PreviousToken().value().start + m_parser_state.PreviousToken().value().length) };

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
            m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Already a variable with this name in this scope.");
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

    int32_t i = static_cast<int32_t>(m_locals_state.locals.size()) - 1;
    for (; i >= 0; --i) {
        auto const& local = m_locals_state.locals.at(static_cast<size_t>(i));
        if (local.local_scope_depth > m_locals_state.current_scope_depth) {
            if (local.is_captured) {
                emitByte(OP_CLOSE_UPVALUE);
            } else {
                emitByte(OP_POP);
            }
            continue;
        } else {
            break;
        }
    }
    LOX_ASSERT(i >= -1);
    m_locals_state.locals.resize(static_cast<size_t>(i + 1));
}

auto Compiler::resolveVariable(std::string_view identifier_name) -> std::optional<uint16_t>
{
    auto it = std::find_if(m_locals_state.locals.rbegin(), m_locals_state.locals.rend(), [&](LocalsState::Local const& local) {
        return local.identifier_name == identifier_name;
    });
    if (it == m_locals_state.locals.rend()) {
        // We don't actually report an error as the identifier could be referring to a global variable or could be captured from enclosing functions
        return {};
    }
    if (it->local_scope_depth == -1) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Can't read local variable in its own initializer.");
    }
    auto index = std::distance(it, m_locals_state.locals.rend()) - 2;
    LOX_ASSERT(index >= 0);
    LOX_ASSERT(index < std::numeric_limits<uint16_t>::max());
    return static_cast<uint16_t>(index);
}

auto Compiler::markInitialized() -> void
{
    if (m_locals_state.current_scope_depth == 0) {
        return;
    }
    LOX_ASSERT(!m_locals_state.locals.empty());
    m_locals_state.locals.back().local_scope_depth = m_locals_state.current_scope_depth;
}

auto Compiler::forStatement() -> void
{
    beginScope(); // Add a scope to restrict variables declared in the initializer clause to within the for-body
    auto _ = m_parser_state.Consume(TokenType::FOR);
    LOX_ASSERT(_);
    if (!m_parser_state.Consume(TokenType::LEFT_PAREN)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected \"(\" after the for keyword");
    }
    ////////////////////////////// Initializer //////////////////////////////
    if (m_parser_state.Match(TokenType::SEMICOLON)) {
        m_parser_state.Consume(TokenType::SEMICOLON);
    } else if (m_parser_state.Consume(TokenType::VAR)) {
        variableDeclaration();
    } else if (!m_parser_state.Match(TokenType::SEMICOLON)) {
        expressionStatement();
    }
    /////////////////////////////////////////////////////////////////////////
    //////////////////////////// Condition clause ///////////////////////////
    auto loop_start = currentChunk()->byte_code.size();

    std::optional<uint64_t> exit_jump;
    if (!m_parser_state.Match(TokenType::SEMICOLON)) {
        expression();
        exit_jump = emitJump(OpCode::OP_JUMP_IF_FALSE);
        emitByte(OP_POP);
    }
    if (!m_parser_state.Consume(TokenType::SEMICOLON)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected \";\" after optional conditional-clause");
    }
    /////////////////////////////////////////////////////////////////////////
    //////////////////////////// Increment clause ///////////////////////////
    if (!m_parser_state.Match(TokenType::RIGHT_PAREN)) {
        auto for_body_jump = emitJump(OP_JUMP);
        auto increment_start = currentChunk()->byte_code.size();
        expression();
        emitByte(OP_POP);
        emitLoop(loop_start);         // Loop back to the start of the condition
        loop_start = increment_start; // Adjust the loop start to jump to the evaluation of the increment expression after the end of the for body
        patchJump(for_body_jump);
    }
    if (!m_parser_state.Consume(TokenType::RIGHT_PAREN)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected \")\" after optional increment-clause");
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
    auto _ = m_parser_state.Consume(TokenType::WHILE);
    LOX_ASSERT(_);
    if (!m_parser_state.Consume(TokenType::LEFT_PAREN)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected \"(\" after the while keyword");
    }
    expression();
    if (!m_parser_state.Consume(TokenType::RIGHT_PAREN)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected \")\" after the while-condition");
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
    emitIndex(static_cast<uint16_t>(offset));
}

auto Compiler::ifStatement() -> void
{
    m_parser_state.Consume(TokenType::IF);
    if (!m_parser_state.Consume(TokenType::LEFT_PAREN)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected \"(\" after the if statement");
    }
    expression();
    if (!m_parser_state.Consume(TokenType::RIGHT_PAREN)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected \")\" after the if-condition");
    }

    auto jump_destination = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    auto else_destination = emitJump(OP_JUMP);
    patchJump(jump_destination);
    emitByte(OP_POP);

    if (m_parser_state.Match(TokenType::ELSE)) {
        m_parser_state.Consume(TokenType::ELSE);
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
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), fmt::format("Jump offset:{} is larger than supported limit: {}", jump, MAX_JUMP_OFFSET));
        return;
    }
    currentChunk()->byte_code[offset] = static_cast<uint8_t>(0x00FFU & jump);
    currentChunk()->byte_code[offset + 1] = static_cast<uint8_t>((0xFF00U & jump) >> 8U);
}

auto Compiler::and_(bool can_assign) -> void
{
    static_cast<void>(can_assign);
    LOX_ASSERT(m_parser_state.PreviousToken().has_value());
    LOX_ASSERT(m_parser_state.PreviousToken()->type == TokenType::AND);
    // We've already parsed the LHS expression
    auto jump_destination = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);                      // To pop the LHS value of the stack as we know it's "true"ish
    parsePrecedence(Precedence::PREC_AND); // Consume all the tokens according to the current precedence level emitting op-codes for the RHS expression
    patchJump(jump_destination);
}

auto Compiler::or_(bool can_assign) -> void
{
    static_cast<void>(can_assign);
    LOX_ASSERT(m_parser_state.PreviousToken().has_value());
    LOX_ASSERT(m_parser_state.PreviousToken()->type == TokenType::OR);
    // We've already parsed the LHS expression
    auto false_destination = emitJump(OP_JUMP_IF_FALSE);
    auto true_destination = emitJump(OP_JUMP);
    patchJump(false_destination);
    emitByte(OP_POP);                     // To pop the LHS value of the stack as we know it's "false"ish
    parsePrecedence(Precedence::PREC_OR); // Consume all the tokens according to the current precedence level emitting op-codes for the RHS expression
    patchJump(true_destination);
}
auto Compiler::currentChunk() -> Chunk*
{
    LOX_ASSERT(m_function != nullptr);
    return &m_function->chunk;
}

auto Compiler::call(bool) -> void
{
    LOX_ASSERT(m_parser_state.PreviousToken().has_value());
    LOX_ASSERT(m_parser_state.PreviousToken().value().type == TokenType::LEFT_PAREN);
    auto const num_args = argumentList();
    emitByte(OP_CALL);
    emitIndex(num_args);
}

auto Compiler::argumentList() -> uint16_t
{
    auto count = 0;
    if (m_parser_state.CurrentToken()->type != TokenType::RIGHT_PAREN) {
        do {
            expression();
            ++count;
            if (count > MAX_NUMBER_OF_FUNCTION_PARAMETERS) {
                m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Exceeded maximum number of arguments in function call");
            }
        } while (m_parser_state.Consume(TokenType::COMMA));
    }

    if (!m_parser_state.Consume(TokenType::RIGHT_PAREN)) {
        m_parser_state.ReportError(m_parser_state.PreviousToken()->line_number, GetTokenSpan(*m_parser_state.PreviousToken()), "Expected closing parenthesis at the end of call expression");
    }
    return static_cast<uint16_t>(count);
}
auto Compiler::resolveUpvalue(std::string_view identifier_name) -> std::optional<uint16_t>
{
    if (m_parent_compiler == nullptr) {
        // We are compiling top-level script. No more scopes to search in
        return {};
    }
    // Check the immediately enclosing scope if we can find the identifier
    auto local_resolution_result = m_parent_compiler->resolveVariable(identifier_name);
    if (local_resolution_result.has_value()) {
        m_parent_compiler->m_locals_state.locals.at(local_resolution_result.value() + 1).is_captured = true;
        return addUpvalue(local_resolution_result.value(), Upvalue::Type::Local);
    }
    // Since we didn't find it as a local variable in the immediately enclosing scope, we recursively search the other enclosing compilers.
    local_resolution_result = m_parent_compiler->resolveUpvalue(identifier_name);
    if (local_resolution_result.has_value()) {
        return addUpvalue(local_resolution_result.value(), Upvalue::Type::NotLocal);
    }
    return {};
}
auto Compiler::addUpvalue(uint16_t const index, Upvalue::Type const type) -> uint16_t
{
    auto const it = std::ranges::find_if(m_upvalues, [index, type](auto const& upvalue) -> bool { return upvalue.index == index && upvalue.type == type; });
    if (it != m_upvalues.end()) {
        // Upvalue already there
        auto const upvalue_index = std::distance(m_upvalues.begin(), it);
        LOX_ASSERT(m_upvalues.size() <= MAX_INDEX_SIZE);
        return static_cast<uint16_t>(upvalue_index);
    }
    m_upvalues.push_back(Upvalue { .type = type, .index = index });
    LOX_ASSERT(m_upvalues.size() <= MAX_INDEX_SIZE);
    return static_cast<uint16_t>(m_upvalues.size() - 1);
}

auto Compiler::DumpCompiledChunk() const -> void
{
    fmt::print(stderr, "############ FUNCTION NAME | {} | START ############\n", m_function->function_name);
    Disassemble_chunk(m_function->chunk);
    fmt::print(stderr, "############ FUNCTION NAME | {} | END ############\n", m_function->function_name);
}
