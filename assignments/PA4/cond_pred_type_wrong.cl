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
    c : C;
	main():C {
        {
            b@C.init(3, true);
            a <- 1;
            if a then b.init(3, true) else c.init(4, false) fi;
            if a = 1 then b.init(3, true) else c.init(4, false) fi;
        }
	};
};
