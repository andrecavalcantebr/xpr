/*
   FILE: 08_funcoes.x
   DESC: Check function values, empty arguments and indexed parameters.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    double = fn(x) { x * 2; };
    print(double(10));
    print("\n");
    constant = fn(unused) { 42; };
    print(constant());
    print("\n");
    transform = fn(x) { local = x * 2; local + 1; };
    print(transform(5));
    print("\n");
    apply = fn(args) { function = args[0]; value = args[1]; function(value); };
    print(apply([double, 21]));
    print("\n");
    square = fn(n) { n * n; };
    print(apply([square, 7]));
    print("\n");
}
