/*
 * symtab.c -- Symbol table with STATIC (lexical) scoping for MiniScope.
 *
 * STARTER CODE -- complete every function marked TODO.
 *
 * Concepts from Lecture 4 implemented here:
 *   - every declaration creates a BINDING between a name and its attributes
 *     (kind, type, declaring scope, line, storage category);
 *   - scopes are nested; each scope knows its STATIC PARENT;
 *   - a reference is resolved by searching the current scope, then its static
 *     parent, then the static ancestors up to the global scope;
 *   - an inner declaration HIDES an outer one with the same name;
 *   - the REFERENCING ENVIRONMENT of a statement = every visible (non-hidden)
 *     name in the current scope and its static ancestors.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "todo.h"
#include "symtab.h"

static char *str_copy(const char *s) {
    char *out = malloc(strlen(s) + 1);
    strcpy(out, s);
    return out;
}

Symbol *symbol_new(const char *name, const char *kind, const char *type, int line, Scope *scope,
                   const char **params, int nparams) {
    Symbol *s = calloc(1, sizeof(Symbol));
    s->name = str_copy(name);
    s->kind = kind;
    s->type = type;
    s->line = line;
    s->scope = scope;
    s->nparams = nparams;
    for (int k = 0; k < nparams; k++) s->params[k] = params[k];
    return s;
}

void symbol_free(Symbol *s) {
    if (s == NULL) return;
    free(s->name);
    free(s);
}

const char *symbol_storage(const Symbol *s) {
    if (strcmp(s->kind, "func") == 0) return "-";
    return s->scope->level == 0 ? "static" : "stack-dynamic";
}

void symbol_describe(const Symbol *s, char *buf, size_t n) {
    if (strcmp(s->kind, "func") == 0) {
        size_t used = (size_t)snprintf(buf, n, "func %s(", s->name);
        for (int k = 0; k < s->nparams && used < n; k++)
            used += (size_t)snprintf(buf + used, n - used, "%s%s", k ? ", " : "", s->params[k]);
        if (used < n) snprintf(buf + used, n - used, ") : %s", s->type);
    } else {
        snprintf(buf, n, "%s %s : %s", s->kind, s->name, s->type);
    }
}

void scope_label(const Scope *sc, char *buf, size_t n) {
    snprintf(buf, n, "'%s' (level %d)", sc->name, sc->level);
}

/* -- scope management ------------------------------------------------------ */
Scope *st_open_scope(SymbolTable *st, const char *name) {
    Scope *sc = calloc(1, sizeof(Scope));
    sc->name = str_copy(name);
    sc->parent = st->current;
    sc->level = (sc->parent == NULL) ? 0 : sc->parent->level + 1;
    st->current = sc;
    return sc;
}

void st_close_scope(SymbolTable *st) {
    Scope *sc = st->current;
    st->current = sc->parent;
    for (int k = 0; k < sc->nsymbols; k++) symbol_free(sc->symbols[k]);
    free(sc->symbols);
    free(sc->name);
    free(sc);
}

Symbol *st_lookup_from(Scope *scope, const char *name);

/* -- declarations ------------------------------------------------------------ */
int st_declare(SymbolTable *st, Symbol *sym, Symbol **previous, Symbol **hidden) {
    /* TODO: Bind the symbol's name in the CURRENT scope.
     *       - if the name is already declared in this same scope: failure, give back the previous symbol;
     *       - otherwise add it and give back the outer declaration (found in a static ancestor)
     *         that the new one hides, or nothing.
     */
    (void)st;
    (void)sym;
    (void)previous;
    (void)hidden;
    // TODO("st_declare");

    for (int i = 0; i < st->current->nsymbols; i++){
        if (strcmp(sym->name, st->current->symbols[i]->name) == 0){
            *previous = st->current->symbols[i];
            return 0;
        }
    }

    *hidden = st_lookup_from(st->current->parent, sym->name);

    if (st->current->cap == st->current->nsymbols){
        st->current->symbols = realloc(st->current->symbols, sizeof(Symbol) * (1 + st->current->nsymbols));
        st->current->cap++;
    }

    st->current->symbols[st->current->nsymbols++] = sym;

    return 1;
}

/* -- lookup (static scoping) ------------------------------------------------- */
Symbol *st_lookup_from(Scope *scope, const char *name) {
    /* TODO: STATIC scoping: search the given scope, then its static parent, then the static
     *       ancestors, up to the global scope. Return the symbol found, or nothing.
     */
    (void)scope;
    (void)name;
    // TODO("st_lookup_from");
    
    while (scope != NULL){
        for (int s = 0; s < scope->nsymbols; s++){
            if (strcmp(scope->symbols[s]->name, name) == 0){
                return scope->symbols[s];
            }
        }
        
        scope = scope->parent;
    }

    return NULL;
}

Symbol *st_lookup(SymbolTable *st, const char *name) {
    return st_lookup_from(st->current, name);
}

/* -- referencing environment ------------------------------------------------- */
void st_referencing_environment(SymbolTable *st, Symbol ***visible, int *nvisible,
                                Symbol ***hidden, int *nhidden) {
    /* TODO: Visible and hidden symbols, innermost scope first, in declaration order inside each
     *       scope. A symbol is hidden when a scope closer to the current one declares the same name.
     */
    (void)st;
    (void)visible;
    (void)nvisible;
    (void)hidden;
    (void)nhidden;
    TODO("st_referencing_environment");
}
