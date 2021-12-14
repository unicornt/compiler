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

class D{
    a : D;
    init() : Object {
        {
            while 7 < 5 loop 1 + 1 pool;
        }
    };
};

class H {
	a : C;
	b : D;
};

Class Main {
    a : Int;
    b : C;
	main():Int {
        {
            a <- 7;
            case b of
                c : C => c.init(3, true);
                d : Int => d + 1;
                f : Bool => not f;
            esac;
            case while 7 < 5 loop 1 + 1 pool of
                c : C => c.init(3, true);
                d : Int => d + 1;
                f : Bool => not f;
            esac;
            a <- 1;
        }
	};
};
