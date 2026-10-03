/*
 * miniscope.c -- Driver for the MiniScope front end (CSI3120A Assignment 1).
 *
 * Usage:
 *     ./miniscope <file.ms> --tokens     # Part A: print the token stream only
 *     ./miniscope <file.ms>              # Part B: lex + parse + scope/type check
 *     ./miniscope <file.ms> --trace      # Part B with the Enter/Exit parse trace
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

/* Reads a whole file into a heap-allocated, '\0'-terminated string (NULL on failure). */
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *text = malloc(size + 1);
    size_t got = fread(text, 1, size, f);
    text[got] = '\0';
    fclose(f);
    return text;
}

static void print_tokens(const LexResult *r) {
    for (int k = 0; k < r->ntokens; k++)
        printf("Line %3d: Next token is: %-12s Next lexeme is: %s\n",
               r->tokens[k].line, r->tokens[k].kind, r->tokens[k].lexeme);
}

static void print_lex_errors(const LexResult *r) {
    for (int k = 0; k < r->nerrors; k++)
        printf("Lexical error (line %d): %s\n", r->errors[k].line, r->errors[k].message);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file.ms> [--tokens | --trace]\n", argv[0]);
        return 2;
    }
    const char *path = argv[1];
    int tokens_only = 0, trace = 0;
    for (int k = 2; k < argc; k++) {
        if (strcmp(argv[k], "--tokens") == 0) tokens_only = 1;
        if (strcmp(argv[k], "--trace") == 0) trace = 1;
    }
    char *text = read_file(path);
    if (text == NULL) {
        printf("cannot open '%s'\n", path);
        return 2;
    }

    printf("=== MiniScope: %s ===\n", path);
    LexResult r = tokenize(text);
    int status = 0;

    if (tokens_only) {
        print_tokens(&r);
        print_lex_errors(&r);
        printf("--- %d token(s), %d lexical error(s) ---\n", r.ntokens, r.nerrors);
        status = r.nerrors ? 1 : 0;
    } else if (r.nerrors > 0) {
        print_lex_errors(&r);
        printf("--- %d lexical error(s); parsing not attempted ---\n", r.nerrors);
        status = 1;
    } else {
        Parser p;
        parser_init(&p, r.tokens, r.ntokens, trace);
        if (!parse_program(&p)) {
            printf("Syntax error (line %d): %s\n", p.err_line, p.err_msg);
            printf("--- parsing stopped at the first syntax error ---\n");
            status = 1;
        } else {
            printf("--- parse successful; %d semantic error(s), %d warning(s) ---\n", p.errors, p.warnings);
            status = p.errors ? 1 : 0;
        }
        parser_free(&p);
    }
    free_lex_result(&r);
    free(text);
    return status;
}
