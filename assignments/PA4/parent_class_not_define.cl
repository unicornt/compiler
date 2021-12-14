class C inherits D{
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
	main():Int {
        {
            a <- 1;
        }
	};
};
