/*
   FILE: 05_blocos.x
   DESC: Check block results and required statement terminators.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    result = { left = 10; right = 20; left * right; };
    print(result);
    print("\n");
    print({ 2 * 3; } + 4);
    print("\n");
    sequence = { 1; 2; 3; };
    print(sequence);
    print("\n");
    print(len({}) == 0);
    print("\n");
}
