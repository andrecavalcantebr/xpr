/*
   FILE: 11_shortcircuit.x
   DESC: Check boolean short-circuiting without lazy multiplication.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    print(false and 1/0);
    print("\n");
    print(true or 1/0);
    print("\n");
    print(true and true and false);
    print("\n");
    print(false or false or true);
    print("\n");
}
