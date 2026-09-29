// t3_valid.ms -- a correct program: no errors, no warnings expected
program Bank {
    const RATE : float = 0.05;
    var balance : float = 1000.0;
    var years : int = 0;

    func interest(amount : float, n : int) : float {
        var result : float = amount;
        var k : int = 0;
        while (k < n) {
            result = result + result * RATE;
            k = k + 1;
        }
        return result;
    }

    func report(label : string, value : float) : void {
        print(label);
        print(value);
    }

    func fact(n : int) : int {         // recursion: fact is bound before its body
        if (n <= 1) { return 1; }
        return n * fact(n - 1);
    }

    while (years < 3) {
        years = years + 1;
    }
    balance = interest(balance, years);
    report("Balance after " + "3 years:", balance);
    print(fact(5));
    if (balance > 1000 && !(years == 0)) {
        print("growing");
    } else {
        print("flat");
    }
}
