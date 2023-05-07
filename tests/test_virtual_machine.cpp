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
        m_vm = std::make_unique<VirtualMachine>(&m_vm_output_stream);
    }
    std::unique_ptr<VirtualMachine> m_vm;
    Source m_source;
    std::string m_vm_output_stream;
};

TEST_F(VMTest, ExpressionTest)
{
    m_source.Append(R"(
  print 8 * 1 + 2 + 3 + 3 * 4 + (2 + 2);
  print 1 / 2;
  print "Hello" + "World";
)");
    static constexpr auto OUTPUT = "29\n"
                                   "0.5\n"
                                   "HelloWorld\n";
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    ASSERT_EQ(m_vm_output_stream, OUTPUT);
}

TEST_F(VMTest, GlobalVariableDeclaration)
{
    m_source.Append(R"(
  var a =  8 * 1 + 2 + 3 + 3 * 4 + (2 + 2);
  print a;
)");
    static constexpr auto EXPECTED_OUTPUT = "29\n";
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, GlobalVariableReAssignment)
{
    m_source.Append(R"(
  var a =  8 * 1 + 2 + 3 + 3 * 4 + (2 + 2);
  print a;
  a = "HelloWorld";
  print a;
)");
    static constexpr auto EXPECTED_OUTPUT = "29\n"
                                            "HelloWorld\n";
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, TestMixedConcatenation)
{
    m_source.Append(R"(
  var a =  1 + "Hello World";
)");
    ASSERT_FALSE(m_vm->Interpret(m_source).has_value());
}

TEST_F(VMTest, TestStringComparison)
{
    m_source.Append(R"(
  var a =  "Hello" +  "World";
  var b = "HelloWorld";
  print a == b;
  print "FooBar" == a;
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "true\n"
                                            "false\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, TestNumberComparison)
{
    m_source.Append(R"(
  print 1 < 2;
  print 1 == 1;
  print 1.0 == 1.1;
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "true\n"
                                            "true\n"
                                            "false\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, TestDefaultValue)
{
    m_source.Append(R"(
  var a;
  print a;
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "Nil\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, TestLocalVaraibles1)
{
    m_source.Append(R"(
{
     var abcd = 10;
     {
         var abcd;
         print abcd;
     }
     print abcd;
 }
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "Nil\n10\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, TestLocalVaraibles2)
{
    m_source.Append(R"(
{
     var abcd = 10;
     {
         var abcd = abcd; // Cannot refer to same variable in the initializer
     }
 }
)");
    ASSERT_FALSE(m_vm->Interpret(m_source).has_value());
}

TEST_F(VMTest, TestLocalVaraibles3)
{
    m_source.Append(R"(
{
    var a = 10;
    print a;
}
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "10\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, IfStatement)
{
    m_source.Append(R"(
{
     if(false) {
         print "If-branch";
     }
     print "Jumped here";
 }
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "Jumped here\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, IfStatement2)
{
    m_source.Append(R"(
{
     if(false) {
         print "If-branch";
     } else{
         print "Else-branch";
     }
     print "Jumped here";
 }
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "Else-branch\nJumped here\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, LogicalOperatorsAnd)
{
    m_source.Append(R"(
{
     print false and true;
 }
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "false\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, LogicalOperatorsOr)
{
    m_source.Append(R"(
{
     print false or true or false;
 }
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "true\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, LogicalOperatorsOr2)
{
    m_source.Append(R"(
{
     if((1 + 2 * 4) < 0 or true or false) {
         print "True branch";
     } else {
         print "False branch";
     }

 }
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "True branch\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, WhileStatement)
{
    m_source.Append(R"(
{
     var a  = 0;
     while(a < 3) {
         print a;
         a = a + 1;
     }
     while(a >= 0) {
         print a;
         a = a - 1;
     }
 }
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "0\n1\n2\n3\n2\n1\n0\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, ForStatement)
{
    m_source.Append(R"(
{
     for(var i = 0; i < 3; i = i + 1){
         print i;
     }
 }
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "0\n1\n2\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, Accumulation)
{
    m_source.Append(R"(
{
     var sum = 0;
     for(var i = 1; i <= 3; i = i + 1){
         sum = sum + i;
     }
     print sum;
 }
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "6\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, AccumulationGlobal)
{
    m_source.Append(R"(
  var sum = 0;
{
     for(var i = 1; i <= 3; i = i + 1){
         sum = sum + i;
     }
     print sum;
 }
  print sum;
  sum = 1;
  print sum;
)");
    ASSERT_TRUE(m_vm->Interpret(m_source).has_value());
    static constexpr auto EXPECTED_OUTPUT = "6\n6\n1\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, ForStatementScopeLeak)
{
    m_source.Append(R"(
        for(var i = 0; i < 3; i = i + 1){
            print i;
        }
        print i;
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_FALSE(result.has_value());
}

TEST_F(VMTest, FunctionDeclaration)
{
    m_source.Append(R"(

fun MyFunction(param1, param2, param3) {
}

print MyFunction;

)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "closure<MyFunction, arity=3>\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, FunctionCall1)
{
    m_source.Append(R"(

fun MyFunction(message) {
    print message;
}
MyFunction("Hello world");
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "Hello world\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, FunctionCall2)
{
    m_source.Append(R"(

fun MyFunction(arg1, arg2, arg3) {
    print arg1 + arg2 + arg3;
}
MyFunction(1, 0, 1);
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "2\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, ReturnValue1)
{
    m_source.Append(R"(

fun MyFunction() {
}
print MyFunction();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "Nil\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, ReturnValue2)
{
    m_source.Append(R"(

fun MyFunction() {
    return 1;
}
print MyFunction();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "1\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, ReturnValue3)
{
    m_source.Append(R"(

fun MyFunction(arg1, arg2, arg3) {
    return arg1 + arg2 + arg3;
}
print MyFunction(0, 1, 2);
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "3\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, FunctionCallRecursion)
{
    m_source.Append(R"(
fun Fib(n) {
    if( n<= 1) {
        return n;
    }
    return Fib(n-2) + Fib(n-1);
}
print Fib(10);
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "55\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, FunctionCall3)
{
    m_source.Append(R"(
fun concatenate(str1, str2) {
 return str1 + str2;
}

print concatenate("Hello", "World");
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "HelloWorld\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, RuntimeError1)
{
    m_source.Append(R"(
fun f(a, b) {
  print a;
  print b;
}

f(1, 2, 3, 4); // expect runtime error: Expected 2 arguments but got 4.
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(!result.has_value());
}

TEST_F(VMTest, RuntimeError2)
{
    m_source.Append(R"(
{
  fun isEven(n) {
    if (n == 0) return true;
    return isOdd(n - 1); // expect runtime error: Undefined variable 'isOdd'.
  }

  fun isOdd(n) {
    if (n == 0) return false;
    return isEven(n - 1);
  }

  isEven(4);
}
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(!result.has_value());
}

TEST_F(VMTest, NativeFunction1)
{
    m_source.Append(R"(
print SystemTimeNow();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    auto time_point_1 = std::stoll(m_vm_output_stream);
    m_vm_output_stream.resize(m_vm_output_stream.size() - 1); // Remove the "\n"
    m_vm_output_stream.clear();
    result = m_vm->Interpret(m_source);
    m_vm_output_stream.resize(m_vm_output_stream.size() - 1); // Remove the "\n"
    ASSERT_TRUE(result.has_value());
    auto time_point_2 = std::stoll(m_vm_output_stream);
    ASSERT_TRUE(time_point_2 > time_point_1);
}

TEST_F(VMTest, NativeFunction2)
{
    m_source.Append(R"(
var a = "Hello World";
print Echo(a);
print Echo(666);
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "Hello World\n666\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal)
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
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "outside\nset from inside\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal2)
{
    m_source.Append(R"(
{
  var a = 1;
  fun f() {
    print a;
  }
  var b = 2;
  fun g() {
    print b;
  }
  var c = 3;
  fun h() {
    print c;
  }
  f();
  g();
  h();
}
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "1\n2\n3\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal3)
{
    m_source.Append(R"(
var globalOne;
var globalTwo;

fun main() {
  {
    var a = "one";
    fun one() {
      print a;
    }
    globalOne = one;
  }

  {
    var a = "two";
    fun two() {
      print a;
    }
    globalTwo = two;
  }
}

main();
globalOne();
globalTwo();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "one\ntwo\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal4)
{
    m_source.Append(R"(
var globalOne;
var globalTwo;

fun main() {
  for (var a = 1; a <= 2; a = a + 1) {
    fun closure() {
      print a;
    }
    if (globalOne == nil) {
      globalOne = closure;
    } else {
      globalTwo = closure;
    }
  }
}

main();
globalOne();
globalTwo();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "3\n3\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal5)
{
    m_source.Append(R"(
var global;
{
  var a = 3;
  {
    {
        {
            {
                {
                    fun f() {
                         print a;
                    }
                    f();
                    global = f;
                }
            }
        }
    }
  }

}
global();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "3\n3\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal6)
{
    m_source.Append(R"(
fun function1() {
    var a = 10;
    fun function2() {
        fun function3() {
            print a;
        }
        return function3;
    }
    return function2;
}

var function2 = function1();
var function3 = function2();
function3();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "10\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal7)
{
    m_source.Append(R"(
var global;
fun function1() {
    var a = 10;
    fun function2() {
        fun function3() {
            print a;
        }
        global = function3;
    }
    return function2;
}

var function2 = function1();
function2();
global();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "10\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal8)
{
    m_source.Append(R"(
var global;
{
    var a = 10;
    {
        {
            fun function() {
                print a;
            }
            global = function;
        }
    }
}
global();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "10\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal9)
{
    m_source.Append(R"(
fun make_adder() {
  var a = 5;
  fun adder(i) {
    return a + i;
  }
  return adder;
}
var add5 = make_adder();
print add5(1);
print add5(2);
print add5(3);
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "6\n7\n8\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal10)
{
    m_source.Append(R"(
var a = "global";

{
  fun assign() {
    a = "assigned";
  }

  var a = "inner";
  assign();
  print a; // expect: inner
}

print a; // expect: assigned
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "inner\nassigned\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, CaptureLocal11)
{
    m_source.Append(R"(
var f;
var g;

{
  var local = "local";
  fun f_() {
    print local;
    local = "after f";
    print local;
  }
  f = f_;

  fun g_() {
    print local;
    local = "after g";
    print local;
  }
  g = g_;
}

f();
// expect: local
// expect: after f

g();
// expect: after f
// expect: after g
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "local\nafter f\nafter f\nafter g\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, ClassTest1)
{
    m_source.Append(R"(
class MyClass {}

print MyClass;
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "class_object[MyClass]\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, InstanceTest1)
{
    m_source.Append(R"(
class MyClass {}

print MyClass();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "instance[class[MyClass]]\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, InstanceTest2)
{
    m_source.Append(R"(
class MyClass {}
var my_instance = MyClass();
my_instance.field1 = 10;
print my_instance.field1;
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "10\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, InstanceTest3)
{
    m_source.Append(R"(
class Toast {}
var toast = Toast();
print toast.jam = "grape"; // Prints "grape".
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "grape\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}

TEST_F(VMTest, InstanceTest4)
{
    m_source.Append(R"(
class Outer {
  method() {
    class Inner {
      method() {
        print this;
      }
    }
    return Inner;
  }
}
var class_internal = Outer().method();
var internal_instance = class_internal();
internal_instance.method();
)");
    auto result = m_vm->Interpret(m_source);
    ASSERT_TRUE(result.has_value());
    static constexpr auto EXPECTED_OUTPUT = "instance[class[Inner]]\n";
    ASSERT_EQ(m_vm_output_stream, EXPECTED_OUTPUT);
}
