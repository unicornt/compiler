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

class C {
	c : Int;
	d : Bool;
	rinit(x : Int, y : Bool) : C {
           {
		c <- x;
		d <- y;
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
