/*
 * lexer.h -- Lexical analyzer for the MiniScope language (CSI3120A Assignment 1).
 */
#ifndef LEXER_H
#define LEXER_H

#define MAX_IDENT_LEN 31   /* like C99 external names (Lecture 4, "Names") */

/* A token: its code (category), the lexeme that matched, and the line. */
typedef struct {
    const char *kind;      /* token code, e.g. "KW_VAR", "IDENT" (static string) */
    char *lexeme;          /* heap-allocated copy of the lexeme */
    int line;
} Token;

/* A lexical error: line and message. */
typedef struct {
    int line;
    char *message;         /* heap-allocated */
} LexError;

/* Result of tokenize(): the token list (ending with EOF) and the lexical errors. */
typedef struct {
    Token *tokens;
    int ntokens, tokcap;
    LexError *errors;
    int nerrors, errcap;
} LexResult;

/* Tokenizes the whole text. The caller must call free_lex_result(). */
LexResult tokenize(const char *text);
void free_lex_result(LexResult *r);

#endif
