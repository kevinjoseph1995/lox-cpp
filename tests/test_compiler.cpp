//
// Created by kevin on 8/11/22.
//

#include "error.h"
#include "fmt/core.h"
#include "gtest/gtest.h"
#include <memory>
#include <optional>

#include "chunk.h"
#include "compiler.h"
#include "heap.h"
#include "object.h"
#include "value_formatter.h"
#include "virtual_machine.h"

using namespace std::literals;
using FunctionMap = std::unordered_map<std::string, FunctionObject const*>;

[[maybe_unused]] [[nodiscard]] static auto ExtractFunctions(Chunk const& chunk) -> FunctionMap
{
    FunctionMap map;
    for (auto& val : chunk.constant_pool) {
        if (val.IsObject() && val.AsObjectPtr()->GetType() == ObjectType::FUNCTION) {
            auto function_object_ptr = static_cast<FunctionObject const*>(val.AsObjectPtr());
            map.insert(std::make_pair(function_object_ptr->function_name, function_object_ptr));
            for (auto const& pair : ExtractFunctions(function_object_ptr->chunk)) {
                map.insert(pair);
            }
        }
    }
    return map;
}

class CompilerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_heap = std::make_unique<Heap>(m_dummy_vm);
        m_compiler
            = std::make_unique<Compiler>(*m_heap, m_parser_state);
        m_heap->SetCompilerContext(m_compiler.get());
    }
    std::unique_ptr<Compiler> m_compiler;
    std::unique_ptr<Heap> m_heap;
    Source m_source;
    ParserState m_parser_state;
    VirtualMachine m_dummy_vm;
};

bool ValidateByteCode(std::vector<uint8_t> expected, std::vector<uint8_t> const& generated_byte_code)
{
    if (expected.size() != generated_byte_code.size()) {
        fmt::print(stderr, "Size mismatch, Expected={} Got={}\n", expected.size(), generated_byte_code.size());
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
        for (auto const& gen : generated_constants) {
            fmt::print("{}", gen);
        }
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

TEST_F(CompilerTest, Comments)
{
    m_source.Append(R"(
{
      var i = 10;
      print i; // TEST COMMENT
}
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
}

TEST_F(CompilerTest, BasicBinaryExpression1)
{
    m_source.Append("1 + 2;");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 0, 0,
                                     OP_CONSTANT, 1, 0,
                                     OP_ADD,
                                     OP_POP,
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> { 1.0, 2.0 }, compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, BasicBinaryExpression2)
{
    m_source.Append("(1 + 2) + 3 + 3 * (20);");
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
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> { 1.0, 2.0, 3.0, 3.0, 20.0 }, compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, VaraibleDeclaration)
{
    m_source.Append(R"(
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
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> { m_heap->AllocateStringObject("a"), 1.0, 2.0, 3.0, 3.0, 20.0 }, compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, StringConcatenation)
{
    m_source.Append(R"(
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
                                     OP_NIL, OP_RETURN },
        compiled_function->chunk.byte_code));
    auto string_objects = std::vector<StringObject> {
        "a"sv, "Hello world"sv, "b"sv, "a"sv, "FooBar"sv
    };
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      &string_objects[0],
                                      &string_objects[1],
                                      &string_objects[2],
                                      &string_objects[3],
                                      &string_objects[4] },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, PrintStatements)
{
    m_source.Append(R"(
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
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> { 1.0, 2.0 },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, AssignmentStatements)
{
    m_source.Append(R"(
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
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    auto string_objects = std::vector<StringObject> {
        "a"sv, "Hello World"sv
    };
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      &string_objects[0],
                                      10.0,
                                      &string_objects[0],
                                      &string_objects[0],
                                      &string_objects[1] },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, InvalidAssignmentTarget)
{
    m_source.Append(R"(
 var a = 10;
 var b = 20;
 a + b = 50; // Syntax error)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_FALSE(compilation_result.has_value());
}

TEST_F(CompilerTest, InvalidBinaryOp)
{
    m_source.Append(R"(
 var a = 10;
 var b = "String";
 a + b; // Runtime error but still valid syntax)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    auto string_objects = std::vector<StringObject> {
        "a"sv, "b"sv, "String"sv
    };
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      &string_objects[0],
                                      10.0,
                                      &string_objects[1],
                                      &string_objects[2],
                                      &string_objects[0],
                                      &string_objects[1],
                                  },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, LocalVariables1)
{
    m_source.Append(R"(
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
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      10.0,
                                  },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, LocalVariables2)
{
    m_source.Append(R"(
{
    var a = 10;
    print a;
}
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
}

TEST_F(CompilerTest, LocalVariablesShadowing)
{
    m_source.Append(R"(
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
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      10.0,
                                      m_heap->AllocateStringObject("Hello World"),
                                  },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, IfStatement)
{
    m_source.Append(R"(
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
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    auto string_objects = std::vector<StringObject> {
        "If-branch"sv, "Jumped here"sv, "String"sv
    };
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      &string_objects[0],
                                      &string_objects[1],
                                  },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, LogicalOperatorsAnd)
{
    m_source.Append(R"(
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
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
}

TEST_F(CompilerTest, LogicalOperatorsOr)
{
    m_source.Append(R"(
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
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
}

TEST_F(CompilerTest, WhileStatement)
{
    m_source.Append(R"(
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
                                     OP_NIL,
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
    m_source.Append(R"(
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
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      0.0,
                                      3.0,
                                      1.0,
                                  },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, TestInvalidSyntax1)
{
    m_source.Append(R"( // 1
                        // 2
                        // 3
                        // 4
                        // 5
                        // 6
                        // 7
{                       // 8
    var a = a;          // 9
}                       // 10
while(1)                // 11
{                       // 12
    var a = a;          // 13
}                       // 14
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_FALSE(compilation_result.has_value());
}

TEST_F(CompilerTest, TestInvalidSyntax2)
{
    m_source.Append(R"( // 1
                        // 2
                        // 3
                        // 4
                        // 5
                        // 6
                        // 7
{ 5 = 3 + 2;}       // 8
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_FALSE(compilation_result.has_value());
}

TEST_F(CompilerTest, FunctionDeclaration1)
{
    m_source.Append(R"(

fun MyFunction() {
}

)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CLOSURE, 1, 0,
                                     OP_DEFINE_GLOBAL, 0, 0,
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    auto string_object = StringObject {
        "MyFunction"sv,
    };
    auto function_object = FunctionObject {
        "MyFunction"sv,
        0
    };
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      &string_object,
                                      &function_object },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, FunctionDeclaration2)
{
    m_source.Append(
        R"(
fun MyFunction(a, b, c) {
    print a + b + c;
}
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    auto const& compiled_function = compilation_result.value();
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CLOSURE, 1, 0,
                                     OP_DEFINE_GLOBAL, 0, 0,
                                     OP_NIL,
                                     OP_RETURN },
        compiled_function->chunk.byte_code));
    auto string_object = StringObject {
        "MyFunction"sv,
    };
    auto function_object = FunctionObject {
        "MyFunction"sv,
        3
    };
    ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                      &string_object,
                                      &function_object },
        compiled_function->chunk.constant_pool));
}

TEST_F(CompilerTest, FunctionCall)
{
    m_source.Append(R"(
fun MyFunction(arg) {
    print arg;
}
MyFunction(1);
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CLOSURE, 1, 0,
                                     OP_DEFINE_GLOBAL, 0, 0,
                                     OP_GET_GLOBAL, 2, 0,
                                     OP_CONSTANT, 3, 0,
                                     OP_CALL, 1, 0,
                                     OP_POP,
                                     OP_NIL,
                                     OP_RETURN },
        compilation_result.value()->chunk.byte_code));

    auto const function_map = ExtractFunctions(compilation_result.value()->chunk);
    LOX_ASSERT(function_map.size() == 1);
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_GET_LOCAL, 0, 0,
                                     OP_PRINT,
                                     OP_NIL,
                                     OP_RETURN },
        function_map.begin()->second->chunk.byte_code));
}

TEST_F(CompilerTest, FunctionCall2)
{
    m_source.Append(R"(
fun Fib(n) {
    if( n<= 1) {
        return n;
    }
    return Fib(n-2) + Fib(n-1);
}
Fib(1);
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CLOSURE, 1, 0,
                                     OP_DEFINE_GLOBAL, 0, 0,
                                     OP_GET_GLOBAL, 2, 0,
                                     OP_CONSTANT, 3, 0,
                                     OP_CALL, 1, 0,
                                     OP_POP,
                                     OP_NIL,
                                     OP_RETURN },
        compilation_result.value()->chunk.byte_code));
    auto const function_map = ExtractFunctions(compilation_result.value()->chunk);
    LOX_ASSERT(function_map.size() == 1);
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_GET_LOCAL, 0, 0,
                                     OP_CONSTANT, 0, 0,
                                     OP_LESS_EQUAL,
                                     OP_JUMP_IF_FALSE, 8, 0,
                                     OP_POP,
                                     OP_GET_LOCAL, 0, 0,
                                     OP_RETURN,
                                     OP_JUMP, 1, 0,
                                     OP_POP,
                                     OP_GET_GLOBAL, 1, 0,
                                     OP_GET_LOCAL, 0, 0,
                                     OP_CONSTANT, 2, 0,
                                     OP_SUBTRACT,
                                     OP_CALL, 1, 0,
                                     OP_GET_GLOBAL, 3, 0,
                                     OP_GET_LOCAL, 0, 0,
                                     OP_CONSTANT, 4, 0,
                                     OP_SUBTRACT,
                                     OP_CALL, 1, 0,
                                     OP_ADD,
                                     OP_RETURN,
                                     OP_NIL, // Technically unreachable and can be pruned
                                     OP_RETURN },
        function_map.begin()->second->chunk.byte_code));
}

TEST_F(CompilerTest, InvalidReturnStatement)
{
    m_source.Append(R"(
var a = 1;
return;
)");
    auto compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_FALSE(compilation_result.has_value());
}

TEST_F(CompilerTest, CaptureLocal)
{
    m_source.Append(R"(
fun outer() {
  var x = "outside";
  fun inner() {
    print x;
    x = "set from inside";
  }
  inner();
  print x;
}
outer();
)");
    auto const compilation_result = m_compiler->CompileSource(m_source);
    ASSERT_TRUE(compilation_result.has_value());
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CLOSURE, 1, 0,
                                     OP_DEFINE_GLOBAL, 0, 0,
                                     OP_GET_GLOBAL, 2, 0,
                                     OP_CALL, 0, 0,
                                     OP_POP,
                                     OP_NIL,
                                     OP_RETURN },
        compilation_result.value()->chunk.byte_code));
    {
        auto string_object = StringObject {
            "outer"sv,
        };
        auto function_object = FunctionObject {
            "outer"sv,
            0
        };

        ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                          &string_object,
                                          &function_object,
                                          &string_object,
                                      },
            compilation_result.value()->chunk.constant_pool));
    }
    auto const function_map
        = ExtractFunctions(compilation_result.value()->chunk);
    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_CONSTANT, 0, 0,
                                     OP_CLOSURE, 1 /*Function_Obj_Cont_index_LSB*/, 0 /*Function_Obj_Cont_index_USB*/, 1 /*Is local*/, 0 /*Upvalue_index_LSB*/, 0 /*Upvalue_index_USB*/,
                                     OP_GET_LOCAL, 1, 0,
                                     OP_CALL, 0, 0,
                                     OP_POP,
                                     OP_GET_LOCAL, 0, 0,
                                     OP_PRINT,
                                     OP_POP,
                                     OP_CLOSE_UPVALUE,
                                     OP_NIL,
                                     OP_RETURN },
        function_map.at("outer")->chunk.byte_code));
    {
        auto string_object = StringObject {
            "outside"sv,
        };
        auto function_object = FunctionObject {
            "inner"sv,
            0
        };
        ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                          &string_object,
                                          &function_object },
            function_map.at("outer")->chunk.constant_pool));
    }

    ASSERT_TRUE(ValidateByteCode(std::vector<uint8_t> {
                                     OP_GET_UPVALUE, 0, 0,
                                     OP_PRINT,
                                     OP_CONSTANT, 0, 0,
                                     OP_SET_UPVALUE, 0, 0,
                                     OP_POP,
                                     OP_NIL,
                                     OP_RETURN },
        function_map.at("inner")->chunk.byte_code));

    {
        auto string_object = StringObject {
            "set from inside"sv,
        };
        ASSERT_TRUE(ValidateConstants(std::vector<Value> {
                                          &string_object },
            function_map.at("inner")->chunk.constant_pool));
    }
}
