/*
   FILE: 09_recursao.x
   DESC: Check that recursive call frames remain independent.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    factorial = fn(n) { iif(n <= 1, 1, n * factorial(n - 1)); };
    print(factorial(5));
    print("\n");
    print(factorial(10));
    print("\n");
    fibonacci = fn(n) { iif(n < 2, n, fibonacci(n - 1) + fibonacci(n - 2)); };
    print(fibonacci(10));
    print("\n");
    print(fibonacci(15));
    print("\n");
}
