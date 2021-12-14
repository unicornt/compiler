class C {
	a : Int;
	b : Bool;
    a : Int;
	init(x : Int, y : Bool) : C {
           {
		a <- x;
		b <- y;
		self;
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
