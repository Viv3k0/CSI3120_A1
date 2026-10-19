// test file with errors 
program Numbers {
    func digitSum(n : int) : bool {
        var s : int = 0;
        while (n > 0){
            s = s + (n % 10);
            n = n / 10;
        }
        return s; //wrong return type
    }

    func isHarshad(n : int) : bool {
        var res : int = n % digitSum(n); //cant use % on bool
        return res == 0;
    }

    func digits(n : int) : int{
        var digits : int = 1; //  digits hides function
        n = n / 10;

        while (n > 0){
            digits = digits + 1;
            n = n / 10;
        }

        return digits;
    }

    func isPalindrome(x : int) : bool {
        var d : int = digits(n); //no variable n in current scope
        var pow : int = 1; 

        var m : int = 0;

        while (m < d){
            pow = pow * 10;
            m = m + 1;
        }

        var x2 : int = x;
        var i : int = 0;

        while (i < d/2){
            if ((x2 % 10) != ((x / pow) % 10)){
                return false;
            }

            x2 = x2 / 10;
            pow = pow / 10;
            i = i + 1;
        }

        return true;
    }

    var i : int = 2;
    const count : int = "0"; // initialized to wrong type 

    while (count < 420){
        if (isHarshad(i) && isPalindrome(i)){
            count = count + 1; // attempt to modify const
            var specialNumber : int = i;
        }
    }

    print(specialNumber); //attempt to access variable out of scope
}