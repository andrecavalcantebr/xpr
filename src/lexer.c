/*
   FILE: lexer.c
   DESC: Implementation of the XPR tokenizer.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#include "lexer.h"
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static inline unsigned char peek(Lexer *l) {
    return *(l->cur);
}

static inline unsigned char ahead(Lexer *l) {
    if(*(l->cur) == '\0') return '\0';
    return *((l->cur)+1);
}

static inline unsigned char advance(Lexer *l) {
    if (*(l->cur) == '\0') {
        return '\0';
    }
    unsigned char c = *l->cur++;
    if (c == '\n') {
        l->line++;
        l->col = 1;
    } else {
        // Columns count characters, not bytes: UTF-8 continuation bytes (10xxxxxx) don't advance it.
        if ((c & 0xC0) != 0x80) l->col++;
    }
    return c;
}

static inline bool match(Lexer *l, char c) {
    return (peek(l) == c);
}

static inline bool match_or(Lexer *l, char c1, char c2) {
    return (peek(l) == c1 || peek(l) == c2);
}

static inline bool match_and_ahead(Lexer *l, char c1, char c2) {
    return (peek(l) == c1 && ahead(l) == c2);
}

typedef enum {LEX_NOT_EXISTS, LEX_EXISTS, LEX_ERROR} lex_status;

static lex_status skip_space(Lexer *l) {
    if(isspace(peek(l))) {
        while(peek(l) != '\0' && isspace(peek(l)))
            advance(l);
        return LEX_EXISTS;
    }
    return LEX_NOT_EXISTS;
}

static lex_status skip_comment(Lexer *l) {
    if(peek(l) != '\0' && match_and_ahead(l, '/', '/')) {
        while(peek(l) != '\0' && peek(l) != '\n')
            advance(l);
        if(peek(l) != '\0') {
            advance(l);    //consumes the last '\n'
        }
        return LEX_EXISTS;
    }
    if(peek(l) != '\0' && match_and_ahead(l, '/', '*')) {
        advance(l);
        advance(l);
        while(peek(l) != '\0' && !match_and_ahead(l, '*', '/'))
            advance(l);
        if(peek(l) == '\0')
            return LEX_ERROR;
        else {
            advance(l);      // cosumes the last */
            advance(l);
            return LEX_EXISTS;
        }
    }
    return LEX_NOT_EXISTS;
}

static inline void consume_digits(Lexer *l) {
    while (isdigit(peek(l)))
        advance(l);
}

static lex_status parse_int_num(Lexer *l) {
    if(isdigit(peek(l))) {
        consume_digits(l);
        return LEX_EXISTS;
    }
    return LEX_NOT_EXISTS;
}

static lex_status parse_exp_part(Lexer *l) {
    if(match_or(l, 'e', 'E')) {
        advance(l);
        if(isalpha(peek(l))) {
            return LEX_ERROR;
        }
        if(match_or(l, '+', '-')) {
            advance(l);
        }
        return parse_int_num(l) == LEX_NOT_EXISTS ? LEX_ERROR : LEX_EXISTS;
    }
    return LEX_NOT_EXISTS;
}

static lex_status parse_number(Lexer *l) {
    if(isdigit(peek(l))) {
        parse_int_num(l);
        if(match(l, '.')) {
            advance(l);
            consume_digits(l);
        }
        if(parse_exp_part(l) == LEX_ERROR)
            return LEX_ERROR;
        else
            return LEX_EXISTS;
    } else if(match(l, '.')) {
        if(isdigit(ahead(l))) {
            advance(l);     //consumes
            parse_int_num(l);
            if(parse_exp_part(l) == LEX_ERROR)
                return LEX_ERROR;
            else
                return LEX_EXISTS;
        } else {
            return LEX_NOT_EXISTS;
        }
    }
    return LEX_NOT_EXISTS;
}

static lex_status parse_identify(Lexer *l) {
    if(isalpha(peek(l)) || peek(l) == '_') {
        advance(l);
        while(peek(l) != '\0' && (isalnum(peek(l)) || peek(l) == '_')) {
            advance(l);
        }
        return LEX_EXISTS;
    }
    return LEX_NOT_EXISTS;
}

static bool match_keyword(const char *start, int len, const char *keyword) {
    if (len != (int)strlen(keyword)) return false;
    return strncmp(start, keyword, len) == 0;
}

// Maps a lexeme to its keyword token; anything else is a plain identifier.
static TokenType keyword_type(const unsigned char *start, int len) {
    static const struct { const char *text; TokenType type; } kw[] = {
        {"fn",    TOKEN_FN},
        {"iif",   TOKEN_IIF},
        {"for",   TOKEN_FOR},
        {"not",   TOKEN_NOT},
        {"and",   TOKEN_AND},
        {"or",    TOKEN_OR},
        {"true",  TOKEN_BOOLEAN},
        {"false", TOKEN_BOOLEAN},
    };
    for(size_t i=0; i<sizeof(kw)/sizeof(kw[0]); i++) {
        if(match_keyword((const char *)start, len, kw[i].text)) return kw[i].type;
    }
    return TOKEN_IDENTIFIER;
}

static lex_status parse_string(Lexer *l) {
    if(peek(l) == '"' || peek(l) == '\'') {
        unsigned char term = advance(l);
        while(peek(l) != '\0' && peek(l) != term) {
            if(peek(l) == '\\' && ahead(l) != '\0') {
                advance(l);
            }
            advance(l);
        }
        if(peek(l) == term) {
            advance(l);   // consumes the term
            return LEX_EXISTS;
        } else
            return LEX_ERROR;
    }
    return LEX_NOT_EXISTS;
}

static Token next_token(Lexer *l) {
    assert(l != NULL);

    Token token = {0},
          token_eof = {.type=TOKEN_EOF, .start=(unsigned char*)"<EOF>", .len=5};

    // skip all that is not token
    while(true) {
        if(peek(l) == '\0') {
            token_eof.line = l->line;
            token_eof.col = l->col;
            return token_eof;
        }
        if(skip_space(l))
            continue;
        if(skip_comment(l))
            continue;

        // Capture start position immediately before parsing the token
        token.start = l->cur;
        token.line = l->line;
        token.col = l->col;

        lex_status s_num = parse_number(l);
        if(s_num == LEX_EXISTS) {
            token.type = TOKEN_NUMBER;
            token.len = (int)(l->cur - token.start);
            return token;
        } else if (s_num == LEX_ERROR) {
            token.type = TOKEN_ERROR;
            token.len = (int)(l->cur - token.start);
            return token;
        }

        if(parse_identify(l) == LEX_EXISTS) {
            token.type = keyword_type(token.start, (int)(l->cur - token.start));
            token.len = (int)(l->cur - token.start);
            return token;
        }

        lex_status s_str = parse_string(l);
        if(s_str == LEX_EXISTS) {
            token.type = TOKEN_STRING;
            token.len = (int)(l->cur - token.start);
            return token;
        } else if (s_str == LEX_ERROR) {
            token.type = TOKEN_ERROR;
            token.len = (int)(l->cur - token.start);
            return token;
        }

        unsigned char c1 = advance(l);
        unsigned char c2 = peek(l);

        switch (c1) {
            case '+':
                token.len = 1;
                token.type = TOKEN_PLUS;
                break;
            case '-':
                token.len = 1;
                token.type = TOKEN_MINUS;
                break;
            case '*':
                token.len = 1;
                token.type = TOKEN_STAR;
                break;
            case '/':
                token.len = 1;
                token.type = TOKEN_SLASH;
                break;
            case '%':
                token.len = 1;
                token.type = TOKEN_PERCENT;
                break;
            case '(':
                token.len = 1;
                token.type = TOKEN_LPAREN;
                break;
            case ')':
                token.len = 1;
                token.type = TOKEN_RPAREN;
                break;
            case '{':
                token.len = 1;
                token.type = TOKEN_LBRACE;
                break;
            case '}':
                token.len = 1;
                token.type = TOKEN_RBRACE;
                break;
            case '[':
                token.len = 1;
                token.type = TOKEN_LBRACKET;
                break;
            case ']':
                token.len = 1;
                token.type = TOKEN_RBRACKET;
                break;
            case ',':
                token.len = 1;
                token.type = TOKEN_COMMA;
                break;
            case ';':
                token.len = 1;
                token.type = TOKEN_SEMICOLON;
                break;
            case '=':
                if(c2 == '=') {
                    advance(l);
                    token.len = 2;
                    token.type = TOKEN_EQ;
                } else {
                    token.len = 1;
                    token.type = TOKEN_ASSIGN;
                }
                break;
            case '!':
                if (c2 == '=') {
                    advance(l);
                    token.len = 2;
                    token.type = TOKEN_NEQ;
                } else {
                    token.len = 1;
                    token.type = TOKEN_ERROR;
                }
                break;
            case '<':
                if (peek(l) == '=') {
                    advance(l);
                    token.len = 2;
                    token.type = TOKEN_LTE;
                } else {
                    token.len = 1;
                    token.type = TOKEN_LT;
                }
                break;
            case '>':
                if (peek(l) == '=') {
                   advance(l);
                   token.len = 2;
                   token.type = TOKEN_GTE;
                } else {
                    token.len = 1;
                    token.type = TOKEN_GT;
                }
                break;
            default:
                token.len = 1;
                token.type = TOKEN_ERROR;
        }
        return token;
    }
}

// ** Public API

void lexer_init(Lexer *l, Source source) {
    l->source = source;
    l->cur = source.content;
    l->line = 1;
    l->col = 1;
}

Token lexer_next_token(Lexer *l) {
    return next_token(l);
}

Token lexer_peek(Lexer *l) {
    Lexer marked = lexer_mark(l);
    Token tok = next_token(l);
    lexer_restore(l, marked);
    return tok;
}

Lexer lexer_mark(Lexer *l) {
    return *l;
}
void lexer_restore(Lexer *l, Lexer marked) {
    *l = marked;
};
