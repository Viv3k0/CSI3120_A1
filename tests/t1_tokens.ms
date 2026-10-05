// t1_tokens.ms -- every kind of token, plus both comment styles
"your gay"
program Tokens {
    /* a block comment
       that spans several lines */
    var count : int = 42;
    var rate  : float = 3.75;
    var name  : string = "Ada Lovelace";
    var done  : bool = false;
    const MAX : int = 100;          // named constant

    count = count + 1 - 2 * 3 / 4 % 5;
    done = (count <= MAX) && !(rate >= 1.0) || count != 7;
    if (count == 0) { print(name); } else { print("non-zero"); }
    while (count > 0 && count < MAX) { count = count - 1; }
    show_env;
}
