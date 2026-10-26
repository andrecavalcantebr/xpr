/*
    FILE: exceptions.c
    DESC: try...catch with setjmp e longjmp
    AUTHOR: Andre Cavalcante
    DATE: October, 2026
    LICENSE: GPL-v3
*/

#include "exceptions.h"

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

constexpr int EXC_MAX_STACK=16;

static struct exc_stack {
    struct {
        jmp_buf env;
        int code;
        bool active;
    } frame[EXC_MAX_STACK];
    int top;
} exc_stack = {.top=0};

jmp_buf* exc_push(void) {
    assert(exc_stack.top >= 0 && exc_stack.top < EXC_MAX_STACK);
    exc_stack.frame[exc_stack.top].code = 0;
    exc_stack.frame[exc_stack.top].active = true;
    return &(exc_stack.frame[exc_stack.top++].env);
}

void exc_end(void) {
    assert(exc_stack.top > 0 && exc_stack.top < EXC_MAX_STACK);
    int pending = exc_stack.frame[exc_stack.top-1].code;
    exc_stack.top--;
    if(pending != 0)
        exc_throw(pending);
}

bool exc_match(int code) {
    assert(exc_stack.top > 0 && exc_stack.top < EXC_MAX_STACK);
    if(code != 0 && exc_stack.frame[exc_stack.top-1].code == code) {
        exc_stack.frame[exc_stack.top-1].active = false;
        exc_stack.frame[exc_stack.top-1].code = 0;
        return true;
    }
    return false;
}

[[noreturn]] void exc_throw(int code) {
    assert(exc_stack.top > 0 && exc_stack.top < EXC_MAX_STACK);
    assert(code != 0);
    for(int pos = exc_stack.top-1; pos >=0; pos--) {
        if(exc_stack.frame[pos].active) {
            exc_stack.frame[pos].code = code;
            exc_stack.top = pos + 1;
            longjmp(exc_stack.frame[pos].env, 1);
        }
    }
    fprintf(stderr, "throw without active try frame\n");
    exit(EXIT_FAILURE);
}
