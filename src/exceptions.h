/*
    FILE: exceptions.h
    DESC: try...catch with setjmp e longjmp
    AUTHOR: Andre Cavalcante
    DATE: October, 2026
    LICENSE: GPL-v3
*/

#ifndef EXCEPTIONS_H_GUARD__
#define EXCEPTIONS_H_GUARD__

#include <setjmp.h>

jmp_buf* exc_push(void);
void exc_end(void);
bool exc_match(int code);
[[noreturn]] void exc_throw(int code);

#define try for(volatile int exc_once_=1; exc_once_; exc_end(), exc_once_=0) \
                if(setjmp(*exc_push()) == 0)

#define catch(error_code) else if(exc_match(error_code))

#define throw(error_code) exc_throw(error_code)

// #define end_try exc_end()

#endif  //EXCEPTIONS_H_GUARD__
