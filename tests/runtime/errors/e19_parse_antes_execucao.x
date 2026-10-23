/*
   FILE: e19_parse_antes_execucao.x
   DESC: Require the intended language error without prior output.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

{
    print("must not print");
    unused = fn(x) { 2 + * 3; };
}
