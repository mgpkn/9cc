#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s foo.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}


assert 25 "{return (5+20);}"
assert 1 "{return 35+65==100;}"
assert 21 "return 5+20-4;"
assert 41 "return 12 + 34 - 5;"
assert 42 "return 6*7;"
assert 45 "return 5+6*7-2;"
assert 15 "return 5*(9-6);"
assert 2 "return 6/3;"
assert 4 "return (3+5)/2;"
assert 3 "return 1 + +2;"
assert 7 "return 10 + -3;"
assert 13 "return 10 - -3;"
assert 14 "return (10 % (7-3))*7;"
assert 0 "return 35+65==50*2+(1+1);"
assert 1 "return 35+65==100;"
assert 1 "return 35+65!=50*2+(1+1);"
assert 0 "return 35+65!=100;"
assert 1 "return 100<101;"
assert 0 "return 100<100;"
assert 1 "return 99<=100;"
assert 1 "return 100<=100;"
assert 1 "return 100<=101;"
assert 1 "return 201>200;"
assert 0 "return 200>200;"
assert 1 "return 200>=200;"
assert 0 "return 199>=200;"
assert 42 "return 10+32;"
assert 3 "return a=3;"
assert 15 "return b=3*4+3;"
assert 10 "return b=a=10;"
assert 20 "a=18;b=2;return c=a+b;"
assert 20 "a=18;b=2;return z=a+b;"
assert 20 "hoge=18+2;return hoge;"
assert 40 "a=18;b=2;return c=(a+b)*2;"
assert 111 "hoge4=18+2;a=10;return hoge4*a/2+11;"
assert 7 "return7=7;return return7;"
assert 8 "return 8;"
assert 20 "return (8*3-4);"
assert 20 "hei=4;return (8*3-hei);"
assert 13 "return7=3;return7+10;"
assert 5 "if(1)return 5;"
assert 13 "a=13;if(1)return a;"
assert 8 "a=13;b=8;if(0)return 8;else return b;"
assert 13 "a=13;b=8;if(a==b+5)return a;else return b;"
assert 21 "a=13;b=8;if(a==b)return a;else return a+b;"
assert 10 "a=3;while(a<10)a=a+1;return a;"
assert 3 "a=3;while(a<1)a=a+1;return a;"
assert 33 "a=3;b=30;while(a<3)a=a+1;return a+b;"
assert 16 "a=100;a=0;while(a<16)a=a+1;return a;"
assert 16 "a=100;for(a=0;a<16;a=a+1)b=1;return a;"
assert 255 "a=255;return a;"
assert 69 "a=5;b=0;while(b<a)b=b+1;for(c=0;c<4;c=c+1)a=a+2;return a*b+c;"
assert 117 "a=5;b=0;while(b<a)b=b+1;for(c=0;c<4;c=c+1)a=a+2;return a*(b+c);"
assert 10 "a=3;for(;a<10;)a=a+1;return a;"
assert 225 "a=0;b=0;c=0;while(a<5){a=a+1;b=b+2;c=c+3;}return (a+b)*c;"
assert 38 "a=0;b=0;c=0;d=0;while(a<5){a=a+1;b=b+2;c=c+3;}if(a<b){d=4;d=d*2;}return a+b+c+d;"
assert 30 "a=0;b=0;c=0;d=0;while(a<5){a=a+1;b=b+2;c=c+3;}if(a>b){d=4;d=d*2;}return a+b+c+d;"
assert 4 "return foo();"
assert 8 "return foo()*baa();"
assert 4 "return foo(foo());"
assert 4 "return foo(a,b,4,5);"
assert 8 "return foo(a,b,4,baa(),(baa()*3)+(3+1))+baa(d,d,d)+baa();"
assert 4 "return foo(1,2,3,4,5,6);"
# assert 4 "return foo(1,2,3,4,5,6,7);" raise error pettern
echo OK
