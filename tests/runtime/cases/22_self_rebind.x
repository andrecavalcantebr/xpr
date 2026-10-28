/*
   FILE: 22_self_rebind.x
   DESC: Check the complete self-reference contract (spec 4.5): the recursive
         call is frozen in the closure (early binding), a call through the
         variable follows reassignment, and a function captured as a free
         variable keeps the function that existed at its definition.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    fat = fn(x) { iif(x <= 1, 1, x * fat(x - 1)); };
    g = fat;
    fat = fn(x) { 999; };
    print(g(5));
    print("\n");
    print(fat(5));
    print("\n");
    h = fn(x) { fat(x); };
    print(h(3));
    print("\n");
    fat = fn(x) { 7; };
    print(fat(3));
    print("\n");
    print(h(3));
    print("\n");
    print(g(5));
    print("\n");
}
