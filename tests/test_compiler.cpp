//
// Created by kevin on 8/11/22.
//

#include "gtest/gtest.h"

#include "chunk.h"
#include "compiler.h"
#include "fmt/core.h"
#include "value_formatter.h"

class CompilerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_compiler = std::make_unique<Compiler>(m_heap, m_parser_state);
    }
    std::unique_ptr<Compiler> m_compiler;
    Heap m_heap;
    Source m_source;
    ParserState m_parser_state;
};

bool ValidateByteCode(std::vector<uint8_t> expected, std::vector<uint8_t> const& generated_byte_code)
{
    if (expected.size() != generated_byte_code.size()) {
        fmt::print(stderr, "Size mismatch\n");
        return false;
    }
    bool success = true;
    for (size_t i = 0; i < expected.size(); i++) {
        if (expected[i] != generated_byte_code[i]) {
            fmt::print(stderr, "Invalid opcode at index:{} Expected:{} Got:{}\n", i, expected[i], generated_byte_code[i]);
            success = false;
        }
    }
    return success;
}

bool ValidateConstants(std::vector<Value> expected, std::vector<Value> const& generated_constants)
{
    if (expected.size() != generated_constants.size()) {
        fmt::print(stderr, "Size mismatch, expected:{} got:{}\n", expected.size(), generated_constants.size());
        return false;
    }
    bool success = true;
    for (size_t i = 0; i < expected.size(); i++) {
        if (expected[i] != generated_constants[i]) {
            fmt::print(stderr, "Invalid constant at index:{} Expected:{} Got:{}\n", i, expected[i], generated_constants[i]);
            success = false;
        }
    }
    return success;
}

TEST_F(CompilerTest, BasicBinaryExpression1)
{
    m_source.AppendFromConsole("1 + 2;");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> { OP_CONSTANT, 0, 0, OP_CONSTANT, 1, 0, OP_ADD, OP_POP, OP_RETURN }, compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> { 1.0, 2.0 }, compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, BasicBinaryExpression2)
{
    m_source.AppendFromConsole("(1 + 2) + 3 + 3 * (20);");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 0, 0,
                                     OP_CONSTANT, 1, 0,
                                     OP_ADD,
                                     OP_CONSTANT, 2, 0,
                                     OP_ADD,
                                     OP_CONSTANT, 3, 0,
                                     OP_CONSTANT, 4, 0,
                                     OP_MULTIPLY,
                                     OP_ADD,
                                     OP_POP,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> { 1.0, 2.0, 3.0, 3.0, 20.0 }, compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, VaraibleDeclaration)
{
    m_source.AppendFromConsole(R"(
 var a = (1 + 2) + 3 + 3 * (20);)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 1, 0,
                                     OP_CONSTANT, 2, 0,
                                     OP_ADD,
                                     OP_CONSTANT, 3, 0,
                                     OP_ADD,
                                     OP_CONSTANT, 4, 0,
                                     OP_CONSTANT, 5, 0,
                                     OP_MULTIPLY,
                                     OP_ADD,
                                     OP_DEFINE_GLOBAL, 0, 0,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> { m_heap.AllocateStringObject("a"), 1.0, 2.0, 3.0, 3.0, 20.0 }, compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, StringConcatenation)
{
    m_source.AppendFromConsole(R"(
 var a = "Hello world";
 var b = a + "FooBar";
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();

    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 1, 0,
                                     OP_DEFINE_GLOBAL, 0, 0,
                                     OP_GET_GLOBAL, 3, 0,
                                     OP_CONSTANT, 4, 0,
                                     OP_ADD,
                                     OP_DEFINE_GLOBAL, 2, 0,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      m_heap.AllocateStringObject("a"),
                                      m_heap.AllocateStringObject("Hello world"),
                                      m_heap.AllocateStringObject("b"),
                                      m_heap.AllocateStringObject("a"),
                                      m_heap.AllocateStringObject("FooBar") },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, Comments)
{
    m_source.AppendFromConsole(R"(
{
      var i = 10;
      print i; // TEST COMMENT
}
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
}

TEST_F(CompilerTest, PrintStatements)
{
    m_source.AppendFromConsole(R"(
 print (((((((1))))))) + 2;
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();

    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 0, 0,
                                     OP_CONSTANT, 1, 0,
                                     OP_ADD,
                                     OP_PRINT,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> { 1.0, 2.0 },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, AssignmentStatements)
{
    m_source.AppendFromConsole(R"(
 var a = 10;
 print a;
 a = "Hello World";)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 1, 0,
                                     OP_DEFINE_GLOBAL, 0, 0,
                                     OP_GET_GLOBAL, 2, 0,
                                     OP_PRINT,
                                     OP_CONSTANT, 4, 0,
                                     OP_SET_GLOBAL, 3, 0,
                                     OP_POP,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      m_heap.AllocateStringObject("a"),
                                      10.0,
                                      m_heap.AllocateStringObject("a"),
                                      m_heap.AllocateStringObject("a"),
                                      m_heap.AllocateStringObject("Hello World") },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, InvalidAssignmentTarget)
{
    m_source.AppendFromConsole(R"(
 var a = 10;
 var b = 20;
 a + b = 50; // Syntax error)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_FALSE(compilation_result.has_value());
}

TEST_F(CompilerTest, InvalidBinaryOp)
{
    m_source.AppendFromConsole(R"(
 var a = 10;
 var b = "String";
 a + b; // Runtime error but still valid syntax)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      m_heap.AllocateStringObject("a"),
                                      10.0,
                                      m_heap.AllocateStringObject("b"),
                                      m_heap.AllocateStringObject("String"),
                                      m_heap.AllocateStringObject("a"),
                                      m_heap.AllocateStringObject("b") },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, LocalVariables1)
{
    m_source.AppendFromConsole(R"(
{
     var abcd = 10;
 }
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 0, 0,
                                     OP_POP,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      10.0,
                                  },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, LocalVariablesShadowing)
{
    m_source.AppendFromConsole(R"(
{
     var abcd = 10;
     {
         var abcd = "Hello World";
     }
 }
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 0, 0,
                                     OP_CONSTANT, 1, 0,
                                     OP_POP,
                                     OP_POP,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      10.0,
                                      m_heap.AllocateStringObject("Hello World"),
                                  },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, IfStatement)
{
    m_source.AppendFromConsole(R"(
{
     if(false) {
         print "If-branch";
     }
     print "Jumped here";
 }
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_FALSE,
                                     OP_JUMP_IF_FALSE, 8, 0,
                                     OP_POP,
                                     OP_CONSTANT, 0, 0,
                                     OP_PRINT,
                                     OP_JUMP, 1, 0,
                                     OP_POP,
                                     OP_CONSTANT, 1, 0,
                                     OP_PRINT,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      m_heap.AllocateStringObject("If-branch"),
                                      m_heap.AllocateStringObject("Jumped here"),
                                  },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, LogicalOperatorsAnd)
{
    m_source.AppendFromConsole(R"(
{
     print false and true;
 }
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_FALSE,
                                     OP_JUMP_IF_FALSE, 2, 0,
                                     OP_POP,
                                     OP_TRUE,
                                     OP_PRINT,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
}

TEST_F(CompilerTest, LogicalOperatorsOr)
{
    m_source.AppendFromConsole(R"(
{
     print false or true or false;
 }
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_FALSE,
                                     OP_JUMP_IF_FALSE, 3, 0,
                                     OP_JUMP, 10, 0,
                                     OP_POP,
                                     OP_TRUE,
                                     OP_JUMP_IF_FALSE, 3, 0,
                                     OP_JUMP, 2, 0,
                                     OP_POP,
                                     OP_FALSE,
                                     OP_PRINT,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
}

TEST_F(CompilerTest, WhileStatement)
{
    m_source.AppendFromConsole(R"(
{
     var a  = 0;
     while(a < 10) {
         print a;
         a = a - 1;
     }
 }
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 0, 0,
                                     OP_GET_LOCAL, 0, 0,
                                     OP_CONSTANT, 1, 0,
                                     OP_LESS,
                                     OP_JUMP_IF_FALSE, 19, 0,
                                     OP_POP,
                                     OP_GET_LOCAL, 0, 0,
                                     OP_PRINT,
                                     OP_GET_LOCAL, 0, 0,
                                     OP_CONSTANT, 2, 0,
                                     OP_SUBTRACT,
                                     OP_SET_LOCAL, 0, 0,
                                     OP_POP,
                                     OP_LOOP, 29, 0,
                                     OP_POP,
                                     OP_POP,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      0.0,
                                      10.0,
                                      1.0,
                                  },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, ForStatement)
{
    m_source.AppendFromConsole(R"(
{
     for(var i = 0; i < 3; i = i + 1){
         print i;
     }
 }
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 0, 0,
                                     OP_GET_LOCAL, 0, 0,
                                     OP_CONSTANT, 1, 0,
                                     OP_LESS,
                                     OP_JUMP_IF_FALSE, 25, 0,
                                     OP_POP,
                                     OP_JUMP, 14, 0,
                                     OP_GET_LOCAL, 0, 0,
                                     OP_CONSTANT, 2, 0,
                                     OP_ADD,
                                     OP_SET_LOCAL, 0, 0,
                                     OP_POP,
                                     OP_LOOP, 28, 0,
                                     OP_GET_LOCAL, 0, 0,
                                     OP_PRINT,
                                     OP_LOOP, 21, 0,
                                     OP_POP,
                                     OP_POP,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      0.0,
                                      3.0,
                                      1.0,
                                  },
        compiled_function->chunk.constant_pool));
}