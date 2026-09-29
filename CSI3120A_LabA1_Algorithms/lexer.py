"""
lexer.py -- Lexical analyzer for the MiniScope language (CSI3120A Lab Assignment 1).

STARTER CODE -- complete every function marked TODO.
You may reuse the lexer you wrote in Lab 1.

The lexer is a hand-written pattern matcher in the style of Sebesta, Ch. 4:
it looks at the class of the next character (letter, digit, quote, operator
symbol, ...) and collects the longest lexeme that matches the corresponding
token pattern.  Reserved words are recognised by first scanning an
identifier-shaped lexeme and then looking it up in a reserved-word table.

Lexical errors are reported and the offending lexeme is skipped, so a single
run can report several errors.
"""

MAX_IDENT_LEN = 31   # like C99 external names (Lecture 4, "Names")

# ---------------------------------------------------------------------------
# Token codes
# ---------------------------------------------------------------------------
RESERVED_WORDS = {
    "program": "KW_PROGRAM", "var": "KW_VAR", "const": "KW_CONST",
    "func": "KW_FUNC", "int": "KW_INT", "float": "KW_FLOAT",
    "bool": "KW_BOOL", "string": "KW_STRING", "void": "KW_VOID",
    "if": "KW_IF", "else": "KW_ELSE", "while": "KW_WHILE",
    "print": "KW_PRINT", "return": "KW_RETURN", "true": "KW_TRUE",
    "false": "KW_FALSE", "show_env": "KW_SHOW_ENV",
}

# Two-character operators are tried before one-character ones (longest match).
TWO_CHAR_OPS = {
    "==": "EQ_OP", "!=": "NEQ_OP", "<=": "LE_OP", ">=": "GE_OP",
    "&&": "AND_OP", "||": "OR_OP",
}
ONE_CHAR_OPS = {
    "=": "ASSIGN_OP", "+": "ADD_OP", "-": "SUB_OP", "*": "MULT_OP",
    "/": "DIV_OP", "%": "MOD_OP", "<": "LT_OP", ">": "GT_OP", "!": "NOT_OP",
    "(": "LEFT_PAREN", ")": "RIGHT_PAREN", "{": "LEFT_BRACE",
    "}": "RIGHT_BRACE", ";": "SEMICOLON", ":": "COLON", ",": "COMMA",
}


class Token:
    """A token: its code (category), the lexeme that matched, and the line."""

    def __init__(self, kind, lexeme, line):
        self.kind = kind
        self.lexeme = lexeme
        self.line = line

    def __repr__(self):
        return f"Token({self.kind}, {self.lexeme!r}, line {self.line})"


class Lexer:
    def __init__(self, text):
        self.text = text
        self.pos = 0
        self.line = 1
        self.tokens = []
        self.errors = []      # list of (line, message)

    # -- character helpers --------------------------------------------------
    def peek(self, k=0):
        i = self.pos + k
        return self.text[i] if i < len(self.text) else ""

    def advance(self):
        ch = self.text[self.pos]
        self.pos += 1
        if ch == "\n":
            self.line += 1
        return ch

    def error(self, line, msg):
        self.errors.append((line, msg))

    # -- main loop ------------------------------------------------------------
    def tokenize(self):
        while True:
            self.skip_blanks_and_comments()
            if self.pos >= len(self.text):
                self.tokens.append(Token("EOF", "EOF", self.line))
                return self.tokens
            ch = self.peek()
            if ch.isalpha() or ch == "_":
                self.lex_identifier()
            elif ch.isdigit():
                self.lex_number()
            elif ch == '"':
                self.lex_string()
            else:
                self.lex_operator()

    def skip_blanks_and_comments(self):
        """Skip white space, // line comments and /* block */ comments.
        TODO:
          * blanks, tabs and newlines are skipped (advance() keeps self.line up to date);
          * '//' skips to the end of the line;
          * '/*' skips to the matching '*/'; if the file ends first, report
            "unclosed block comment (opened here)" at the line where it was opened.
        """
        raise NotImplementedError("skip_blanks_and_comments")

    def lex_identifier(self):
        """Identifier or reserved word: (letter | _) { letter | digit | _ }
        TODO:
          * collect the longest identifier-shaped lexeme;
          * if it is in RESERVED_WORDS, emit that reserved-word token;
          * else if it is longer than MAX_IDENT_LEN, report an error and emit nothing;
          * else emit an IDENT token.
        """
        raise NotImplementedError("lex_identifier")

    def lex_number(self):
        """INT_LIT: digit {digit}     FLOAT_LIT: digit {digit} . digit {digit}
        TODO:
          * '12.' (no digit after the point) -> error "malformed float literal";
          * a number immediately followed by a letter or '_' (e.g. 2ndPlace)
            -> error "illegal identifier ... (identifiers cannot start with a digit)";
            skip the whole bad lexeme;
          * otherwise emit INT_LIT or FLOAT_LIT.
        """
        raise NotImplementedError("lex_number")

    def lex_string(self):
        """STRING_LIT: " { any character except " and newline } "
        TODO: the lexeme includes the quotes. If a newline or the end of the file
        comes before the closing quote, report "unterminated string literal".
        """
        raise NotImplementedError("lex_string")

    def lex_operator(self):
        """Operators and punctuation.
        TODO: try TWO_CHAR_OPS first (longest match), then ONE_CHAR_OPS.
        Anything else is an "illegal character" error (e.g. '@', a single '&').
        """
        raise NotImplementedError("lex_operator")


def tokenize(text):
    """Convenience wrapper: returns (tokens, errors)."""
    lx = Lexer(text)
    toks = lx.tokenize()
    return toks, lx.errors
