/*
   FILE: ast.h
   DESC: Infrastructure to AST for the XPR language (interface).
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#ifndef XPR_AST_H__
#define XPR_AST_H__

#include <lexer.h>

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

typedef struct ASTNode ASTNode;

typedef struct {
    ASTNode **items;
    int count;
} NodeList;

typedef struct {
    char *chars;
    int len;
} string;

struct ASTNode {
    NodeType type;
    int line, col;  /* run-time errors must report the source position */
    union {
        double num;
        int boolean;
        string str;   /* len needed: "\0" is a valid escape */
        char *name;                             /* NODE_VAR */
        NodeList list;                          /* NODE_LIST, NODE_BLOCK */
        struct { TokenType op; ASTNode *operand; } unary;
        struct { TokenType op; ASTNode *left, *right; } binary;
        struct { ASTNode *target, *value; } assign;
        struct { ASTNode *target, *index; } index;
        struct { ASTNode *cond, *then_br, *else_br; } iif;
        struct { ASTNode *init, *cond, *step, *body; } loop;
        struct { char *name; ASTNode *param, *body; } fn_def;  /* param: VAR or LIST of VARs */
        struct { char *name; ASTNode *arg; } call;
    } as;
};


//function definition here


#endif   // XPR_AST_H__
