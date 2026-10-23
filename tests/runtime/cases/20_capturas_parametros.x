/*
   FILE: 20_capturas_parametros.x
   DESC: Check frozen captures, parameter shadowing and self binding.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    c = 9;
    add = fn(x) { x + c; };
    c = 19;
    print(add(1));
    print("\n");
    x = 100;
    increment = fn(x) { x + 1; };
    print(increment(2));
    print("\n");
    print(x);
    print("\n");
    factorial = fn(n) { iif(n <= 1, 1, n * factorial(n - 1)); };
    saved = factorial;
    factorial = fn(n) { 999; };
    print(saved(3));
    print("\n");
}
