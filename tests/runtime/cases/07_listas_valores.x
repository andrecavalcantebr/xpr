/*
   FILE: 07_listas_valores.x
   DESC: Check independent copies, indexing and boolean reductions.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    items = [1, 2, 3];
    copy = items;
    copy[0] = 9;
    print(items);
    print("\n");
    print(copy);
    print("\n");
    print(items[2]);
    print("\n");
    print(items[0]);
    print("\n");
    print(any([false, true, false]));
    print("\n");
    print(any([false, false]));
    print("\n");
    print(all([true, true]));
    print("\n");
    print(all([true, false]));
    print("\n");
}
