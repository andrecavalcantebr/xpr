/*
   FILE: diagnostic.h
   DESC: Interface for diagnostics.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

typedef enum {
    INFO,
    WARNING,
    ERROR,
    FATAL
} Severity;

typedef struct {
    char *diag;
    Severity severity;
    char *message;
} Diagnostic;

extern Diagnostic table[];

#endif // DIAGNOSTIC_H
