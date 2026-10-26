/*
   FILE: loader.c
   DESC: Loads .x source files into GC-managed memory and caches them by name.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#include "loader.h"
#include <stdio.h>
#include <string.h>

typedef struct FileNode {
    char *filename;
    Source source;
    struct FileNode *next;
} FileNode;

static FileNode *loaded_files = NULL;
static tgc_t *global_gc = NULL;

void loader_init(tgc_t *gc) {
    global_gc = gc;
    loaded_files = NULL;
}

Source loader_get_loaded(const char *filename) {
    FileNode *curr = loaded_files;
    while (curr) {
        if (strcmp(curr->filename, filename) == 0)
            return curr->source;
        curr = curr->next;
    }
    return SOURCE_NULL;
}

Source loader_load_file(const char *filename) {
    if (!global_gc) return SOURCE_NULL;

    Source src = loader_get_loaded(filename);
    if (src.content) return src;

    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Erro: Não foi possível abrir o arquivo %s\n", filename);
        return SOURCE_NULL;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Alocação via TGC para o conteúdo do arquivo (TGC_LEAF pois é apenas dados)
    unsigned char *buffer = tgc_alloc_opt(global_gc, length + 1, TGC_LEAF, NULL);
    if (!buffer) {
        fclose(f);
        return SOURCE_NULL;
    }

    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);

    // Alocação via TGC para o nó de controle
    FileNode *node = tgc_alloc(global_gc, sizeof(FileNode));
    node->filename = tgc_alloc(global_gc, strlen(filename) + 1);
    strcpy(node->filename, filename);
    node->source.content = (const unsigned char *)buffer;
    node->source.size = (size_t)length;
    node->source.filename = filename;
    node->next = loaded_files;
    loaded_files = node;

    return node->source;
}
