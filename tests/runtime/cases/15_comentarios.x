/*
   FILE: 15_comentarios.x
   DESC: Check line and block comments around expressions.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    // A line comment does not consume the next expression.
    /* A block comment
       may span multiple lines. */
    print(1 + 1); // Trailing comment.
    print("\n");
    /* Between statements. */ print(2 * 2);
    print("\n");
}
