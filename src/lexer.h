/*
   FILE: lexer.h
   DESC: Tokenizer definitions and interface for the XPR language.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#ifndef LEXER_H
#define LEXER_H

#include "xpr.h"

typedef enum {
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_FN,
    TOKEN_IIF,
    TOKEN_FOR,
    TOKEN_NOT,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_BOOLEAN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LTE,
    TOKEN_GTE,
    TOKEN_ASSIGN,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COMMA,
    TOKEN_SEMICOLON
} TokenType;

static inline const char *token_name(TokenType type) {
    const char* t_names[] = {
        "EOF",
        "ERROR",
        "NUMBER",
        "STRING",
        "IDENTIFIER",
        "FN",
        "IIF",
        "FOR",
        "NOT",
        "AND",
        "OR",
        "BOOLEAN",
        "PLUS",
        "MINUS",
        "STAR",
        "SLASH",
        "PERCENT",
        "EQ",
        "NEQ",
        "LT",
        "GT",
        "LTE",
        "GTE",
        "ASSIGN",
        "LPAREN",
        "RPAREN",
        "LBRACE",
        "RBRACE",
        "LBRACKET",
        "RBRACKET",
        "COMMA",
        "SEMICOLON"
    };
    return t_names[type];
}

typedef struct {
    TokenType type;
    const unsigned char *start;
    int len;
    int line, col;
} Token;

typedef struct {
    Source source;
    const unsigned char *cur;
    int line, col;
} Lexer;

//Initializes the lexer
void lexer_init(Lexer *l, Source source);

//Returns the next token
Token lexer_next_token(Lexer *l);

//Just sees the next token, but don't move up the pointer
Token lexer_peek(Lexer *l);

//Mark the state of the lexer
Lexer lexer_mark(Lexer *l);

//Restore the state of the lexer
void lexer_restore(Lexer *l, Lexer marked);

#endif
