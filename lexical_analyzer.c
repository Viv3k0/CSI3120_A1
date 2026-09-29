#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

/* Global declarations */
/* Variables */
int charClass;
char lexeme[100];
char error[100];
char nextChar;
int lexLen;
int token;
int nextToken;
FILE *in_fp; /* Removed redundant *fopen() declaration */
char tokenType[100];

/* Function declarations */
void addChar();
void getChar();
void getNonBlank();
int lex();
bool isComment();

/* Character classes */
#define LETTER 0
#define DIGIT 1
#define UNDERSCORE 2
#define UNKNOWN 99

/* Token codes */
#define INT_LIT 10
#define FLOAT_LIT 11
#define IDENT 12
#define STR_LIT 13
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define LEFT_PAREN 25
#define RIGHT_PAREN 26
#define LEFT_BRACE 27
#define RIGHT_BRACE 28
#define SEMICOLON 29
#define LESS_THAN 30
#define GREATER_THAN 31
#define EQUALS 32
#define NOT_EQUALS 33
#define AND_OP 34
#define OR_OP 35
#define IF 36
#define ELSE 37
#define FOR 38
#define WHILE 39
#define COMMENT 40
#define QUESTION_MARK 41
#define COLON 42

#define KW_PROGRAM 43
#define KW_CONST 44
#define KW_FUNC 45
#define KW_INT 46
#define KW_FLOAT 47
#define KW_BOOL 48
#define KW_STRING 49
#define KW_VOID 50
#define KW_IF 51
#define KW_ELSE 52

/******************/
/* main driver */
int main() {
    if ((in_fp = fopen("input.txt", "r")) == NULL)
        printf("ERROR - cannot open input.txt\n");
    else {
        getChar();
        do {
            lex();
        } while (nextToken != EOF);
        fclose(in_fp);
    }
    return 0;
}

/*******************/
/* lookup - a function to search for operators and keywords and return the token */
int lookup(char ch) {
    switch (ch) {
        case '(':
            addChar();
            nextToken = LEFT_PAREN;
            strcpy(tokenType, "a delimiter");
            break;
        case ')':
            addChar();
            nextToken = RIGHT_PAREN;
            strcpy(tokenType, "a delimiter");
            break;
        case '{':
            addChar();
            nextToken = LEFT_BRACE;
            strcpy(tokenType, "a delimiter");
            break;
        case '}':
            addChar();
            nextToken = RIGHT_BRACE;
            strcpy(tokenType, "a delimiter");
            break;
        case '+':
            addChar();
            nextToken = ADD_OP;
            strcpy(tokenType, "an arithmetic operator");
            break;
        case '-':
            addChar();
            nextToken = SUB_OP;
            strcpy(tokenType, "an arithmetic operator");
            break;
        case '*':
            addChar();
            nextToken = MULT_OP;
            strcpy(tokenType, "an arithmetic operator");
            break;
        case '/':
            addChar();
            getChar();
            nextToken = isComment() ? COMMENT : DIV_OP;
            break;
        case '=':
            addChar();
            getChar();
            if (nextChar == '=') {
                addChar();
                nextToken = EQUALS;
                strcpy(tokenType, "an equality sign");
            } else {
                nextToken = ASSIGN_OP;
                strcpy(tokenType, "a comparison operator");
            }
            break;
        case ';':
            addChar();
            nextToken = SEMICOLON;
            strcpy(tokenType, "a delimiter");
            break;
        case '<':
            addChar();
            getChar();
            if (nextChar == '=') {
                addChar();
                nextToken = LESS_THAN;
                strcpy(tokenType, "a comparison operator");
            } else {
                nextToken = LESS_THAN;
                strcpy(tokenType, "a comparison operator");
            }
            break;
        case '>':
            addChar();
            getChar();
            if (nextChar == '=') {
                addChar();
                nextToken = GREATER_THAN;
                strcpy(tokenType, "a comparison operator");
            } else {
                nextToken = GREATER_THAN;
                strcpy(tokenType, "a comparison operator");
            }
            break;
        case '!':
            addChar();
            getChar();
            if (nextChar == '=') {
                addChar();
                nextToken = NOT_EQUALS;
                strcpy(tokenType, "a comparison operator");
            } else {
                nextToken = UNKNOWN;
                strcpy(tokenType, "");
            }
            break;
        case '&':
            addChar();
            getChar();
            if (nextChar == '&') {
                addChar();
                nextToken = AND_OP;
                strcpy(tokenType, "a boolean operator");
            } else {
                nextToken = UNKNOWN;
                strcpy(tokenType, "");
            }
            break;
        case '|':
            addChar();
            getChar();
            if (nextChar == '|') {
                addChar();
                nextToken = OR_OP;
                strcpy(tokenType, "a boolean operator");
            } else {
                nextToken = UNKNOWN;
                strcpy(tokenType, "");
            }
            break;
        case '?':
            addChar();
            nextToken = QUESTION_MARK;
            strcpy(tokenType, "a punctuation mark");
            break;
        case ':':
            addChar();
            nextToken = COLON;
            strcpy(tokenType, "a punctuation mark");
            break;
        case '\"':
            addChar();
            getChar();
            while (nextChar != '\"' && nextChar != EOF) {
                addChar();
                getChar();
            }
            if (nextChar == '\"') {
                addChar();
                getChar();
                nextToken = STR_LIT;
                strcpy(tokenType, "a string literal");
            } else {
                strncpy(error, "Error - unclosed string literal", 100);
                nextToken = EOF;
                strcpy(tokenType, "");
            }
            break;
        default:
            addChar();
            nextToken = EOF;
            strcpy(tokenType, "a delimiter");
            break;
    }
    return nextToken;
}

bool isComment() {
    if (nextChar == '/') {
        /* Beginning of a single-line comment */
        while (nextChar != '\n' && nextChar != EOF)
            getChar(); /* Ignore comment contents */
        nextToken = COMMENT;
        strncpy(lexeme, "a single line comment", 100);
        strcpy(tokenType, "a comment");
    } else if (nextChar == '*') {
        /* Beginning of a block comment */
        addChar();
        getChar();
        while (true) {
            if (nextChar == EOF) {
                strncpy(error, "Error - unclosed block comment", 100);
                nextToken = EOF;
                strcpy(tokenType, "");
                return true;
            }
            if (nextChar == '*') {
                getChar();
                if (nextChar == '/') {
                    getChar();
                    break;
                }
            } else {
                getChar();
            }
        }
        nextToken = COMMENT;
        strncpy(lexeme, "a block comment", 100);
        strcpy(tokenType, "a comment");
    } else {
        return false;
    }
    return true;
}

/*******************/
/* addChar - A function to append nextChar to lexeme */
void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = '\0';
    } else {
        printf("Error - lexeme is too long\n");
    }
}

/*******************/
/* getChar - A function to get the next character from input and determine its character class */
void getChar() {
    if ((nextChar = getc(in_fp)) != EOF) {
        if (isalpha(nextChar))
            charClass = LETTER;
        else if (nextChar == '_')
            charClass = UNDERSCORE;
        else if (isdigit(nextChar))
            charClass = DIGIT;
        else
            charClass = UNKNOWN;
    } else {
        charClass = EOF;
    }
}

/*******************/
/* getNonBlank - A function to call getChar until it returns a non-whitespace character */
void getNonBlank() {
    while (isspace(nextChar))
        getChar();
}

/*******************/
/* lex - A simple lexical analyzer for arithmetic expressions */
int lex() {
    lexLen = 0;
    error[0] = '\0'; /* Clear previous errors */
    getNonBlank();
    switch (charClass) {
        /* Parse identifiers or keywords */
        case LETTER:
        case UNDERSCORE:
            addChar();
            getChar();
            while (charClass == LETTER || charClass == DIGIT || charClass == UNDERSCORE) {
                addChar();
                getChar();
            }
            
            strcpy(tokenType, "the keyword ");
            strcat(tokenType, lexeme);
            if (strcmp(lexeme, "if") == 0) {
                nextToken = IF;
            } else if (strcmp(lexeme, "else") == 0) {
                nextToken = ELSE;
            } else if (strcmp(lexeme, "for") == 0) {
                nextToken = FOR;
            } else if (strcmp(lexeme, "while") == 0) {
                nextToken = WHILE;
            } else if (charClass == UNKNOWN && !isspace(nextChar) && !strchr("(+-*/<>)", nextChar)) {
                addChar();
                strncpy(error, "Error - illegal identifier", 100);
                nextToken = EOF;
                strcpy(tokenType, "");
            } else {
                nextToken = IDENT;
                strcpy(tokenType, "an identifier");
            }
            break;
        case DIGIT:
            addChar();
            getChar();
            while (charClass == DIGIT) {
                addChar();
                getChar();
            }
            if (nextChar == '.') {
                addChar(); /* Include decimal point */
                getChar();
                while (charClass == DIGIT) {
                    addChar();
                    getChar();
                }
                nextToken = FLOAT_LIT;
                strcpy(tokenType, "a float");
            } else if (charClass == LETTER || nextChar == '_') {
                while (charClass == LETTER || charClass == DIGIT || nextChar == '_') {
                    addChar();
                    getChar();
                }
                strncpy(error, "Error - illegal identifier", 100);
                nextToken = EOF;
                strcpy(tokenType, "");
            } else {
                nextToken = INT_LIT;
                strcpy(tokenType, "an integer");
            }
            break;
        case UNKNOWN:
            lookup(nextChar);
            getChar();
            break;
        case EOF:
            nextToken = EOF;
            lexeme[0] = 'E';
            lexeme[1] = 'O';
            lexeme[2] = 'F';
            lexeme[3] = '\0';
            strcpy(tokenType, "end of file");
            break;
    }
    printf("Next token is: %d %s, next lexeme is %s", nextToken, tokenType, lexeme);
    printf("\t%s\n", error);
    return nextToken;
}