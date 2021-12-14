class C {
	a : Int;
	b : Bool;
	init(x : Int, y : Bool) : C {
           {
		a <- x;
		b <- y;
           }
	};
};

Class Main {
    a : Int;
	main():Int {
        {
            a <- 1;
        }
	};
};
