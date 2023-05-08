# A byte-code virtual machine lox interpreter written in C++

This is a C++ implementation of the Lox programming language as I work through Robert Nystrom's wonderful book [Crafting Interpreters](https://craftinginterpreters.com/).

The implementation does lean on some C++23 language/library features and so will require gcc-13 or clang-16. You can find docker development environments here [clang](https://github.com/kevinjoseph1995/dev_env/tree/main/clang) and [gcc](https://github.com/kevinjoseph1995/dev_env/tree/main/gcc).

Work in progress
- [x] Global variables
- [x] Lexical Scopes and scoped local variables
- [x] Conditionals
- [x] Loops
- [x] Functions
- [x] Closures
- [x] Garbage Collection
- [x] Classes and Instances (Partial WIP)
- [ ] Methods and Initializers
- [ ] Superclasses

```lox
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
```

```lox
fun Fib(n) {
    if( n<= 1) {
        return n;
    }
    return Fib(n-2) + Fib(n-1);
}
print Fib(10);
```

```lox
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
```

```lox
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
```
