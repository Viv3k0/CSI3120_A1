CSI3120A - Lab Assignment 1 (MiniScope) - Python starter kit

  miniscope.py   driver (complete)
  lexer.py       Part A1 - complete the functions marked TODO
  parser.py      Part B1/B2 - complete the functions marked TODO
  symtab.py      Part B2 - complete the SymbolTable methods marked TODO
  tests/         test programs (see Section 7 of the handout)
  expected/      expected outputs for t0-t4 (t5, t6 and q7 are not provided)

Run from this folder:
  python3 miniscope.py tests/t1_tokens.ms --tokens
  python3 miniscope.py tests/t3_valid.ms
  python3 miniscope.py tests/t0_trace.ms --trace

Compare with the expected output:
  python3 miniscope.py tests/t3_valid.ms | diff -w - expected/t3_valid.out
