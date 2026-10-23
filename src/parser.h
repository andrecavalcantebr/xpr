/*
   FILE: parser.h
   DESC: Simple parser to verify token stream.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#ifndef PARSER_H
#define PARSER_H

#include "xpr.h"
#include "lexer.h"

typedef enum {
    /* literals and names */
    NODE_NUM,
    NODE_STR,
    NODE_BOOL,
    NODE_LIST,      /* [e1, e2, ...] */
    NODE_VAR,
    /* operations */
    NODE_ASSIGN,    /* target = value; target is VAR, INDEX or LIST */
    NODE_UN_OP,     /* not x, -x, +x */
    NODE_BIN_OP,
    NODE_INDEX,     /* v[i] */
    /* constructs */
    NODE_BLOCK,     /* { s1; s2; } */
    NODE_IIF,
    NODE_FOR,
    NODE_FN_DEF,    /* fn(param) body */
    NODE_FN_CALL    /* name(arg) */
} NodeType;

typedef struct Node Node;

typedef struct {
    Node **items;
    int count;
} NodeList;

struct Node {
    NodeType type;
    int line, col;  /* run-time errors must report the source position */
    union {
        double num;
        int boolean;
        struct { char *chars; int len; } str;   /* len needed: "\0" is a valid escape */
        char *name;                             /* NODE_VAR */
        NodeList list;                          /* NODE_LIST, NODE_BLOCK */
        struct { TokenType op; Node *operand; } unary;
        struct { TokenType op; Node *left, *right; } binary;
        struct { Node *target, *value; } assign;
        struct { Node *target, *index; } index;
        struct { Node *cond, *then_br, *else_br; } iif;
        struct { Node *init, *cond, *step, *body; } loop;
        struct { char *name; Node *param, *body; } fn_def;  /* param: VAR or LIST of VARs */
        struct { char *name; Node *arg; } call;
    } as;
};

typedef struct {
    Node* root;
    Node* childs[];
} AST;

void parser_dump_lexer(Source source);

#endif