// t5_scopes.ms -- static scoping, nesting, hiding and referencing environments
program Scopes {
    var x : int = 1;
    var y : int = 2;
    const LIMIT : int = 10;

    func outer(a : int) : int {
        var x : float = 2.5;          // hides global x
        func inner(b : int) : int {
            var y : string = "in";    // hides global y
            show_env;                 // point 1
            return a + b + LIMIT;     // a: nonlocal (outer), LIMIT: global
        }
        show_env;                     // point 2
        return inner(a) * 2;
    }

    {
        var y : bool = true;          // block scope, hides global y
        var z : int = x + 1;          // x: global
        show_env;                     // point 3
    }
    x = outer(y);                     // y: global again (block has ended)
    show_env;                         // point 4
}
