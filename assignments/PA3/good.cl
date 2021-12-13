class A {
ana(): Int {
(let x:Int <- 1 in 2)+3
};
};

Class BB__ inherits A {
};

CLASS PRE{
    pre(x:Int):Int {
        x <- ~(new PRE)@PRE.pre(1 + 2 * (3 - 5) / 6) <= NOT 5
    };
};

(*
class C {
    h(n:Int):Int {
        {
            let i:Int <- 1, j:Int <- 2 in (i + j + n);
            let b:BB__ <- new BB__ in (b@BB__.g(i, j));
        }
    };

};
*)
(*
    检测多个feature/多个expr的情况
*)
class Mutifeat_expr {
    k : Int <- 1 + 2 * 3 / 4;
    t(): Int{
        {
            let i :Int <- 1 in i;
            let j :Int <- 1 in j;
        }
    };
    h(): Int{
        {
            let i :Int <- 1 in i;
            let j :Int <- 1 in j;
        }
    };
};

class Mutiform {
    f(x: Int, y: Int): Int{
        {
            k <- 1;
        }
    };
};

Class BB__ inherits A {
    f(x:Int):Int {
        if x<=1 then 1 else f(x-1) + f(x-2) fi
    };
    g(a:Int, b:Int):Int{
        if a<b then g(b,a)
        else if b=1 then a
            else g(a-b,b)
            fi
        fi
    };
    c(x:Int):Int {
        case x*x of
            a:A => 1;
            b:BB__ => 2;
            c:Int => 3;
        esac
    };
};

