/*
   FILE: 18_strings_codepoints.x
   DESC: Check embedded NUL and UTF-8 lengths.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    print(len("olá"));
    print("\n");
    print(len("a\0b"));
    print("\n");
    print(len("世界"));
    print("\n");
}
