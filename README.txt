CSI3120A - Assignment 1 (MiniScope) - C starter kit

  miniscope.c                 driver (complete)
  lexer.h, symtab.h, parser.h interfaces (complete - do not change them)
  lexer.c                     Part A1   - complete the functions marked TODO
  parser.c                    Parts B1/B2 - complete the functions marked TODO
  symtab.c                    Part B2   - complete the functions marked TODO
  todo.h                      the TODO(...) macro used by the unfinished functions
  Makefile                    build with: make   (Windows/MinGW: mingw32-make)
  tests/                      test programs (see Section 7 of the handout)
  expected/                   expected outputs for t0-t4 (t5, t6 and q7 are not provided)

Build and run, from this folder:
  make
  ./miniscope tests/t1_tokens.ms --tokens
  ./miniscope tests/t3_valid.ms
  ./miniscope tests/t0_trace.ms --trace

Compare with the expected output:
  ./miniscope tests/t3_valid.ms | diff -w - expected/t3_valid.out

Check your memory management:
  valgrind --leak-check=full ./miniscope tests/t5_scopes.ms

A function that is not written yet prints "TODO: ... is not implemented yet" and exits with code 3.
Syntax errors are reported with setjmp/longjmp (see parser.h): parse_program() returns 0 at the
first syntax error, and parser_free() releases the scopes that were left open.
