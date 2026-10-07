/*
 * parser.c -- Recursive-descent parser + static-scope checker for MiniScope.
 *
 * STARTER CODE -- complete every function marked TODO.
 *
 * One parsing function per non-terminal of the grammar (Sebesta, Ch. 4).
 * The left-recursive expression rules of the BNF were rewritten in EBNF
 * (iteration instead of left recursion) before being coded, e.g.
 *
 *     <add_expr> -> <term> { ( + | - ) <term> }
 *
 * The parser stops at the first syntax error (with longjmp). Semantic actions
 * (declarations, name resolution, type checks) are executed while parsing, in a
 * single pass, so a name is only visible AFTER its declaration.
 *
 * Types are represented by the strings "int", "float", "bool", "string" and
 * "void"; NULL means "unknown" (an error has already been reported).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "todo.h"
#include "parser.h"

#define MSG 1024   /* size of a message buffer */
#define LBL 200    /* size of a label, description or lexeme buffer */

/* -- small helpers on types -------------------------------------------------- */
static int same(const char *a, const char *b) {
    return a != NULL && b != NULL && strcmp(a, b) == 0;
}

static int is_numeric(const char *t) {
    return same(t, "int") || same(t, "float");
}

/* Can a value of type source be stored in a variable of type target? */
static int assignable(const char *target, const char *source) {
    if (target == NULL || source == NULL) return 1;      /* unknown type: error already reported */
    return same(target, source) || (same(target, "float") && same(source, "int"));
}

static const char *type_of_token(const char *kind) {
    if (strcmp(kind, "KW_INT") == 0) return "int";
    if (strcmp(kind, "KW_FLOAT") == 0) return "float";
    if (strcmp(kind, "KW_BOOL") == 0) return "bool";
    if (strcmp(kind, "KW_STRING") == 0) return "string";
    return NULL;
}

/* Human-readable spelling of token codes, for error messages */
static const char *spelling(const char *kind) {
    static const char *table[][2] = {
        {"SEMICOLON", "';'"}, {"COLON", "':'"}, {"COMMA", "','"}, {"LEFT_PAREN", "'('"},
        {"RIGHT_PAREN", "')'"}, {"LEFT_BRACE", "'{'"}, {"RIGHT_BRACE", "'}'"},
        {"ASSIGN_OP", "'='"}, {"IDENT", "identifier"}, {"KW_PROGRAM", "'program'"},
        {"EOF", "end of file"}, {NULL, NULL}};
    for (int k = 0; table[k][0] != NULL; k++)
        if (strcmp(table[k][0], kind) == 0) return table[k][1];
    return kind;
}

/* ============================================================================
 * Token helpers
 * ============================================================================ */
static Token *tok(Parser *p) { return &p->tokens[p->i]; }

static int at(Parser *p, const char *kind) { return strcmp(tok(p)->kind, kind) == 0; }

static Token *advance(Parser *p) {
    Token *t = tok(p);
    if (strcmp(t->kind, "EOF") != 0) p->i++;
    return t;
}

static void syntax_error(Parser *p, const char *expected) {
    char found[LBL];
    if (at(p, "EOF")) snprintf(found, sizeof found, "end of file");
    else snprintf(found, sizeof found, "'%s'", tok(p)->lexeme);
    p->err_line = tok(p)->line;
    snprintf(p->err_msg, sizeof p->err_msg, "expected %s but found %s", expected, found);
    longjmp(p->on_syntax_error, 1);
}

static Token *expect_what(Parser *p, const char *kind, const char *what) {
    if (!at(p, kind)) syntax_error(p, what);
    return advance(p);
}

static Token *expect(Parser *p, const char *kind) {
    return expect_what(p, kind, spelling(kind));
}

/* ============================================================================
 * Tracing and diagnostics
 * ============================================================================ */
static void enter(Parser *p, const char *nt) {
    if (p->trace) printf("%*sEnter <%s>\n", 2 * p->depth, "", nt);
    p->depth++;
}

static void leave(Parser *p, const char *nt) {
    p->depth--;
    if (p->trace) printf("%*sExit <%s>\n", 2 * p->depth, "", nt);
}

static void report(const char *tag, int line, const char *msg) {
    printf("[%s] line %d: %s\n", tag, line, msg);
}

static void sem_error(Parser *p, int line, const char *msg) {
    p->errors++;
    report("error", line, msg);
}

static void warn(Parser *p, int line, const char *msg) {
    p->warnings++;
    report("warn", line, msg);
}

/* ============================================================================
 * Semantic helpers
 * ============================================================================ */
static void add_orphan(Parser *p, Symbol *s) {
    if (p->norphans == p->orphcap) {
        p->orphcap = p->orphcap ? 2 * p->orphcap : 8;
        p->orphans = realloc(p->orphans, p->orphcap * sizeof(Symbol *));
    }
    p->orphans[p->norphans++] = s;
}

/* Declares sym in the current scope (the symbol table takes ownership of it). */
static void declare(Parser *p, Symbol *sym) {
    Symbol *previous, *hidden;
    char msg[MSG], label[LBL], desc[LBL];
    if (!st_declare(&p->st, sym, &previous, &hidden)) {
        scope_label(p->st.current, label, sizeof label);
        snprintf(msg, sizeof msg, "'%s' is already declared in scope %s at line %d",
                 sym->name, label, previous->line);
        sem_error(p, sym->line, msg);
        add_orphan(p, sym);
        return;
    }
    symbol_describe(sym, desc, sizeof desc);
    scope_label(sym->scope, label, sizeof label);
    snprintf(msg, sizeof msg, "%s in scope %s [%s]", desc, label, symbol_storage(sym));
    report("decl", sym->line, msg);
    if (hidden != NULL) {
        scope_label(hidden->scope, label, sizeof label);
        snprintf(msg, sizeof msg, "'%s' hides %s '%s' declared at line %d in scope %s",
                 sym->name, hidden->kind, hidden->name, hidden->line, label);
        warn(p, sym->line, msg);
    }
}

static Symbol *resolve(Parser *p, const char *name, int line) {
    char msg[MSG], label[LBL], desc[LBL];
    Symbol *sym = st_lookup(&p->st, name);
    if (sym == NULL) {
        scope_label(p->st.current, label, sizeof label);
        snprintf(msg, sizeof msg, "'%s' is not declared (no binding visible in scope %s)", name, label);
        sem_error(p, line, msg);
    } else {
        symbol_describe(sym, desc, sizeof desc);
        scope_label(sym->scope, label, sizeof label);
        snprintf(msg, sizeof msg, "'%s' -> %s, declared at line %d in scope %s", name, desc, sym->line, label);
        report("bind", line, msg);
    }
    return sym;
}

/* Forward declarations: one function per non-terminal */
static void parse_block(Parser *p, int new_scope);
static void parse_item(Parser *p);
static const char *parse_type(Parser *p);
static void parse_var_decl(Parser *p);
static void parse_const_decl(Parser *p);
static void parse_func_decl(Parser *p);
static void parse_stmt(Parser *p);
static void parse_ident_stmt(Parser *p);
static void parse_if_stmt(Parser *p);
static void parse_while_stmt(Parser *p);
static void parse_print_stmt(Parser *p);
static void parse_return_stmt(Parser *p);
static void parse_env_stmt(Parser *p);
static const char *parse_call(Parser *p, Token *name);
static const char *parse_expr(Parser *p);
static const char *parse_and_expr(Parser *p);
static const char *parse_rel_expr(Parser *p);
static const char *parse_add_expr(Parser *p);
static const char *parse_term(Parser *p);
static const char *parse_factor(Parser *p);
static const char *parse_primary(Parser *p);

/* ============================================================================
 * <program> -> program IDENT <block>
 * ============================================================================ */
void parser_init(Parser *p, Token *tokens, int ntokens, int trace) {
    memset(p, 0, sizeof *p);
    p->tokens = tokens;
    p->ntokens = ntokens;
    p->trace = trace;
}

int parse_program(Parser *p) {
    if (setjmp(p->on_syntax_error) != 0) return 0;     /* we come back here after a syntax error */
    enter(p, "program");
    expect(p, "KW_PROGRAM");
    expect_what(p, "IDENT", "program name");
    st_open_scope(&p->st, "global");
    parse_block(p, 0);
    st_close_scope(&p->st);
    if (!at(p, "EOF")) syntax_error(p, "end of file");
    leave(p, "program");
    return 1;
}

void parser_free(Parser *p) {
    while (p->st.current != NULL) st_close_scope(&p->st);
    for (int k = 0; k < p->norphans; k++) symbol_free(p->orphans[k]);
    free(p->orphans);
    p->orphans = NULL;
    p->norphans = 0;
}

/* <block> -> { <item_list> }
 * <item_list> -> <item> <item_list> | epsilon */
static void parse_block(Parser *p, int new_scope) {
    char name[64];
    enter(p, "block");
    Token *lbrace = expect(p, "LEFT_BRACE");
    if (new_scope) {
        snprintf(name, sizeof name, "block@%d", lbrace->line);
        st_open_scope(&p->st, name);
    }
    while (!at(p, "RIGHT_BRACE") && !at(p, "EOF")) parse_item(p);
    expect(p, "RIGHT_BRACE");
    if (new_scope) st_close_scope(&p->st);
    leave(p, "block");
}

/* <item> -> <decl> | <stmt> */
static void parse_item(Parser *p) {
    enter(p, "item");
    if (at(p, "KW_VAR")) parse_var_decl(p);
    else if (at(p, "KW_CONST")) parse_const_decl(p);
    else if (at(p, "KW_FUNC")) parse_func_decl(p);
    else parse_stmt(p);
    leave(p, "item");
}

/* <type> -> int | float | bool | string */
static const char *parse_type(Parser *p) {
    enter(p, "type");
    const char *t = type_of_token(tok(p)->kind);
    if (t == NULL) syntax_error(p, "a type (int, float, bool or string)");
    advance(p);
    leave(p, "type");
    return t;
}

/* <var_decl> -> var IDENT : <type> <init> ;
 * <init>     -> = <expr> | epsilon */
static void parse_var_decl(Parser *p) {
    char msg[MSG];
    enter(p, "var_decl");
    expect(p, "KW_VAR");
    Token *name = expect(p, "IDENT");
    expect(p, "COLON");
    const char *typ = parse_type(p);
    if (at(p, "ASSIGN_OP")) {
        advance(p);
        const char *etype = parse_expr(p);      /* initializer is evaluated BEFORE the name is bound */
        if (!assignable(typ, etype)) {
            snprintf(msg, sizeof msg, "type mismatch: cannot initialize '%s' (%s) with a %s value",
                     name->lexeme, typ, etype);
            sem_error(p, name->line, msg);
        }
    }
    expect(p, "SEMICOLON");
    declare(p, symbol_new(name->lexeme, "var", typ, name->line, p->st.current, NULL, 0));
    leave(p, "var_decl");
}

/* <const_decl> -> const IDENT : <type> = <expr> ; */
static void parse_const_decl(Parser *p) {
    /* TODO: Like parse_var_decl, but the initializer is mandatory and the symbol's kind is "const".
     */
    (void)p;
    // TODO("parse_const_decl");

    char msg[MSG];
    enter(p, "const_decl");
    expect(p, "KW_CONST");

    Token *name = expect(p, "IDENT");
    expect(p, "COLON");
    const char *assigned_type = parse_type(p);

    if (!at(p, "ASSIGN_OP")){
        printf("Whatttt\n");
        return;
    }

    advance(p);    
    const char *found_type = parse_expr(p);   
    expect(p, "SEMICOLON");

    declare(p, symbol_new(name->lexeme, "const", assigned_type, name->line, p->st.current, NULL, 0));
    leave(p, "const_decl");
}

/* <param> -> IDENT : <type>   (the name, line and type are returned through pointers) */
static void parse_param(Parser *p, Token **name, const char **type) {
    /* TODO: <param> -> IDENT : <type>. Give back the name, the line and the type.
     */
    (void)p;
    (void)name;
    (void)type;
    TODO("parse_param");
}

/* <func_decl>  -> func IDENT ( <params> ) : <ret_type> <block>
 * <params>     -> <param_list> | epsilon
 * <param_list> -> <param> | <param> , <param_list>
 * <ret_type>   -> <type> | void */
static void parse_func_decl(Parser *p) {
    /* TODO: Parse the header, then: (1) declare the function name in the ENCLOSING scope (so it can
     *       call itself), (2) open a scope named after the function, (3) declare the parameters there
     *       (kind "param"), (4) push the function on the function stack, parse the body WITHOUT a new
     *       scope, pop, and close the scope.
     */
    (void)p;
    (void)parse_param;
    TODO("parse_func_decl");
}

/* <stmt> -> <ident_stmt> | <if_stmt> | <while_stmt> | <print_stmt>
 *         | <return_stmt> | <env_stmt> | <block> */
static void parse_stmt(Parser *p) {
    enter(p, "stmt");
    if (at(p, "IDENT")) parse_ident_stmt(p);
    else if (at(p, "KW_IF")) parse_if_stmt(p);
    else if (at(p, "KW_WHILE")) parse_while_stmt(p);
    else if (at(p, "KW_PRINT")) parse_print_stmt(p);
    else if (at(p, "KW_RETURN")) parse_return_stmt(p);
    else if (at(p, "KW_SHOW_ENV")) parse_env_stmt(p);
    else if (at(p, "LEFT_BRACE")) parse_block(p, 1);
    else syntax_error(p, "a declaration or a statement");
    leave(p, "stmt");
}

/* <ident_stmt> -> IDENT <ident_tail> ;
 * <ident_tail> -> = <expr> | ( <args> ) */
static void parse_ident_stmt(Parser *p) {
    /* TODO: Assignment or call statement (left-factored on IDENT).
     *       Assignment: resolve the name; report an error if it is a const or a func, or if the type
     *       of the expression is not assignable to the type of the variable.
     */
    (void)p;
    TODO("parse_ident_stmt");
}

static void check_condition(Parser *p, const char *t, int line, const char *what) {
    char msg[MSG];
    if (t != NULL && !same(t, "bool")) {
        snprintf(msg, sizeof msg, "condition of '%s' must be bool, found %s", what, t);
        sem_error(p, line, msg);
    }
}

/* <if_stmt>   -> if ( <expr> ) <block> <else_part>
 * <else_part> -> else <block> | epsilon */
static void parse_if_stmt(Parser *p) {
    /* TODO: Use check_condition(...) on the type of the condition. Each block opens a new scope.
     */
    (void)p;
    (void)check_condition;
    TODO("parse_if_stmt");
}

/* <while_stmt> -> while ( <expr> ) <block> */
static void parse_while_stmt(Parser *p) {
    /* TODO: Use check_condition(...) on the type of the condition. The body opens a new scope.
     */
    (void)p;
    TODO("parse_while_stmt");
}

/* <print_stmt> -> print ( <expr> ) ; */
static void parse_print_stmt(Parser *p) {
    enter(p, "print_stmt");
    expect(p, "KW_PRINT");
    expect(p, "LEFT_PAREN");
    parse_expr(p);
    expect(p, "RIGHT_PAREN");
    expect(p, "SEMICOLON");
    leave(p, "print_stmt");
}

/* <return_stmt> -> return <ret_val> ;
 * <ret_val>     -> <expr> | epsilon */
static void parse_return_stmt(Parser *p) {
    /* TODO: Check: return outside a function; a void function returning a value; a non-void
     *       function returning nothing; a returned value of the wrong type.
     */
    (void)p;
    TODO("parse_return_stmt");
}

static void print_environment(Parser *p, int line) {
    Symbol **visible, **hidden;
    int nv, nh;
    char label[LBL];
    st_referencing_environment(&p->st, &visible, &nv, &hidden, &nh);
    scope_label(p->st.current, label, sizeof label);
    printf("----- referencing environment at line %d, scope %s -----\n", line, label);
    printf("  name      kind   type    declared in           line  storage\n");
    for (int k = 0; k < nv; k++) {
        Symbol *s = visible[k];
        scope_label(s->scope, label, sizeof label);
        printf("  %-10s%-7s%-8s%-22s%-6d%s\n", s->name, s->kind, s->type, label, s->line, symbol_storage(s));
    }
    if (nh > 0) {
        printf("  hidden: ");
        for (int k = 0; k < nh; k++) {
            scope_label(hidden[k]->scope, label, sizeof label);
            printf("%s%s (%s, line %d)", k ? ", " : "", hidden[k]->name, label, hidden[k]->line);
        }
        printf("\n");
    } else {
        printf("  hidden: none\n");
    }
    printf("------------------------------------------------------------------------\n");
    free(visible);
    free(hidden);
}

/* <env_stmt> -> show_env ; */
static void parse_env_stmt(Parser *p) {
    enter(p, "env_stmt");
    Token *kw = expect(p, "KW_SHOW_ENV");
    expect(p, "SEMICOLON");
    print_environment(p, kw->line);
    leave(p, "env_stmt");
}

/* <call> -> IDENT ( <args> )      (IDENT already consumed)
 * <args> -> <arg_list> | epsilon
 * <arg_list> -> <expr> | <expr> , <arg_list> */
static const char *parse_call(Parser *p, Token *name) {
    /* TODO: Resolve the name, parse the argument list, then check that the name is a function,
     *       the number of arguments and the type of each argument. Return the function's return
     *       type (or the unknown type).
     */
    (void)p;
    (void)name;
    TODO("parse_call");
    return NULL;
}

/* ============================================================================
 * Expressions (EBNF form of the left-recursive BNF rules)
 * ============================================================================ */
static const char *logical(Parser *p, const char *a, const char *b, Token *op) {
    char msg[MSG];
    if (a == NULL || b == NULL) return "bool";
    if (!same(a, "bool") || !same(b, "bool")) {
        snprintf(msg, sizeof msg, "operator '%s' needs bool operands, found %s and %s", op->lexeme, a, b);
        sem_error(p, op->line, msg);
    }
    return "bool";
}

static const char *arith(Parser *p, const char *a, const char *b, Token *op) {
    char msg[MSG];
    if (a == NULL || b == NULL) return NULL;
    if (same(op->kind, "ADD_OP") && same(a, "string") && same(b, "string"))
        return "string";                                        /* concatenation */
    if (same(op->kind, "MOD_OP")) {
        if (same(a, "int") && same(b, "int")) return "int";
    } else if (is_numeric(a) && is_numeric(b)) {
        return (same(a, "float") || same(b, "float")) ? "float" : "int";
    }
    snprintf(msg, sizeof msg, "operator '%s' cannot be applied to %s and %s", op->lexeme, a, b);
    sem_error(p, op->line, msg);
    return NULL;
}

/* <expr> -> <and_expr> { || <and_expr> } */
static const char *parse_expr(Parser *p) {
    /* TODO: EBNF iteration, like parse_add_expr. Use logical(...) for the type.
     */
    (void)p;
    (void)parse_and_expr;
    (void)logical;
    // TODO("parse_expr");

    enter(p, "expr");
    const char *r = parse_and_expr(p);

    while (at(p, "OR_OP")){
        Token *op = advance(p);
        r = logical(p, r, parse_and_expr(p), op);
    }
    
    leave(p, "expr");
    return r;
}

/* <and_expr> -> <rel_expr> { && <rel_expr> } */
static const char *parse_and_expr(Parser *p) {
    /* TODO: EBNF iteration, like parse_add_expr. Use logical(...) for the type.
     */
    (void)p;
    (void)parse_rel_expr;
    TODO("parse_and_expr");

    enter(p, "and_expr");
    const char *r = parse_rel_expr(p);

    while(at(p, "AND_OP")){
        Token *op = advance(p);
        r = logical(p, r, parse_rel_expr(p), op);
    }

    leave(p, "and_expr");
    return r;
}

/* <rel_expr> -> <add_expr> [ <rel_op> <add_expr> ] */
static const char *parse_rel_expr(Parser *p) {
    /* TODO: At most ONE relational operator (non-associative). The result type is bool.
     *       == and != accept two values of the same type (or two numbers);
     *       < <= > >= accept numbers only.
     */
    (void)p;
    (void)parse_add_expr;
    TODO("parse_rel_expr");
    enter(p, "rel_expr");

    const char *a = parse_add_expr(p);
    const char *b;

    if (at(p, "rel_op")){
        Token *op = advance(p);
        b = parse_add_expr(p);

        if((same(op->kind, "EQ_OP") || same(op->kind, "NEQ_OP"))){
            if (!same(a, b)){
                printf("comparing different types\n");
                return NULL;
            }
        } else if ((same(op->kind, "LT_OP") || same(op->kind, "GT_OP") || same(op->kind, "LE_OP") || same(op->kind, "GE_OP")) ){
            if (!(is_numeric(a) && is_numeric(b))){
                printf("cant order non-numeric characters\n");
                return NULL;
            }
        } else {
            printf("invalid operator\n");
            return NULL;
        }
    }

    leave(p, "rel_expr");
    return "bool";
}

/* <add_expr> -> <term> { ( + | - ) <term> } */
static const char *parse_add_expr(Parser *p) {
    enter(p, "add_expr");
    const char *t = parse_term(p);
    while (at(p, "ADD_OP") || at(p, "SUB_OP")) {
        Token *op = advance(p);
        t = arith(p, t, parse_term(p), op);
    }
    leave(p, "add_expr");
    return t;
}

/* <term> -> <factor> { ( * | / | % ) <factor> } */
static const char *parse_term(Parser *p) {
    /* TODO: EBNF iteration, like parse_add_expr. Use arith(...) for the type.
     */
    (void)p;
    (void)parse_factor;
    TODO("parse_term");

    enter(p, "term");
    const char *t = parse_factor(p);

    while (at(p, "MULT_OP") || at(p, "DIV_OP") || at(p, "MOD_OP")){
        Token *op = advance(p);
        t = arith(p, t, parse_factor(p), op);
    }
    
    leave(p, "term");
    return t;
}

/* <factor> -> ! <factor> | - <factor> | <primary> */
static const char *parse_factor(Parser *p) {
    /* TODO: '!' needs a bool operand, unary '-' a numeric one. Return the type.
     */
    (void)p;
    (void)parse_primary;
    TODO("parse_factor");
    return NULL;
}

/* <primary> -> INT_LIT | FLOAT_LIT | STRING_LIT | true | false
 *            | IDENT | IDENT ( <args> ) | ( <expr> ) */
static const char *parse_primary(Parser *p) {
    /* TODO: Literals, true/false, IDENT (resolve it: it must not be a func), IDENT ( <args> )
     *       (a call; a void function cannot be used in an expression), and ( <expr> ).
     *       Anything else is a syntax error: "an expression" expected. Return the type.
     */
    (void)p;
    (void)parse_call;
    (void)resolve;
    TODO("parse_primary");
    return NULL;
}
