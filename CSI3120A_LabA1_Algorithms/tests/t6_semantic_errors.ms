// t6_semantic_errors.ms -- scope and type errors (the parser must continue after each)
program Semantics {
    var total : int = 0;
    const MAX : int = 5;
    var total : float = 1.5;          // E1 redeclaration in the same scope
    count = 3;                        // E2 undeclared variable
    MAX = 6;                          // E3 assignment to a named constant
    total = "five";                   // E4 type mismatch (string -> int)
    var ratio : float = total;        // OK: int value widened to float
    var n : int = ratio;              // E5 type mismatch (float -> int)
    print(later);                     // E6 used before its declaration
    var later : int = 1;
    {
        var inside : int = 7;
    }
    inside = 8;                       // E7 out of scope: block variable no longer visible
    func sq(v : int) : int {
        var v : int = 0;              // E8 redeclares parameter v (same scope)
        return v * v;
    }
    total = sq(2, 3);                 // E9 wrong number of arguments
    if (total) { print(1); }          // E10 condition must be bool
    total = sq;                       // E11 function used as a variable
    return 0;                         // E12 return outside a function
}
