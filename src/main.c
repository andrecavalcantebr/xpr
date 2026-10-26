/*
   FILE: main.c
   DESC: Entry point for the XPR interpreter, handling CLI arguments and file loading.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

#include "xpr.h"
#include "parser.h"
#include "loader.h"
#include "tgc.h"
#include "exceptions.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

tgc_t global_gc;

typedef enum {
    EXEC_NOERROR,
    EXEC_ARGERROR,
    EXEC_FILENOTFOUND,
    EXEC_PARSERERROR
} ExecutorError;

typedef struct {
    bool help;
    bool lex_dump;
    bool expression_mode;
    char *target;
} Config;

void help(const char *program) {
    const char *msg =
        "%s version %d.%d.%d\n"
        "USE: \n"
        "    %s <filename.x>       :  executes the script\n"
        "    %s -x \"expression\"  :  executes the expression directly\n"
        "    %s --lex-dump <file>   :  dumps the tokens of a file\n"
        "    %s -h | --help        :  this help\n"
        "\n";
    printf(msg,
        program, VERSION_MAJOR, VERSION_MINOR, VERSION_BUGS,
        program,
        program,
        program,
        program
    );
}

// Dummy: Modo interativo/stdin
void handle_interactive(void) {
    (void)printf("Interactive mode not implemented yet.\n");
}

// Trata execução de expressão direta (-x)
void handle_expression(const char *expr) {
    Source src;
    src.content = (const unsigned char *)expr;
    src.size = strlen(expr);
    src.filename = "NONAME";
    parser_dump_lexer(src);
}

// Trata execução de arquivo
void handle_file(const char *filename) {
    Source src = loader_load_file(filename);
    if (src.content == NULL) {
        throw(EXEC_FILENOTFOUND);
    }
    parser_dump_lexer(src);
}

Config parse_args(int argc, char *argv[]) {
    Config cfg = {0};
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help(argv[0]);
            cfg.help = true;
        } else if (strcmp(argv[i], "--lex-dump") == 0) {
            cfg.lex_dump = true;
        } else if (strcmp(argv[i], "-x") == 0) {
            cfg.expression_mode = true;
            if (i + 1 < argc) {
                cfg.target = argv[++i];
            } else {
                fprintf(stderr, "Error: Expression missing after -x\n");
                throw(EXEC_ARGERROR);
            }
        } else if (argv[i][0] != '-') {
            cfg.target = argv[i];
        } else {
            fprintf(stderr, "Argument '%s' not recognized!\n", argv[i]);
            throw(EXEC_ARGERROR);
        }
    }
    return cfg;
}

int main(int argc, char *argv[]) {
    int stack_bottom;
    tgc_start(&global_gc, &stack_bottom);

    // Inicializa o loader com o GC global
    loader_init(&global_gc);

    ExecutorError result = EXEC_NOERROR;

    volatile Config cfg = {0};
    try {
        cfg = parse_args(argc, argv);
        if (cfg.expression_mode && cfg.target) {
            handle_expression(cfg.target);
        } else if (cfg.target) {
            handle_file(cfg.target);
        } else if (argc == 1) {
            handle_interactive();
        } else {
            if(!cfg.help)
                throw(EXEC_ARGERROR);
        }
    } catch(EXEC_FILENOTFOUND) {
        fprintf(stderr, "File %s not found\n", cfg.target);
        result = EXEC_FILENOTFOUND;
    } catch(EXEC_ARGERROR) {
        result = EXEC_ARGERROR;
    }

    tgc_stop(&global_gc);
    return result;
}
