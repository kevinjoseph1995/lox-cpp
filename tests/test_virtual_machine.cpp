//
// Created by kevin on 8/11/22.
//

#include "gtest/gtest.h"

#include "fmt/core.h"
#include "virtual_machine.h"

class VMTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_vm.SetExternalOutputStringStream(&m_vm_output_stream);
    }
    VirtualMachine m_vm;
    Source m_source;
    std::string m_vm_output_stream;
};

TEST_F(VMTest, ExpressionTest)
{
    m_source.AppendFromConsole(R"(
print 8 * 1 + 2 + 3 + 3 * 4 + (2 + 2);
print 1 / 2;
print "Hello" + "World";
)");
    static constexpr auto OUTPUT = "29\n"
                                   "0.5\n"
                                   "HelloWorld\n";
    ASSERT_TRUE(m_vm.Interpret(m_source).IsValue());
    ASSERT_EQ(m_vm_output_stream, OUTPUT);
}

TEST_F(VMTest, GlobalVariableDeclaration)
{
    m_source.AppendFromConsole(R"(
var a =  8 * 1 + 2 + 3 + 3 * 4 + (2 + 2);
print a;
)");
    static constexpr auto EXPECTED_OUTPUT = "29\n";
    ASSERT_TRUE(m_vm.Interpret(m_source).IsValue());
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, GlobalVariableReAssignment)
{
    m_source.AppendFromConsole(R"(
var a =  8 * 1 + 2 + 3 + 3 * 4 + (2 + 2);
print a;
a = "HelloWorld";
print a;
)");
    static constexpr auto EXPECTED_OUTPUT = "29\n"
                                            "HelloWorld\n";
    ASSERT_TRUE(m_vm.Interpret(m_source).IsValue());
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, TestMixedConcatenation)
{
    m_source.AppendFromConsole(R"(
var a =  1 + "Hello World";
)");
    ASSERT_FALSE(m_vm.Interpret(m_source).IsValue());
}

TEST_F(VMTest, TestStringComparison)
{
    m_source.AppendFromConsole(R"(
var a =  "Hello" +  "World";
var b = "HelloWorld";
print a == b;
print "FooBar" == a;
)");
    ASSERT_TRUE(m_vm.Interpret(m_source).IsValue());
    static constexpr auto EXPECTED_OUTPUT = "true\n"
                                            "false\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, TestNumberComparison)
{
    m_source.AppendFromConsole(R"(
print 1 < 2;
print 1 == 1;
print 1.0 == 1.1;
)");
    ASSERT_TRUE(m_vm.Interpret(m_source).IsValue());
    static constexpr auto EXPECTED_OUTPUT = "true\n"
                                            "true\n"
                                            "false\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, TestDefaultValue)
{
    m_source.AppendFromConsole(R"(
var a;
print a;
)");
    ASSERT_TRUE(m_vm.Interpret(m_source).IsValue());
    static constexpr auto EXPECTED_OUTPUT = "Nil\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}