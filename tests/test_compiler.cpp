//
// Created by kevin on 8/11/22.
//

#include "gtest/gtest.h"

#include "compiler.h"
#include "fmt/core.h"

class CompilerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_compiler = std::make_unique<Compiler>(m_heap);
    }
    void TearDown() override
    {
        m_compiler.reset();
        m_heap.Reset();
        m_source.Clear();
    }
    std::unique_ptr<Compiler> m_compiler;
    Heap m_heap;
    Source m_source;
    Chunk m_chunk;
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

TEST_F(CompilerTest, BasicBinaryExpression1)
{
    m_source.AppendFromConsole("1 + 2;");
    ASSERT_TRUE(m_compiler->CompileSource(m_source, m_chunk).IsValue());
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> { OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD, OP_POP }, m_chunk.byte_code));
}

TEST_F(CompilerTest, BasicBinaryExpression2)
{
    m_source.AppendFromConsole("(1 + 2) + 3 + 3 * (20);");
    ASSERT_TRUE(m_compiler->CompileSource(m_source, m_chunk).IsValue());
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 0,
                                     OP_CONSTANT, 1,
                                     OP_ADD,
                                     OP_CONSTANT, 2,
                                     OP_ADD,
                                     OP_CONSTANT, 3,
                                     OP_CONSTANT, 4,
                                     OP_MULTIPLY,
                                     OP_ADD,
                                     OP_POP },
        m_chunk.byte_code));
}

TEST_F(CompilerTest, VaraibleDeclaration)
{
    m_source.AppendFromConsole("var a = (1 + 2) + 3 + 3 * (20);");
    ASSERT_TRUE(m_compiler->CompileSource(m_source, m_chunk).IsValue());
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 1,
                                     OP_CONSTANT, 2,
                                     OP_ADD,
                                     OP_CONSTANT, 3,
                                     OP_ADD,
                                     OP_CONSTANT, 4,
                                     OP_CONSTANT, 5,
                                     OP_MULTIPLY,
                                     OP_ADD,
                                     OP_DEFINE_GLOBAL, 0 },
        m_chunk.byte_code));
}

TEST_F(CompilerTest, StringConcatenation)
{
    m_source.AppendFromConsole(R"(
var a = "Hello world";
var b = a + "FooBar";
)");
    ASSERT_TRUE(m_compiler->CompileSource(m_source, m_chunk).IsValue());
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 1,
                                     OP_DEFINE_GLOBAL, 0,
                                     OP_GET_GLOBAL, 3,
                                     OP_CONSTANT, 4,
                                     OP_ADD,
                                     OP_DEFINE_GLOBAL, 2 },
        m_chunk.byte_code));
}

TEST_F(CompilerTest, PrintStatements)
{
    m_source.AppendFromConsole(R"(
print (((((((1))))))) + 2;
)");
    ASSERT_TRUE(m_compiler->CompileSource(m_source, m_chunk).IsValue());
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 0,
                                     OP_CONSTANT, 1,
                                     OP_ADD,
                                     OP_PRINT },
        m_chunk.byte_code));
}

TEST_F(CompilerTest, AssignmentStatements)
{
    m_source.AppendFromConsole(R"(
var a = 10;
print a;
a = "Hello World";
)");
    ASSERT_TRUE(m_compiler->CompileSource(m_source, m_chunk).IsValue());
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 1,
                                     OP_DEFINE_GLOBAL, 0,
                                     OP_GET_GLOBAL, 2,
                                     OP_PRINT,
                                     OP_CONSTANT, 4,
                                     OP_SET_GLOBAL, 3,
                                     OP_POP },
        m_chunk.byte_code));
}

TEST_F(CompilerTest, InvalidAssignmentTarget)
{
    m_source.AppendFromConsole(
        R"(var a = 10;
                      var b = 20;
                      a + b = 50; // Syntax error
    )");
    ASSERT_TRUE(m_compiler->CompileSource(m_source, m_chunk).IsError());
}

TEST_F(CompilerTest, InvalidBinaryOp)
{
    m_source.AppendFromConsole(
        R"(  var a = 10;
                        var b = "String";
                        a + b; // Runtime error but still valid syntax)");
    ASSERT_TRUE(m_compiler->CompileSource(m_source, m_chunk).IsValue());
}
