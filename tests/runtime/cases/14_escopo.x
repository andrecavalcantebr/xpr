/*
   FILE: 14_escopo.x
   DESC: Check updates to an outer variable from an inner block.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    x = 1;
    {
        x = 2;
        internal = 10;
        print(internal);
        print("\n");
    };
    print(x);
    print("\n");
}
