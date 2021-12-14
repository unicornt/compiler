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
            case b of
                c : D => c.init(3, true);
                d : Int => d + 1;
                f : Bool => not f;
            esac;
            while a < 5 loop a <- a + 1 pool;
            a <- 1;
        }
	};
};
