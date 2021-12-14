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

class E inherits C {
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
    c : E;
	main():Int {
        {
            a + 2;
            b + 3;
            true + false;
            true + 4;
            5 + true;
            5 + b;
            a - 2;
            b - 3;
            true - false;
            true - 4;
            5 - true;
            5 - b;
            a * 2;
            b * 3;
            true * false;
            true * 4;
            5 * true;
            5 * b;
            a / 2;
            b / 3;
            true / false;
            true / 4;
            5 / true;
            5 / b;
            ~5;
            ~b;
            ~true;
            a < 2;
            b < 3;
            true < false;
            true < 4;
            5 < true;
            5 < b;
            a = 2;
            b = 3;
            b = c;
            true = false;
            true = 4;
            5 = true;
            5 = b;
            "adklfjs" = "skdfjldkfj";
            "skdfjlksdf" = 1;
            "skdlfjds" = true;
            a <= 2;
            b <= 3;
            true <= false;
            true <= 4;
            5 <= true;
            5 <= b;
            not true;
            not false;
            not b;
            not 1;
            not "dskfjlskd";
            1;

        }
	};
};
