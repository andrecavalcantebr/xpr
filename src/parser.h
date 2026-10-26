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
#include "ast.h"

ASTNode *parser_parse(Source source);
void parser_dump_lexer(Source source);

#endif
