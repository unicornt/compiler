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
            b@C.init(3);
            b@C.init(3, true, 3);
            b@C.init(true, 5);
            a <- 1;
        }
	};
};
