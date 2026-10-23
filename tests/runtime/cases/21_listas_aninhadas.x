/*
   FILE: 21_listas_aninhadas.x
   DESC: Check recursive operations and deep copies of nested lists.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    items = [[1], [2]] + [[3], [4]];
    copy = items;
    copy[0][0] = 9;
    print(items[0][0]);
    print("\n");
    print(items[1][0]);
    print("\n");
    print(copy[0][0]);
    print("\n");
}
