/*
   FILE: loader.h
   DESC: Interface for loading .x source files into memory, once per file.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#ifndef LOADER_H
#define LOADER_H

#include <stdbool.h>
#include "xpr.h"
#include "tgc.h"

// Inicializa o loader com a instância do GC do projeto
void loader_init(tgc_t *gc);

// Carrega um arquivo. Retorna a estrutura Source ou SOURCE_NULL se falhar
Source loader_load_file(const char *filename);

// Verifica se um arquivo já está na memória; se sim retorna-o, se não, retorna SOURCE_NULL
Source loader_get_loaded(const char *filename);

// Retorna o tamanho do conteúdo de um arquivo carregado
static inline size_t loader_get_size(Source src) {
    return src.size;
};

#endif