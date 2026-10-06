/*
 * lexer.c -- Lexical analyzer for the MiniScope language (CSI3120A Assignment 1).
 *
 * STARTER CODE -- complete every function marked TODO.
 *
 * The lexer is a hand-written pattern matcher in the style of Sebesta, Ch. 4:
 * it looks at the class of the next character (letter, digit, quote, operator
 * symbol, ...) and collects the longest lexeme that matches the corresponding
 * token pattern. Reserved words are recognised by first scanning an
 * identifier-shaped lexeme and then looking it up in a reserved-word table.
 *
 * Lexical errors are reported and the offending lexeme is skipped, so a single
 * run can report several errors.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "todo.h"
#include "lexer.h"

/* Token codes */
static const char *RESERVED_WORDS[][2] = {
    {"program", "KW_PROGRAM"}, {"var", "KW_VAR"}, {"const", "KW_CONST"},
    {"func", "KW_FUNC"}, {"int", "KW_INT"}, {"float", "KW_FLOAT"},
    {"bool", "KW_BOOL"}, {"string", "KW_STRING"}, {"void", "KW_VOID"},
    {"if", "KW_IF"}, {"else", "KW_ELSE"}, {"while", "KW_WHILE"},
    {"print", "KW_PRINT"}, {"return", "KW_RETURN"}, {"true", "KW_TRUE"},
    {"false", "KW_FALSE"}, {"show_env", "KW_SHOW_ENV"}, {NULL, NULL}
};

/* Two-character operators are tried before one-character ones (longest match). */
static const char *TWO_CHAR_OPS[][2] = {
    {"==", "EQ_OP"}, {"!=", "NEQ_OP"}, {"<=", "LE_OP"}, {">=", "GE_OP"},
    {"&&", "AND_OP"}, {"||", "OR_OP"}, {NULL, NULL}
};
static const char *ONE_CHAR_OPS[][2] = {
    {"=", "ASSIGN_OP"}, {"+", "ADD_OP"}, {"-", "SUB_OP"}, {"*", "MULT_OP"},
    {"/", "DIV_OP"}, {"%", "MOD_OP"}, {"<", "LT_OP"}, {">", "GT_OP"}, {"!", "NOT_OP"},
    {"(", "LEFT_PAREN"}, {")", "RIGHT_PAREN"}, {"{", "LEFT_BRACE"},
    {"}", "RIGHT_BRACE"}, {";", "SEMICOLON"}, {":", "COLON"}, {",", "COMMA"}, {NULL, NULL}
};

typedef struct {
    const char *text;
    int len;
    int pos;
    int line;
    LexResult r;
} Lexer;

/* -- character helpers ---------------------------------------------------- */
/* Character k positions ahead, or '\0' at the end of the input. */
static char peek(Lexer *lx, int k) {
    int i = lx->pos + k;
    return i < lx->len ? lx->text[i] : '\0';
}

static int at_end(Lexer *lx) { return lx->pos >= lx->len; }

static char advance(Lexer *lx) {
    char ch = lx->text[lx->pos++];
    if (ch == '\n') lx->line++;
    return ch;
}

static int is_letter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static int is_digit(char c) { return c >= '0' && c <= '9'; }
static int is_ident_char(char c) { return is_letter(c) || is_digit(c) || c == '_'; }

/* Heap-allocated copy of a string (like POSIX strdup, which is not standard C). */
static char *str_copy(const char *s) {
    char *out = malloc(strlen(s) + 1);
    strcpy(out, s);
    return out;
}

static char *substr(const char *s, int start, int end) {
    char *out = malloc(end - start + 1);
    memcpy(out, s + start, end - start);
    out[end - start] = '\0';
    return out;
}

static const char *lookup(const char *table[][2], const char *key) {
    for (int k = 0; table[k][0] != NULL; k++){
        if (strcmp(table[k][0], key) == 0) return table[k][1];

    }
    return NULL;
}

static void add_token(Lexer *lx, const char *kind, char *lexeme, int line) {
    LexResult *r = &lx->r;
    if (r->ntokens == r->tokcap) {
        r->tokcap = r->tokcap ? 2 * r->tokcap : 64;
        r->tokens = realloc(r->tokens, r->tokcap * sizeof(Token));
    }
    r->tokens[r->ntokens].kind = kind;
    r->tokens[r->ntokens].lexeme = lexeme;
    r->tokens[r->ntokens].line = line;
    r->ntokens++;
}

static void error(Lexer *lx, int line, const char *msg) {
    LexResult *r = &lx->r;
    if (r->nerrors == r->errcap) {
        r->errcap = r->errcap ? 2 * r->errcap : 8;
        r->errors = realloc(r->errors, r->errcap * sizeof(LexError));
    }
    r->errors[r->nerrors].line = line;
    r->errors[r->nerrors].message = str_copy(msg);
    r->nerrors++;
}

/* -- token recognizers ------------------------------------------------------ */
static void skip_blanks_and_comments(Lexer *lx) {
    /* TODO: Skip white space, line comments and block comments.
     *       - blanks, tabs and newlines are skipped (advance() keeps the line counter up to date);
     *       - a line comment starts with two slashes and ends at the end of the line;
     *       - a block comment starts with a slash followed by a star and ends at the next star
     *         followed by a slash; if the file ends first, report
     *         "unclosed block comment (opened here)" at the line where it was opened.
     */
    (void)lx;
    (void)peek;
    (void)advance;
    (void)at_end;
    (void)error;    
    char current = peek(lx, 0);

    while (!at_end(lx)){

        if (current <= 32 || current == '\n' || current == '\t'){
            advance(lx);
        } else if (current == '/' && peek(lx, 1) == '/'){
            current = advance(lx);
            while (current != '\n'){
                current = advance(lx);
            }
        } else if (current == '/' && peek(lx, 1) == '*'){
            //skip /*
            int block_open = lx->line;
            advance(lx);
            advance(lx);

            //skip block comment
            while (!at_end(lx)){
                current = advance(lx);
                //if we hit end of block comment break
                if (current == '*' && peek(lx, 0) == '/'){
                    current = advance(lx);
                    break;
                }
            }

            if (at_end(lx)){
                error(lx, block_open, "unclosed block comment (opened here)");
            }
        } else {
            break; //non-blank/comment break out of while
        }

        current = peek(lx, 0);
    }

    printf("skipped all blanks/comments, currently at line %d \n", lx->line);
}

static void lex_identifier(Lexer *lx) {
    /* TODO: Identifier or reserved word: (letter | _) { letter | digit | _ }
     *       - collect the longest identifier-shaped lexeme;
     *       - if it is a reserved word, add the reserved-word token;
     *       - else if it is longer than MAX_IDENT_LEN characters, report an error and add nothing;
     *       - else add an IDENT token.
     */
    (void)lx;
    (void)substr;
    (void)lookup;
    (void)is_ident_char;
    (void)add_token;
    (void)RESERVED_WORDS;

    char curr = peek(lx, 0);
    int start = lx->pos;

    while(!at_end(lx) && is_ident_char(curr)){
        advance(lx);        
        curr = peek(lx, 0);
    }
    
    char* lexeme = substr(lx->text, start, lx->pos);
    const char *word = lookup(RESERVED_WORDS, lexeme);

    if(word != NULL){
        add_token(lx, word, lexeme, lx->line);
        return;
    }
    
    if(strlen(lexeme) > MAX_IDENT_LEN){
        char *err_msg = malloc(sizeof(char) * (46 + strlen(lexeme)));
        sprintf(err_msg, "identifier \'%s\' is longer than 31 characters", lexeme);
        err_msg[45 + strlen(lexeme)] = '\0';

        error(lx, lx->line, err_msg);
        return;
    }
    add_token(lx, "IDENT", lexeme, lx->line);
    
}

static void lex_number(Lexer *lx) {
    /* TODO: INT_LIT: digit {digit}     FLOAT_LIT: digit {digit} . digit {digit}
     *       - '12.' (no digit after the point) -> error "malformed float literal";
     *       - a number immediately followed by a letter or '_' (e.g. 2ndPlace) -> error
     *         "illegal identifier ... (identifiers cannot start with a digit)"; skip the whole bad lexeme;
     *       - otherwise add INT_LIT or FLOAT_LIT.
     */
    (void)lx;
    
    int start = lx->pos;
    int is_float = 0;
    int skip_ident = 0;

    while (is_digit(peek(lx, 0))){
        advance(lx);

        if (peek(lx, 0) == '.' && !is_float){
            is_float = 1;
            advance(lx);
            if (!is_digit(peek(lx, 0))){
                skip_ident = 1;
                error(lx, lx->line, "malformed float literal");
            }
        } else if (peek(lx, 0) == '_' || is_letter(peek(lx, 0))){
            error(lx, lx->line, "illegal identifier ... (identifiers cannot start with a digit)");
            skip_ident = 1;
        }
        
        //skip the whole identifier and quit
        if (skip_ident){
            while (is_ident_char(peek(lx, 0))){
                advance(lx);
            }
            return;
        }
    }

    char *num = substr(lx->text, start, lx->pos);

    if (is_float){
        add_token(lx, "FLOAT_LIT", num, lx->line);
    } else {
        add_token(lx, "INT_LIT", num, lx->line);
    }

    printf("NUMBER: %s\n", num);
}

static void lex_string(Lexer *lx) {
    /* TODO: STRING_LIT: " { any character except " and newline } "
     *       The lexeme includes the quotes. If a newline or the end of the file comes before the
     *       closing quote, report "unterminated string literal".
     */
    (void)lx;

    //skip first "
    int start = lx->pos;
    advance(lx);

    char c = peek(lx, 0);

    //skip through characters until we find the ending "
    while (!at_end(lx) && c != '"'){
        //cant have newline in the middle of a string
        if (c == '\n'){
            char *unterminated_str = substr(lx->text, start, lx->pos);
            char *err_msg = malloc(sizeof(char) * (32 + strlen(unterminated_str)));
            err_msg[31 + strlen(unterminated_str)] = '\0';
            sprintf(err_msg, "unterminated string literal %s", unterminated_str);
            
            error(lx, lx->line, err_msg);
            free(unterminated_str);
            return;
        }
        advance(lx);
        c = peek(lx, 0);
    }
    //skip ending "
    advance(lx);

    char *str = substr(lx->text, start, lx->pos);
    add_token(lx, "STRING_LIT", str, lx->line);


    printf("Found string: %s\n", str);
}

static void lex_operator(Lexer *lx) {
    /* TODO: Operators and punctuation: try the two-character operators first (longest match),
     *       then the one-character ones. Anything else is an "illegal character" error
     *       (e.g. '@', a single '&').
     */
    (void)lx;
    (void)TWO_CHAR_OPS;
    (void)ONE_CHAR_OPS;

    char *op = substr(lx->text, lx->pos, lx->pos+2);
    const char *two_char_op = lookup(TWO_CHAR_OPS, op);

    if (two_char_op != NULL){
        add_token(lx, two_char_op, op, lx->line);
        //skip the opertator characters
        advance(lx);
        advance(lx);
        return;
    }

    //opertion wasn't a 2 character operatioon, free the pointer for the string
    free(op);
    op = substr(lx->text, lx->pos, lx->pos+1);
    const char *one_char_op = lookup(ONE_CHAR_OPS, op);

    if (one_char_op != NULL){
        add_token(lx, one_char_op, op, lx->line);
    } else {
        char *err_msg = malloc(sizeof(char) * 24);
        sprintf(err_msg, "illegal character \'%c\'", *op);
        err_msg[23] = '\0';

        error(lx, lx->line, err_msg);
        free(op); //dont need the character anymore
    }

    advance(lx);
}

/* -- main loop -------------------------------------------------------------- */
LexResult tokenize(const char *text) {
    Lexer lx = {text, (int)strlen(text), 0, 1, {0}};
    while (1) {
        skip_blanks_and_comments(&lx);
        if (at_end(&lx)) {
            add_token(&lx, "EOF", str_copy("EOF"), lx.line);
            return lx.r;
        }
        char ch = peek(&lx, 0);
        if (is_letter(ch) || ch == '_') lex_identifier(&lx);
        else if (is_digit(ch)) lex_number(&lx);
        else if (ch == '"') lex_string(&lx);
        else lex_operator(&lx);
    }
}

void free_lex_result(LexResult *r) {
    for (int k = 0; k < r->ntokens; k++) free(r->tokens[k].lexeme);
    for (int k = 0; k < r->nerrors; k++) free(r->errors[k].message);
    free(r->tokens);
    free(r->errors);
    r->tokens = NULL; r->errors = NULL;
    r->ntokens = r->nerrors = 0;
}
