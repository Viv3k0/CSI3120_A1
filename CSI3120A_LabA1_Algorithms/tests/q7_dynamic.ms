// q7_dynamic.ms -- Question 7: static vs. dynamic scoping
program Dyn {
    var x : int = 1;
    func show() : void { print(x); }
    func first() : void { var x : int = 2; show(); }
    func second() : void { var x : int = 3; first(); }
    show();
    first();
    second();
}
