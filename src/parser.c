/*
   FILE: parser.c
   DESC: Implementation of a simple token-printing parser for debugging.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#include <stdio.h>
#include "parser.h"
#include "lexer.h"
// #include "diagnostic.h"

void parse_program(AST *ast) {
    (void)ast;
}

void parser_dump_lexer(Source source) {
    Lexer lex, *l = &lex;
    // Passamos a struct Source completa para o lexer
    lexer_init(l, source);
    Token t;
    do {
        t = lexer_next_token(l);
        char line_col[10] = {0};
        snprintf(line_col, sizeof(line_col), "%d:%d", t.line, t.col);
        if (t.start != NULL && t.len > 0) {
            printf("%-6s | Token: %-12s | Value: %.*s\n",
                line_col,
                token_name(t.type), t.len, t.start);
        } else {
            printf("%-6s | Token: %-12s | Value: <?>\n",
                line_col,
                token_name(t.type));
        }
    } while (t.type != TOKEN_EOF && t.type != TOKEN_ERROR);
}