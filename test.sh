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
COUT
assert 15 "int main(){return (15);}"
assert 15 "int main(){int b;b=3*4+3;return b;}"
assert 30 "int main(){int i;i=(3+3)*5;return i;}"
assert 2 "int main(){int i;i=(13-3)/5;return i;}"
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
assert 100 "int main(){int a;int x;int y;int z;a=100;x=3;y=5;z=&y+16;return *z;}"
assert 33 "int main(int hoge){int x;x=13;int y;y=20;return x+y;}"
assert 33 "int main(int hoge,int fuga){int x;x=13;int y;y=20;return x+y;}"
assert 7 "int main(int ababa,char ufufu){return ufufu();} int ufufu(int ora){int fuga;fuga=1;while(fuga<7){fuga=fuga+1;} return fuga; }"
assert 10 "int main(){int x;int *y;y=&x;*y=10;return x;}"
echo OK
