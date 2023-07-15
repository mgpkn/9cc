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

<<COUT
assert 25 "main(){return (5+20);}"
assert 25 "main(){i=0;x=0;y=0;return (5+20);}"
assert 50 "main(){a=2;b=5;b=b+3;c=3;return (b-c+ababa())*a;} ababa(){a=20;return 20;}"
assert 42 "int main(){return 10+32;}"
assert 15 "int main(){return b=3*4+3;}"
assert 10 "int main(){return b=a=10;}"
assert 20 "int main(){a=18;b=2;return c=a+b;}"
assert 20 "int main(){a=18;b=2;return z=a+b;}"
assert 20 "int main(){hoge=18+2;return hoge;}"
assert 40 "int main(){a=18;b=2;return c=(a+b)*2;}"
assert 111 "int main(){hoge4=18+2;a=10;return hoge4*a/2+11;}"
assert 7 "int main(){return7=7;return return7;}"
assert 8 "int main(){return 8;}"
assert 20 "int main(){return (8*3-4);}"
assert 20 "int main(){hei=4;return (8*3-hei);}"
assert 13 "int main(){return7=3;return7+10;}"
assert 5 "int main(){if(1)return 5;}"
assert 13 "int main(){a=13;if(1)return a;}"
assert 13 "int main(){a=13;b=8;if(a==b+5)return a;else return b;}"
assert 21 "int main(){a=13;b=8;if(a==b)return a;else return a+b;}"
assert 10 "int main(){a=3;while(a<10)a=a+1;return a;}"
assert 3 "int main(){a=3;while(a<1)a=a+1;return a;}"
assert 33 "int main(){a=3;b=30;while(a<3)a=a+1;return a+b;}"
assert 16 "int main(){a=100;a=0;while(a<16)a=a+1;return a;}"
assert 16 "int main(){a=100;for(a=0;a<16;a=a+1)b=1;return a;}"
assert 255 "int main(){a=255;return a;}"
assert 69 "int main(){a=5;b=0;while(b<a)b=b+1;for(c=0;c<4;c=c+1)a=a+2;return a*b+c;}"
assert 117 "int main(){a=5;b=0;while(b<a)b=b+1;for(c=0;c<4;c=c+1)a=a+2;return a*(b+c);}"
assert 10 "int main(){a=3;for(;a<10;)a=a+1;return a;}"
assert 225 "int main(){a=0;b=0;c=0;while(a<5){a=a+1;b=b+2;c=c+3;}return (a+b)*c;}"
assert 38 "int main(){a=0;b=0;c=0;d=0;while(a<5){a=a+1;b=b+2;c=c+3;}if(a<b){d=4;d=d*2;}return a+b+c+d;}"
assert 30 "int main(){a=0;b=0;c=0;d=0;while(a<5){a=a+1;b=b+2;c=c+3;}if(a>b){d=4;d=d*2;}return a+b+c+d;}"
assert 4 "int main(){return foo();}"
assert 8 "int main(){return foo()*baa();}"
assert 4 "int main(){return foo(foo());}"
assert 4 "int main(){return foo(a,b,4,5);}"
assert 8 "int main(){return foo(a,b,4,baa(),(baa()*3)+(3+1))+baa(d,d,d)+baa();}"
assert 4 "int main(){return foo(1,2,3,4,5,6);}"
assert 4 "int main(a,b,c){return foo(1,2,3,4,5,6);}"
#assert 4 "int main(){return foo(1,2,3,4,5,6,7);}" #raise error pettern
assert 4 "int main(a,b,c){return foo(1,2,3,4,5,6);}"
assert 4 "int main(){b=4; return b;}"
assert 7 "int main(){a=4;b=7;return b;}"
assert 5 "int main(){a=5;b=&a;return *b;}"
assert 13 "int main(){x=13;y=&x;return *y;}"
COUT
assert 1 "int main(){return 100<101;}"
assert 0 "int main(){return 100<100;}"
assert 1 "int main(){return 99<=100;}"
assert 1 "int main(){return 100<=100;}"
assert 1 "int main(){return 100<=101;}"
assert 1 "int main(){return 201>200;}"
assert 0 "int main(){return 200>200;}"
assert 1 "int main(){return 200>=200;}"
assert 0 "int main(){return 199>=200;}"
assert 7 "int main(){return ufufu();} int ufufu(){int fuga;fuga=1;while(fuga<7){fuga=fuga+1;} return fuga; }"
assert 13 "int main(){int i;i=13;return i;}"
assert 33 "int main(){int x;x=13;int y;y=20;return x+y;}"
assert 8 "int main(){int a;int ub;int d;return foo(a,ub,4,baa(),(baa()*3)+(3+1))+baa(d,d,d)+baa();}"
assert 8 "int main(){int a;int b;a=13;b=8;if(0)return a;else return b;}"
assert 15 "int main(){int a;int b;a=5;b=&a;return *b*3;}"
assert 3 "int main(){int x;int y;int z;x=3;y=5;z=&y+8;return *z;}"
echo OK
