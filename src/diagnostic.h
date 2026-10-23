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

Diagnostic table[] = {
  (Diagnostic){"[type-mismatch]", ERROR, "Error near token %.*s. [type-mimatch]"}
};

#endif // DIAGNOSTIC_H
