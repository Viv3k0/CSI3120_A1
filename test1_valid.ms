// no semantic/syntax errors 
program Prime {

    /* find the square root of an integer using binary search */
    func sqrt(n : int) : int {
        var l : int = 1;
        var u : int = n;

        while (l <= u){
            var m : int = (l + u) / 2;

            if (m * m >= n && (m-1) * (m-1) < n){
                return m;
            }

            if (m * m < n){
                l = m+1;
            } else {
                u = m-1;
            }
        }

        return -1;
    }

    func isPrime(n : int) : bool {
        if (n == 1){ return false; }
        if (n == 2){ return true; }

        var i : int = 2;
        const max : int = sqrt(n);
        
        while (i < max){
            if (n % i == 0){
                return false;
            }
        }

        return true;
    }

    print(isPrime(6767));
}