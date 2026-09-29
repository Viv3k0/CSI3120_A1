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
FILE *in_fp; /* Removed *fopen() redundant declaration */
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
/* lookup - une fonction pour rechercher les opérateurs et les mots-clés et retourner le jeton */
int lookup(char ch) {
    switch (ch) {
        case '(':
            addChar();
            nextToken = LEFT_PAREN;
            strcpy(tokenType, "un delimiteur");
            break;
        case ')':
            addChar();
            nextToken = RIGHT_PAREN;
            strcpy(tokenType, "un delimiteur");
            break;
        case '{':
            addChar();
            nextToken = LEFT_BRACE;
            strcpy(tokenType, "un delimiteur");
            break;
        case '}':
            addChar();
            nextToken = RIGHT_BRACE;
            strcpy(tokenType, "un delimiteur");
            break;
        case '+':
            addChar();
            nextToken = ADD_OP;
            strcpy(tokenType, "un operateur arithmetique");
            break;
        case '-':
            addChar();
            nextToken = SUB_OP;
            strcpy(tokenType, "un operateur arithmetique");
            break;
        case '*':
            addChar();
            nextToken = MULT_OP;
            strcpy(tokenType, "un operateur arithmetique");
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
                strcpy(tokenType, "un signe Egalite");
            } else {
                nextToken = ASSIGN_OP;
                strcpy(tokenType, "un operateur de comparaison");
            }
            break;
        case ';':
            addChar();
            nextToken = SEMICOLON;
            strcpy(tokenType, "un delimiteur");
            break;
        case '<':
            addChar();
            getChar();
            if (nextChar == '=') {
                addChar();
                nextToken = LESS_THAN;
                strcpy(tokenType, "un operateur de comparaison");
            } else {
                nextToken = LESS_THAN;
                strcpy(tokenType, "un operateur de comparaison");
            }
            break;
        case '>':
            addChar();
            getChar();
            if (nextChar == '=') {
                addChar();
                nextToken = GREATER_THAN;
                strcpy(tokenType, "un operateur de comparaison");
            } else {
                nextToken = GREATER_THAN;
                strcpy(tokenType, "un operateur de comparaison");
            }
            break;
        case '!':
            addChar();
            getChar();
            if (nextChar == '=') {
                addChar();
                nextToken = NOT_EQUALS;
                strcpy(tokenType, "un operateur de comparaison");
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
                strcpy(tokenType, "un operateur booleen");
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
                strcpy(tokenType, "un operateur booleen");
            } else {
                nextToken = UNKNOWN;
                strcpy(tokenType, "");
            }
            break;
        case '?':
            addChar();
            nextToken = QUESTION_MARK;
            strcpy(tokenType, "une ponctuation");
            break;
        case ':':
            addChar();
            nextToken = COLON;
            strcpy(tokenType, "une ponctuation");
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
                strcpy(tokenType, "une chaine de caracteres");
            } else {
                strncpy(error, "Erreur - chaine de caracteres non fermee", 100);
                nextToken = EOF;
                strcpy(tokenType, "");
            }
            break;
        default:
            addChar();
            nextToken = EOF;
            strcpy(tokenType, "un delimiteur");
            break;
    }
    return nextToken;
}

bool isComment() {
    if (nextChar == '/') {
        while (nextChar != '\n' && nextChar != EOF)
            getChar();
        nextToken = COMMENT;
        strncpy(lexeme, "a single line comment", 100);
        strcpy(tokenType, "un commentaire");
    } else if (nextChar == '*') {
        addChar();
        getChar();
        while (true) {
            if (nextChar == EOF) {
                strncpy(error, "Erreur - block de commentaire non ferme", 100);
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
        strncpy(lexeme, "un block de commentaire", 100);
        strcpy(tokenType, "un commentaire");
    } else {
        return false;
    }
    return true;
}

/*******************/
/* addChar - Une fonction qui ajoute nextChar a lexeme */
void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = '\0';
    } else {
        printf("Error - lexeme is too long\n");
    }
}

/*******************/
/* getChar */
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
/* getNonBlank */
void getNonBlank() {
    while (isspace(nextChar))
        getChar();
}

/*******************/
/* lex */
int lex() {
    lexLen = 0;
    error[0] = '\0'; /* Clear previous errors */
    getNonBlank();
    switch (charClass) {
        case LETTER:
        case UNDERSCORE:
            addChar();
            getChar();
            while (charClass == LETTER || charClass == DIGIT || charClass == UNDERSCORE) {
                addChar();
                getChar();
            }
            
            strcpy(tokenType, "le mot-cle ");
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
                strncpy(error, "Erreur - identifiant illegal", 100);
                nextToken = EOF;
                strcpy(tokenType, "");
            } else {
                nextToken = IDENT;
                strcpy(tokenType, "un identifiant");
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
                addChar();
                getChar();
                while (charClass == DIGIT) {
                    addChar();
                    getChar();
                }
                nextToken = FLOAT_LIT;
                strcpy(tokenType, "un decimal");
            } else if (charClass == LETTER || nextChar == '_') {
                while (charClass == LETTER || charClass == DIGIT || nextChar == '_') {
                    addChar();
                    getChar();
                }
                strncpy(error, "Erreur - identifiant illegal", 100);
                nextToken = EOF;
                strcpy(tokenType, "");
            } else {
                nextToken = INT_LIT;
                strcpy(tokenType, "un entier");
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
            strcpy(tokenType, "fin du fichier");
            break;
    }
    printf("Le token suivant est: %d %s, le lexeme suivant est %s", nextToken, tokenType, lexeme);
    printf("\t%s\n", error);
    return nextToken;
}