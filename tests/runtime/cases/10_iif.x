/*
   FILE: 10_iif.x
   DESC: Check that only the selected conditional branch executes.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    print(iif(true, "sim", "nao"));
    print("\n");
    print(iif(false, "sim", "nao"));
    print("\n");
    print(iif(1 < 2, 10, 20));
    print("\n");
    print(iif(true, 1, 1/0));
    print("\n");
    print(iif(false, 1/0, 2));
    print("\n");
}
