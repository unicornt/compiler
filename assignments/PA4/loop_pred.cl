class C {
	a : Int;
	b : Bool;
	init(x : Int, y : Bool) : C {
           {
		a <- x;
		b <- y;
		self;
           }
	};
};

class D inherits C {
	init(x : Int, y : Bool) : C {
           {
		a <- x + 1;
		b <- y;
		self;
           }
	};
};

Class Main {
    a : Int;
    b : D;
	main():Int {
        {
            b@C.init(3, true);
            while a < 5 loop a <- a + 1 pool;
            while a loop a <- a + 1 pool;
            0;
        }
	};
};
