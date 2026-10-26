/*
   FILE: xpr.h
   DESC: Global definitions and common types for the XPR project.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#ifndef XPR_H
#define XPR_H

#include <stddef.h>

/* Versioning */
#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_BUGS  0

/* Source definition: centralized representation of a code buffer */
typedef struct {
    const unsigned char *content;
    size_t size;
    const char *filename;
} Source;

#define SOURCE_NULL ((Source){0})

#endif
