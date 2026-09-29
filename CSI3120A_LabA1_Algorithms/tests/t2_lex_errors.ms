// t2_lex_errors.ms -- the lexer must report every error below and keep going
program LexErrors {
    var 2ndPlace : int = 2;           // illegal identifier (starts with a digit)
    var price : float = 12.;          // malformed float literal
    var total : int = 5 @ 3;          // illegal character
    var ok : bool = a & b;            // single '&' is not an operator
    var this_identifier_is_far_too_long_for_miniscope : int = 1;
    var Count : int = 1;              // legal: names are case sensitive
    var msg : string = "never closed;
    /* this comment is never closed
}
