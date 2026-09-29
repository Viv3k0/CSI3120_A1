"""
miniscope.py -- Driver for the MiniScope front end (CSI3120A Lab Assignment 1).

STARTER CODE -- this driver is complete; you should not need to change it.

Usage:
    python miniscope.py <file.ms> --tokens     # Part A: print the token stream only
    python miniscope.py <file.ms>              # Part B: lex + parse + scope/type check
    python miniscope.py <file.ms> --trace      # Part B with the Enter/Exit parse trace
"""

import sys
from lexer import tokenize
from parser import Parser, MiniScopeSyntaxError


def print_tokens(tokens):
    for t in tokens:
        print(f"Line {t.line:>3}: Next token is: {t.kind:<12} Next lexeme is: {t.lexeme}")


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    path = argv[1]
    flags = set(argv[2:])
    with open(path, encoding="utf-8") as f:
        text = f.read()

    print(f"=== MiniScope: {path} ===")
    tokens, lex_errors = tokenize(text)

    if "--tokens" in flags:
        print_tokens(tokens)
        for line, msg in lex_errors:
            print(f"Lexical error (line {line}): {msg}")
        print(f"--- {len(tokens)} token(s), {len(lex_errors)} lexical error(s) ---")
        return 1 if lex_errors else 0

    if lex_errors:
        for line, msg in lex_errors:
            print(f"Lexical error (line {line}): {msg}")
        print(f"--- {len(lex_errors)} lexical error(s); parsing not attempted ---")
        return 1

    p = Parser(tokens, trace="--trace" in flags)
    try:
        p.parse_program()
    except MiniScopeSyntaxError as e:
        print(f"Syntax error (line {e.line}): {e.msg}")
        print("--- parsing stopped at the first syntax error ---")
        return 1

    print(f"--- parse successful; {p.errors} semantic error(s), {p.warnings} warning(s) ---")
    return 1 if p.errors else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
