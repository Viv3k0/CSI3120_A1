/*
 * symtab.h -- Symbol table with STATIC (lexical) scoping for MiniScope (CSI3120A Assignment 1).
 */
#ifndef SYMTAB_H
#define SYMTAB_H

#include <stddef.h>

#define MAX_PARAMS 64

typedef struct Scope Scope;

/* One binding: a name and its attributes. */
typedef struct Symbol {
    char *name;                        /* heap-allocated copy */
    const char *kind;                  /* "var", "const", "param" or "func" */
    const char *type;                  /* "int", "float", "bool", "string" (return type for funcs, may be "void") */
    int line;                          /* line of the declaration */
    Scope *scope;                      /* scope that declares the name */
    const char *params[MAX_PARAMS];    /* parameter types (functions only) */
    int nparams;
} Symbol;

/* A scope: a name, a nesting level, its bindings (in declaration order) and its STATIC PARENT. */
struct Scope {
    char *name;
    Scope *parent;                     /* static parent */
    int level;
    Symbol **symbols;                  /* insertion-ordered */
    int nsymbols, cap;
};

typedef struct {
    Scope *current;
} SymbolTable;

Symbol *symbol_new(const char *name, const char *kind, const char *type, int line, Scope *scope,
                   const char **params, int nparams);
void symbol_free(Symbol *s);
const char *symbol_storage(const Symbol *s);              /* storage category (Lecture 4, Part 5) */
void symbol_describe(const Symbol *s, char *buf, size_t n);
void scope_label(const Scope *sc, char *buf, size_t n);  /* 'name' (level k) */

/* -- scope management -- */
Scope *st_open_scope(SymbolTable *st, const char *name);
void st_close_scope(SymbolTable *st);                    /* also frees the scope and its symbols */

/* -- declarations --
 * Binds sym->name in the CURRENT scope. Returns 1 on success and sets *hidden to the outer
 * declaration that the new one hides (or NULL). Returns 0 if the name is already declared in
 * the same scope: *previous is set and sym is NOT stored (the caller still owns it). */
int st_declare(SymbolTable *st, Symbol *sym, Symbol **previous, Symbol **hidden);

/* -- lookup (static scoping) -- */
Symbol *st_lookup_from(Scope *scope, const char *name);
Symbol *st_lookup(SymbolTable *st, const char *name);

/* -- referencing environment --
 * Fills two arrays allocated with malloc (the caller frees them): the visible and the hidden
 * symbols, innermost scope first. */
void st_referencing_environment(SymbolTable *st, Symbol ***visible, int *nvisible,
                                Symbol ***hidden, int *nhidden);

#endif
