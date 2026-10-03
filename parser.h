/*
 * parser.h -- Recursive-descent parser + static-scope checker for MiniScope (CSI3120A Assignment 1).
 */
#ifndef PARSER_H
#define PARSER_H

#include <setjmp.h>
#include "lexer.h"
#include "symtab.h"

#define MAX_NESTING 256

typedef struct {
    Token *tokens;
    int ntokens;
    int i;                              /* index of the current token */
    int trace;                          /* print Enter/Exit lines? */
    int depth;                          /* trace indentation */
    SymbolTable st;
    Symbol *func_stack[MAX_NESTING];    /* enclosing function symbols (for 'return') */
    int nfunc;
    Symbol **orphans;                   /* symbols that could not be declared (freed at the end) */
    int norphans, orphcap;
    int errors, warnings;
    jmp_buf on_syntax_error;            /* where syntax_error() jumps to */
    int err_line;
    char err_msg[512];
} Parser;

void parser_init(Parser *p, Token *tokens, int ntokens, int trace);

/* Parses the whole program. Returns 1 if the program is syntactically correct, 0 at the first
 * syntax error (p->err_line and p->err_msg describe it). Semantic messages are printed while parsing. */
int parse_program(Parser *p);

/* Frees everything the parser still owns (scopes left open by a syntax error, orphan symbols). */
void parser_free(Parser *p);

#endif
