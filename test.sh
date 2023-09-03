#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s helper.o
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
assert 20 "int main(){return 11+9+8-4-4;}"
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
assert 4 "int main(){return foo();}"
assert 10 "int main(){return aba(10);}"
assert 21 "int main(){return sum_all_param(1,2,3,4,5,6);}"
assert 32 "int main(){int x;x=3;return sum_all_param((x+1)*x,2,3,4,5,6);}"
assert 7 "int main(int ababa){return ufufu();} int ufufu(){int fuga;fuga=1;while(fuga<7){fuga=fuga+1;} return fuga; }"
assert 8 "int main(){int a;int ub;int d;return foo(a,ub,4,baa(),(baa()*3)+(3+1))+baa(d,d,d)+baa();}"
assert 8 "int main(){int a;int b;a=13;b=8;if(0)return a;else return b;}"
assert 15 "int main(){int a;int b;a=5;b=&a;return *b*3;}"
assert 33 "int main(int hoge){int x;x=13;int y;y=20;return x+y;}"
assert 33 "int main(int hoge,int fuga){int x;x=13;int y;y=20;return x+y;}"
assert 10 "int main(){int x;int *y;y=&x;*y=10;return x;}"
assert 13 "int main(){int x;int *y;int **z;y=&x;z=&y;**z=13;return x;}"
assert 10 "int main(){int x;int *y;int **z;y=&x;*y=10;return x;}"
assert 85 "int main(){int x;x=10;int *z;alloc4(&z,85,2,3,4);return *z;}"
assert 85 "int main(){int x;x=10;int *z;alloc4(&z,85,2,3,4);return *z;}"
assert 22 "int main(){int x;x=10;int *z;alloc4(&z,13,22,35,48);return *(z+1);}"
assert 49 "int main(){int x;x=10;int *z;alloc4(&z,13,22,35,48);return *(z+3)+1;}"
assert 48 "int main(){int x;x=10;int *z;alloc4(&z,13,22,35,48);z=z+3;return *z;}"
assert 22 "int main(){int x;x=10;int *z;alloc4(&z,13,22,35,48);z=z+3;return *(z-2);}"
assert 3 "int main(){int *y;int *z;alloc4(&z,13,22,35,48);y=z+2;return y+1-z;}"
#assert 16 "int main(){int *y;int *z;alloc4(&z,13,22,35,48);y=z+2;return *z+(y+1-z);}" 
assert 4 "int main(){return sizeof(12);}"
assert 4 "int main(){return sizeof 12+18 ;}"
assert 4 "int main(){return sizeof(100+10);}"
assert 4 "int main(){int x;return sizeof(x);}"
assert 8 "int main(){int *x;return sizeof(x);}"
assert 8 "int main(){int x;return sizeof(&x);}"
assert 8 "int main(){int *x;return sizeof(x+4);}"
assert 8 "int main(){int **x;return sizeof(x);}"
echo OK
