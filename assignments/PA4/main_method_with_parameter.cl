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

Class Main {
    a : Int;
	main(x: Int):C {
        {
            a <- x;
        }
	};
};
