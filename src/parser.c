/*
   FILE: parser.c
   DESC: Implementation of a simple token-printing parser for debugging.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "diagnostic.h"
#include "tgc.h"

#include <stdio.h>

static tgc_t global_gc;

static bool parse_assigment(Lexer *l) {
    (void)l;
    return false;
}

static bool parse_fn_def(Lexer *l) {
    (void)l;
    return false;
}

static bool parse_logical_or(Lexer *l) {
    (void)l;
    return false;
}

static bool parse_block(Lexer *l) {
    (void)l;
    return false;
}

static ASTNode  *parse_expression(Lexer *l) {
    // if(parse_block(l, ast)) return true;
    // if(parse_fn_def(l, ast)) return true;
    // if(parse_assigment(l, ast)) return true;
    // if(parse_logical_or(l, ast)) return true;
    return NULL;
}

static ASTNode *parse_program(Lexer *l) {
    return parse_expression(l);
}

// -- Public API

void parser_init(tgc_t gc) {
    global_gc = gc;
}

ASTNode *parser_parse(Source source) {
    Lexer lex, *l = &lex;
    lexer_init(l, source);
    ASTNode *ast = parse_program(l);
    return ast;
}

void parser_dump_lexer(Source source) {
    Lexer lex, *l = &lex;
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
