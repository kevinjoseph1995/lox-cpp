# A byte-code virtual machine lox interpreter written in C++
Work in progress
- [x] Global variables
- [x] Lexical Scopes and scoped local variables
- [x] Conditionals
- [x] Loops
- [x] Functions
- [ ] Closures
- [ ] Garbage Collection
- [ ] Classes and Instances
- [ ] Methods and Initializers
- [ ] Superclasses

```
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

```
fun Fib(n) {
    if( n<= 1) {
        return n;
    }
    return Fib(n-2) + Fib(n-1);
}
print Fib(10);
```
