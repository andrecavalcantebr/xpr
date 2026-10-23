/*
   FILE: 06_listas.x
   DESC: Check element-wise list operations and flat list output.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    items = [1, 2, 3];
    print(items);
    print("\n");
    print(len(items));
    print("\n");
    print([1, 2, 3] + [4, 5, 6]);
    print("\n");
    print([1, 2, 3] * [2, 2, 2]);
    print("\n");
    print(all(([1, 2, 3] - [1, 1, 1]) == [0, 1, 2]));
    print("\n");
    print([1, 2, 3] < [2, 3, 4]);
    print("\n");
    print([1, 2] == [1, 2]);
    print("\n");
    print(any([1, 2] == [1, 9]));
    print("\n");
    print(all([1, 2] == [1, 2]));
    print("\n");
    mixed = [1, "dois", true];
    print(mixed);
    print("\n");
    print(len(mixed));
    print("\n");
}
